#include "workspace.h"
#include <assert.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui>

Workspace::Workspace()
{
    this->Clear();
}

Workspace::~Workspace()
{

}

void Workspace::AddSource(QString &fina)
{
    this->sources.push_back(fina);
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
    this->defaultFilename = "";
}

void Workspace::Load(QString &fina)
{

}

int Workspace::Save()
{
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
        out << "\t<source file=\""<<Qt::escape(dir.relativeFilePath(this->sources[i]))<<"\">" << endl;

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
