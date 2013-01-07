#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include "eventloop.h"

class AnnotThread : public MessagableThread
{
public:
    AnnotThread(class Annotation *annIn);
    virtual ~AnnotThread();

    void Update();

protected:
    class Annotation *parentAnn;
    int srcDurationSet;
    long long unsigned srcDuration;
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

    bool visible;
    QUuid uid;
    std::tr1::shared_ptr<class AnnotThread> annotThread;

protected:
    QMutex lock;
    class SimpleSceneController * track;
    QUuid algUid;
    QString source;
};


#endif // ANNOTATION_H
