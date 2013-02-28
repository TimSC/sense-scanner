#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "eventloop.h"
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QUuid>
#include <QtCore/QTimer>
#include <vector>
#include "localmutex.h"

class AlgorithmProcess : public QProcess
{
    /*!
    * AlgorithmProcess is a facade for a separate process. It enables easy
    * stopping and starting, as well as handling serialisation of events and
    * handling of the process standard out (console) response.
    *
    * The GUI thread should periodically call update to process event messages.
    */
    Q_OBJECT

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

    QUuid GetUid();
    void SetUid(QUuid newUid);
    void Init();
    int Start();
    int Stop();
    void StopNonBlocking();
    int IsBlockingShutdown();

public slots:
    void Update();

protected:
    int IsStopFlagged();
    ProcessState GetState();
    QByteArray ReadLineFromBuffer(QByteArray &buff, int popLine = 1, int skipLines = 0);
    void ProcessAlgOutput();
    void Pause();
    void Unpause();

    void SendCommand(QString cmd);
    void SendRawDataBlock(QString args, QByteArray data);
    unsigned int EncodedLength(QString cmd);
    void GetModel();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    int stopping;
    int pausing;
    int paused;
    int initDone;
    QFile *algOutLog;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

    QByteArray algOutBuffer;
    QByteArray algErrBuffer;
    QUuid uid;
    QTimer timer;
    QList<unsigned long long> saveModelRequestIds;
};

#endif // ALGORITHM_H
