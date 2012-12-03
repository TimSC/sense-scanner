#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <vector>
#include <QtCore/QString>
#include "mediabuffer.h"

class Workspace
{
public:
    Workspace();
    virtual ~Workspace();

    void AddSource(QString &fina);
    unsigned int GetNumSources();
    QString GetSourceName(unsigned int index);
protected:

    std::vector<QString> sources;

};

#endif // WORKSPACE_H
