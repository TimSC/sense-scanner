#include "avbinmedia.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include "eventloop.h"
#include "mediabuffer.h"
#include <sstream>
using namespace std;
#ifdef _MSC_VER
#define STR_TO_ULL _strtoui64
#else
#define STR_TO_ULL std::strtoull
#endif
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)

AvBinMedia::AvBinMedia() : AbstractMedia()
{
    this->eventReceiver = NULL;
    this->eventLoop = NULL;
    this->active = 0;
}

AvBinMedia::~AvBinMedia()
{
    cout << "AvBinMedia::~AvBinMedia()" << endl;
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventReceiver = NULL;
}

int AvBinMedia::OpenFile(QString fina)
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");
    assert(this->eventLoop);
    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> openEv(new Event("AVBIN_OPEN_FILE", id));
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


QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti,
                                       long long unsigned &outFrameTi) //in milliseconds
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");
    outFrameTi = 0;

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event("AVBIN_GET_FRAME", id));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);
    std::tr1::shared_ptr<class Event> frameResponse;
    try
    {
        //Wait for the response from the media backend
        assert(this->eventReceiver);
        frameResponse = this->eventReceiver->WaitForEventId(id);

        if (frameResponse->type == "AVBIN_FRAME_FAILED")
        {
            std::ostringstream tmp;
            tmp << "Warning: getting frame from media backend failed at ";
            tmp << ti;
            throw runtime_error(tmp.str());
        }

        if(frameResponse->type != "AVBIN_FRAME_RESPONSE")
            throw runtime_error("Warning: incorrect response type");

        assert(frameResponse->type == "AVBIN_FRAME_RESPONSE");
        assert(frameResponse->raw != NULL);
        DecodedFrame *frame = (DecodedFrame *)frameResponse->raw;

        //Convert to a QImage object
        QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                              QImage::Format_RGB888));
        RawImgToQImage(frame, *img);
        cout << "frame->timestamp " << frame->timestamp << endl;

        assert(!img->isNull());
        return img;
    }
    catch(std::runtime_error &err)
    {
        cout << "Warning: wait for frame response encountered an error" << endl;
        cout << err.what() << endl;
    }

    //Return something if things fail
    QSharedPointer<QImage> img(new QImage(100, 100, QImage::Format_RGB888));
    return img;

}

long long unsigned AvBinMedia::GetNumFrames()
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");
    assert(0); //Not implemented
	return 0;
}

long long unsigned AvBinMedia::Length() //Get length (ms)
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> durationEvent(new Event("AVBIN_GET_DURATION", id));
    this->eventLoop->SendEvent(durationEvent);
    assert(this->eventReceiver);
    std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(id);
    assert(ev->type == "AVBIN_DURATION_RESPONSE");
    return ROUND_TIMESTAMP(STR_TO_ULL(ev->data.c_str(),NULL,10) / 1000.);
}

long long unsigned AvBinMedia::GetFrameStartTime(long long unsigned ti) //in milliseconds
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    long long unsigned outFrameTi = 0;
    QSharedPointer<QImage> out = this->Get(ti, outFrameTi);
    cout << "Frame start" << outFrameTi << endl;
    return outFrameTi;
}

void AvBinMedia::SetEventLoop(class EventLoop *eventLoopIn)
{
    if(this->eventReceiver == NULL)
    {
        this->eventReceiver = new class EventReceiver(eventLoopIn);
        this->eventLoop = eventLoopIn;
        this->eventLoop->AddListener("AVBIN_DURATION_RESPONSE", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_FRAME_RESPONSE", *this->eventReceiver);
        this->eventLoop->AddListener("AVBIN_FRAME_FAILED", *this->eventReceiver);
    }
}

//*******************************************

int AvBinMedia::RequestFrame(long long unsigned ti) //in milliseconds
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event("AVBIN_GET_FRAME", id));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);
    return id;
}

void AvBinMedia::Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                              unsigned long long endTimestamp,
                                              void *raw), void *raw)
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    //Check for new frames from media backend.
    int checking = 1;
    while(checking)
    {
        try
        {
            assert(this->eventReceiver);
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
            if(ev->type == "AVBIN_FRAME_RESPONSE")
            {
                DecodedFrame *frame = (DecodedFrame *)ev->raw;

                //Convert to a QImage object
                QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                                      QImage::Format_RGB888));
                RawImgToQImage(frame, *img);
                assert(!img->isNull());

                //Return image to calling object by callback
                frameCallback(*img, ROUND_TIMESTAMP(frame->timestamp / 1000.),
                              ROUND_TIMESTAMP(frame->endTimestamp / 1000.), raw);
            }

        }
        catch(runtime_error &err)
        {
            //This is normal, no messages found
            checking = 0;
        }
    }

}

void AvBinMedia::SetActive(int activeIn)
{
    this->active = activeIn;
}

//************************************

AvBinThread::AvBinThread(class EventLoop *eventLoopIn)
{
    //this->setStackSize(1024*1024*100);
    this->eventReceiver = new EventReceiver(eventLoopIn);
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("STOP_THREADS", *eventReceiver);
    this->stopThreads = 0;
    this->avBinBackend.SetEventLoop(eventLoopIn);
}

AvBinThread::~AvBinThread()
{
    this->StopThread();
    if(this->eventReceiver)
        delete this->eventReceiver;
    this->eventReceiver = NULL;
}

void AvBinThread::run()
{
    std::tr1::shared_ptr<class Event> startEvent (new Event("THREAD_STARTING"));
    this->eventLoop->SendEvent(startEvent);

    int running = true;

    while(running)
    {
        this->mutex.lock();
        running = !this->stopThreads;
        this->mutex.unlock();

        //cout << "x" << this->eventReceiver.BufferSize() << endl;
        int foundEvent = 0;
        try
        {
            assert(this->eventReceiver);
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
            cout << "Event type " << ev->type << endl;
            foundEvent = 1;
            this->HandleEvent(ev);
        }
        catch(std::runtime_error e) {}

        //Update the backend to actually do something useful
        foundEvent = this->avBinBackend.PlayUpdate();

        if(!foundEvent)
            msleep(10);
        else
            msleep(0);
    }

    std::tr1::shared_ptr<class Event> stopEvent(new Event("THREAD_STOPPING"));
    this->eventLoop->SendEvent(stopEvent);
    cout << "Stopping AvBinThread" << endl;
}

void AvBinThread::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type == "STOP_THREADS")
    {
        this->mutex.lock();
        this->stopThreads = 1;
        this->mutex.unlock();
    }
}
int AvBinThread::StopThread()
{
    this->mutex.lock();
    this->stopThreads = 1;
    this->mutex.unlock();

    for(int i=0;i<500;i++)
    {
        if(this->isFinished())
        {
            return 1;
        }
        usleep(10000); //microsec
    }
    if(this->isRunning())
    {
        cout << "Warning: terminating media thread" << endl;
        this->terminate();
    }
    return 0;
}
