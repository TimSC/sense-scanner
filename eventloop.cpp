#include "eventloop.h"
#include <assert.h>
#include <exception>
#include <iostream>
using namespace std;



//***********************************************

Event::Event()
{
    this->type = "UNKNOWN";
    this->id = 0;
    this->raw = 0;
    this->rawSize = 0;
}

Event::Event(std::string typeIn, unsigned long long idIn)
{
    this->type = typeIn;
    this->id = idIn;
    this->raw = 0;
    this->rawSize = 0;
}

Event::Event(const Event& other)
{
    this->type = other.type;
    this->id = other.id;
    this->data = other.data;
    //this->raw is not modified and is shared while the message exists
    this->rawSize = other.rawSize;
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
    class Event ev = this->eventBuffer[0]; //Get the first in buffer FIFO
    this->eventBuffer.erase(this->eventBuffer.begin());
    this->mutex.unlock();
    return ev;
}

class Event EventReceiver::WaitForEventId(unsigned long long idIn,
                                          unsigned timeOutMs)
{
    unsigned waitingTime = 0;
    while(waitingTime < timeOutMs)
    {
        this->mutex.lock();
        for(unsigned i=0; i<this->eventBuffer.size(); i++)
        {
            class Event &ev = this->eventBuffer[i];
            if(ev.id == idIn)
            {
                class Event out = this->eventBuffer[i];
                this->eventBuffer.erase(this->eventBuffer.begin()+i);
                this->mutex.unlock();
                return out;

            }
        }
        this->mutex.unlock();

        usleep(10000);
        waitingTime += 10;
    }

    throw std::runtime_error("Wait for message has timed out");
}

//*****************************************************

EventLoop::EventLoop()
{

}

void EventLoop::SendEvent(const class Event &event)
{
    cout << "Sent event "<< event.type << endl;
    //Get a local copy of listeners
    this->mutex.lock();
    std::map<std::string, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.find(event.type);
    if(it == this->eventReceivers.end())
    {
        //No listeners found
        this->mutex.unlock();
        return;
    }
    std::vector<EventReceiver *> eventListeners = it->second;
    this->mutex.unlock();

    //Dispatch message to listeners
    for(unsigned i=0;i<eventListeners.size();i++)
    {
        eventListeners[i]->AddMessage(event);
    }
}

void EventLoop::AddListener(std::string type, class EventReceiver &rx)
{
    this->mutex.lock();
    std::map<std::string, std::vector<EventReceiver *> >::iterator it = this->eventReceivers.find(type);
    if(it == this->eventReceivers.end())
        this->eventReceivers[type] = std::vector<EventReceiver *> ();
    it = this->eventReceivers.find(type);
    assert(it != this->eventReceivers.end());
    it->second.push_back(&rx);
    this->mutex.unlock();
}

unsigned long long EventLoop::GetId()
{
    this->mutex.lock();
    unsigned long long out = this->nextId;
    this->nextId ++;
    this->mutex.unlock();
    return out;
}

