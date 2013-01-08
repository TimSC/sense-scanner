#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <QtCore/QThread>
#include <map>
#include "localmutex.h"
#include "localints.h"
#include <string>
#ifdef _MSC_VER
	#include <memory>
#else
	#include <tr1/memory>
#endif

class Deletable
{
public:
    Deletable() {};
    virtual ~Deletable() {};
};

class Event
{
public:
    Event();
    Event(std::string typeIn, unsigned long long idIn = 0);
    Event(const Event& other);
    virtual ~Event();

    std::string type;
    unsigned long long id;

    //String data
    std::string data;

    //Raw binary data
    uint8_t *raw;
    unsigned long long rawSize;
};

class EventReceiver
{
public:
    EventReceiver(class EventLoop *elIn);
    virtual ~EventReceiver();
    void AddMessage(std::tr1::shared_ptr<class Event> event);
    int BufferSize();
    void MessageLoopDeleted();
    std::tr1::shared_ptr<class Event> PopEvent();
    std::tr1::shared_ptr<class Event> WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 50000);

protected:
    std::vector<std::tr1::shared_ptr<class Event> > eventBuffer;
    Mutex mutex;
    class EventLoop *el;
};


class EventLoop
{
public:
    EventLoop();
    virtual ~EventLoop();

    void SendEvent(std::tr1::shared_ptr<class Event> event);
    void AddListener(std::string type, class EventReceiver &rx);
    void RemoveListener(class EventReceiver &rx);
    unsigned long long GetId();

protected:
    std::map<std::string, std::vector<EventReceiver *> > eventReceivers;
    Mutex mutex;
    unsigned long long nextId;
};

class MessagableThread : public QThread
{
public:
    MessagableThread();
    virtual ~MessagableThread();

    void SetEventLoop(class EventLoop *eventLoopIn);
    void run();
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    int Stop();
    void StopNonBlocking();
    int Start();
    int IsStopFlagged();
    virtual void Update()=0;
    void SetId(int idIn);
    int GetId();

protected:
    void start (Priority priority = InheritPriority);

    class EventReceiver *eventReceiver;
    int stopThreads;
    class EventLoop *eventLoop;
    Mutex mutex;
    int id;
};

#endif // EVENTLOOP_H
