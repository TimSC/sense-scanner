#include "useractions.h"

UserActions::UserActions() : MessagableThread()
{

}

UserActions::~UserActions()
{

}

void UserActions::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{


    MessagableThread::HandleEvent(ev);
}

void UserActions::Update()
{

}
