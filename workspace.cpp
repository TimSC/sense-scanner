#include "workspace.h"
#include <assert.h>

Workspace::Workspace()
{

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
