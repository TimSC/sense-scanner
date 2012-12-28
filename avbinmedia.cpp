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

AvBinMedia::AvBinMedia() : AbstractMedia()
{
    this->eventReceiver = NULL;
    this->eventLoop = NULL;
    this->active = 0;
    this->id = 0;
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

void AvBinMedia::SetId(int idIn)
{
    this->id = idIn;
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti,
                                       long long unsigned &outFrameStart,
                                       long long unsigned &outFrameEnd,
                                       long long unsigned timeout) //in milliseconds
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");
    outFrameStart = 0;
    outFrameEnd = 0;

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
    std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(id,timeout);
    assert(ev->type == "AVBIN_FRAME_RESPONSE");
    assert(ev->raw!=NULL);

    DecodedFrame *frame = (DecodedFrame *)ev->raw;

    outFrameStart = frame->timestamp;
    outFrameEnd = frame->endTimestamp;
    //Convert to a QImage object
    QSharedPointer<QImage> img(new QImage(frame->width, frame->height,
                                          QImage::Format_RGB888));
    RawImgToQImage(frame, *img);
    assert(!img->isNull());
    return img;

    //Return something if things fail
    //QSharedPointer<QImage> img(new QImage(100, 100, QImage::Format_RGB888));
    //return img;

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

    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_DURATION%1").arg(this->id);
    std::tr1::shared_ptr<class Event> durationEvent(new Event(eventName.toLocal8Bit().constData(), evid));
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

    long long unsigned outFrameTi = 0, outFrameEndTi = 0;
    QSharedPointer<QImage> out = this->Get(ti, outFrameTi, outFrameEndTi);
    cout << "Frame start" << outFrameTi << endl;
    return outFrameTi;
}

void AvBinMedia::SetEventLoop(class EventLoop *eventLoopIn)
{
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
}

//*******************************************

int AvBinMedia::RequestFrame(long long unsigned ti) //in milliseconds
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    //Request the frame from the backend thread
    assert(this->eventLoop != NULL);
    unsigned long long evid = this->eventLoop->GetId();
    QString eventName = QString("AVBIN_GET_FRAME%1").arg(this->id);
    std::tr1::shared_ptr<class Event> getFrameEvent(new Event(eventName.toLocal8Bit().constData(), evid));
    std::ostringstream tmp;
    tmp << ti * 1000;
    getFrameEvent->data = tmp.str();
    this->eventLoop->SendEvent(getFrameEvent);
    return id;
}

void AvBinMedia::Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                              unsigned long long endTimestamp,
                                              unsigned long long requestTimestamp,
                                              void *raw), void *raw)
{
    assert(this->active);
    if(!this->active)
        throw runtime_error("Media interface not active");

    //Check for new frames from media backend.
    int checking = 1;
    //cout << "GUI receive queue " << this->eventReceiver->BufferSize() << endl;
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

void AvBinMedia::SetActive(int activeIn)
{
    this->active = activeIn;
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
