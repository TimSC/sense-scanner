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

    int Stop();
    void StopNonBlocking();
    int Start();
    int IsStopFlagged();
    void SetId(unsigned int idIn);
    bool isRunning();

protected:
    int stopping;
    unsigned int threadId;
};

#endif // ALGORITHM_H
