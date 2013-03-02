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

void RawImgToQImage(QByteArray &pix, unsigned width, unsigned height, QImage &img)
{
    assert(width > 0);
    assert(height > 0);
    assert(pix.size() > 0);

    uint8_t *raw = (uint8_t *)pix.constBegin();
    unsigned int cursor = 0;
    for(unsigned int j=0;j<height;j++)
        for(unsigned int i=0;i<width;i++)
        {
            cursor = i * 3 + (j * width * 3);
            assert(cursor + 2 < pix.size());

            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
            img.setPixel(i, j, value);
        }
}

MediaResponseFrame::MediaResponseFrame()
{
    start = 0;
    end = 0;
    req = 0;
}

MediaResponseFrame::MediaResponseFrame(std::tr1::shared_ptr<class Event> ev)
{
    this->Process(ev);
}

void MediaResponseFrame::Process(std::tr1::shared_ptr<class Event> ev)
{
    assert(ev->type=="MEDIA_FRAME_RESPONSE");
    if(ev->data!="FAILED")
        throw std::runtime_error("Failed to get frame.");

    std::string tmp(ev->data.toLocal8Bit().constData());
    std::vector<std::string> args = split(tmp,',');
    this->start = atof(args[0].c_str());
    this->end = atof(args[1].c_str());
    this->req = atof(args[2].c_str());
    unsigned width = atoi(args[3].c_str());
    unsigned height = atoi(args[4].c_str());

    //Convert to a QImage object
    QImage img2(width, height,QImage::Format_RGB888);
    RawImgToQImage(ev->buffer, width, height, img2);
    this->img = img2;
}

//***************************************

MediaResponseFrameBasic::MediaResponseFrameBasic(std::tr1::shared_ptr<class Event> ev)
{
    assert(ev->type=="AVBIN_FRAME_RESPONSE");
    DecodedFrame *frame = (DecodedFrame *)ev->raw;
    assert(frame!=NULL);
    this->start = ROUND_TIMESTAMP(frame->timestamp / 1000.);
    this->end = ROUND_TIMESTAMP(frame->endTimestamp / 1000.);
    this->req = ROUND_TIMESTAMP(frame->requestedTimestamp / 1000.);
    this->height = frame->height;
    this->width = frame->width;
    this->img = QByteArray((const char *)frame->buff, frame->buffSize);
}

//***************************************

AvBinMedia::AvBinMedia(class EventLoop *eventLoopIn, bool removeOldRequestsIn) : MessagableThread()
{
    this->eventReceiver = NULL;
    this->eventLoop = eventLoopIn;
    this->mediaThread = NULL;
    this->removeOldRequests = removeOldRequestsIn;
    this->uuid = QUuid::createUuid();
    MessagableThread::SetEventLoop(eventLoopIn);

    if(this->eventReceiver != NULL)
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

}

AvBinMedia::~AvBinMedia()
{
    cout << "AvBinMedia::~AvBinMedia()" << endl;
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

/*long long unsigned  AvBinMedia::RequestFrame(QString source, long long unsigned ti) //in milliseconds
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
}*/

void AvBinMedia::Update()
{
    //cout << "AvBinMedia::Update " << (unsigned long long) this << endl;
    this->msleep(5);
}

void AvBinMedia::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    //Only process events for this module
    if(ev->toUuid != this->uuid)
    {
        MessagableThread::HandleEvent(ev);
        return;
    }

    if(ev->type == "AVBIN_FRAME_RESPONSE")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_FRAME_RESPONSE"));

        cout << "x" ;
    }

    if(ev->type=="AVBIN_FRAME_FAILED")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_DURATION_RESPONSE"));

        cout << "y";
    }

    if(ev->type == "GET_MEDIA_DURATION")
    {
        if(ev->data != this->currentFina)
            this->ChangeVidSource(ev->data);

        std::tr1::shared_ptr<class Event> requestEv(new Event("AVBIN_GET_DURATION"));
        requestEv->toUuid = this->uuid;
        requestEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(requestEv);

        std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(requestEv->id);
        if(response->type=="AVBIN_REQUEST_FAILED")
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_DURATION_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            responseEv->data = "FAILED";
            this->eventLoop->SendEvent(responseEv);
        }
        else
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_DURATION_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            responseEv->data = QString("%1").arg(response->data.toULongLong() / 1000.);
            this->eventLoop->SendEvent(responseEv);
        }
    }

    if(ev->type == "GET_MEDIA_FRAME")
    {
        if(this->removeOldRequests)
        {
            //To prevent a backlog of frame requests developing, old requests
            //may be discarded from the queue at this stage
            try
            {
                ev = this->eventReceiver->GetLatestDiscardOlder("GET_MEDIA_FRAME");
            }
            catch(std::runtime_error &err)
            {
                //No later event found
            }
        }

        if(ev->data != this->currentFina)
            this->ChangeVidSource(ev->data);

        std::tr1::shared_ptr<class Event> requestEv(new Event("AVBIN_GET_FRAME"));
        requestEv->toUuid = this->uuid;
        requestEv->data = QString("%1").arg(ev->buffer.toULongLong() * 1000);
        requestEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(requestEv);

        //If request id is non-zero, block until the result is returned
        //This makes it conceptually easier to return the frames to the appropriate clients
        if(requestEv->id>0)
        {
        std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(requestEv->id);
        if(response->type=="AVBIN_FRAME_FAILED")
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_FRAME_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            responseEv->data = "FAILED";
            this->eventLoop->SendEvent(responseEv);
        }
        else
        {
            QString test = response->type;
            MediaResponseFrameBasic fd(response);
            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_FRAME_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            QString datastr = QString("%1,%2,%3,%4,%5").arg(fd.start).arg(fd.end).arg(fd.req).arg(fd.width).arg(fd.height);
            responseEv->data = datastr;
            responseEv->buffer = fd.img;
            this->eventLoop->SendEvent(responseEv);
        }
        }
    }

    MessagableThread::HandleEvent(ev);
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

unsigned long long AvBinMedia::GetMediaDuration(QString fina,
                                                QUuid mediaUuid,
                                                class EventLoop *eventLoop,
                                                class EventReceiver *eventReceiver)
{
    std::tr1::shared_ptr<class Event> requestEv(new Event("GET_MEDIA_DURATION"));
    requestEv->toUuid = mediaUuid;
    requestEv->data = fina;
    requestEv->id = eventLoop->GetId();
    eventLoop->SendEvent(requestEv);

    std::tr1::shared_ptr<class Event> response = eventReceiver->WaitForEventId(requestEv->id);
    assert(response->type == "MEDIA_DURATION_RESPONSE");
    return response->data.toULongLong();

}

void AvBinMedia::GetMediaFrame(QString fina,
                     unsigned long long ti,
                     QUuid mediaUuid,
                     class EventLoop *eventLoop,
                     class EventReceiver *eventReceiver,
                     MediaResponseFrame &out)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_MEDIA_FRAME"));
    reqEv->toUuid = mediaUuid;
    reqEv->data = fina;
    QString tiStr = QString("%1").arg(ti);
    reqEv->buffer = tiStr.toLocal8Bit().constData();
    reqEv->id = eventLoop->GetId();
    eventLoop->SendEvent(reqEv);

    std::tr1::shared_ptr<class Event> resp = eventReceiver->WaitForEventId(reqEv->id);
    assert(resp->type=="MEDIA_FRAME_RESPONSE");

    out.Process(resp);
}
