#include "workspace.h"
#include "localsleep.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <iostream>
using namespace std;

Workspace::Workspace(int activeIn, QObject *parentIn) : MessagableThread()
{
    this->parent = parentIn;
    this->active = activeIn;
    this->ClearAnnotation();
    this->ClearProcessing();
    this->eventLoop = NULL;
    this->eventReceiver = NULL;
}

Workspace::Workspace(int activeIn, QObject *parentIn, const Workspace &other) : MessagableThread()
{
    this->operator =(other);
    this->active = activeIn;
    this->parent = parentIn;
}

Workspace::~Workspace()
{

}

Workspace& Workspace::operator= (const Workspace &other)
{
    this->lock.lock();
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

    this->lock.unlock();
    return *this;
}

bool Workspace::operator!= (const Workspace &other)
{
    this->lock.lock();
    if(this->annotations.size() != other.annotations.size())
    {
        this->lock.unlock();
        return true;
    }
    for(unsigned int i=0;i<this->annotations.size();i++)
        if(*annotations[i] != *other.annotations[i])
        {
            this->lock.unlock();
            return true;
        }
    if(processingList != other.processingList)
    {
        this->lock.unlock();
        return true;
    }
    this->lock.unlock();
    return false;
}

void Workspace::SetEventLoop(class EventLoop &eventLoopIn)
{
    MessagableThread::SetEventLoop(&eventLoopIn);
    this->applyModelPool.SetEventLoop(&eventLoopIn);

    this->eventLoop->AddListener("NEW_ANNOTATION", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ANNOTATION_UUIDS", *this->eventReceiver);

    this->eventLoop->AddListener("NEW_PROCESSING", *this->eventReceiver);
    this->eventLoop->AddListener("REMOVE_PROCESSING", *this->eventReceiver);
    this->eventLoop->AddListener("GET_PROCESSING_UUIDS", *this->eventReceiver);

    this->eventLoop->AddListener("THREAD_PROGRESS_UPDATE", *this->eventReceiver);
    this->eventLoop->AddListener("THREAD_STATUS_CHANGED", *this->eventReceiver);

    this->eventLoop->AddListener("NEW_WORKSPACE", *this->eventReceiver);

}

unsigned int Workspace::AddSourceFromMain(QUuid uuid)
{
    this->lock.lock();
    unsigned int out = this->AddSource(uuid);
    this->lock.unlock();
    return out;
}

unsigned int Workspace::AddSource(QUuid uuid)
{

    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);

    TrackingAnnotationData *scenePtr = new TrackingAnnotationData();
    ann->SetTrack(scenePtr);
    ann->SetAnnotUid(uuid);

    assert(!this->mediaUuid.isNull());
    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann));
    annotThread->SetEventLoop(this->eventLoop);
    annotThread->SetThreadId(uuid);
    annotThread->Start();

    this->annotations.push_back(ann);
    this->annotationUuids.push_back(uuid);
    this->annotationThreads.push_back(annotThread);
    unsigned int out = this->annotations.size();

    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
    this->eventLoop->SendEvent(changeEv);

    return out;
}

void Workspace::RemoveSourceFromMain(QUuid uuid)
{
    this->lock.lock();
    try
    {
        this->RemoveSource(uuid);
    }
    catch(std::runtime_error err)
    {
        this->lock.unlock();
        throw std::runtime_error(err.what());
    }
    this->lock.unlock();
}

void Workspace::RemoveSource(QUuid uuid)
{

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
        throw std::runtime_error("No such uuid");
    }

    //Prepare annotation for delete
    std::tr1::shared_ptr<class Annotation> ann = this->annotations[ind];
    if(ann!=NULL) ann->PreDelete();

	//Stop thread
	this->annotationThreads[ind]->Stop();

	//Free memory
    this->annotations.erase(this->annotations.begin()+ind);
    this->annotationUuids.erase(this->annotationUuids.begin()+ind);
    this->annotationThreads.erase(this->annotationThreads.begin()+ind);

    //Remove helper thread
    this->applyModelPool.Remove(uuid);

    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
    this->eventLoop->SendEvent(changeEv);

}

int Workspace::AddHelperThreadFromMain(QUuid algUuid, QUuid annotUuid, QUuid mediaInterface)
{
    this->lock.lock();
    int ret = this->applyModelPool.Add(algUuid, annotUuid, mediaInterface);
    this->lock.unlock();
    return ret;
}

QList<QUuid> Workspace::GetAnnotationUuids()
{
    QList<QUuid> out = this->annotationUuids;
    return out;
}

//***********************************************************************

void Workspace::AddProcessingFromMain(std::tr1::shared_ptr<class AlgorithmProcess> alg)
{
    this->lock.lock();
    this->AddProcessing(alg);
    this->lock.unlock();
}

void Workspace::AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg)
{
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
        return;
    }

    //Start worker process
    alg->Start();

    this->processingList.push_back(alg);
    this->processingProgress.push_back(0.);
    this->processingUuids.push_back(alg->GetUid());

    std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
    this->eventLoop->SendEvent(changeEv);
}

int Workspace::RemoveProcessingFromMain(QUuid uuid)
{
    this->lock.lock();
    int out = this->RemoveProcessing(uuid);
    this->lock.unlock();
    return out;
}

int Workspace::RemoveProcessing(QUuid uuid)
{

    //Find index of references processing
    int ind = 0, found = 0;
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            ind = i;
            found = 1;
        }
    }
    if(!found)
    {
        return -1;
    }

    //Stop the process
    this->processingList[ind]->Stop();

	//Confirm processing has stopped
    assert(this->processingList[ind]->GetState()==AlgorithmProcess::STOPPED);

    this->processingList.erase(this->processingList.begin()+ind);
    this->processingProgress.erase(this->processingProgress.begin()+ind);
    this->processingUuids.erase(this->processingUuids.begin()+ind);

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

AlgorithmProcess::ProcessState Workspace::GetProcessingState(QUuid uuid)
{
    for(unsigned int i=0;i<this->processingUuids.size();i++)
    {
        if(uuid == this->processingUuids[i])
        {
            AlgorithmProcess::ProcessState out = this->processingList[i]->GetState();
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

void Workspace::ClearProcessingFromMain()
{
    this->lock.lock();
    this->ClearProcessing();
    this->lock.unlock();
}

void Workspace::ClearAnnotationFromMain()
{
    this->lock.lock();
    this->ClearAnnotation();
    this->lock.unlock();
}

void Workspace::ClearProcessing()
{
    for(unsigned int i=0;i<this->processingList.size();i++)
        this->processingList[i]->Stop();

    this->processingList.clear();
    this->processingProgress.clear();
    this->processingUuids.clear();

    this->applyModelPool.Clear();

    if(this->eventLoop!=NULL)
    {
        std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
        this->eventLoop->SendEvent(changeEv);
    }

}

void Workspace::ClearAnnotation()
{
    for(unsigned int i=0;i<this->annotationThreads.size();i++)
        this->annotationThreads[i]->Stop();

    this->annotations.clear();
    this->annotationThreads.clear();
    this->annotationUuids.clear();

    if(this->eventLoop!=NULL)
    {
        std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
        this->eventLoop->SendEvent(changeEv);
    }
}

void Workspace::Update()
{
    this->lock.lock();
    //Whatever
    this->lock.unlock();

    this->msleep(100);
}

void Workspace::UpdateFromMain()
{
    this->lock.lock();

    //Add processing modules, which needs to be done from the main thread
    for(unsigned int i=0;i<this->newAlgEvents.size();i++)
    {
        std::tr1::shared_ptr<class Event> ev = this->newAlgEvents[i];
        std::tr1::shared_ptr<class AlgorithmProcess> alg(
                    new class AlgorithmProcess(this->eventLoop, this->parent));
        alg->Init();
        alg->SetUid(QUuid(ev->data));
        this->AddProcessing(alg);

        std::tr1::shared_ptr<class Event> changeEv2(new Event("PROCESSING_ADDED"));
        changeEv2->id = ev->id;
        changeEv2->data = ev->data;
        this->eventLoop->SendEvent(changeEv2);
    }
    this->newAlgEvents.clear();

    this->lock.unlock();

}

void Workspace::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    this->lock.lock();
    //If this is not an active workspace, ignore most events
    if(!this->active)
    {
        MessagableThread::HandleEvent(ev);
        this->lock.unlock();
        return;
    }

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
        this->AddSource(QUuid(ev->data));

        std::tr1::shared_ptr<class Event> changeEv2(new Event("ANNOTATION_ADDED"));
        changeEv2->id = ev->id;
        changeEv2->data = ev->data;
        this->eventLoop->SendEvent(changeEv2);
    }

    if(ev->type=="NEW_PROCESSING")
    {
        this->newAlgEvents.push_back(ev);
    }

    if(ev->type=="NEW_WORKSPACE")
    {
        this->ClearAnnotation();
        this->ClearProcessing();
    }

    if(ev->type=="REMOVE_PROCESSING")
    {
        this->RemoveProcessing(QUuid(ev->data));
    }

    if(ev->type=="THREAD_PROGRESS_UPDATE")
    {
        //Check for matching annotation
        QUuid seekUuid = QUuid(ev->fromUuid);

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
                this->processingProgress[i] = ev->data.toFloat();
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }

    }

    if(ev->type=="THREAD_STATUS_CHANGED")
    {
        AlgorithmProcess::ProcessState state;
        if(ev->data=="paused") state = AlgorithmProcess::PAUSED;
        if(ev->data=="running") state = AlgorithmProcess::RUNNING;
        if(ev->data=="finished") state = AlgorithmProcess::STOPPED;

        //Check for matching annotation
        QUuid seekUuid = QUuid(ev->fromUuid);

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
                std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_PROCESSING_CHANGED"));
                this->eventLoop->SendEvent(changeEv);
            }
        }

    }
    this->lock.unlock();
}

void Workspace::TerminateThreads()
{
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        std::tr1::shared_ptr<class Annotation> ann = this->annotations[i];
        ann->Terminate();
    }
}

QList<QUuid> Workspace::GetProcessingUuids()
{
    QList<QUuid> out = this->processingUuids;
    return out;
}

QList<QUuid> Workspace::GetProcessingUuidsFromMain()
{
    this->lock.lock();
    QList<QUuid> out = this->processingUuids;
    this->lock.unlock();
    return out;
}

QList<QUuid> Workspace::GetAnnotationUuidsFromMain()
{
    this->lock.lock();
    QList<QUuid> out = this->annotationUuids;
    this->lock.unlock();
    return out;
}

void Workspace::SetMediaUuid(QUuid mediaUuidIn)
{
    this->mediaUuid = mediaUuidIn;
}

void Workspace::AddProcessing(QUuid uid,
                              class EventLoop *eventLoop,
                              class EventReceiver *eventReceiver)
{
    //Create processing module
    std::tr1::shared_ptr<class Event> newAnnEv(new Event("NEW_PROCESSING"));
    QString dataStr = QString("%1").arg(uid.toString());
    newAnnEv->data = dataStr.toLocal8Bit().constData();
    newAnnEv->id = eventLoop->GetId();
    eventLoop->SendEvent(newAnnEv);

    //Wait for workspace to register this annotation
    std::tr1::shared_ptr<class Event> response = eventReceiver->WaitForEventId(newAnnEv->id);
    assert(response->type == "PROCESSING_ADDED");
}

void Workspace::RemoveProcessing(QUuid uid,
                              class EventLoop *eventLoop,
                              class EventReceiver *eventReceiver)
{
    //Create processing module
    std::tr1::shared_ptr<class Event> removeEv(new Event("REMOVE_PROCESSING"));
    removeEv->data = uid.toString();
    removeEv->id = eventLoop->GetId();
    eventLoop->SendEvent(removeEv);
}

int Workspace::IsReadyForSave()
{
    this->lock.lock();
    //Check process is ready to be removed
    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        AlgorithmProcess::ProcessState state = this->processingList[i]->GetState();
        if(state!=AlgorithmProcess::STOPPED &&
                state!=AlgorithmProcess::PAUSED &&
                state!=AlgorithmProcess::READY)
        {
            cout << "Process cannot be saved while it is running" << endl;
            this->lock.unlock();
            return 0;
        }
    }
    this->lock.unlock();
    return 1;
}
