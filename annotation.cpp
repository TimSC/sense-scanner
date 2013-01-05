#include "annotation.h"
#include "scenecontroller.h"
#include <assert.h>
#include <iostream>
using namespace std;

AnnotThread::AnnotThread(class Annotation *annIn)
{
    this->parentAnn = annIn;
}

AnnotThread::~AnnotThread()
{

}

void AnnotThread::Update()
{
    //cout << "x" << (unsigned long)this << endl;
    assert(this->parentAnn != NULL);
    QUuid algUid = this->parentAnn->GetAlgUid();
    if(!algUid.isNull())
        cout << algUid.toString().toLocal8Bit().constData() << endl;

    class SimpleSceneController *track = this->parentAnn->GetTrack();


    this->msleep(100);
}

//*****************************************************

Annotation::Annotation()
{
    this->track = NULL;
    this->visible = true;
}

Annotation::~Annotation()
{
    this->SetTrack(NULL);
}

Annotation& Annotation::operator= (const Annotation &other)
{
    source = other.source;
    uid = other.uid;
    visible = other.visible;
    algUid = other.algUid;
    if(this->track) delete this->track;
    this->track = NULL;

    QObject *par = other.track->parent();
    this->SetTrack(new SimpleSceneController(par));
    *this->track = *other.track;
    return *this;
}

bool Annotation::operator!= (const Annotation &other)
{
    if(source != other.source) return true;
    if(visible != other.visible) return true;
    if(uid != other.uid) return true;
    if(track != other.track) return true;
    if(algUid != other.algUid) return true;
    return false;
}

void Annotation::Clear()
{
    this->SetTrack(NULL);
    this->visible = true;
    QUuid uidBlank;
    this->uid = uidBlank;
    this->algUid = algUid;
    this->source = "";
    std::tr1::shared_ptr<class AnnotThread> thd;
    this->annotThread = thd;
}

void Annotation::SetTrack(class SimpleSceneController *trackIn)
{
    if(this->track != NULL) delete this->track;
    this->track = trackIn;
}

void Annotation::CloneTrack(class SimpleSceneController *trackIn)
{
    this->SetTrack(NULL);
    this->track = new class SimpleSceneController(trackIn->parent());
    *this->track = *trackIn;
}

class SimpleSceneController *Annotation::GetTrack()
{
    this->lock.lock();
    class SimpleSceneController *out = this->track;
    this->lock.unlock();
    return out;
}

void Annotation::SetAlgUid(QUuid uidIn)
{
    this->lock.lock();
    this->algUid = uidIn;
    this->lock.unlock();
}

QUuid Annotation::GetAlgUid()
{
    this->lock.lock();
    QUuid out = this->algUid;
    this->lock.unlock();
    return out;
}

void Annotation::SetSource(QString sourceIn)
{
    this->lock.lock();
    this->source = sourceIn;
    this->lock.unlock();
}

QString Annotation::GetSource()
{
    this->lock.lock();
    QString out = this->source;
    this->lock.unlock();
    return out;
}
