#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <vector>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include "mediabuffer.h"
#include "scenecontroller.h"
#include "algorithm.h"

class Workspace
{
public:
    Workspace();
    Workspace(const Workspace &other);
    virtual ~Workspace();
    Workspace& operator= (const Workspace &other);
    bool operator!= (const Workspace &other);

    //** Sources
    unsigned int AddSource(QString &fina);
    void RemoveSource(unsigned int num);

    //void SetTrack(unsigned int trackNum, SimpleSceneController *track);
    SimpleSceneController *GetTrack(unsigned int trackNum);

    unsigned int GetNumSources();
    QString GetSourceName(unsigned int index);

    //** Processing
    unsigned int AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    void PauseProcessing(unsigned int num);
    int RemoveProcessing(unsigned int num);
    int StartProcessing(unsigned int num);
    void ProcessingUpdate(unsigned int threadIdIn, float progress);
    float GetProgress(unsigned int num);
    AlgorithmProcess::ProcessState GetState(unsigned int num);

    int NumProcessesBlockingShutdown();
    void Update(class EventLoop &ev);

    unsigned int GetNumProcessing();
    QString GetProcessingName(unsigned int index);

    void Clear();
    void Load(QString fina);
    int Save();
    void SaveAs(QString &fina);
    int HasChanged();

protected:
    std::vector<std::tr1::shared_ptr<class AlgorithmProcess> > processingList;
    std::vector<QString> sources;
    QString defaultFilename;
    std::vector<SimpleSceneController *> tracks;
    std::vector<bool> visible;
    unsigned int nextThreadId;
    std::vector<float> threadProgress;
    std::vector<unsigned int> threadId;
};

#endif // WORKSPACE_H
