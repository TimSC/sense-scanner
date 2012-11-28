#include "eventloop.h"
#include <assert.h>
#include <exception>
#include <iostream>
#include "mediabuffer.h"
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
    this->raw = other.raw; //Note: this is a raw pointer
    this->rawSize = other.rawSize;
}

Event::~Event()
{
    if(this->raw)
        delete (class DecodedFrame *)raw;
    raw = 0;
}

//************************************************

EventReceiver::EventReceiver(class EventLoop *elIn)
{
    this->el = elIn;
}

EventReceiver::~EventReceiver()
{
    cout << "EventReceiver::~EventReceiver()" << endl;

    if(this->el) this->el->RemoveListener(*this);
}

void EventReceiver::AddMessage(std::tr1::shared_ptr<class Event> event)
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

std::tr1::shared_ptr<class Event> EventReceiver::PopEvent()
{
    this->mutex.lock();
    if(this->eventBuffer.size() == 0)
    {
        this->mutex.unlock();
        throw std::runtime_error("Buffer is empty");
    }
    std::tr1::shared_ptr<class Event> ev = this->eventBuffer[0]; //Get the first in buffer FIFO
    this->eventBuffer.erase(this->eventBuffer.begin());
    this->mutex.unlock();
    return ev;
}

std::tr1::shared_ptr<class Event> EventReceiver::WaitForEventId(unsigned long long idIn,
                                          unsigned timeOutMs)
{
    unsigned waitingTime = 0;
    while(waitingTime < timeOutMs)
    {
        this->mutex.lock();
        for(unsigned i=0; i<this->eventBuffer.size(); i++)
        {
            std::tr1::shared_ptr<class Event> ev = this->eventBuffer[i];
            if(ev->id == idIn)
            {
                std::tr1::shared_ptr<class Event> out = this->eventBuffer[i];
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

void EventReceiver::MessageLoopDeleted()
{
    this->mutex.lock();
    this->el = NULL;
    this->mutex.unlock();
}

//*****************************************************

EventLoop::EventLoop()
{

}

EventLoop::~EventLoop()
{
    cout << "EventLoop::~EventLoop()" << endl;
    this->mutex.lock();
    std::map<std::string, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.begin();
    while(it != this->eventReceivers.end())
    {
        std::vector<EventReceiver *> &eventListeners = it->second;

        //Dispatch message to listeners
        for(unsigned i=0;i<eventListeners.size();i++)
        {
            eventListeners[i]->MessageLoopDeleted();
        }

        it++;
    }
    this->mutex.unlock();
}

void EventLoop::SendEvent(std::tr1::shared_ptr<class Event> event)
{
    //cout << "Sent event "<< event->type << endl;
    //Get a local copy of listeners
    this->mutex.lock();
    std::map<std::string, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.find(event->type);
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

void EventLoop::RemoveListener(class EventReceiver &rx)
{
    cout << "EventLoop::RemoveListener(...)" << endl;
    this->mutex.lock();

    //Remove listener based on pointer location
    std::map<std::string, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.begin();
    while(it != this->eventReceivers.end())
    {
        std::vector<EventReceiver *> prefiltered = it->second;
        std::vector<EventReceiver *> &eventListeners = it->second;
        eventListeners.clear();

        //Dispatch message to listeners
        for(unsigned i=0;i<prefiltered.size();i++)
        {
            if(&rx != prefiltered[i])
                eventListeners.push_back(prefiltered[i]);
        }

        it++;
    }

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

