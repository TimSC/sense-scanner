#include "avbinmedia.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include "eventloop.h"
#include "mediabuffer.h"
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
    class Event openEv = Event("AVBIN_OPEN_FILE", id);
    openEv.data = fina.toLocal8Bit().constData();
    this->eventLoop->SendEvent(openEv);
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti) //in milliseconds
{

    //Get the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long id = this->eventLoop->GetId();
    class Event getFrameEvent = Event("AVBIN_GET_FRAME", id);
    std::ostringstream tmp;
    tmp << ti;
    getFrameEvent.data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);
    DecodedFrame *frame2 = NULL;
    try
    {
        class Event frameResponse = this->eventReceiver.WaitForEventId(id);
        assert(frameResponse.type == "AVBIN_FRAME_RESPONSE");
        assert(frameResponse.raw != NULL);
        frame2 = (DecodedFrame *)frameResponse.raw;
    }
    catch(std::runtime_error &err)
    {

    }

    assert(frame2 != NULL);
    DecodedFrame &frame = *frame2;

    //Convert raw image format to QImage
    QSharedPointer<QImage> img(new QImage(frame.width, frame.height, QImage::Format_RGB888));
    //QSharedPointer<QImage> img(new QImage(100, 100, QImage::Format_RGB888));

    cout << "frame time " << ti << endl;
    assert(frame.raw != NULL);
    assert(frame.raw->buffSize > 0);
    uint8_t *raw = &*frame.raw->buff;
    int cursor = 0;
    for(int j=0;j<frame.height;j++)
        for(int i=0;i<frame.width;i++)
        {
            cursor = i * 3 + (j * frame.width * 3);
            assert(cursor >= 0);
            assert(cursor + 2 < frame.raw->buffSize);

            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
            img->setPixel(i, j, value);
        }

    return img;
}

long long unsigned AvBinMedia::GetNumFrames()
{
    assert(0); //Not implemented
}

long long unsigned AvBinMedia::Length() //Get length (ms)
{
    unsigned long long id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(Event("AVBIN_GET_DURATION", id));
    class Event ev = this->eventReceiver.WaitForEventId(id);
    assert(ev.type == "AVBIN_DURATION_RESPONSE");
    return std::strtoull(ev.data.c_str(),NULL,10);

    //return this->backend->Length() / 1000.;
}

long long unsigned AvBinMedia::GetFrameStartTime(long long unsigned ti) //in milliseconds
{
    return ti;
}

void AvBinMedia::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("AVBIN_DURATION_RESPONSE", this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_FRAME_RESPONSE", this->eventReceiver);
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
    this->eventLoop->SendEvent(Event("THREAD_STARTING"));

    while(!this->stopThreads)
    {
        //cout << "x" << this->eventReceiver.BufferSize() << endl;
        int foundEvent = 0;
        try
        {
            class Event ev = this->eventReceiver.PopEvent();
            cout << "Event type " << ev.type << endl;
            foundEvent = 1;
            this->HandleEvent(ev);
        }
        catch(std::runtime_error e) {}

        //Update the backend to actually do something useful
        this->avBinBackend.PlayUpdate();

        if(!foundEvent)
            msleep(10);
    }

    this->eventLoop->SendEvent(Event("THREAD_STOPPING"));
    cout << "Stopping AvBinThread" << endl;
}

void AvBinThread::HandleEvent(class Event &ev)
{
    if(ev.type == "STOP_THREADS")
        this->stopThreads = 1;
    //if(ev.type == "AVBIN_GET_DURATION")


}

