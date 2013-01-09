#include "algorithm.h"
#include "localsleep.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
#include <QtCore/QTextStream>
using namespace std;

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
    this->eventReceiver = new class EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("PREDICT_FRAME_REQUEST", *this->eventReceiver);
}

AlgorithmProcess::~AlgorithmProcess()
{
    this->Stop();
    if(this->eventReceiver != NULL)
        delete this->eventReceiver;
    this->eventReceiver = NULL;
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

QString AlgorithmProcess::ReadLineFromBuffer(QByteArray &buff)
{
    //Check if a new line has occured
    int newLinePos = -1;
    for(unsigned i=0;i<buff.length();i++)
    {
        if(buff[i]=='\n' && newLinePos == -1)
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
    QString cmd = buff.left(newLinePos);
    buff = buff.mid(newLinePos+1);

    if(buff != NULL)
    {
        this->algOutLog->write(cmd.toLocal8Bit().constData());
        this->algOutLog->write("\n");
        this->algOutLog->flush();
    }

    return cmd;
}

void AlgorithmProcess::Update()
{
    //Process events from application
    int flushEvents = 1;
    while(flushEvents)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
        this->HandleEvent(ev);
    }
    catch(std::runtime_error e)
    {
        flushEvents = 0;
    }

    //Get standard output from algorithm process
    QByteArray ret = this->readAllStandardOutput();
    this->algOutBuffer.append(ret);

    while(true)
    {
        QString cmd = ReadLineFromBuffer(this->algOutBuffer);
        if(cmd.length() == 0) break;
        this->ProcessAlgOutput(cmd);
    }

    //Get errors from console error out
    ret = this->readAllStandardError();
    this->algErrBuffer.append(ret);

    while(true)
    {
        QString err = ReadLineFromBuffer(this->algErrBuffer);
        if(err.length() == 0) break;
        cout << "Algorithm Error: " << err.toLocal8Bit().constData() << endl;
    }


}

void AlgorithmProcess::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    cout << "Event type " << ev->type << endl;
    if(QString(ev->data.c_str()) != this->uid.toString()) return; //Check if this is the appropriate algorithm

    cout << ev->data.c_str() << endl;
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
        return;
    }

    if(cmd=="NOW_PAUSED")
    {
        this->paused = 1;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        std::ostringstream tmp;
        tmp << this->threadId << ",paused";
        openEv->data = tmp.str();
        el.SendEvent(openEv);
        return;
    }

    if(cmd=="NOW_RUNNING")
    {
        this->paused = 0;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        std::ostringstream tmp;
        tmp << this->threadId << ",running";
        openEv->data = tmp.str();
        el.SendEvent(openEv);
        return;
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
        return;
    }

    if(cmd.left(11)=="DATA_BLOCK=")
    {
        QString blockArg = this->ReadLineFromBuffer(this->algOutBuffer);
        while(blockArg.length()==0)
        {
            blockArg = this->ReadLineFromBuffer(this->algOutBuffer);
        }
        int blockLen = cmd.mid(11).toInt();

        //Wait for process to write the entire data block to standard output
        while(this->algOutBuffer.length() < blockLen)
        {
            //Get standard output from algorithm process
            QByteArray ret = this->readAllStandardOutput();
            this->algOutBuffer.append(ret);
            //int currentLen = this->algOutBuffer.length();
        }

        QByteArray blockData = this->algOutBuffer.left(blockLen);
        this->algOutBuffer = this->algOutBuffer.mid(blockLen);

        //std::tr1::shared_ptr<class Event> dataEv(new Event("ALG_DATA_BLOCK"));
        //dataEv->data = blockData.constData();
        //el.SendEvent(dataEv);
        this->dataBlock = blockData;
        this->dataBlockReceived = 1;
        return;
    }

    if(cmd.length()>0)
        cout << "Algorithm: " << cmd.toLocal8Bit().constData() << endl;

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
    int count = 0;
    while(!this->dataBlockReceived)
    {
        this->waitForFinished(100);
        this->Update();
        count ++;
    }
    if(!this->dataBlockReceived)
    {
        throw std::runtime_error("Algorithm process timed out during GetModel()");
    }

    QByteArray out;
    out.append(this->dataBlock);
    int currentLen = out.length();
    return out;
}

QUuid AlgorithmProcess::GetUid()
{
    return this->uid;
}

void AlgorithmProcess::SetUid(QUuid newUid)
{
    this->uid = newUid;
}

