#include "algorithm.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <QtCore/QTextStream>
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

}

AlgorithmProcess::~AlgorithmProcess()
{
    this->Stop();
}

int AlgorithmProcess::Stop()
{
    this->write("QUIT\n");
    this->waitForFinished();
}

void AlgorithmProcess::StopNonBlocking()
{
    this->write("QUIT\n");

}

int AlgorithmProcess::Start()
{
    QString program = "/home/tim/dev/QtMedia/echosrv";
    QStringList arguments;
    this->start(program, arguments);

    //this->waitForFinished();

}

int AlgorithmProcess::IsStopFlagged()
{
    return this->stopping;
}

void AlgorithmProcess::SetId(unsigned int idIn)
{

    this->threadId = idIn;
}

bool AlgorithmProcess::isRunning()
{
    return (this->state() == QProcess::Running);
}

void AlgorithmProcess::Update()
{
    QByteArray ret = this->readAllStandardOutput();

    QTextStream dec(&ret);
    dec.setCodec("UTF-8");
    QString line;
    do
    {
        line = dec.readLine();
        if(line.length()>0)
            cout << line.toLocal8Bit().constData() << endl;
    }
    while (!line.isNull());
}
