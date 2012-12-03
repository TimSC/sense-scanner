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

    void Clear();
    void Load(QString fina);
    int Save();
    void SaveAs(QString &fina);

protected:

    std::vector<QString> sources;
    QString defaultFilename;

};

#endif // WORKSPACE_H
