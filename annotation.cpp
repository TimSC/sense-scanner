#include "annotation.h"
#include "scenecontroller.h"

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
    if(this->track) delete this->track;
    this->track = NULL;

    QObject *par = other.track->parent();
    this->SetTrack(new SimpleSceneController(par));
    *this->track = *other.track;
}

bool Annotation::operator!= (const Annotation &other)
{
    if(source != other.source) return true;
    if(visible != other.visible) return true;
    if(uid != other.uid) return true;
    if(track != other.track) return true;
    return false;
}

void Annotation::Clear()
{
    this->SetTrack(NULL);
    this->visible = true;
    QUuid uidBlank;
    this->uid = uidBlank;
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
    return this->track;
}
