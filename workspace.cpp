#include "workspace.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <iostream>
using namespace std;

Workspace::Workspace()
{
    this->Clear();
    nextThreadId = 1;
}

Workspace::Workspace(const Workspace &other)
{
    this->operator =(other);
}

Workspace::~Workspace()
{

}

Workspace& Workspace::operator= (const Workspace &other)
{
    sources = other.sources;
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
    for(unsigned int i=0;i<other.tracks.size();i++)
    {
        if(*tracks[i] != *other.tracks[i]) return true;
    }
    return false;
}

unsigned int Workspace::AddSource(QString &fina)
{
    this->sources.push_back(fina);

    SimpleSceneController *scenePtr = new SimpleSceneController(0);
    this->tracks.push_back(scenePtr);
    this->visible.push_back(true);

    return this->sources.size();
}

void Workspace::RemoveSource(unsigned int num)
{
    this->visible.erase(this->visible.begin()+num);
    this->tracks.erase(this->tracks.begin()+num);
    this->sources.erase(this->sources.begin()+num);
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

//***********************************************************************

unsigned int Workspace::AddProcessing(std::tr1::shared_ptr<class Algorithm> alg)
{
    alg->SetThreadId(this->nextThreadId);
    this->processingList.push_back(alg);
    this->threadProgress.push_back(0.);
    this->threadId.push_back(this->nextThreadId);

    this->nextThreadId ++;
    return this->nextThreadId;
}

void Workspace::PauseProcessing(unsigned int num)
{
    assert(num >= 0 && num < this->processingList.size());
    if(this->processingList[num]->isRunning())
    {
        this->processingList[num]->StopThread();
    }
}

void Workspace::RemoveProcessing(unsigned int num)
{
    if(this->processingList[num]->isRunning())
    {
        cout << "Process cannot be removed while it is running" << endl;
        return;
    }

    assert(num >= 0 && num < this->processingList.size());
    assert(!this->processingList[num]->isRunning());
    this->processingList.erase(this->processingList.begin()+num);
    this->threadProgress.erase(this->threadProgress.begin()+num);
    this->threadId.erase(this->threadId.begin()+num);
}

int Workspace::StartProcessing(unsigned int num)
{
    assert(num >= 0 && num < this->processingList.size());
    return this->processingList[num]->StartThread();
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

//************************************************************************

void Workspace::Clear()
{
    this->sources.clear();
    this->tracks.clear();
    this->defaultFilename = "";
    this->visible.clear();
    this->processingList.clear();
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
        }
        n = n.nextSibling();
    }
}

int Workspace::Save()
{
    assert(this->tracks.size()==this->sources.size());
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
        out << "\t<source id=\""<<i<<"\" file=\""<<Qt::escape(dir.relativeFilePath(this->sources[i]))<<"\">" << endl;
        this->tracks[i]->WriteAnnotationXml(out);
        out << "\t</source>" << endl;
    }

    out << "</sources>" << endl;
    out << "</workspace>" << endl;
    f.close();
    return 1;
}

void Workspace::SaveAs(QString &fina)
{
    this->defaultFilename = fina;
    this->Save();
}

