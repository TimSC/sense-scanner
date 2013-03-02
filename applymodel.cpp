#include "applymodel.h"

ApplyModel::ApplyModel() : MessagableThread()
{

}

ApplyModel::~ApplyModel()
{

}

void ApplyModel::Update()
{

    this->msleep(5);
}

void ApplyModel::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{

}


ApplyModelPool::ApplyModelPool()
{

}

ApplyModelPool::~ApplyModelPool()
{

}

void ApplyModelPool::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
}

void ApplyModelPool::Add(QUuid uuid)
{
    std::tr1::shared_ptr<class ApplyModel> am(new ApplyModel());
    this->pool[uuid] = am;
    this->pool[uuid]->SetEventLoop(this->eventLoop);
    this->pool[uuid]->Start();
}

void ApplyModelPool::Remove(QUuid uuid)
{
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> >::iterator it;
    for(it = this->pool.begin(); it!=this->pool.end();it++)
    {
        it.value()->Stop();
    }
}

