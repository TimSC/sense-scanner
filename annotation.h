#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include "eventloop.h"

class AnnotThread : public MessagableThread
{
public:
    AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterface);
    virtual ~AnnotThread();

    void Update();
    void Finished();

protected:
    class Annotation *parentAnn;
    int srcDurationSet;
    long long unsigned srcDuration;
    class AvBinMedia* mediaInterface;

    unsigned long long currentStartTimestamp, currentEndTimestamp;
    int currentTimeSet;

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
