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

    void TerminateThread();

    long long unsigned RequestFrame(QString source, long long unsigned ti);
    void Update();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    QUuid GetUuid();

protected:
    int OpenFile(QString fina);
    void ChangeVidSource(QString fina);

    AvBinThread *mediaThread;
    QString currentFina;
    QUuid uuid;
    bool removeOldRequests;

};

#endif // AVBINMEDIA_H
