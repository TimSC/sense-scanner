#include "eventloop.h"
#include <assert.h>
#include <exception>
#include <iostream>
using namespace std;

Event::Event()
{
    this->type = "UNKNOWN";
}

Event::Event(std::string typeIn)
{
    this->type = typeIn;
}

//************************************************

EventReceiver::EventReceiver()
{

}

void EventReceiver::AddMessage(const class Event &event)
{
    this->mutex.lock();
    this->eventBuffer.push_back(event);
    this->mutex.unlock();
}

int EventReceiver::BufferSize()
{
    this->mutex.lock();
    unsigned s = this->eventBuffer.size();
    this->mutex.unlock();
    return s;
}

class Event EventReceiver::PopEvent()
{
    this->mutex.lock();
    if(this->eventBuffer.size() == 0)
    {
        this->mutex.unlock();
        throw std::runtime_error("Buffer is empty");
    }
    class Event ev = this->eventBuffer[this->eventBuffer.size()-1];
    this->eventBuffer.erase(this->eventBuffer.end()-1);
    this->mutex.unlock();
    return ev;
}

//*****************************************************

EventLoop::EventLoop()
{
    cout << "EventLoop::EventLoop()" << (unsigned long long)this << endl;
}

void EventLoop::SendEvent(const class Event &event)
{
    std::map<std::string, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.find(event.type);
    if(it == this->eventReceivers.end()) return; //No listeners found

    for(unsigned i=0;i<it->second.size();i++)
    {
        it->second[i]->AddMessage(event);
    }
}

void EventLoop::AddListener(std::string type, class EventReceiver &rx)
{
    std::map<std::string, std::vector<EventReceiver *> >::iterator it = this->eventReceivers.find(type);
    if(it == this->eventReceivers.end())
        this->eventReceivers[type] = std::vector<EventReceiver *> ();
    it = this->eventReceivers.find(type);
    assert(it != this->eventReceivers.end());
    it->second.push_back(&rx);
}
