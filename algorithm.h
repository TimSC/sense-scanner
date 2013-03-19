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
        RUNNING_PREPARING = 3,
        RUNNING = 4, //Training
        RUNNING_PAUSING = 5,
        RUNNING_STOPPING = 6,
        STOPPED = 7, //Failed?
        READY = 8 //Ready to make prediction
    };

    QUuid GetUid();
    void SetUid(QUuid newUid);
    void Init();
    int Start();
    int Stop();
    void StopNonBlocking();
    int IsBlockingShutdown();

    static int PredictFrame(QSharedPointer<QImage> img,
                            std::vector<std::vector<float> > &model,
                            QUuid algUuid,
                            QUuid annotUuid,
                            class EventLoop *eventLoop,
                            class EventReceiver *eventReceiver,
                            std::vector<std::vector<float> > &out);

    //This version is for all threads except the main thread
    static ProcessState GetState(QUuid algUuid,
                          class EventLoop *eventLoop,
                          class EventReceiver *eventReceiver);

    //This is safe for the main thread to call, but no other thread may do so
    ProcessState GetState();

    static int IsReadyToWork(QUuid algUuid,
                          class EventLoop *eventLoop,
                          class EventReceiver *eventReceiver);

public slots:
    void Update();

    void StdOutReady();
    void StdErrReady();

    void ProcessStateChanged(QProcess::ProcessState newState);

protected:
    int IsStopFlagged();

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
    int dataLoaded;
    int initDone;
    QFile *algOutLog;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

    QByteArray algOutBuffer;
    QByteArray algErrBuffer;
    QUuid uid;
    QTimer timer;
    double progress;
    QList<unsigned long long> saveModelRequestIds;
    QTime keepAliveTimer;
    std::map<unsigned long long, std::tr1::shared_ptr<class Event> > requestsPending;
};

class ProcessingRequestOrResponse : public Deletable
{
    /*!
    * DecodedFrame contains a video frame and a tracking point model.
    * This is used for requesting a tracking position and returning the
    * result to the GUI.
    */

public:
    QSharedPointer<QImage> img;

    std::vector<std::vector<std::vector<float> > > pos;

    ProcessingRequestOrResponse();
    ProcessingRequestOrResponse(const ProcessingRequestOrResponse &other);
    ProcessingRequestOrResponse& operator=(const ProcessingRequestOrResponse& other);
    virtual ~ProcessingRequestOrResponse();
};


#endif // ALGORITHM_H
