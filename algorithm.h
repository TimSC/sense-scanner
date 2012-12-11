#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "eventloop.h"
#include <QtCore/QProcess>

class AlgorithmThread : public MessagableThread
{
public:
    AlgorithmThread(class EventLoop *eventLoopIn, QObject *parent);
    virtual ~AlgorithmThread();

    void Update();
    void SetId(unsigned int idIn);

protected:
    float progress;
    unsigned int threadId;
};

class AlgorithmProcess : public QProcess
{
public:
    AlgorithmProcess(class EventLoop *eventLoopIn, QObject *parent);
    virtual ~AlgorithmProcess();

    enum ProcessState
    {
        PAUSED = 1,
        RUNNING = 2,
        RUNNING_PAUSING = 3,
        RUNNING_STOPPING = 4,
        STOPPED = 5
    };

    int Stop();
    void StopNonBlocking();
    int Start();
    int IsStopFlagged();
    void SetId(unsigned int idIn);
    ProcessState GetState();
    void Update(class EventLoop &ev);
    void Pause();
    void Unpause();

protected:
    int stopping;
    int pausing;
    int paused;
    unsigned int threadId;
};

#endif // ALGORITHM_H
