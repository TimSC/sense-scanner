#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <vector>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include "mediabuffer.h"
#include "scenecontroller.h"

class Workspace
{
public:
    Workspace();
    virtual ~Workspace();

    unsigned int AddSource(QString &fina);
    //void SetTrack(unsigned int trackNum, SimpleSceneController *track);
    SimpleSceneController *GetTrack(unsigned int trackNum);

    unsigned int GetNumSources();
    QString GetSourceName(unsigned int index);

    void Clear();
    void Load(QString fina);
    int Save();
    void SaveAs(QString &fina);

protected:

    std::vector<QString> sources;
    QString defaultFilename;
    std::vector<SimpleSceneController *> tracks;
    std::vector<QMenu *>annotationMenus;
};

#endif // WORKSPACE_H
