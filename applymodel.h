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

protected:
    QUuid annotUuid;
};

class ApplyModelPool
{
public:
    ApplyModelPool();
    virtual ~ApplyModelPool();

    void SetEventLoop(class EventLoop *eventLoopIn);
    void Add(QUuid uuid, QUuid annotUuid);
    void Remove(QUuid uuid);
    void Clear();

protected:
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> > pool;
    class EventLoop *eventLoop;
};

#endif // APPLYMODEL_H
