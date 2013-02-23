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
#include "localmutex.h"

class Workspace : public QObject
{
    /*!
    * Workshape contains all annotation track objects and algorithm process objects.
    * Functionality for loading and saving is provided. Various thread starting and
    * stopping is also available to control multiple algorithms at once.
    */

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
    void AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    int RemoveProcessing(QUuid uuid);
    //std::tr1::shared_ptr<class AlgorithmProcess> GetProcessing(unsigned int num);
    void ProcessingProgressChanged(QUuid uuid, float progress);
    float GetProcessingProgress(QUuid uuid);
    AlgorithmProcess::ProcessState GetProcessingState(QUuid uuid);
    QList<QUuid> GetProcessingUuids();

    int NumProcessesBlockingShutdown();
    void Update();

    void ClearProcessing();
    void ClearAnnotation();
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
    class EventLoop *eventLoop;

    //Processing data
    std::vector<std::tr1::shared_ptr<class AlgorithmProcess> > processingList;
    std::vector<float> processingProgress;
    QList<QUuid> processingUuids;
    QList<AlgorithmProcess::ProcessState> processingStates;
    Mutex lock;
};

#endif // WORKSPACE_H
