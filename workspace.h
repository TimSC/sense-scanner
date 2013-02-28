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
    virtual ~Workspace();

    Workspace& operator= (const Workspace &other);
    bool operator!= (const Workspace &other);

    void SetEventLoop(class EventLoop &eventLoopIn);

    //** Sources and annotations
    unsigned int AddSource(QUuid uuid);

    void RemoveSource(QUuid uuid);
    unsigned int AddAutoAnnot(QUuid annotUid, QUuid algUid, class AvBinMedia* mediaInterface);

    //void SetTrack(unsigned int trackNum, TrackingAnnotation *track);
    //TrackingAnnotation *GetTrack(unsigned int trackNum);

    //** Processing
    void AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    int RemoveProcessing(QUuid uuid);
    //std::tr1::shared_ptr<class AlgorithmProcess> GetProcessing(unsigned int num);
    void ProcessingProgressChanged(QUuid uuid, float progress);
    float GetProcessingProgress(QUuid uuid);

    void ProcessingStateChanged(QUuid uuid, AlgorithmProcess::ProcessState state);
    AlgorithmProcess::ProcessState GetProcessingState(QUuid uuid);



    int NumProcessesBlockingShutdown();
    void Update();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    int HasChanged();

    void TerminateThreads();
    void SetAnnotThreadsInactive();

    void ClearProcessing();
    void ClearAnnotation();
    QList<QUuid> GetProcessingUuids();
    QList<QUuid> GetAnnotationUuids();

    void SetMediaUuid(QUuid mediaUuidIn);

protected:
    //Sources and annotation data
    std::vector<std::tr1::shared_ptr<class Annotation> > annotations;
    QList<QUuid> annotationUuids;
    std::vector<std::tr1::shared_ptr<class AnnotThread> > annotationThreads;

    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

    //Processing data
    std::vector<std::tr1::shared_ptr<class AlgorithmProcess> > processingList;
    std::vector<float> processingProgress;
    QList<QUuid> processingUuids;
    QList<AlgorithmProcess::ProcessState> processingStates;
    Mutex lock;
    QUuid mediaUuid;
};

#endif // WORKSPACE_H
