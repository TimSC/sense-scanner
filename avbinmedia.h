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
    Q_OBJECT
public:
    explicit AvBinMedia(QObject *parent);
    virtual ~AvBinMedia();

public slots:
    virtual QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    virtual long long unsigned GetNumFrames();
    virtual long long unsigned Length(); //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds
    void SetEventLoop(QSharedPointer<class EventLoop> &eventLoopIn);
    int OpenFile(QString fina);

protected:
    std::vector<std::tr1::shared_ptr<class FrameGroup> > groupCache;
    class DecodedFrame singleFrame;
    QSharedPointer<class EventLoop> eventLoop;
};

class AvBinThread : public QThread
{
public:
    AvBinThread(QSharedPointer<class EventLoop> &eventLoopIn);
    virtual ~AvBinThread();
    void run();
    void HandleEvent(class Event &ev);
    void SetEventLoop(QSharedPointer<class EventLoop> &eventLoopIn);

protected:
    class EventReceiver eventReceiver;
    int stopThreads;
    class AvBinBackend avBinBackend;
    QSharedPointer<class EventLoop> eventLoop;
};

#endif // AVBINMEDIA_H
