#ifndef AVBINMEDIA_H
#define AVBINMEDIA_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QThread>
#include <vector>
#include <tr1/memory>
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

protected:
    class EventReceiver eventReceiver;
    class EventLoop *eventLoop;
};

class AvBinThread : public QThread
{
public:
    AvBinThread(class EventLoop *eventLoopIn);
    virtual ~AvBinThread();
    void run();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);

protected:
    class EventReceiver eventReceiver;
    int stopThreads;
    class AvBinBackend avBinBackend;
    class EventLoop *eventLoop;
};

#endif // AVBINMEDIA_H
