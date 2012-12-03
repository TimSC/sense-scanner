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

    void AddSource(class AbstractMedia *media);
protected:



};

#endif // WORKSPACE_H
