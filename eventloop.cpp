#include "eventloop.h"
#include <assert.h>
#include <exception>
#include <sstream>
#include <iostream>
#include "localsleep.h"
#include "mediabuffer.h"
#include <QtCore/QTime>
using namespace std;

//***********************************************

Event::Event()
{
    this->type = "UNKNOWN";
    this->id = 0;
    this->raw = NULL;
}

Event::Event(QString typeIn, unsigned long long idIn)
{
    this->type = typeIn;
    this->id = idIn;
    this->raw = NULL;
}

Event::Event(const Event& other)
{
    assert(0); //Avoid copying with a raw pointer?
    this->type = other.type;
    this->id = other.id;
    this->data = other.data;
    this->raw = other.raw; //Note: this is a raw pointer
    this->toUuid = other.toUuid;
    this->fromUuid = other.fromUuid;
    this->buffer = other.buffer;
}

Event::~Event()
{
    if(this->raw!=NULL)
    {
        delete this->raw;
    }
    this->raw = NULL;
}

//************************************************

EventReceiver::EventReceiver(class EventLoop *elIn,
                             const char *filenameIn,
                             unsigned int lineIn)
{
    this->stopping = false;
    this->el = elIn;
    this->threadId = QUuid::createUuid();
    this->line = lineIn;
    this->filename = filenameIn;
}

EventReceiver::~EventReceiver()
{
    cout << "EventReceiver::~EventReceiver()" << endl;

    this->mutex.lock();
    this->stopping = 1;
    this->mutex.unlock();
    this->RespondToBufferedRequests();

    if(this->el) this->el->RemoveListener(*this);
}

int EventReceiver::AddMessage(std::tr1::shared_ptr<class Event> event)
{
    this->mutex.lock(); 
    if(this->stopping)
    {
        this->mutex.unlock();
        return 0; //Cannot add event to receiver that is stopping
    }

    this->eventBuffer.push_back(event);
    if(this->eventBuffer.size()>100)
    {
        cout << "Warning: event listener queue is size "<< this->eventBuffer.size() << endl;
    }

    this->mutex.unlock();
    return 1;
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
    if(this->eventBuffer.size() > 100)
    {
        cout << "Warning: event listener queue is size "<< this->eventBuffer.size() << endl;
    }
    std::tr1::shared_ptr<class Event> ev = this->eventBuffer[0]; //Get the first in buffer FIFO
    this->eventBuffer.erase(this->eventBuffer.begin());
    this->mutex.unlock();
    return ev;
}

std::tr1::shared_ptr<class Event> EventReceiver::GetLatestDiscardOlder(QString typeIn, QUuid filterUuid)
{
    this->mutex.lock();
    std::tr1::shared_ptr<class Event> tmp;
    std::tr1::shared_ptr<class Event> ev;
    unsigned i=0, count=0;

    while(i<this->eventBuffer.size())
    {
        //Only process events of specified type
        tmp = this->eventBuffer[i];
        if(tmp->type!=typeIn)
        {
            i++;
            continue;
        }

        //Only process events of specified uuid
        if(!filterUuid.isNull() && filterUuid != tmp->toUuid)
        {
            i++;
            continue;
        }

        ev = tmp;
        this->eventBuffer.erase(this->eventBuffer.begin()+i);
        count++;
    }

    this->mutex.unlock();
    if(count==0)
        throw std::runtime_error("Event type not found");
    return ev;
}

std::tr1::shared_ptr<class Event> EventReceiver::WaitForEventId(unsigned long long idIn,
                                          unsigned timeOutMs)
{
    QTime elapse, warningTime;
    elapse.start();
    warningTime.start();
    this->mutex.lock();
    int stop = this->stopping;
    this->mutex.unlock();
    if(stop) throw std::runtime_error("Cannot wait in a thread that is stopping");

    while(elapse.elapsed() < timeOutMs || timeOutMs < 0)
    {
        //cout << "Waiting..." << (unsigned long)this << endl;
        this->mutex.lock();
        //For each message in the buffer
        for(unsigned i=0; i<this->eventBuffer.size(); i++)
        {
            //Check there ID numbers
            std::tr1::shared_ptr<class Event> ev = this->eventBuffer[i];
            if(ev->id == idIn)
            {
                std::tr1::shared_ptr<class Event> out = this->eventBuffer[i];
                this->eventBuffer.erase(this->eventBuffer.begin()+i);
                this->mutex.unlock();
                return out;

            }
        }

        if(elapse.elapsed()>30000 && warningTime.elapsed() > 1000)
        {
            warningTime.restart();
            cout << "Warning: thread waiting for a long time" << endl;
        }

        //Check if threads are stopping
        for(unsigned i=0; i<this->eventBuffer.size(); i++)
        {
            std::tr1::shared_ptr<class Event> ev = this->eventBuffer[i];
            if(ev->type == "STOP_THREADS")
            {
                this->stopping = true;
                this->mutex.unlock();
                throw std::runtime_error("Stop threads encountered, wait aborted");
            }

            if(ev->type == "STOP_SPECIFIC_THREAD")
            {
                if(ev->toUuid == this->threadId)
                {
                    this->stopping = true;
                    this->mutex.unlock();
                    throw std::runtime_error("Thread shopping, wait aborted");
                }
            }
        }

        this->mutex.unlock();

        LocalSleep::msleep(10);
    }

    throw std::runtime_error("Wait for message has timed out");
}

void EventReceiver::MessageLoopDeleted()
{
    this->mutex.lock();
    this->el = NULL;
    this->mutex.unlock();
}

void EventReceiver::SetThreadId(QUuid idIn)
{
    assert(this!=NULL);
    this->mutex.lock();
    this->threadId = idIn;
    this->mutex.unlock();
}

QUuid EventReceiver::GetThreadId()
{
    assert(this!=NULL);
    this->mutex.lock();
    QUuid out = this->threadId;
    this->mutex.unlock();
    return out;
}

void EventReceiver::Stop()
{
    assert(this!=NULL);
    this->mutex.lock();
    this->stopping = true;
    this->mutex.unlock();
}

void EventReceiver::RespondToBufferedRequests()
{
    this->mutex.lock();
    //For pending messages, response with errors to those waiting for a response
    for(unsigned int i=0;i<this->eventBuffer.size();i++)
    {
        std::tr1::shared_ptr<class Event> ev = this->eventBuffer[i];
        if(ev->id==0) continue; //This don't need a specific response
        if(ev->type=="RECEIVER_DELETED") continue; //This don't need a specific response

        std::tr1::shared_ptr<class Event> err(new Event("RECEIVER_DELETED"));
        err->id = ev->id;
        err->fromUuid = this->threadId;

        //This mutex stuff is so a receiver and receive events from this response flush
        this->mutex.unlock();
        this->el->SendEvent(err);
        this->mutex.lock();
    }
    this->eventBuffer.clear();
    this->mutex.unlock();
}

//*****************************************************

EventLoop::EventLoop()
{
    this->nextId = 1; //Don't use ID zero, that refers to an unknown ID
}

EventLoop::~EventLoop()
{
    cout << "EventLoop::~EventLoop()" << endl;
    this->mutex.lock();
    std::map<QString, std::vector<EventReceiver *> >::iterator it =
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

unsigned int EventLoop::SendEvent(std::tr1::shared_ptr<class Event> event)
{
    assert(this!=NULL);
    //cout << "Sent event "<< event->type << endl;
    //Get a local copy of listeners
    this->mutex.lock();

    std::map<QString, std::vector<EventReceiver *> >::iterator it =
            this->eventReceivers.find(event->type);
    if(it == this->eventReceivers.end())
    {
        cout << "Warning: No listeners for event " << qPrintable(event->type) << endl;
        //No listeners found
        this->mutex.unlock();
        return 0;
    }
    std::vector<EventReceiver *> eventListeners = it->second;
    this->mutex.unlock();

    //Dispatch message to listeners
    unsigned int countUuidMatches = 0;
    for(unsigned i=0;i<eventListeners.size();i++)
    {
        //Add message to receiver queue
        EventReceiver *receiver = eventListeners[i];
        int addRet = receiver->AddMessage(event);
        if(addRet==0)
        {
            //Add event failed for this receiver
            //It may be stopping
            continue;
        }

        //If a matching uuid received the message, increase count
        if(receiver->GetThreadId() == event->toUuid)
            countUuidMatches ++;

        if(event->type=="STOP_THREADS")
            cout << "Dispatch STOP_THREADS to " << (unsigned long)eventListeners[i] << endl;
    }

    return countUuidMatches;
}

void EventLoop::AddListener(QString type, class EventReceiver &rx)
{
    //cout << "Add listener " << type.c_str() << "," << (unsigned long)&rx << endl;
    assert(this!=NULL);

    this->mutex.lock();
    std::map<QString, std::vector<EventReceiver *> >::iterator it = this->eventReceivers.find(type);
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
    std::map<QString, std::vector<EventReceiver *> >::iterator it =
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

//************************************

MessagableThread::MessagableThread()
{
    this->eventReceiver = NULL;
    this->stopThreads = 0;
    this->eventLoop = NULL;
    this->threadId = QUuid::createUuid();
}

MessagableThread::~MessagableThread()
{
    cout << "MessagableThread::~MessagableThread() "
         << (unsigned long) this << endl;
    this->Stop();
    if(this->eventReceiver)
        delete this->eventReceiver;
    this->eventReceiver = NULL;
}

void MessagableThread::SetEventLoop(class EventLoop *eventLoopIn)
{
    if(this->eventReceiver!=NULL)
        delete this->eventReceiver;
    this->eventReceiver = new EventReceiver(eventLoopIn,__FILE__,__LINE__);
    this->eventReceiver->SetThreadId(this->threadId);
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("STOP_THREADS", *eventReceiver);
}

void MessagableThread::run()
{
    assert(this->eventLoop!=NULL);
    std::tr1::shared_ptr<class Event> startEvent (new Event("THREAD_STARTING"));
    this->eventLoop->SendEvent(startEvent);

    int running = true;

    while(running)
    {
        this->mutex.lock();
        running = !this->stopThreads;
        this->mutex.unlock();

        int flush=1;
        while(flush)
        {
        try
        {
            assert(this->eventReceiver);
            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
            cout << "Event type " << qPrintable(ev->type) << "," << ev->id << endl;
            this->HandleEvent(ev);
        }
        catch(std::runtime_error e)
        {
            //No message was waiting or handle event threw exception
            flush = 0;
        }
        }

        //Check again if running
        this->mutex.lock();
        running = !this->stopThreads;
        this->mutex.unlock();

        //Call child specific update function
        try
        {
            if(running) this->Update();
        }
        catch(std::runtime_error e)
        {
            cout << "Error: thread update threw an exception" << endl;
            cout << e.what() << endl;
        }
    }

    std::tr1::shared_ptr<class Event> stopEvent(new Event("THREAD_STOPPING"));
    this->eventLoop->SendEvent(stopEvent);
    this->Finished();
}

void MessagableThread::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    QString evType = ev->type;
    if(evType == "STOP_THREADS")
    {
        this->mutex.lock();
        this->stopThreads = 1;
        this->eventReceiver->Stop();
        this->mutex.unlock();
    }
}

int MessagableThread::Start()
{
    if(this->isRunning()) return 0;
    this->mutex.lock();
    this->stopThreads = 0;
    this->mutex.unlock();
    this->start();
    return 1;
}

int MessagableThread::Stop()
{
    this->mutex.lock();
    this->stopThreads = 1;
    this->mutex.unlock();

    std::tr1::shared_ptr<class Event> stopReq(new Event("STOP_SPECIFIC_THREAD"));
    stopReq->toUuid = this->threadId;
    this->eventReceiver->Stop();
    this->eventLoop->SendEvent(stopReq);

    for(int i=0;i<500;i++)
    {
        if(this->isFinished())
        {
            return 1;
        }
        usleep(10000); //microsec
    }
    if(this->isRunning())
    {
        cout << "Warning: terminating messagable thread" << endl;
        this->terminate();
    }
    return 0;
}

void MessagableThread::StopNonBlocking()
{
    this->mutex.lock();
    this->stopThreads = 1;
    this->mutex.unlock();
}

int MessagableThread::IsStopFlagged()
{
    this->mutex.lock();
    int out = this->stopThreads;
    this->mutex.unlock();
    return out;
}

void MessagableThread::start (Priority priority)
{
    QThread::start(priority);
}

void MessagableThread::SetThreadId(QUuid idIn)
{
    this->mutex.lock();
    this->threadId = idIn;
    assert(this->eventReceiver!=NULL); //Remember to use SetEventLoop first!
    this->eventReceiver->SetThreadId(idIn);
    this->mutex.unlock();
}

//**************************************************

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

