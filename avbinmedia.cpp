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

AvBinMedia::AvBinMedia(int idIn, class EventLoop *eventLoopIn) : AbstractMedia()
{
    this->eventReceiver = NULL;
    this->eventLoop = eventLoopIn;
    this->id = idIn;
    this->mediaThread = NULL;

    if(this->eventReceiver == NULL)
    {
        this->eventReceiver = new class EventReceiver(eventLoopIn);
        this->eventLoop = eventLoopIn;
        QString eventName = QString("AVBIN_DURATION_RESPONSE%1").arg(this->id);
        this->eventLoop->AddListener(eventName.toLocal8Bit().constData(), *this->eventReceiver);
        QString eventName2 = QString("AVBIN_FRAME_RESPONSE%1").arg(this->id);
        this->eventLoop->AddListener(eventName2.toLocal8Bit().constData(), *this->eventReceiver);
        QString eventName3 = QString("AVBIN_FRAME_FAILED%1").arg(this->id);
        this->eventLoop->AddListener(eventName3.toLocal8Bit().constData(), *this->eventReceiver);
    }

    this->mediaThread = new AvBinThread();
    this->mediaThread->SetId(this->id);
    this->mediaThread->SetEventLoop(this->eventLoop);
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
    QString eventName = QString("AVBIN_OPEN_FILE%1").arg(this->id);
    std::tr1::shared_ptr<class Event> openEv(new Event(eventName.toLocal8Bit().constData(), evid));
    openEv->data = fina.toLocal8Bit().constData();
    this->eventLoop->SendEvent(openEv);
    return 1;
}

void RawImgToQImage(DecodedFrame *frame, QImage &img)
{
    assert(frame != NULL);
    assert(frame->width > 0);
    assert(frame->height > 0);
    assert(frame->buff != NULL);
    assert(frame->buffSize > 0);

    uint8_t *raw = &*frame->buff;
    unsigned int cursor = 0;
    for(unsigned int j=0;j<frame->height;j++)
        for(unsigned int i=0;i<frame->width;i++)
        {
            cursor = i * 3 + (j * frame->width * 3);
            assert(cursor + 2 < frame->buffSize);

            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
            img.setPixel(i, j, value);
        }
}

void AvBinMedia::TerminateThread()
{
    if(this->mediaThread!=NULL && this->mediaThread->isRunning())
    {
        cout << "Warning: terminating buffer media tread" << endl;
        this->mediaThread->terminate();
    }
}

QSharedPointer<QImage> AvBinMedia::Get(QString source,
                                       long long unsigned ti,
                                       long long unsigned &outFrameStart,
                                       long long unsigned &outFrameEnd,
                                       long long unsigned timeout) //in milliseconds
{

    this->lock.lock();
    outFrameStart = 0;
    outFrameEnd = 0;

    //Check source is what is currently loaded
    this->ChangeVidSource(source);

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_FRAME%1").arg(this->id);
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event(eventName.toLocal8Bit().constData(), evid));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);

    //Wait for frame response
    assert(this->eventReceiver);
    std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(evid,timeout);
    QString evType = ev->type.c_str();
    if(evType.left(18) == "AVBIN_FRAME_FAILED")
    {
        this->lock.unlock();
        throw runtime_error("Get frame failed");
    }

    assert(evType.left(20) == "AVBIN_FRAME_RESPONSE");
    assert(ev->raw!=NULL);

    DecodedFrame *frame = (DecodedFrame *)ev->raw;

    outFrameStart = frame->timestamp;
    outFrameEnd = frame->endTimestamp;
    //Convert to a QImage object
    QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                          QImage::Format_RGB888));
    RawImgToQImage(frame, *img);
    this->lock.unlock();
    assert(!img->isNull());
    return img;

    //Return something if things fail
    //QSharedPointer<QImage> img(new QImage(100, 100, QImage::Format_RGB888));
    //return img;

}

long long unsigned AvBinMedia::Length(QString source) //Get length (ms)
{
    this->lock.lock();
    //Check source is what is currently loaded
    this->ChangeVidSource(source);

    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_DURATION%1").arg(this->id);
    std::tr1::shared_ptr<class Event> durationEvent(new Event(eventName.toLocal8Bit().constData(), evid));
    this->eventLoop->SendEvent(durationEvent);
    assert(this->eventReceiver);
    std::tr1::shared_ptr<class Event> ev;
    try
    {
        ev = this->eventReceiver->WaitForEventId(evid);
    }
    catch(std::runtime_error &err)
    {
        this->lock.unlock();
        throw std::runtime_error(err.what());
    }

    QString eventNameRx = QString("AVBIN_DURATION_RESPONSE%1").arg(this->id);
    this->lock.unlock();
    assert(ev->type == eventNameRx.toLocal8Bit().constData());
    return ROUND_TIMESTAMP(STR_TO_ULL(ev->data.c_str(),NULL,10) / 1000.);
}

long long unsigned AvBinMedia::GetFrameStartTime(QString source, long long unsigned ti) //in milliseconds
{
    long long unsigned outFrameTi = 0, outFrameEndTi = 0;
    QSharedPointer<QImage> out = this->Get(source, ti, outFrameTi, outFrameEndTi);
    cout << "Frame start" << outFrameTi << endl;
    return outFrameTi;
}

//*******************************************

int AvBinMedia::RequestFrame(QString source, long long unsigned ti) //in milliseconds
{
    this->lock.lock();

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_FRAME%1").arg(this->id);
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event(eventName.toLocal8Bit().constData(), evid));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);
    this->lock.unlock();
    return id;
}

void AvBinMedia::Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                              unsigned long long endTimestamp,
                                              unsigned long long requestTimestamp,
                                              void *raw), void *raw)
{

    //Check for new frames from media backend.
    int checking = 1;
    //cout << "GUI receive queue " << this->eventReceiver->BufferSize() << endl;
    while(checking)
    {
        try
        {
            assert(this->eventReceiver);
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
            QString evType = ev->type.c_str();
            cout << evType.toLocal8Bit().constData() << endl;
            if(evType.left(20) == "AVBIN_FRAME_RESPONSE")
            {
                DecodedFrame *frame = (DecodedFrame *)ev->raw;

                //Convert to a QImage object
                QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                                      QImage::Format_RGB888));
                RawImgToQImage(frame, *img);
                assert(!img->isNull());

                //Return image to calling object by callback
                frameCallback(*img, ROUND_TIMESTAMP(frame->timestamp / 1000.),
                              ROUND_TIMESTAMP(frame->endTimestamp / 1000.),
                              ROUND_TIMESTAMP(frame->requestedTimestamp / 1000.),
                              raw);
            }

        }
        catch(runtime_error &err)
        {
            //This is normal, no messages found
            checking = 0;
        }
    }

}

void AvBinMedia::ChangeVidSource(QString fina)
{
    if(fina == this->currentFina) return;
    assert(this->mediaThread != NULL);

    //Mark media interface as inactive
    int threadId = this->mediaThread->GetId();

    //Shut down media thread and delete
    int result = this->mediaThread->Stop();
    cout << "stop thread result=" << result << endl;
    delete this->mediaThread;
    this->mediaThread = NULL;

    //Create a new source
    this->mediaThread = new AvBinThread();
    this->mediaThread->SetId(threadId);
    this->mediaThread->SetEventLoop(this->eventLoop);
    this->mediaThread->Start();

    cout << "Opening " << fina.toLocal8Bit().constData() << endl;
    this->OpenFile(fina.toLocal8Bit().constData());
    this->currentFina = fina;
}

//*************************************************

AvBinThread::AvBinThread() : MessagableThread()
{

}

AvBinThread::~AvBinThread()
{

}

void AvBinThread::Update()
{
    //if(id==1)
    //{
    //    cout << "AvBinThread::Update()" << this->id <<"\t"<< (unsigned long)this << endl;
    //}

    int foundEvent = 0;

    //Update the backend to actually do something useful
    foundEvent = this->avBinBackend.PlayUpdate();

    if(!foundEvent)
        msleep(10);
    else
        msleep(0);
}

void AvBinThread::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->avBinBackend.SetEventLoop(eventLoopIn);
    MessagableThread::SetEventLoop(eventLoopIn);
}

void AvBinThread::SetId(int idIn)
{
    this->avBinBackend.SetId(idIn);
    MessagableThread::SetId(idIn);
}

//************************************************

