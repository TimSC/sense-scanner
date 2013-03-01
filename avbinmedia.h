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

class AvBinMedia
{
    /*!
    * A high level interface to retrieve video frames. The decoding is
    * performed by AvBinThread and AvBinBackend.
    */

public:
    explicit AvBinMedia(class EventLoop *eventLoopIn, int selfTimerIn);
    virtual ~AvBinMedia();

    void TerminateThread();

    long long unsigned RequestFrame(QString source, long long unsigned ti);
    void Update();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
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
