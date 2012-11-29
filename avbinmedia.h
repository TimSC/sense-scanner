#ifndef AVBINMEDIA_H
#define AVBINMEDIA_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtCore/QThread>
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

class AvBinMedia : public AbstractMedia
{
public:
    explicit AvBinMedia();
    virtual ~AvBinMedia();

    virtual QSharedPointer<QImage> Get(long long unsigned ti,
                                       long long unsigned &outFrameTi); //in milliseconds
    virtual long long unsigned GetNumFrames();
    virtual long long unsigned Length(); //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds
    void SetEventLoop(class EventLoop *eventLoopIn);
    int OpenFile(QString fina);
    void SetActive(int activeIn);

    int RequestFrame(long long unsigned ti);
    void Update(void (*frameCallback)(QImage& fr, unsigned long long timestamp, void *raw), void *raw);

protected:
    class EventReceiver *eventReceiver;
    class EventLoop *eventLoop;
    int active;
};

class AvBinThread : public QThread
{
public:
    AvBinThread(class EventLoop *eventLoopIn);
    virtual ~AvBinThread();
    void run();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    int StopThread();

protected:
    class EventReceiver *eventReceiver;
    int stopThreads;
    class AvBinBackend avBinBackend;
    class EventLoop *eventLoop;
    Mutex mutex;
};

#endif // AVBINMEDIA_H
