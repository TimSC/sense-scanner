#include "workspace.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
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
    this->defaultFilename = other.defaultFilename;
    this->processingList = other.processingList;

    return *this;
}

bool Workspace::operator!= (const Workspace &other)
{
    if(defaultFilename != other.defaultFilename) return true;
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

unsigned int Workspace::AddSource(QString &fina, QString UidStr, class AvBinMedia* mediaInterface)
{
    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
    ann->SetSource(fina);
    TrackingAnnotation *scenePtr = new TrackingAnnotation(0);
    ann->SetTrack(scenePtr);
    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, mediaInterface));
    annotThread->SetEventLoop(this->eventLoop);
    annotThread->Start();
    ann->annotThread = annotThread;
    this->annotations.push_back(ann);
    return this->annotations.size();
}

void Workspace::RemoveSource(unsigned int num)
{
    assert(num < this->annotations.size());

    //Prepare annotation for delete
    std::tr1::shared_ptr<class Annotation> ann = this->annotations[num];
    if(ann!=NULL) ann->PreDelete();

    this->annotations.erase(this->annotations.begin()+num);
}

unsigned int Workspace::AddAutoAnnot(QString annotUid, QString algUid, class AvBinMedia* mediaInterface)
{
    QUuid annotUidObj(annotUid);
    int basedOnAnnot = FindAnnotWithUid(annotUidObj);
    assert(basedOnAnnot>=0 && basedOnAnnot < this->GetNumSources());

    class Annotation &parent = *this->annotations[basedOnAnnot];

    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
    ann->SetSource(parent.GetSource());
    ann->SetAnnotUid(QUuid::createUuid());
    ann->CloneTrack(parent.GetTrack());
    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, mediaInterface));
    annotThread->SetEventLoop(this->eventLoop);
    annotThread->Start();
    ann->annotThread = annotThread;
    ann->SetAlgUid(algUid);

    this->annotations.push_back(ann);
    return this->annotations.size();
}

TrackingAnnotation *Workspace::GetTrack(unsigned int trackNum)
{
    return this->annotations[trackNum]->GetTrack();
}

unsigned int Workspace::GetNumSources()
{
    return this->annotations.size();
}

QString Workspace::GetSourceName(unsigned int index)
{
    assert(index < this->annotations.size());
    return this->annotations[index]->GetSource();
}

QUuid Workspace::GetAnnotUid(unsigned int index)
{
    assert(index < this->annotations.size());
    return this->annotations[index]->GetAnnotUid();
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

int Workspace::FindAnnotWithUid(QUuid uidIn)
{
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        if(this->annotations[i]->GetAnnotUid() == uidIn)
            return i;
    }
    return -1;
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

    this->defaultFilename = "";
    this->processingList.clear();
    this->processingProgress.clear();
    this->processingUuids.clear();
    this->processingStates.clear();
}

void Workspace::ClearAnnotation()
{
    this->annotations.clear();

}

void Workspace::Load(QString fina, class AvBinMedia* mediaInterface)
{
    this->ClearAnnotation();
    this->ClearProcessing();
    this->defaultFilename = fina;

    //Parse XML to DOM
    QFile f(fina);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        return;
    }
    f.close();

    //Get dir of data file
    QFileInfo pathInfo(fina);
    QDir dir(pathInfo.absoluteDir());

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    QDomNode n = rootElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {

            if(e.tagName() == "sources")
            {
                QDomNode sourceNode = e.firstChild();
                while(!sourceNode.isNull())
                {
                    QDomElement sourceEle = sourceNode.toElement(); // try to convert the node to an element.
                    if(sourceEle.tagName() != "source") {sourceNode = sourceNode.nextSibling(); continue;}

                    QString sourceFiNa = sourceEle.attribute("file");
                    QString sourceFiNaAbs = dir.absoluteFilePath(sourceFiNa);
                    QFileInfo fileInfo(sourceFiNaAbs);
                    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
                    ann->SetSource(fileInfo.absoluteFilePath());

                    //Set source UID
                    QString uidStr = sourceEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();
                    ann->SetAnnotUid(uid);

                    //Set alg Uid
                    QString algStr = sourceEle.attribute("alg");
                    QUuid alg(algStr);
                    ann->SetAlgUid(alg);

                    //Start annot worker thread
                    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, mediaInterface));
                    annotThread->SetEventLoop(this->eventLoop);
                    annotThread->Start();
                    ann->annotThread = annotThread;

                    TrackingAnnotation *track =
                            new TrackingAnnotation(NULL);

                    QDomNode trackData = sourceNode.firstChild();
                    while(!trackData.isNull())
                    {
                        QDomElement et = trackData.toElement(); // try to convert the node to an element.
                        if(et.isNull()) continue;
                        if(et.tagName() != "tracking") {trackData = trackData.nextSibling(); continue;}

                        track->ReadAnnotationXml(et);

                        trackData = trackData.nextSibling();

                    }


                    ann->SetTrack(track);
                    this->annotations.push_back(ann);
                    sourceNode = sourceNode.nextSibling();
                }

            }

            if(e.tagName() == "models")
            {
                QDomNode modelNode = e.firstChild();
                while(!modelNode.isNull())
                {
                    QDomElement modelEle = modelNode.toElement(); // try to convert the node to an element.
                    if(modelEle.tagName() != "model") {modelNode = modelNode.nextSibling(); continue;}

                    QByteArray modelData = QByteArray::fromBase64(modelEle.text().toLocal8Bit().constData());
                    std::tr1::shared_ptr<class AlgorithmProcess> alg(
                                new class AlgorithmProcess(this->eventLoop, this));
                    alg->Init();

                    QString uidStr = modelEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();
                    alg->SetUid(uid);
                    this->AddProcessing(alg);

                    //Send data to algorithm process
                    std::tr1::shared_ptr<class Event> foundModelEv(new Event("ALG_MODEL_FOUND"));
                    QString imgPreamble2 = QString("MODEL\n");
                    foundModelEv->data = imgPreamble2.toLocal8Bit().constData();
                    class BinaryData *modelRaw = new BinaryData();
                    modelRaw->Copy((const unsigned char *)modelData.constData(), modelData.size());
                    foundModelEv->raw = modelRaw;
                    foundModelEv->toUuid = uid;
                    this->eventLoop->SendEvent(foundModelEv);

                    //Continue to train if needed
                    std::tr1::shared_ptr<class Event> trainingFinishEv(new Event("TRAINING_DATA_FINISH"));
                    trainingFinishEv->toUuid = uid;
                    this->eventLoop->SendEvent(trainingFinishEv);

                    //Ask process to provide progress update
                    std::tr1::shared_ptr<class Event> getProgressEv(new Event("GET_PROGRESS"));
                    getProgressEv->toUuid = uid;
                    this->eventLoop->SendEvent(getProgressEv);

                    modelNode = modelNode.nextSibling();
                }

            }
        }
        n = n.nextSibling();
    }
}

int Workspace::Save()
{
    this->lock.lock();
    int finaLen = this->defaultFilename.length();
    if(finaLen==0)
    {
        this->lock.unlock();
        return 0;
    }

    QString tmpFina = this->defaultFilename;
    tmpFina.append(".tmp");
    QFileInfo pathInfo(this->defaultFilename);
    QDir dir(pathInfo.absoluteDir());

    //Save data to file
    QFile f(tmpFina);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<workspace>" << endl;
    out << "<sources>" << endl;
    for(unsigned int i=0;i<this->annotations.size();i++)
    {
        try
        {
            out << "\t<source id=\""<<i<<"\" uid=\""<<Qt::escape(this->annotations[i]->GetAnnotUid())<<"\" file=\""<<
                   Qt::escape(dir.relativeFilePath(this->annotations[i]->GetSource()))<<"\"";
            QUuid uid = this->annotations[i]->GetAlgUid();
            if(!uid.isNull())
                out << " alg=\"" << uid.toString().toLocal8Bit().constData() << "\"";;

            out << ">" << endl;
            this->annotations[i]->GetTrack()->WriteAnnotationXml(out);
            out << "\t</source>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
    }


    out << "</sources>" << endl;
    out << "<models>" << endl;
    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        try
        {
            std::tr1::shared_ptr<class AlgorithmProcess> alg = this->processingList[i];
            QByteArray model = alg->GetModel();
            QByteArray modelBase64 = model.toBase64();
            out << "<model uid=\""<< alg->GetUid() <<"\">" << endl;
            for(unsigned int pos=0;pos<modelBase64.length();pos+=512)
            {
                out << modelBase64.mid(pos, 512) << endl;
            }
            out << "</model>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
	}
    out << "</models>" << endl;
    out << "</workspace>" << endl;
    f.close();

    //Rename temporary file to final name
    QFile targetFina(this->defaultFilename);
    if(targetFina.exists())
        QFile::remove(this->defaultFilename);
    QFile::rename(tmpFina, this->defaultFilename);
    this->lock.unlock();

    return 1;
}

void Workspace::SaveAs(QString &fina)
{
    this->lock.lock();
    this->defaultFilename = fina;
    this->lock.unlock();
    this->Save();
}

void Workspace::Update()
{

    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        this->processingList[i]->Update();
    }

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
