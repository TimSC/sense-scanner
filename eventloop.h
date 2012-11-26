#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <map>
#include <mutex>
#include <string>

class Event
{
public:
    Event();
    Event(std::string typeIn, unsigned long long idIn = 0);
    Event(const Event& other);

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
    void AddMessage(const class Event &event);
    int BufferSize();
    class Event PopEvent();
    class Event WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 5000);

protected:
    std::vector<class Event> eventBuffer;
    std::mutex mutex;
};


class EventLoop
{
public:
    EventLoop();
    void SendEvent(const class Event &event);
    void AddListener(std::string type, class EventReceiver &rx);
    unsigned long long GetId();

protected:
    std::map<std::string, std::vector<EventReceiver *> > eventReceivers;
    std::mutex mutex;
    unsigned long long nextId;
};

#endif // EVENTLOOP_H
