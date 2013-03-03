#ifndef APPLYMODEL_H
#define APPLYMODEL_H

#include "eventloop.h"

class ApplyModel : public MessagableThread
{
public:
    ApplyModel(QUuid annotUuidIn);
    virtual ~ApplyModel();

    void Update();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    void SetEventLoop(class EventLoop *eventLoopIn);
    void SetMediaInterface(QUuid mediaInterfaceIn);
protected:
    QUuid annotUuid;
    QUuid algUuid;
    bool algUuidSet, srcFinaSet, srcDurationSet;
    QString srcFina;
    unsigned long long srcDuration;
    QUuid mediaInterface;

    bool currentTimeSet;
    unsigned long long currentStartTimestamp, currentEndTimestamp;

    bool issueEnountered;

    std::vector<std::vector<float> > currentModel;
    bool currentModelSet;
};

class ApplyModelPool
{
public:
    ApplyModelPool();
    virtual ~ApplyModelPool();

    void SetEventLoop(class EventLoop *eventLoopIn);
    void Add(QUuid uuid, QUuid annotUuid, QUuid mediaInterface);
    void Remove(QUuid uuid);
    void Clear();

protected:
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> > pool;
    class EventLoop *eventLoop;
};

#endif // APPLYMODEL_H
