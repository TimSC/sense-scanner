#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "eventloop.h"

class Algorithm : public MessagableThread
{
public:
    Algorithm(class EventLoop *eventLoopIn);
    virtual ~Algorithm();

    void Update();
    void SetThreadId(unsigned int idIn);

protected:
    float progress;
    unsigned int threadId;
};

#endif // ALGORITHM_H