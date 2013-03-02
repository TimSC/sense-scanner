#ifndef APPLYMODEL_H
#define APPLYMODEL_H

#include "eventloop.h"

class ApplyModel : public MessagableThread
{
public:
    ApplyModel();
    virtual ~ApplyModel();

    void Update();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);
};

class ApplyModelPool
{
public:
    ApplyModelPool();
    virtual ~ApplyModelPool();

    void SetEventLoop(class EventLoop *eventLoopIn);
    void Add(QUuid uuid);
    void Remove(QUuid uuid);

protected:
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> > pool;
    class EventLoop *eventLoop;
};

#endif // APPLYMODEL_H
