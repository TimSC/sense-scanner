#include "avbinmedia.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include "eventloop.h"
#include "mediabuffer.h"
#include <sstream>
using namespace std;


AvBinMedia::AvBinMedia() : AbstractMedia()
{

}

AvBinMedia::~AvBinMedia()
{

}

int AvBinMedia::OpenFile(QString fina)
{
    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> openEv(new Event("AVBIN_OPEN_FILE", id));
    openEv->data = fina.toLocal8Bit().constData();
    this->eventLoop->SendEvent(openEv);
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti,
                                       long long unsigned &outFrameTi) //in milliseconds
{
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
        frameResponse = this->eventReceiver.WaitForEventId(id);

        if (frameResponse->type == "AVBIN_FRAME_FAILED")
        {
            std::ostringstream tmp;
            tmp << "Getting frame from media backend failed at ";
            tmp << ti;
            throw runtime_error(tmp.str());
        }

        assert(frameResponse->type == "AVBIN_FRAME_RESPONSE");
        assert(frameResponse->raw != NULL);
        DecodedFrame *frame = (DecodedFrame *)frameResponse->raw;

        assert(frame != NULL);

        assert(frame->width > 0);
        assert(frame->height > 0);

        //Convert to a QImage object
        QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                              QImage::Format_RGB888));
        assert(frame->buff != NULL);
        assert(frame->buffSize > 0);
        uint8_t *raw = &*frame->buff;
        outFrameTi = frame->timestamp / 1000;
        int cursor = 0;
        for(int j=0;j<frame->height;j++)
            for(int i=0;i<frame->width;i++)
            {
                cursor = i * 3 + (j * frame->width * 3);
                assert(cursor >= 0);
                assert(cursor + 2 < frame->buffSize);

                QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
                img->setPixel(i, j, value);
            }
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
    assert(0); //Not implemented
}

long long unsigned AvBinMedia::Length() //Get length (ms)
{
    unsigned long long id = this->eventLoop->GetId();
    std::tr1::shared_ptr<class Event> durationEvent(new Event("AVBIN_GET_DURATION", id));
    this->eventLoop->SendEvent(durationEvent);
    std::tr1::shared_ptr<class Event> ev = this->eventReceiver.WaitForEventId(id);
    assert(ev->type == "AVBIN_DURATION_RESPONSE");
    return std::strtoull(ev->data.c_str(),NULL,10) / 1000;
}

long long unsigned AvBinMedia::GetFrameStartTime(long long unsigned ti) //in milliseconds
{
    long long unsigned outFrameTi = 0;
    QSharedPointer<QImage> out = this->Get(ti, outFrameTi);
    return outFrameTi;
}

void AvBinMedia::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("AVBIN_DURATION_RESPONSE", this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_FRAME_RESPONSE", this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_FRAME_FAILED", this->eventReceiver);
}

//************************************

AvBinThread::AvBinThread(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("STOP_THREADS", eventReceiver);
    this->stopThreads = 0;
    this->avBinBackend.SetEventLoop(eventLoopIn);
}

AvBinThread::~AvBinThread()
{

}

void AvBinThread::run()
{
    std::tr1::shared_ptr<class Event> startEvent (new Event("THREAD_STARTING"));
    this->eventLoop->SendEvent(startEvent);

    while(!this->stopThreads)
    {
        //cout << "x" << this->eventReceiver.BufferSize() << endl;
        int foundEvent = 0;
        try
        {
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver.PopEvent();
            cout << "Event type " << ev->type << endl;
            foundEvent = 1;
            this->HandleEvent(ev);
        }
        catch(std::runtime_error e) {}

        //Update the backend to actually do something useful
        this->avBinBackend.PlayUpdate();

        if(!foundEvent)
            msleep(10);
    }

    std::tr1::shared_ptr<class Event> stopEvent(new Event("THREAD_STOPPING"));
    this->eventLoop->SendEvent(stopEvent);
    cout << "Stopping AvBinThread" << endl;
}

void AvBinThread::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type == "STOP_THREADS")
        this->stopThreads = 1;
    //if(ev.type == "AVBIN_GET_DURATION")


}

