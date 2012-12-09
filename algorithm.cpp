#include "algorithm.h"
#include "localsleep.h"
#include <iostream>
using namespace std;

//**********************************

Algorithm::Algorithm(class EventLoop *eventLoopIn) : MessagableThread(eventLoopIn)
{
    progress = 0.f;
}

Algorithm::~Algorithm()
{

}

void Algorithm::Update()
{
    msleep(1000);
    progress += 0.01f;
    cout << "progress " << progress << endl;

    if(progress >= 1.f)
    {
       progress = 1.f;
       this->mutex.lock();
       this->stopThreads = 1;
       this->mutex.unlock();
    }
}
