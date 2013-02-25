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
}

Workspace::Workspace(const Workspace &other) : QObject()
{
    this->operator =(other);
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
}

unsigned int Workspace::AddSource(QString &fina, QUuid uuid, class AvBinMedia* mediaInterface)
{
    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
    ann->SetSource(fina);
    TrackingAnnotationData *scenePtr = new TrackingAnnotationData();
    ann->SetTrack(scenePtr);
    return this->AddSource(ann, mediaInterface, uuid);
}

unsigned int Workspace::AddSource(std::tr1::shared_ptr<class Annotation> ann,
                                  class AvBinMedia* mediaInterface,
                                  QUuid uuid)
{
    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, mediaInterface));
    annotThread->SetEventLoop(this->eventLoop);
    annotThread->Start();
    ann->SetAnnotUid(uuid);

    this->annotations.push_back(ann);
    this->annotationUuids.push_back(uuid);
    this->annotationThreads.push_back(annotThread);
    return this->annotations.size();
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
    if (ind == -1) throw std::runtime_error("No such uuid");

    //Prepare annotation for delete
    std::tr1::shared_ptr<class Annotation> ann = this->annotations[ind];
    if(ann!=NULL) ann->PreDelete();

    this->annotations.erase(this->annotations.begin()+ind);
    this->annotationUuids.erase(this->annotationUuids.begin()+ind);
    this->annotationThreads.erase(this->annotationThreads.begin()+ind);
}

unsigned int Workspace::AddAutoAnnot(QUuid annotUid, QUuid algUid, class AvBinMedia* mediaInterface)
{
    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);

    //TODO copy source filename and annotated frames to destination annotation
    assert(0);

    this->AddSource(ann, mediaInterface, QUuid::createUuid());
}

QList<QUuid> Workspace::GetAnnotationUuids()
{
    return this->annotationUuids;

}

//***********************************************************************

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
    this->processingStates.push_back(AlgorithmProcess::STARTING);
}

int Workspace::RemoveProcessing(QUuid uuid)
{
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
    return 1;
}

/*int Workspace::StartProcessing(unsigned int num)
{
    assert(num < this->processingList.size());
    if(this->processingList[num]->GetState() == AlgorithmProcess::PAUSED)
    {
        this->processingList[num]->Unpause();
        return 1;
    }
    return this->processingList[num]->Start();
}*/

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
            return this->processingStates[i];
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
    this->processingList.clear();
    this->processingProgress.clear();
    this->processingUuids.clear();
    this->processingStates.clear();
}

void Workspace::ClearAnnotation()
{
    this->annotations.clear();

}



void Workspace::Update()
{

    //This tries to update but if mutex is locked, it returns immediately
    bool havelock = this->lock.try_lock();
    if(!havelock) return;

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
    return this->processingUuids;
}
