#include "avbinmedia.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include "eventloop.h"
#include "mediabuffer.h"
#include "localints.h"
#include <sstream>
using namespace std;
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)


AvBinMediaTimer::AvBinMediaTimer(class AvBinMedia *in) : QObject()
{
    this->timer = new QTimer(this);
    this->parent = in;
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec
}

AvBinMediaTimer::~AvBinMediaTimer()
{
    delete this->timer;
    this->timer = NULL;
}

void AvBinMediaTimer::Update()
{
    if(this->parent) parent->Update();
}

//***************************************

AvBinMedia::AvBinMedia(class EventLoop *eventLoopIn, int selfTimerIn)
{
    this->eventReceiver = NULL;
    this->eventLoop = eventLoopIn;
    this->mediaThread = NULL;
    this->uuid = QUuid::createUuid();

    if(this->eventReceiver == NULL)
    {
        this->eventReceiver = new class EventReceiver(eventLoopIn);
        this->eventLoop = eventLoopIn;
        this->eventLoop->AddListener("AVBIN_DURATION_RESPONSE", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_FRAME_RESPONSE", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_FRAME_FAILED", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_REQUEST_FAILED", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_OPEN_RESULT", *this->eventReceiver);
        this->eventLoop->AddListener("GET_MEDIA_DURATION", *this->eventReceiver);
        this->eventLoop->AddListener("GET_MEDIA_FRAME", *this->eventReceiver);
    }

    this->mediaThread = new AvBinThread();
    this->mediaThread->SetEventLoop(this->eventLoop);
    this->mediaThread->SetUuid(this->uuid);
    this->mediaThread->Start();

    if(selfTimerIn)
        this->timer = new AvBinMediaTimer(this);
    else
        this->timer = NULL;

}

AvBinMedia::~AvBinMedia()
{
    cout << "AvBinMedia::~AvBinMedia()" << endl;
    if(this->timer!=NULL) delete this->timer;
    this->timer = NULL;
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventReceiver = NULL;
    if(this->mediaThread!=NULL)
        delete this->mediaThread;
    this->mediaThread = NULL;
}

int AvBinMedia::OpenFile(QString fina)
{
    assert(this->eventLoop);
    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_OPEN_FILE");
    std::tr1::shared_ptr<class Event> openEv(new Event(eventName.toLocal8Bit().constData(), evid));
    openEv->data = fina.toLocal8Bit().constData();
    openEv->toUuid = this->uuid;
    this->eventLoop->SendEvent(openEv);

    std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(evid);
    return ev->data.toInt();
}

void AvBinMedia::TerminateThread()
{
    if(this->mediaThread!=NULL && this->mediaThread->isRunning())
    {
        cout << "Warning: terminating buffer media thread " << this->uuid.toString().constData() <<endl;
        this->mediaThread->terminate();
    }
}

//*******************************************

long long unsigned  AvBinMedia::RequestFrame(QString source, long long unsigned ti) //in milliseconds
{
    if(this->mediaThread->IsStopFlagged())
    {
        throw runtime_error("Worker thread has been stopped");
    }
    if(!this->mediaThread->isRunning())
    {
        throw runtime_error("Worker thread is not running");
    }

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_FRAME");
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event(eventName.toLocal8Bit().constData(), evid));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->toUuid = this->uuid;
    getFrameEvent->id = this->eventLoop->GetId();
    getFrameEvent->data = tmp.str().c_str();
    this->eventLoop->SendEvent(getFrameEvent);

    return getFrameEvent->id;
}

void AvBinMedia::Update()
{

    //Update to check for async frames send to video widget
    //cout << "AvBinMedia::Update " << (unsigned long long) this << endl;
    //Check for new frames from media backend.
    int checking = 1;
    //cout << "GUI receive queue " << this->eventReceiver->BufferSize() << endl;
    while(checking)
    {
        try
        {
            assert(this->eventReceiver);
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
            this->HandleEvent(ev);
        }
        catch(runtime_error &err)
        {
            //This is normal, no messages found
            checking = 0;
        }
    }

}

void AvBinMedia::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    //Only process events for this module
    if(ev->toUuid != this->uuid) return;

    if(ev->type == "AVBIN_FRAME_RESPONSE")
    {
        /*DecodedFrame *frame = (DecodedFrame *)ev->raw;

        //Convert to a QImage object
        QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                              QImage::Format_RGB888));
        RawImgToQImage(frame, *img);
        assert(!img->isNull());*/

        std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_FRAME_RESPONSE"));
        responseEv->data = ev->data;
        this->eventLoop->SendEvent(responseEv);

        //Return image to calling object by callback
        /*assert(frameCallback!=NULL);
        frameCallback(*img, ROUND_TIMESTAMP(frame->timestamp / 1000.),
                      ROUND_TIMESTAMP(frame->endTimestamp / 1000.),
                      ROUND_TIMESTAMP(frame->requestedTimestamp / 1000.),
                      raw);*/
    }

    if(ev->type == "GET_MEDIA_DURATION")
    {
        //Estimate progress and generate an event
        std::tr1::shared_ptr<class Event> requestEv(new Event("AVBIN_GET_DURATION"));
        requestEv->toUuid = this->uuid;
        requestEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(requestEv);

        std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(requestEv->id);

        std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_DURATION_RESPONSE"));
        responseEv->toUuid = ev->fromUuid;
        responseEv->id = ev->id;
        responseEv->data = response->data;
        this->eventLoop->SendEvent(responseEv);
    }

    if(ev->type == "GET_MEDIA_FRAME")
    {
        int debug = 1;
        assert(0);
    }
}


void AvBinMedia::ChangeVidSource(QString fina)
{
    if(fina == this->currentFina) return;
    assert(this->mediaThread != NULL);

    //Shut down media thread and delete
    int result = this->mediaThread->Stop();
    cout << "stop thread result=" << result << endl;
    delete this->mediaThread;
    this->mediaThread = NULL;

    //Create a new source
    this->mediaThread = new AvBinThread();
    this->mediaThread->SetEventLoop(this->eventLoop);
    this->mediaThread->SetUuid(this->uuid);
    this->mediaThread->Start();

    cout << "Opening " << fina.toLocal8Bit().constData() << endl;
    int ret = this->OpenFile(fina.toLocal8Bit().constData());
    this->currentFina = fina;
    if(ret==0)
        throw std::runtime_error("Error opening file");

}

QUuid AvBinMedia::GetUuid()
{
    assert(this!=NULL);
    return this->uuid;
}

//************************************************

