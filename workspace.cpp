#include "workspace.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui>
#include <QtXml/QtXml>
#include <iostream>
using namespace std;

Workspace::Workspace()
{
    this->Clear();
}

Workspace::~Workspace()
{

}

unsigned int Workspace::AddSource(QString &fina)
{
    this->sources.push_back(fina);

    SimpleSceneController *scenePtr = new SimpleSceneController(0);
    this->tracks.push_back(scenePtr);

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

void Workspace::Clear()
{
    this->sources.clear();
    this->tracks.clear();
    this->defaultFilename = "";
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

int Workspace::HasChanged()
{
    return 1;
}
