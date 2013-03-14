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
    if(ev->data.left(13)=="AVBIN_ERROR: ")
    {
        throw std::runtime_error(ev->data.toLocal8Bit().constData());
    }

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
    this->logToFile = false;
    this->removeOldRequests = removeOldRequestsIn;
    this->uuid = QUuid::createUuid();
    this->SetEventLoop(eventLoopIn);

    this->mediaThread = new AvBinThread();
    this->mediaThread->SetEventLoop(this->eventLoop);
    this->mediaThread->SetUuid(this->uuid);
    this->mediaThread->Start();

    this->mediaLog = NULL;
    if(this->logToFile)
    {
        QString fina = QString("mediaLog%1.txt").arg(this->uuid);
        this->mediaLog = new QFile(fina);
        this->mediaLog->open(QIODevice::WriteOnly);
    }

}

AvBinMedia::~AvBinMedia()
{
    cout << "AvBinMedia::~AvBinMedia()" << endl;
    if(this->mediaThread!=NULL)
        delete this->mediaThread;
    this->mediaThread = NULL;
}

void AvBinMedia::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);

    this->eventLoop->AddListener("AVBIN_DURATION_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_FRAME_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_FRAME_FAILED", *this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_REQUEST_FAILED", *this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_OPEN_RESULT", *this->eventReceiver);
    this->eventLoop->AddListener("GET_MEDIA_DURATION", *this->eventReceiver);
    this->eventLoop->AddListener("GET_MEDIA_FRAME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_MEDIA_FRAME", *this->eventReceiver);
}

int AvBinMedia::OpenFile(QString fina)
{
    assert(this->eventLoop);
    if(this->mediaLog!=NULL)
        this->mediaLog->write(QString("AvBinMedia::OpenFile %1\n")
                              .arg(this->uuid.toString()).toLocal8Bit().constData());

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
        //Ignore this event
        if(this->mediaLog!=NULL)
        {
            this->mediaLog->write(QString("GOT_ASYNC_FRAME %1\n").arg(this->uuid.toString())
                              .toLocal8Bit().constData());
            this->mediaLog->flush();
        }
    }

    if(ev->type=="AVBIN_FRAME_FAILED")
    {
        //Ignore this event
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
            responseEv->buffer = this->currentFina.toUtf8();
            responseEv->data = "FAILED";
            this->eventLoop->SendEvent(responseEv);
        }
        else
        {
            assert(response->type=="AVBIN_DURATION_RESPONSE");
            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_DURATION_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            responseEv->buffer = this->currentFina.toUtf8();
            unsigned long long len = (unsigned long long)(0.5 + (response->data.toULongLong() / 1000.));
            responseEv->data = QString("%1").arg(len);
            this->eventLoop->SendEvent(responseEv);
        }
    }

    if(ev->type == "GET_MEDIA_FRAME")
    {
        if(this->mediaLog!=NULL)
        {
            this->mediaLog->write(QString("GET_MEDIA_FRAME %1 %2\n").arg(this->uuid.toString())
                              .arg(ev->buffer.toULongLong()).toLocal8Bit().constData());
            this->mediaLog->flush();
        }

        if(this->removeOldRequests)
        {
            //To prevent a backlog of frame requests developing, old requests
            //may be discarded from the queue at this stage
            try
            {
                ev = this->eventReceiver->GetLatestDiscardOlder("GET_MEDIA_FRAME", this->uuid);
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
            if(this->mediaLog!=NULL)
            {
                this->mediaLog->write(QString("AVBIN_FRAME_FAILED %1\n").arg(this->uuid.toString())
                                  .toLocal8Bit().constData());
                this->mediaLog->flush();
            }

            std::tr1::shared_ptr<class Event> responseEv(new Event("MEDIA_FRAME_RESPONSE"));
            responseEv->toUuid = ev->fromUuid;
            responseEv->fromUuid = this->uuid;
            responseEv->id = ev->id;
            responseEv->data = "AVBIN_ERROR: ";
            responseEv->data.append(response->data);        //Pass along the type of error
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

            if(this->mediaLog!=NULL)
            {
                this->mediaLog->write(QString("GOT_REQ_FRAME %1 %2 %3\n").arg(this->uuid.toString())
                                  .arg(fd.start)
                                  .arg(fd.end)
                                  .toLocal8Bit().constData());
                this->mediaLog->flush();
            }
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
    unsigned long long out = response->data.toULongLong();
    return out;

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
