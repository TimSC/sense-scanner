#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <vector>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include "mediabuffer.h"
#include "scenecontroller.h"
#include "algorithm.h"
#include "annotation.h"

class Workspace : public QObject
{
    Q_OBJECT
public:
    explicit Workspace();
    explicit Workspace(const Workspace &other);
    Workspace& operator= (const Workspace &other);
    bool operator!= (const Workspace &other);

    void SetEventLoop(class EventLoop &eventLoopIn);

    //** Sources and annotations
    unsigned int AddSource(QString &fina, QString UidStr, class AvBinMedia* mediaInterface);
    void RemoveSource(unsigned int num);
    unsigned int AddAutoAnnot(QString annotUid, QString algUid, class AvBinMedia* mediaInterface);
    int FindAnnotWithUid(QUuid uidIn);

    //void SetTrack(unsigned int trackNum, TrackingAnnotation *track);
    TrackingAnnotation *GetTrack(unsigned int trackNum);

    unsigned int GetNumSources();
    QString GetSourceName(unsigned int index);
    QUuid GetAnnotUid(unsigned int index);

    //** Processing
    unsigned int AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    std::tr1::shared_ptr<class AlgorithmProcess> GetProcessing(unsigned int num);
    void PauseProcessing(unsigned int num);
    int RemoveProcessing(unsigned int num);
    int StartProcessing(unsigned int num);
    void ProcessingUpdate(unsigned int threadIdIn, float progress);
    float GetProgress(unsigned int num);
    AlgorithmProcess::ProcessState GetState(unsigned int num);

    int NumProcessesBlockingShutdown();
    void Update();

    unsigned int GetNumProcessing();
    QString GetProcessingName(unsigned int index);

    void Clear();
    void Load(QString fina, class AvBinMedia* mediaInterface);
    int Save();
    void SaveAs(QString &fina);
    int HasChanged();

    void TerminateThreads();
    void SetAnnotThreadsInactive();

protected:
    //Sources and annotation data
    std::vector<std::tr1::shared_ptr<class Annotation> > annotations;

    QString defaultFilename;
    unsigned int nextThreadId;
    class EventLoop *eventLoop;

    //Processing data
    std::vector<std::tr1::shared_ptr<class AlgorithmProcess> > processingList;
    std::vector<float> threadProgress;
    std::vector<unsigned int> threadId;

};

#endif // WORKSPACE_H
