#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <map>
#include <mutex>
#include <string>
#include <tr1/memory>

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
    EventReceiver();
    void AddMessage(std::tr1::shared_ptr<class Event> event);
    int BufferSize();
    std::tr1::shared_ptr<class Event> PopEvent();
    std::tr1::shared_ptr<class Event> WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 5000);

protected:
    std::vector<std::tr1::shared_ptr<class Event> > eventBuffer;
    std::mutex mutex;
};


class EventLoop
{
public:
    EventLoop();
    void SendEvent(std::tr1::shared_ptr<class Event> event);
    void AddListener(std::string type, class EventReceiver &rx);
    unsigned long long GetId();

protected:
    std::map<std::string, std::vector<EventReceiver *> > eventReceivers;
    std::mutex mutex;
    unsigned long long nextId;
};

#endif // EVENTLOOP_H
