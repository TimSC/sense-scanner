#ifndef AVBINMEDIA_H
#define AVBINMEDIA_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <vector>
#include "localmutex.h"
#ifdef _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif
#include "mediabuffer.h"
#include "eventloop.h"
#include "avbinbackend.h"

class MediaResponseFrame
{
public:
    MediaResponseFrame();
    MediaResponseFrame(std::tr1::shared_ptr<class Event> ev);
    void Process(std::tr1::shared_ptr<class Event> ev);

    QImage img;

    unsigned long long start;
    unsigned long long end;
    unsigned long long req;
};

class MediaResponseFrameBasic
{
public:
    MediaResponseFrameBasic(std::tr1::shared_ptr<class Event> ev);

    QByteArray img;

    unsigned long long start;
    unsigned long long end;
    unsigned long long req;
    unsigned height, width;
};

//*************************************************

class AvBinMedia : public MessagableThread
{
    /*!
    * A high level interface to retrieve video frames. The decoding is
    * performed by AvBinThread and AvBinBackend.
    */

public:
    explicit AvBinMedia(class EventLoop *eventLoopIn, bool removeOldRequestsIn);
    virtual ~AvBinMedia();

    void SetEventLoop(class EventLoop *eventLoopIn);
    void TerminateThread();

    long long unsigned RequestFrame(QString source, long long unsigned ti);
    void Update();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    QUuid GetUuid();

    static unsigned long long GetMediaDuration(QString fina,
                                                    QUuid annotUuid,
                                                    class EventLoop *eventLoop,
                                                    class EventReceiver *eventReceiver);

    static void GetMediaFrame(QString fina,
                         unsigned long long ti,
                         QUuid mediaUuid,
                         class EventLoop *eventLoop,
                         class EventReceiver *eventReceiver,
                         MediaResponseFrame &out);

protected:
    int OpenFile(QString fina);
    void ChangeVidSource(QString fina);

    AvBinThread *mediaThread;
    QString currentFina;
    QUuid uuid;
    bool removeOldRequests;

    QFile *mediaLog;
    bool logToFile;
};

#endif // AVBINMEDIA_H
