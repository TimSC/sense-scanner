#include "workspace.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <iostream>
using namespace std;

Workspace::Workspace() : QObject()
{
    this->ClearAnnotation();
    this->ClearProcessing();
    this->eventLoop = NULL;
    this->eventReceiver = NULL;
}

Workspace::Workspace(const Workspace &other) : QObject()
{
    this->operator =(other);
}

Workspace::~Workspace()
{
    if(this->eventReceiver!=NULL) delete this->eventReceiver;
    this->eventReceiver = NULL;
}

Workspace& Workspace::operator= (const Workspace &other)
{
    this->annotations.clear();
    for(unsigned int i=0;i<other.annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
        *ann = *other.annotations[i];
        this->annotations.push_back(ann);
    }

    this->processingList = other.processingList;
    this->annotationUuids = other.annotationUuids;
    this->processingUuids = other.processingUuids;

    return *this;
}

bool Workspace::operator!= (const Workspace &other)
{
    if(this->annotations.size() != other.annotations.size()) return true;
    for(unsigned int i=0;i<this->annotations.size();i++)
        if(*annotations[i] != *other.annotations[i]) return true;
    if(processingList != other.processingList) return true;
    return false;
}

void Workspace::SetEventLoop(class EventLoop &eventLoopIn)
{
    this->eventLoop = &eventLoopIn;
    if(this->eventReceiver!=NULL) delete this->eventReceiver;
    this->eventReceiver = new EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("NEW_ANNOTATION", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ANNOTATION_UUIDS", *this->eventReceiver);
    this->eventLoop->AddListener("GET_PROCESSING_UUIDS", *this->eventReceiver);

    this->eventLoop->AddListener("THREAD_PROGRESS_UPDATE", *this->eventReceiver);
    this->eventLoop->AddListener("THREAD_STATUS_CHANGED", *this->eventReceiver);
}

unsigned int Workspace::AddSource(QUuid uuid)
{
    this->lock.lock();
    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
    TrackingAnnotationData *scenePtr = new TrackingAnnotationData();
    ann->SetTrack(scenePtr);

    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, this->mediaUuid));
    annotThread->SetEventLoop(this->eventLoop);
    annotThread->Start();
    ann->SetAnnotUid(uuid);

    this->annotations.push_back(ann);
    this->annotationUuids.push_back(uuid);
    this->annotationThreads.push_back(annotThread);
    unsigned int out = this->annotations.size();

    this->lock.unlock();
    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
    this->eventLoop->SendEvent(changeEv);

    return out;
}

void Workspace::RemoveSource(QUuid uuid)
{
    this->lock.lock();
    //Find index of uuid
    int ind = -1;
    for(unsigned int i=0;i<this->annotationUuids.size();i++)
    {
        if(this->annotationUuids[i] == uuid)
        {
            ind = i;
            break;
        }
    }
    if (ind == -1)
    {
        this->lock.unlock();
        throw std::runtime_error("No such uuid");
    }


    //Prepare annotation for delete
    std::tr1::shared_ptr<class Annotation> ann = this->annotations[ind];
    if(ann!=NULL) ann->PreDelete();

    this->annotations.erase(this->annotations.begin()+ind);
    this->annotationUuids.erase(this->annotationUuids.begin()+ind);
    this->annotationThreads.erase(this->annotationThreads.begin()+ind);
    this->lock.unlock();

    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
    this->eventLoop->SendEvent(changeEv);

}

unsigned int Workspace::AddAutoAnnot(QUuid annotUid, QUuid algUid, class AvBinMedia* mediaInterface)
{
    QUuid uuid = QUuid::createUuid();
    this->AddSource(uuid);

    //TODO copy source filename and annotated frames to destination annotation
    assert(0);
}

QList<QUuid> Workspace::GetAnnotationUuids()
{
    this->lock.lock();
    QList<QUuid> out = this->annotationUuids;
    this->lock.unlock();
    return out;
}

//***********************************************************************

void Workspace::AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg)
{
    this->lock.lock();
    try
    {
        alg->Init();
    }
    catch(std::runtime_error &err)
    {
        //If python/executable is not found, an error is thrown to be caught here
        QErrorMessage *errPopUp = new QErrorMessage();
        errPopUp->showMessage(err.what());
        errPopUp->exec();
        delete errPopUp;
        this->lock.unlock();
        return;
    }

    //Start worker process
    alg->Start();

    this->processingList.push_back(alg);
    this->processingProgress.push_back(0.);
    this->processingUuids.push_back(alg->GetUid());
    this->processingStates.push_back(AlgorithmProcess::STARTING);

    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
    this->eventLoop->SendEvent(changeEv);
    this->lock.unlock();
}

int Workspace::RemoveProcessing(QUuid uuid)
{
    this->lock.lock();
    int ind = 0, found = 0;
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            ind = i;
            found = 0;
        }
    }
    if(!found) return -1;

    //Check process is ready to be removed
    AlgorithmProcess::ProcessState state = this->processingStates[ind];
    if(state!=AlgorithmProcess::STOPPED &&
            state!=AlgorithmProcess::PAUSED)
    {
        cout << "Process cannot be removed while it is running" << endl;
        return 0;
    }

    //If paused, stop the process
    if(this->processingStates[ind]!=AlgorithmProcess::PAUSED)
        this->processingList[ind]->Stop();

    assert(this->processingStates[ind]==AlgorithmProcess::STOPPED);
    this->processingList.erase(this->processingList.begin()+ind);
    this->processingProgress.erase(this->processingProgress.begin()+ind);
    this->processingUuids.erase(this->processingUuids.begin()+ind);
    this->processingStates.erase(this->processingStates.begin()+ind);

    this->lock.unlock();
    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
    this->eventLoop->SendEvent(changeEv);
    return 1;
}

void Workspace::ProcessingProgressChanged(QUuid uuid, float progress)
{
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            this->processingProgress[i] = progress;
            return;
        }
    }
    throw std::runtime_error ("Uuid not found");
}

float Workspace::GetProcessingProgress(QUuid uuid)
{
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            return this->processingProgress[i];
        }
    }
    throw std::runtime_error ("Uuid not found");
}

void Workspace::ProcessingStateChanged(QUuid uuid, AlgorithmProcess::ProcessState state)
{
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            this->processingStates[i] = state;
            return;
        }
    }
    throw std::runtime_error ("Uuid not found");
}

AlgorithmProcess::ProcessState Workspace::GetProcessingState(QUuid uuid)
{
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            this->lock.lock();
            AlgorithmProcess::ProcessState out = this->processingStates[i];
            this->lock.unlock();
            return out;
        }
    }
    throw std::runtime_error ("Uuid not found");
}

int Workspace::NumProcessesBlockingShutdown()
{
    int count = 0;
    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        int bl = this->processingList[i]->IsBlockingShutdown();
        count += bl;
    }
    return count;
}

//************************************************************************

void Workspace::ClearProcessing()
{
    this->lock.lock();
    this->processingList.clear();
    this->processingProgress.clear();
    this->processingUuids.clear();
    this->processingStates.clear();
    this->lock.unlock();
}

void Workspace::ClearAnnotation()
{
    this->lock.lock();
    this->annotations.clear();
    this->annotationThreads.clear();
    this->annotationUuids.clear();
    this->lock.unlock();
}

void Workspace::Update()
{

    //Process events from application
    int flushEvents = 1;
    while(flushEvents)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();

        this->HandleEvent(ev);
    }
    catch(std::runtime_error e)
    {
        flushEvents = 0;
    }

    //Set annotations to inactive when they do not need active status
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        if(!ann->GetActiveStateDesired() && ann->GetActive())
            ann->SetActive(0);
    }

    //Check which annotation track is active (in terms of accessing the video interface)
    int count = 0;
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        count += ann->GetActive();
    }

    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        if(ann->GetActiveStateDesired() && count == 0)
        {
            ann->SetActive(1);
            count ++;
        }
    }
}

void Workspace::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type=="GET_ANNOTATION_UUIDS")
    {
        QList<QUuid> uuids = this->GetAnnotationUuids();
        QString dataStr;
        for(unsigned int i=0;i<uuids.size();i++)
        {
            if(i>0) dataStr.append(",");
            dataStr.append(uuids[i].toString());
        }

        std::tr1::shared_ptr<class Event> respEv(new Event("ANNOTATION_UUIDS"));
        respEv->data = dataStr.toLocal8Bit().constData();
        respEv->id = ev->id;
        this->eventLoop->SendEvent(respEv);
    }

    if(ev->type=="GET_PROCESSING_UUIDS")
    {
        QList<QUuid> uuids = this->GetProcessingUuids();
        QString dataStr;
        for(unsigned int i=0;i<uuids.size();i++)
        {
            if(i>0) dataStr.append(",");
            dataStr.append(uuids[i].toString());
        }

        std::tr1::shared_ptr<class Event> respEv(new Event("PROCESSING_UUIDS"));
        respEv->data = dataStr.toLocal8Bit().constData();
        respEv->id = ev->id;
        this->eventLoop->SendEvent(respEv);
    }

    if(ev->type=="NEW_ANNOTATION")
    {
        this->AddSource(QUuid(ev->data.c_str()));

        std::tr1::shared_ptr<class Event> changeEv2(new Event("ANNOTATION_ADDED"));
        changeEv2->id = ev->id;
        changeEv2->data = ev->data;
        this->eventLoop->SendEvent(changeEv2);
    }

    if(ev->type=="THREAD_PROGRESS_UPDATE")
    {
        //Check for matching annotation
        QUuid seekUuid = QUuid(ev->fromUuid);
        this->lock.lock();
        for(unsigned int i = 0; i < this->annotationUuids.size(); i++)
        {
            if(seekUuid == this->annotationUuids[i])
            {
                //TODO
                //this->annotationProgress[i] = atof(ev->data.c_str());
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }

        //Check for matching processing algorithm
        for(unsigned int i = 0; i < this->processingUuids.size(); i++)
        {
            if(seekUuid == this->processingUuids[i])
            {
                this->processingProgress[i] = atof(ev->data.c_str());
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }
        this->lock.unlock();
    }

    if(ev->type=="THREAD_STATUS_CHANGED")
    {
        AlgorithmProcess::ProcessState state;
        if(ev->data=="paused") state = AlgorithmProcess::PAUSED;
        if(ev->data=="running") state = AlgorithmProcess::RUNNING;
        if(ev->data=="finished") state = AlgorithmProcess::STOPPED;

        //Check for matching annotation
        QUuid seekUuid = QUuid(ev->fromUuid);
        this->lock.lock();
        for(unsigned int i = 0; i < this->annotationUuids.size(); i++)
        {
            if(seekUuid == this->annotationUuids[i])
            {
                //TODO
                //this->annotationStates[i] = state;
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }

        //Check for matching processing algorithm
        for(unsigned int i = 0; i < this->processingUuids.size(); i++)
        {
            if(seekUuid == this->processingUuids[i])
            {
                this->processingStates[i] = state;
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }
        this->lock.unlock();


    }

}

void Workspace::TerminateThreads()
{
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        ann->Terminate();
    }
}

void Workspace::SetAnnotThreadsInactive()
{
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        ann->SetActive(0);
    }
}

QList<QUuid> Workspace::GetProcessingUuids()
{
    this->lock.lock();
    QList<QUuid> out = this->processingUuids;
    this->lock.unlock();
    return out;
}

void Workspace::SetMediaUuid(QUuid mediaUuidIn)
{
    this->mediaUuid = mediaUuid;

}
