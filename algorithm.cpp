#include "algorithm.h"
#include <iostream>
#include <sstream>
#include <assert.h>
using namespace std;

//**********************************

Algorithm::Algorithm(class EventLoop *eventLoopIn) : MessagableThread(eventLoopIn)
{
    this->progress = 0.f;
    this->threadId = 0;
}

Algorithm::~Algorithm()
{

}

void Algorithm::Update()
{
    this->progress += 0.01f;
    if(this->progress >= 1.f)
    {
       this->progress = 1.f;
       this->mutex.lock();
       this->stopThreads = 1;
       this->mutex.unlock();
    }

    assert(this->eventLoop);
    std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_PROGRESS_UPDATE"));
    std::ostringstream tmp;
    tmp << this->progress << "," << this->threadId;
    openEv->data = tmp.str();
    this->eventLoop->SendEvent(openEv);

    this->msleep(10000);
}
void Algorithm::SetThreadId(unsigned int idIn)
{
    assert(!this->isRunning());
    this->threadId = idIn;
}
