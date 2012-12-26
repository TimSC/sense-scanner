#include "algorithm.h"
#include "localsleep.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
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
    this->paused = 1;
    this->pausing = 0;
    this->initDone = 0;
    QString fina = "algout.txt";
    this->eventLoop = eventLoopIn;
    this->algOutLog = new QFile(fina);
    this->algOutLog->open(QIODevice::WriteOnly);
}

AlgorithmProcess::~AlgorithmProcess()
{
    this->Stop();
}

void AlgorithmProcess::Init()
{
    if(this->initDone) return;
    assert(this->state() != QProcess::Running);

    QString program = "/usr/bin/python";
    QFile programFile(program);
    if(!programFile.exists())
    {
        throw std::runtime_error("Process executable not found");
    }
    QStringList arguments;
    arguments.append("../QtMedia/echosrv.py");
    this->start(program, arguments);
    this->stopping = 0;
    this->paused = 1;
    this->pausing = 0;
    this->initDone = 1;
    this->dataBlockReceived = 0;
}

void AlgorithmProcess::Pause()
{
    this->pausing = 1;
    this->SendCommand("PAUSE\n");
}

void AlgorithmProcess::Unpause()
{
    if(!this->paused) return;
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
    if(!this->paused) return 0;
    this->pausing = 0;
    this->stopping = 0;
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
    if(this->state() == QProcess::Starting)
        return AlgorithmProcess::STARTING;
    int running = (this->state() == QProcess::Running);
    if(!running)
    {
        this->stopping = 0;
        this->paused = 1;
        return AlgorithmProcess::STOPPED;
    }
    if(this->paused) return AlgorithmProcess::PAUSED;
    if(this->stopping) return AlgorithmProcess::RUNNING_STOPPING;
    if(this->pausing) return AlgorithmProcess::RUNNING_PAUSING;
    return AlgorithmProcess::RUNNING;
}

QString AlgorithmProcess::ReadLineFromBuffer()
{
    //Check if a new line has occured
    int newLinePos = -1;
    for(unsigned i=0;i<this->algOutBuffer.length();i++)
    {
        if(this->algOutBuffer[i]=='\n' && newLinePos == -1)
        {
            newLinePos = i;
        }
    }

    //If no new line found, so return
    if(newLinePos==-1)
    {
        QString empty;
        return empty;
    }

    //Extract a string for recent command
    QString cmd = this->algOutBuffer.left(newLinePos);
    this->algOutBuffer = this->algOutBuffer.mid(newLinePos+1);

    if(this->algOutLog != NULL)
    {
        this->algOutLog->write(cmd.toLocal8Bit().constData());
        this->algOutLog->write("\n");
        this->algOutLog->flush();
    }

    return cmd;
}

void AlgorithmProcess::Update()
{
    //Get standard output from algorithm process
    QByteArray ret = this->readAllStandardOutput();
    if(ret.length()>0)
    {
        cout << "Read from alg " << ret.length() << endl;
    }
    this->algOutBuffer.append(ret);

    while(true)
    {
        QString cmd = ReadLineFromBuffer();
        if(cmd.length() == 0) return;
        this->ProcessAlgOutput(cmd);
    }
}

void AlgorithmProcess::ProcessAlgOutput(QString &cmd)
{
    class EventLoop &el = *this->eventLoop;
    if(cmd.length()==0) return;

    if(cmd.left(9)=="PROGRESS=")
    {
        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_PROGRESS_UPDATE"));
        std::ostringstream tmp;
        tmp << cmd.mid(9).toLocal8Bit().constData() << "," << this->threadId;
        openEv->data = tmp.str();
        el.SendEvent(openEv);
    }

    if(cmd=="NOW_PAUSED")
    {
        this->paused = 1;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        std::ostringstream tmp;
        tmp << this->threadId << ",paused";
        openEv->data = tmp.str();
        el.SendEvent(openEv);
    }

    if(cmd=="NOW_RUNNING")
    {
        this->paused = 0;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        std::ostringstream tmp;
        tmp << this->threadId << ",running";
        openEv->data = tmp.str();
        el.SendEvent(openEv);
    }

    if(cmd=="FINISHED")
    {
        this->SendCommand("BYE\n");
        this->waitForFinished(); //Just wait for final finishing of process
        this->initDone = 0;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        std::ostringstream tmp;
        tmp << this->threadId << ",finished";
        openEv->data = tmp.str();
        el.SendEvent(openEv);
    }

    if(cmd.left(11)=="DATA_BLOCK=")
    {
        cout << cmd.toLocal8Bit().constData() << endl;
        /*QString dataBlockArgs = dec.readLine();
        if(this->algOutLog != NULL)
            this->algOutLog->write(dataBlockArgs.toLocal8Bit().constData());
        int blockLen = line.mid(11).toInt();
        QString blockData = dec.read(blockLen);

        //std::tr1::shared_ptr<class Event> dataEv(new Event("ALG_DATA_BLOCK"));
        //dataEv->data = blockData.constData();
        //el.SendEvent(dataEv);
        this->dataBlock = blockData;
        this->dataBlockReceived = 1;*/
    }

    //if(line.length()>0)
    //    cout << line.toLocal8Bit().constData() << endl;

}

void AlgorithmProcess::SendCommand(QString cmd)
{
    QProcess::ProcessState state = this->state();
    if(state == QProcess::Starting)
    {
        //Wait for program to be ready
        this->waitForStarted();
        state = this->state();
    }
    int running = (state == QProcess::Running);
    if(!running) return;
    assert(this->initDone);
    QTextStream enc(this);
    enc.setCodec("UTF-8");
    enc << cmd;
    enc.flush();
}

void AlgorithmProcess::SendRawData(QByteArray cmd)
{
    int running = (this->state() == QProcess::Running);
    if(!running) return;
    assert(this->initDone);
    this->write(cmd);
}

unsigned int AlgorithmProcess::EncodedLength(QString cmd)
{
    QString encodedCmd;
    QTextStream enc(&encodedCmd);
    enc.setCodec("UTF-8");
    enc << cmd;
    return encodedCmd.length();
}

QByteArray AlgorithmProcess::GetModel()
{
    //Send request to algorithm process
    assert(this->paused);
    this->dataBlock = "";
    this->dataBlockReceived = 0;
    this->SendCommand("SAVE_MODEL\n");

    //Wait for response
    this->waitForBytesWritten(10000);
    int count = 0;
    while(!this->dataBlockReceived && count < 300)
    {
        this->Update();
        count ++;
    }
    if(!this->dataBlockReceived)
    {
        throw std::runtime_error("Algorithm process timed out during GetModel()");
    }

    QByteArray out;
    out.append(this->dataBlock);
    return out;
}
