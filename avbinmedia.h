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

class AvBinThread : public QThread
{
public:
    AvBinThread(QSharedPointer<class EventLoop> &eventLoopIn);
    virtual ~AvBinThread();
    void run();
    void HandleEvent(class Event &ev);

protected:
    QSharedPointer<class EventLoop> eventLoop;
    class EventReceiver eventReceiver;
    int stopThreads;
};


class AvBinMedia : public AbstractMedia
{
    Q_OBJECT
public:
    explicit AvBinMedia(QObject *parent, QString fina);
    virtual ~AvBinMedia();

public slots:
    virtual QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    virtual long long unsigned GetNumFrames();
    virtual long long unsigned Length(); //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds

protected:
    class AvBinBackend *backend;
    std::vector<std::tr1::shared_ptr<class FrameGroup> > groupCache;
    class DecodedFrame singleFrame;
};


#endif // AVBINMEDIA_H