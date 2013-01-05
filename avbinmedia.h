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

class AvBinMedia : public AbstractMedia
{
public:
    explicit AvBinMedia();
    virtual ~AvBinMedia();

    virtual QSharedPointer<QImage> Get(long long unsigned ti,
                                       long long unsigned &outFrameStart,
                                       long long unsigned &outFrameEnd,
                                       long long unsigned timeout = 5000); //in milliseconds
    virtual long long unsigned GetNumFrames();
    virtual long long unsigned Length(); //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds
    void SetEventLoop(class EventLoop *eventLoopIn);
    int OpenFile(QString fina);
    void SetActive(int activeIn);
    void SetId(int idIn);

    int RequestFrame(long long unsigned ti);
    void Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                      unsigned long long endTimestamp,
                                      unsigned long long requestedTimestamp,
                                      void *raw), void *raw);

protected:
    class EventReceiver *eventReceiver;
    class EventLoop *eventLoop;
    int active;
    int id;
};

class AvBinThread : public MessagableThread
{
public:
    AvBinThread();
    virtual ~AvBinThread();
    void SetEventLoop(class EventLoop *eventLoopIn);
    void SetId(int idIn);

    void Update();
protected:
    class AvBinBackend avBinBackend;
};

void ChangeVidSource(AvBinThread **mediaThread,
                     AvBinMedia *mediaInterface,
                     class EventLoop *eventLoop,
                     QString fina);

#endif // AVBINMEDIA_H
