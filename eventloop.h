#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <QtCore/QThread>
#include <QtCore/QUuid>
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
    /*!
    * This base class is used to derive classes that are later to
    * be deallocated by the event system. The most important point
    * is that the destructor is virtual.
    */

public:
    Deletable() {};
    virtual ~Deletable() {};
};

class Event
{
    /*!
    * Event contains a message that is send to one or more EventReceivers.
    * This facilitates communication between modules without more
    * complex interdependency.
    */

public:
    Event();
    Event(std::string typeIn, unsigned long long idIn = 0);
    Event(const Event& other);
    virtual ~Event();

    std::string type;
    unsigned long long id;

    //String data
    std::string data;

    //Pointer to a custom class that contains additional data
    class Deletable *raw;

};

class EventReceiver
{
    /*!
    * EventReciever is a FIFO buffer that holds messages received from
    * the event system until some thread requests the information. They
    * must be registered with the EventLoop object to receive messages.
    */

public:
    EventReceiver(class EventLoop *elIn);
    virtual ~EventReceiver();
    void AddMessage(std::tr1::shared_ptr<class Event> event);
    int BufferSize();
    void MessageLoopDeleted();
    std::tr1::shared_ptr<class Event> PopEvent();
    std::tr1::shared_ptr<class Event> WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 50000);
    void SetThreadId(QUuid idIn);

protected:
    std::vector<std::tr1::shared_ptr<class Event> > eventBuffer;
    Mutex mutex;
    class EventLoop *el;
    QUuid threadId;
};


class EventLoop
{
public:
    /*!
    * The event loop is an object to dispatch Event messages to pre-registered
    * EventReceivers, based on the Event type.
    */

    EventLoop();
    virtual ~EventLoop();

    void SendEvent(std::tr1::shared_ptr<class Event> event);
    void AddListener(std::string type, class EventReceiver &rx);
    void RemoveListener(class EventReceiver &rx);

    //! This generates a unique message ID number that can be used to wait
    //! for message responses. A request and a response can share an ID.
    unsigned long long GetId();

protected:
    std::map<std::string, std::vector<EventReceiver *> > eventReceivers;
    Mutex mutex;
    unsigned long long nextId;
};

class MessagableThread : public QThread
{
    /*!
    * MessagableThread is a worker thread with an event receiver and
    * convenience functions to start and stop the thread. Other worker
    * threads are derived from this base thread.
    */

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
    virtual void Finished()=0;
    void SetThreadId(QUuid idIn);

protected:
    void start (Priority priority = InheritPriority);

    class EventReceiver *eventReceiver;
    int stopThreads;
    class EventLoop *eventLoop;
    Mutex mutex;
    QUuid threadId;
    int id;
};

/*!
* Convenience functions for splitting a string based on a specified delimiter
*/

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

#endif // EVENTLOOP_H
