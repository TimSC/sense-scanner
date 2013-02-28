#ifndef USERACTIONS_H
#define USERACTIONS_H

#include "eventloop.h"

class UserActions : public MessagableThread
{
public:
    UserActions();
    virtual ~UserActions();

    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    virtual void Update();

};

#endif // USERACTIONS_H
