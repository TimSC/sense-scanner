#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QImage>
#include "eventloop.h"

class AnnotThread : public MessagableThread
{
public:
    AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterface);
    virtual ~AnnotThread();

    void SetEventLoop(class EventLoop *eventLoopIn);
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void Update();
    void Finished();

protected:

    void ImageToProcess(QSharedPointer<QImage> img,
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
public:
    Annotation();
    virtual ~Annotation();
    Annotation& operator= (const Annotation &other);
    bool operator!= (const Annotation &other);
    void Clear();
    void SetTrack(class SimpleSceneController *trackIn);
    void CloneTrack(class SimpleSceneController *trackIn);
    class SimpleSceneController *GetTrack();

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
    class SimpleSceneController * track;
    QUuid algUid;
    QString source;
    int active;
    int activeStateDesired;
};


#endif // ANNOTATION_H
