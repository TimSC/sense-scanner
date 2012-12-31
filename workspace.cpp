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
    this->Clear();
    nextThreadId = 1;
    this->eventLoop = NULL;
}

Workspace::Workspace(const Workspace &other) : QObject()
{
    this->operator =(other);
}



Workspace& Workspace::operator= (const Workspace &other)
{
    sources = other.sources;
    annotUids = other.annotUids;
    defaultFilename = other.defaultFilename;
    visible = other.visible;
    processingList = other.processingList;
    tracks.clear();
    for(unsigned int i=0;i<other.tracks.size();i++)
    {
        QObject *par = other.tracks[i]->parent();
        SimpleSceneController *tr = new SimpleSceneController(par);
        *tr = *other.tracks[i];
        tracks.push_back(tr);
    }
    return *this;
}

bool Workspace::operator!= (const Workspace &other)
{
    if(defaultFilename != other.defaultFilename) return true;
    if(tracks.size() != other.tracks.size()) return true;
    if(sources != other.sources) return true;
    if(visible != other.visible) return true;
    if(processingList != other.processingList) return true;
    if(annotUids != other.annotUids) return true;
    for(unsigned int i=0;i<other.tracks.size();i++)
    {
        if(*tracks[i] != *other.tracks[i]) return true;
    }
    return false;
}

void Workspace::SetEventLoop(class EventLoop &eventLoopIn)
{
    this->eventLoop = &eventLoopIn;
}

unsigned int Workspace::AddSource(QString &fina, QString UidStr)
{
    this->sources.push_back(fina);
    std::tr1::shared_ptr<class AnnotThread> annotThread;
    this->annotThreads.push_back(annotThread);
    QUuid uid(UidStr);
    this->annotUids.push_back(uid);

    SimpleSceneController *scenePtr = new SimpleSceneController(0);
    this->tracks.push_back(scenePtr);
    this->visible.push_back(true);

    return this->sources.size();
}

void Workspace::RemoveSource(unsigned int num)
{
    assert(num < this->visible.size());
    this->visible.erase(this->visible.begin()+num);
    this->tracks.erase(this->tracks.begin()+num);
    this->sources.erase(this->sources.begin()+num);
    assert(num < this->annotThreads.size());
    this->annotThreads.erase(this->annotThreads.begin()+num);
    this->annotUids.erase(this->annotUids.begin()+num);
}

unsigned int Workspace::AddAutoAnnot(QString annotUid, QString algUid)
{
    QUuid annotUidObj(annotUid);
    int basedOnAnnot = FindAnnotWithUid(annotUidObj);
    assert(basedOnAnnot>=0 && basedOnAnnot < this->GetNumSources());

    std::tr1::shared_ptr<class AnnotThread> ann(new class AnnotThread);
    ann = this->annotThreads[basedOnAnnot];
    this->annotThreads.push_back(ann);

    this->sources.push_back(this->GetSourceName(basedOnAnnot));
    QUuid uid(QUuid::createUuid());
    this->annotUids.push_back(uid);

    SimpleSceneController *scenePtr = new SimpleSceneController(0);
    scenePtr = this->tracks[basedOnAnnot];
    this->tracks.push_back(scenePtr);
    this->visible.push_back(true);

    return this->sources.size();
}

/*void Workspace::SetTrack(unsigned int trackNum, SimpleSceneController *track)
{
    this->tracks[trackNum] = track;
}*/

SimpleSceneController *Workspace::GetTrack(unsigned int trackNum)
{
    assert(this->tracks.size() == this->sources.size());
    return this->tracks[trackNum];
}


unsigned int Workspace::GetNumSources()
{
    return this->sources.size();
}

QString Workspace::GetSourceName(unsigned int index)
{
    assert(index < this->sources.size());
    return this->sources[index];
}

QUuid Workspace::GetAnnotUid(unsigned int index)
{
    assert(index < this->sources.size());
    return this->annotUids[index];
}

//***********************************************************************

unsigned int Workspace::AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg)
{
    alg->SetId(this->nextThreadId);
    this->processingList.push_back(alg);
    this->threadProgress.push_back(0.);
    this->threadId.push_back(this->nextThreadId);

    this->nextThreadId ++;
    return this->nextThreadId;
}

std::tr1::shared_ptr<class AlgorithmProcess> Workspace::GetProcessing(unsigned int num)
{
    return this->processingList[num];
}

void Workspace::PauseProcessing(unsigned int num)
{
    assert(num < this->processingList.size());
    this->processingList[num]->Pause();
}

int Workspace::RemoveProcessing(unsigned int num)
{
    assert(num < this->processingList.size());

    //Check process is ready to be removed
    AlgorithmProcess::ProcessState state = this->processingList[num]->GetState();
    if(state!=AlgorithmProcess::STOPPED &&
            state!=AlgorithmProcess::PAUSED)
    {
        cout << "Process cannot be removed while it is running" << endl;
        return 0;
    }

    //If paused, stop the process
    if(this->processingList[num]->GetState()!=AlgorithmProcess::PAUSED)
        this->processingList[num]->Stop();

    assert(!this->processingList[num]->GetState()!=AlgorithmProcess::STOPPED);
    this->processingList.erase(this->processingList.begin()+num);
    this->threadProgress.erase(this->threadProgress.begin()+num);
    this->threadId.erase(this->threadId.begin()+num);
    return 1;
}

int Workspace::FindAnnotWithUid(QUuid uidIn)
{
    for(unsigned int i=0;i<this->annotUids.size();i++)
    {
        if(this->annotUids[i] == uidIn)
            return i;
    }
    return -1;
}

int Workspace::StartProcessing(unsigned int num)
{
    assert(num < this->processingList.size());
    if(this->processingList[num]->GetState() == AlgorithmProcess::PAUSED)
    {
        this->processingList[num]->Unpause();
        return 1;
    }
    return this->processingList[num]->Start();
}

unsigned int Workspace::GetNumProcessing()
{
    assert(this->processingList.size() == this->threadProgress.size());
    return this->processingList.size();
}

QString Workspace::GetProcessingName(unsigned int index)
{
    QString out = "Alg";
    return out;
}

void Workspace::ProcessingUpdate(unsigned int threadIdIn, float progress)
{
    for(unsigned int i=0;i<this->threadId.size();i++)
    {
        if(threadIdIn == this->threadId[i])
        {
            this->threadProgress[i] = progress;
        }
    }
}

float Workspace::GetProgress(unsigned int num)
{
    assert(num >= 0 && num < this->threadProgress.size());
    return this->threadProgress[num];
}

AlgorithmProcess::ProcessState Workspace::GetState(unsigned int num)
{
    assert(num < this->processingList.size());
    return this->processingList[num]->GetState();

}

int Workspace::NumProcessesBlockingShutdown()
{
    int count = 0;
    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        AlgorithmProcess::ProcessState state = this->processingList[i]->GetState();
        if(state == AlgorithmProcess::RUNNING) count += 1;
        if(state == AlgorithmProcess::RUNNING_PAUSING) count += 1;
        if(state == AlgorithmProcess::RUNNING_STOPPING) count += 1;
    }
    return count;
}

//************************************************************************

void Workspace::Clear()
{
    this->sources.clear();
    this->annotUids.clear();
    this->annotThreads.clear();
    this->tracks.clear();
    this->defaultFilename = "";
    this->visible.clear();
    this->processingList.clear();
    this->threadProgress.clear();
    this->threadId.clear();
}

void Workspace::Load(QString fina)
{
    this->Clear();
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
                    this->sources.push_back(fileInfo.absoluteFilePath());
                    this->visible.push_back(true);

                    QString uidStr = sourceEle.attribute("uid");
                    QUuid uid(uidStr);
                    this->annotUids.push_back(uid);

                    std::tr1::shared_ptr<class AnnotThread> annotThread;
                    this->annotThreads.push_back(annotThread);

                    SimpleSceneController *track =
                            new SimpleSceneController(NULL);

                    QDomNode trackData = sourceNode.firstChild();
                    while(!trackData.isNull())
                    {
                        QDomElement et = trackData.toElement(); // try to convert the node to an element.
                        if(et.isNull()) continue;
                        if(et.tagName() != "tracking") {trackData = trackData.nextSibling(); continue;}

                        track->ReadAnnotationXml(et);

                        trackData = trackData.nextSibling();

                    }

                    this->tracks.push_back(track);
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
                    alg->Pause(); //Start paused

                    //Send data to algorithm process
                    QString modelPreamble1 = QString("DATA_BLOCK=%1\n").arg(modelData.length());
                    QString modelPreamble2 = QString("MODEL\n");
                    alg->SendCommand(modelPreamble1);
                    alg->SendCommand(modelPreamble2);
                    alg->SendRawData(modelData);

                    alg->SendCommand("TRAIN\n"); //Continue to train if needed

                    //Ask process to provide progress update
                    alg->SendCommand("GET_PROGRESS\n");

                    QString uidStr = modelEle.attribute("uid");
                    QUuid uid(uidStr);
                    alg->SetUid(uid);
                    this->AddProcessing(alg);
                    modelNode = modelNode.nextSibling();
                }

            }
        }
        n = n.nextSibling();
    }
}

int Workspace::Save()
{
    assert(this->tracks.size()==this->sources.size());
    assert(this->annotUids.size()==this->sources.size());

    if(this->defaultFilename.length()==0)
        return 0;
    QFileInfo pathInfo(this->defaultFilename);
    QDir dir(pathInfo.absoluteDir());

    //Save data to file
    QFile f(this->defaultFilename);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<workspace>" << endl;
    out << "<sources>" << endl;
    for(unsigned int i=0;i<this->sources.size();i++)
    {
        try
        {
            out << "\t<source id=\""<<i<<"\" uid=\""<<Qt::escape(this->annotUids[i])<<"\" file=\""<<
                   Qt::escape(dir.relativeFilePath(this->sources[i]))<<"\">" << endl;
            this->tracks[i]->WriteAnnotationXml(out);
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
    return 1;
}

void Workspace::SaveAs(QString &fina)
{
    this->defaultFilename = fina;
    this->Save();
}

void Workspace::Update()
{
    for(unsigned int i=0;i<this->processingList.size();i++)
    {
        this->processingList[i]->Update();
    }

}
