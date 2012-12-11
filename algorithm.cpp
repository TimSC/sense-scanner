#include "algorithm.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
using namespace std;

//**********************************

AlgorithmThread::AlgorithmThread(class EventLoop *eventLoopIn, QObject *parent) : MessagableThread(eventLoopIn)
{
    this->progress = 0.f;
    this->threadId = 0;
}

AlgorithmThread::~AlgorithmThread()
{

}

void AlgorithmThread::Update()
{
    this->progress += 0.01f;
    if(this->progress >= 1.f)
    {
       this->progress = 1.f;
       this->mutex.lock();
       this->stopThreads = 1;
       this->mutex.unlock();
    }

    assert(this->eventLoop);
    std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_PROGRESS_UPDATE"));
    std::ostringstream tmp;
    tmp << this->progress << "," << this->threadId;
    openEv->data = tmp.str();
    this->eventLoop->SendEvent(openEv);

    this->msleep(10000);
}
void AlgorithmThread::SetId(unsigned int idIn)
{
    assert(!this->isRunning());
    this->threadId = idIn;
}

//************************************************

AlgorithmProcess::AlgorithmProcess(class EventLoop *eventLoopIn, QObject *parent) : QProcess(parent)
{
    this->stopping = 0;
    this->paused = 0;
    this->pausing = 0;
    this->initDone = 0;
}

AlgorithmProcess::~AlgorithmProcess()
{
    this->Stop();
}

void AlgorithmProcess::Init()
{
    if(this->initDone) return;
    QString program = "../QtMedia/echosrv";
    QFile programFile(program);
    if(!programFile.exists())
    {
        throw std::runtime_error("Process executable not found");
    }
    QStringList arguments;
    this->start(program, arguments);
    this->initDone = 1;
}

void AlgorithmProcess::Pause()
{
    assert(this->initDone);
    this->pausing = 1;
    this->SendCommand("PAUSE\n");
}

void AlgorithmProcess::Unpause()
{
    assert(this->initDone);
    this->pausing = 0;
    this->stopping = 0;
    this->SendCommand("RUN\n");
}

int AlgorithmProcess::Stop()
{
    if(this->state() != QProcess::Running)
        return 0;
    assert(this->initDone);
    this->pausing = 0;
    this->stopping = 1;
    this->SendCommand("QUIT\n");
    this->waitForFinished();
    return 1;
}

void AlgorithmProcess::StopNonBlocking()
{
    assert(this->initDone);
    this->pausing = 0;
    this->stopping = 1;
    this->SendCommand("QUIT\n");
}

int AlgorithmProcess::Start()
{
    Init();
    this->pausing = 0;
    this->stopping = 0;
    assert(this->state() != QProcess::Running);
    this->SendCommand("RUN\n");
    //this->waitForFinished();
    return 1;
}

int AlgorithmProcess::IsStopFlagged()
{
    return this->stopping;
}

void AlgorithmProcess::SetId(unsigned int idIn)
{

    this->threadId = idIn;
}

AlgorithmProcess::ProcessState AlgorithmProcess::GetState()
{
    int running = (this->state() == QProcess::Running);
    if(!running) return AlgorithmProcess::STOPPED;
    if(this->paused) return AlgorithmProcess::PAUSED;
    if(this->stopping) return AlgorithmProcess::RUNNING_STOPPING;
    if(this->pausing) return AlgorithmProcess::RUNNING_PAUSING;
    return AlgorithmProcess::RUNNING;
}

void AlgorithmProcess::Update(class EventLoop &el)
{
    QByteArray ret = this->readAllStandardOutput();

    QTextStream dec(&ret);
    dec.setCodec("UTF-8");
    QString line;
    do
    {
        line = dec.readLine();
        if(line.length()==0) continue;
        if(line.left(9)=="PROGRESS=")
        {
            std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_PROGRESS_UPDATE"));
            std::ostringstream tmp;
            tmp << line.mid(9).toLocal8Bit().constData() << "," << this->threadId;
            openEv->data = tmp.str();
            el.SendEvent(openEv);
        }
        if(line=="NOW_PAUSED")
        {
            this->paused = 1;

            std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
            std::ostringstream tmp;
            tmp << this->threadId << ",paused";
            openEv->data = tmp.str();
            el.SendEvent(openEv);
        }
        if(line=="NOW_RUNNING")
        {
            this->paused = 0;

            std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
            std::ostringstream tmp;
            tmp << this->threadId << ",running";
            openEv->data = tmp.str();
            el.SendEvent(openEv);
        }

        if(line=="FINISHED")
        {
            this->waitForFinished(); //Just wait for final finishing of process
            this->initDone = 0;

            std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
            std::ostringstream tmp;
            tmp << this->threadId << ",finished";
            openEv->data = tmp.str();
            el.SendEvent(openEv);
        }

        //if(line.length()>0)
        //    cout << line.toLocal8Bit().constData() << endl;
    }
    while (!line.isNull());
}

void AlgorithmProcess::SendCommand(QString cmd)
{
    assert(this->initDone);
    QTextStream enc(this);
    enc.setCodec("UTF-8");
    enc << cmd;
}
