#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "eventloop.h"

class Algorithm : public MessagableThread
{
public:
    Algorithm(class EventLoop *eventLoopIn);
    virtual ~Algorithm();

    void Update();
};

#endif // ALGORITHM_H
