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

//*************************************************

class AvBinMediaTimer : public QObject
{
    Q_OBJECT
public:
    explicit AvBinMediaTimer(class AvBinMedia *in);
    virtual ~AvBinMediaTimer();

public slots:
    void Update();

protected:
    QTimer *timer;
    class AvBinMedia *parent;
};

class AvBinMedia : public AbstractMedia
{
    /*!
    * A high level interface to retrieve video frames. The decoding is
    * performed by AvBinThread and AvBinBackend.
    */

public:
    explicit AvBinMedia(class EventLoop *eventLoopIn, int selfTimerIn);
    virtual ~AvBinMedia();

    virtual QSharedPointer<QImage> Get(QString source,
                                       long long unsigned ti,
                                       long long unsigned &outFrameStart,
                                       long long unsigned &outFrameEnd,
                                       long long unsigned timeout = 5000); //in milliseconds

    virtual long long unsigned Length(QString source); //Get length (ms)
    virtual long long unsigned GetFrameStartTime(QString source, long long unsigned ti); //in milliseconds
    void TerminateThread();

    long long unsigned RequestFrame(QString source, long long unsigned ti);
    void Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                      unsigned long long endTimestamp,
                                      unsigned long long requestedTimestamp,
                                      void *raw), void *raw);
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev,
                             void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                                                           unsigned long long endTimestamp,
                                                                           unsigned long long requestTimestamp,
                                                                           void *raw), void *raw);
    QUuid GetUuid();

protected:
    int OpenFile(QString fina);
    void ChangeVidSource(QString fina);

    class EventReceiver *eventReceiver;
    class EventLoop *eventLoop;
    AvBinThread *mediaThread;
    QString currentFina;
    QUuid uuid;
    class AvBinMediaTimer *timer;

};

#endif // AVBINMEDIA_H
