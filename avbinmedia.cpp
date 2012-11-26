#include "avbinmedia.h"
#include "avbinbackend.h"
#include <assert.h>
#include <iostream>
#include <exception>
#include "eventloop.h"
using namespace std;


AvBinMedia::AvBinMedia(QObject *parent, QString fina) : AbstractMedia(parent)
{
    this->backend = new AvBinBackend;
    this->backend->OpenFile(fina.toLocal8Bit().constData());
}

AvBinMedia::~AvBinMedia()
{
    if(this->backend) delete this->backend;
    this->backend = NULL;
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti) //in milliseconds
{
    class DecodedFrame &frame = this->singleFrame;

    //Get the frame from the backend thread
    //this->eventLoop->SendEvent(Event("GET_FRAME"));
    this->backend->GetFrame(ti * 1000, frame);

    //Convert raw image format to QImage
    QSharedPointer<QImage> img(new QImage(frame.width, frame.height, QImage::Format_RGB888));

    cout << "frame time " << ti << endl;
    assert(frame.buffSize > 0);
    uint8_t *raw = &*frame.buff;
    int cursor = 0;
    for(int j=0;j<frame.height;j++)
        for(int i=0;i<frame.width;i++)
        {
            cursor = i * 3 + (j * frame.width * 3);
            uint8_t *raw = &*frame.buff;
            assert(cursor >= 0);
            assert(cursor + 2 < frame.buffSize);

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
    return this->backend->Length() / 1000.;
}

long long unsigned AvBinMedia::GetFrameStartTime(long long unsigned ti) //in milliseconds
{
    return ti;
}

void AvBinMedia::SetEventLoop(QSharedPointer<class EventLoop> &eventLoopIn)
{
    this->eventLoop = eventLoopIn;
}

//************************************

AvBinThread::AvBinThread(QSharedPointer<class EventLoop> &eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("STOP_THREADS", eventReceiver);
    this->stopThreads = 0;
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
        if(!foundEvent)
            msleep(200);
    }

    this->eventLoop->SendEvent(Event("THREAD_STOPPING"));
    cout << "Stopping AvBinThread" << endl;
}

void AvBinThread::HandleEvent(class Event &ev)
{
    if(ev.type == "STOP_THREADS")
    {
        this->stopThreads = 1;

    }

}
