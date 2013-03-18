#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <QtCore/QThread>
#include <QtCore/QUuid>
#include <QtCore/QtCore>
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

class BinaryData : public Deletable
{
    /*!
    * A generic byte buffer than can be passed though the message system
    */

public:
    BinaryData()
    {
        this->raw = NULL;
        this->size = 0;
    }

    virtual ~BinaryData()
    {
        this->Clear();
    }

    void Copy(const unsigned char *buff, unsigned int buffSize)
    {
        if(this->raw!=NULL) delete [] this->raw;
        this->size = buffSize;
        this->raw = new unsigned char [this->size];
        memcpy(this->raw, buff, this->size);
    }

    void Clear()
    {
        if(this->raw!=NULL) delete [] this->raw;
        this->size = 0;
    }

    unsigned char *raw;
    unsigned int size;
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
    Event(QString typeIn, unsigned long long idIn = 0);
    Event(const Event& other);
    virtual ~Event();

    QString type;
    unsigned long long id;

    //String data
    QString data;
    QByteArray buffer;

    //Pointer to a custom class that contains additional data
    class Deletable *raw;

    QUuid toUuid;
    QUuid fromUuid;
};

class EventReceiver
{
    /*!
    * EventReciever is a FIFO buffer that holds messages received from
    * the event system until some thread requests the information. They
    * must be registered with the EventLoop object to receive messages.
    */

public:
    EventReceiver(class EventLoop *elIn, const char *filenameIn, unsigned int lineIn);
    virtual ~EventReceiver();
    void AddMessage(std::tr1::shared_ptr<class Event> event);
    int BufferSize();
    void MessageLoopDeleted();
    std::tr1::shared_ptr<class Event> PopEvent();
    std::tr1::shared_ptr<class Event> GetLatestDiscardOlder(QString type, QUuid filterUuid);
    std::tr1::shared_ptr<class Event> WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 50000);
    void SetThreadId(QUuid idIn);
    QUuid GetThreadId();
    void Stop();

protected:
    void RespondToBufferedRequests(); //Used to clear buffer before shut down

    std::vector<std::tr1::shared_ptr<class Event> > eventBuffer;
    Mutex mutex;
    class EventLoop *el;
    QUuid threadId;
    bool stopping;
    QString filename;
    unsigned int line;
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

    unsigned int SendEvent(std::tr1::shared_ptr<class Event> event);
    void AddListener(QString, class EventReceiver &rx);
    void RemoveListener(class EventReceiver &rx);

    //! This generates a unique message ID number that can be used to wait
    //! for message responses. A request and a response can share an ID.
    unsigned long long GetId();

protected:
    std::map<QString, std::vector<EventReceiver *> > eventReceivers;
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
    //void SetId(int idIn);
    //int GetId();
    virtual void Finished() {};
    void SetThreadId(QUuid idIn);

protected:
    void start (Priority priority = InheritPriority);

    class EventReceiver *eventReceiver;
    class EventLoop *eventLoop;
    int stopThreads;    
    Mutex mutex;
    QUuid threadId;
    //int id;
};

/*!
* Convenience functions for splitting a string based on a specified delimiter
*/

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

#endif // EVENTLOOP_H
