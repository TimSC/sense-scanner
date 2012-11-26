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
    Event(std::string typeIn);

    std::string type;
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
    void AddListener(std::string type, class EventReceiver &rx);

protected:
    std::map<std::string, std::vector<EventReceiver *> > eventReceivers;

};

#endif // EVENTLOOP_H
