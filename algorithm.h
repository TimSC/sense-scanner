#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "eventloop.h"
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QUuid>
#include <vector>

class AlgorithmProcess : public QProcess
{
public:
    AlgorithmProcess(class EventLoop *eventLoopIn, QObject *parent);
    virtual ~AlgorithmProcess();

    enum ProcessState
    {
        PAUSED = 1,
        STARTING = 2,
        RUNNING = 3,
        RUNNING_PAUSING = 4,
        RUNNING_STOPPING = 5,
        STOPPED = 6
    };

    void Init();
    int Stop();
    void StopNonBlocking();
    int Start();
    int IsStopFlagged();
    void SetId(unsigned int idIn);
    ProcessState GetState();
    QString ReadLineFromBuffer(QByteArray &buff);
    void Update();
    void ProcessAlgOutput(QString &cmd);
    void Pause();
    void Unpause();
    void SendCommand(QString cmd);
    void SendRawData(QByteArray cmd);
    unsigned int EncodedLength(QString cmd);
    QByteArray GetModel();
    QUuid GetUid();
    void SetUid(QUuid newUid);
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);

protected:
    int stopping;
    int pausing;
    int paused;
    unsigned int threadId;
    int initDone;
    QFile *algOutLog;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

    QByteArray dataBlock;
    int dataBlockReceived;

    QByteArray algOutBuffer;
    QByteArray algErrBuffer;
    QUuid uid;
};

#endif // ALGORITHM_H
