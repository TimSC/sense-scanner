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
#include "applymodel.h"

class Workspace : public MessagableThread
{
    /*!
    * Workshape contains all annotation track objects and algorithm process objects.
    * Functionality for loading and saving is provided. Various thread starting and
    * stopping is also available to control multiple algorithms at once.
    */

    Q_OBJECT
public:
    explicit Workspace(int activeIn, QObject *parent);
    explicit Workspace(int activeIn, QObject *parent, const Workspace &other);
    virtual ~Workspace();

    Workspace& operator= (const Workspace &other);
    bool operator!= (const Workspace &other);

    void SetEventLoop(class EventLoop &eventLoopIn);

    //This direct access functions are bit of a hack.
    //TODO consider removing them and using proper events
    unsigned int AddSourceFromMain(QUuid uuid);
    void RemoveSourceFromMain(QUuid uuid);

    void AddProcessingFromMain(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    int RemoveProcessingFromMain(QUuid uuid);

    int AddHelperThreadFromMain(QUuid algUuid, QUuid annotUuid, QUuid mediaInterface);

    //** Processing
    //std::tr1::shared_ptr<class AlgorithmProcess> GetProcessing(unsigned int num);
    void ProcessingProgressChanged(QUuid uuid, float progress);
    float GetProcessingProgress(QUuid uuid);

    void ClearProcessingFromMain();
    void ClearAnnotationFromMain();

    AlgorithmProcess::ProcessState GetProcessingState(QUuid uuid);

    int NumProcessesBlockingShutdown();
    void Update();
    void UpdateFromMain();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    int HasChanged();

    void TerminateThreads();

    void SetMediaUuid(QUuid mediaUuidIn);

    QList<QUuid> GetProcessingUuidsFromMain();
    QList<QUuid> GetAnnotationUuidsFromMain();

    int IsReadyForSave();
    void SetDemoMode(int mode);

protected:
    unsigned int AddSource(QUuid uuid);
    void RemoveSource(QUuid uuid);

    void AddProcessing(std::tr1::shared_ptr<class AlgorithmProcess> alg);
    int RemoveProcessing(QUuid uuid);

    void ClearProcessing();
    void ClearAnnotation();

    QList<QUuid> GetProcessingUuids();
    QList<QUuid> GetAnnotationUuids();

    //** Sources and annotations
    //Sources and annotation data
    std::vector<std::tr1::shared_ptr<class Annotation> > annotations;
    QList<QUuid> annotationUuids;
    std::vector<std::tr1::shared_ptr<class AnnotThread> > annotationThreads;

    //Processing data
    std::vector<std::tr1::shared_ptr<class AlgorithmProcess> > processingList;
    std::vector<float> processingProgress;
    QList<QUuid> processingUuids;
    Mutex lock;
    QUuid mediaUuid;
    int active;
    QObject *parent;
    ApplyModelPool applyModelPool;
    int demoMode;
};

#endif // WORKSPACE_H
