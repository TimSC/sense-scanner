#include "workspace.h"

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
