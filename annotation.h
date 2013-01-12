#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QImage>
#include "eventloop.h"

class AnnotThread : public MessagableThread
{
    /*!
    * AnnotThread is a worker thread that allows annotations to create events
    * and continue processing without interupting the GUI thread. It also
    * manages the current tracker positions and frame iteration during tracking
    * of new videos.
    */

public:
    AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterface);
    virtual ~AnnotThread();

    void SetEventLoop(class EventLoop *eventLoopIn);
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void Update();
    void Finished();

protected:

    void ImageToProcess(unsigned long long startTi,
                        unsigned long long endTi,
                        QSharedPointer<QImage> img,
                        std::vector<std::vector<float> > &model);

    class Annotation *parentAnn;
    int srcDurationSet;
    long long unsigned srcDuration;
    class AvBinMedia* mediaInterface;

    unsigned long long currentStartTimestamp, currentEndTimestamp;
    int currentTimeSet;
    std::vector<std::vector<float> > currentModel;
    int currentModelSet;
};

class Annotation
{
    /*!
    * Annotation contains various shared state information between
    * the AnnotThread and the GUI. Some of the methods are thread safe
    * to enable iteraction between the two.
    */

public:
    Annotation();
    virtual ~Annotation();
    Annotation& operator= (const Annotation &other);
    bool operator!= (const Annotation &other);
    void Clear();
    void SetTrack(class TrackingAnnotation *trackIn);
    void CloneTrack(class TrackingAnnotation *trackIn);
    class TrackingAnnotation *GetTrack();

    void SetAlgUid(QUuid uidIn); //Thread safe
    QUuid GetAlgUid(); //Thread safe

    void SetSource(QString uidIn); //Thread safe
    QString GetSource(); //Thread safe

    void SetActive(int activeIn);
    int GetActive();

    void SetActiveStateDesired(int desiredIn);
    int GetActiveStateDesired();

    void Terminate();

    bool visible;
    QUuid uid;
    std::tr1::shared_ptr<class AnnotThread> annotThread;

protected:
    QMutex lock;
    class TrackingAnnotation * track;
    QUuid algUid;
    QString source;
    int active;
    int activeStateDesired;
};


#endif // ANNOTATION_H
