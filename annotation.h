#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include "eventloop.h"

class AnnotThread : public MessagableThread
{
public:
    AnnotThread() {};
    virtual ~AnnotThread() {};

    void Update() {};

protected:


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
    class SimpleSceneController *GetTrack();

    bool visible;
    QUuid uid;
    QString source;
    std::tr1::shared_ptr<class AnnotThread> annotThread;

protected:
    class SimpleSceneController * track;

};


#endif // ANNOTATION_H
