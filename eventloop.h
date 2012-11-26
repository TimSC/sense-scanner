#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <map>
#include <mutex>

class Event
{
public:

    enum Type
    {
        EVENT_UNKNOWN = 0,
        EVENT_GET_FRAME = 1,
        EVENT_FOUND_FRAME = 10,
        EVENT_STOP_THREADS = 100,
        EVENT_THREAD_STARTING = 101,
        EVENT_THREAD_STOPPING = 102
    };

    Event();
    Event(Event::Type typeIn);

    Type type;
};

class EventReceiver
{
public:
    EventReceiver();
    void AddMessage(const class Event &event);
    int BufferSize();
    class Event PopEvent();


    std::vector<class Event> eventBuffer;
    std::mutex mutex;
};


class EventLoop
{
public:
    EventLoop();
    void SendEvent(const class Event &event);
    void AddListener(Event::Type type, class EventReceiver &rx);

protected:
    std::map<Event::Type, std::vector<EventReceiver *> > eventReceivers;

};

#endif // EVENTLOOP_H
