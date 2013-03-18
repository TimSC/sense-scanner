#include "algorithm.h"
#include "localsleep.h"
#include "mediabuffer.h"
#include "version.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
#include <QtCore/QTextStream>
#include <QtCore/QSharedPointer>
#include <QtXml/QtXml>
using namespace std;

//**************************************

ProcessingRequestOrResponse::ProcessingRequestOrResponse()
{
    //cout << "ProcessingRequestOrResponse::ProcessingRequestOrResponse()";
}

ProcessingRequestOrResponse::~ProcessingRequestOrResponse()
{
    //cout << "ProcessingRequestOrResponse::~ProcessingRequestOrResponse()";
}

ProcessingRequestOrResponse::ProcessingRequestOrResponse(const ProcessingRequestOrResponse &other)
{
    operator=(other);
}

ProcessingRequestOrResponse& ProcessingRequestOrResponse::operator=(const ProcessingRequestOrResponse& other)
{
    this->img = other.img; //Smart pointer
    this->pos = other.pos;
    return *this;
}

//************************************************

AlgorithmProcess::AlgorithmProcess(class EventLoop *eventLoopIn, QObject *parent) : QProcess(parent)
{
    this->stopping = 0;
    this->paused = 1;
    this->pausing = 0;
    this->initDone = 0;
    this->dataLoaded = 0;
    this->progress = 0.;

    //QString fina = "algout.txt";
    this->eventLoop = eventLoopIn;
    //this->algOutLog = new QFile(fina);
    //this->algOutLog->open(QIODevice::WriteOnly);
    this->eventReceiver = new class EventReceiver(this->eventLoop,__FILE__,__LINE__);
    this->eventLoop->AddListener("PREDICT_FRAME_REQUEST", *this->eventReceiver);
    this->eventLoop->AddListener("TRAINING_IMG_FOUND", *this->eventReceiver);
    this->eventLoop->AddListener("TRAINING_POS_FOUND", *this->eventReceiver);
    this->eventLoop->AddListener("TRAINING_DATA_FINISH", *this->eventReceiver);
    this->eventLoop->AddListener("GET_PROGRESS", *this->eventReceiver);
    this->eventLoop->AddListener("GET_MODEL", *this->eventReceiver);
    this->eventLoop->AddListener("SET_MODEL", *this->eventReceiver);
    this->eventLoop->AddListener("GET_STATE", *this->eventReceiver);

    this->eventLoop->AddListener("PAUSE_ALGORITHM", *this->eventReceiver);
    this->eventLoop->AddListener("RUN_ALGORITHM", *this->eventReceiver);

    QObject::connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(StdOutReady()));
    QObject::connect(this, SIGNAL(readyReadStandardError()), this, SLOT(StdErrReady()));
    QObject::connect(this, SIGNAL(stateChanged(newState)), this, SLOT(ProcessStateChanged(newState)));

    QObject::connect(&this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer.start(10); //in millisec
    this->keepAliveTimer.start();
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

    //Determine which python executable to run
    QList<QString> programCandidates;
    programCandidates.append("python.exe");
#ifndef RELEASE_MODE
    programCandidates.append("/usr/bin/python");
    programCandidates.append("c:\\dev\\Python27\\python.exe");
#endif

    QString program = "";
    for(unsigned i=0;i<programCandidates.size();i++)
    {
        QFile programFile(programCandidates[i]);
        if(programFile.exists())
        {
            program = programCandidates[i];
            break;
        }
    }
    if(program.length()==0)
    {
        throw std::runtime_error("Python runtime executable not found");
    }

    //Find the main python script
    QList<QString> scriptCandidates;
    scriptCandidates.append("echosrv.py");
#ifndef RELEASE_MODE
    scriptCandidates.append("../echosrv.py");
	scriptCandidates.append("../QtMedia/echosrv.py");
#endif
    QString mainScript = "";
    for(unsigned i=0;i<scriptCandidates.size();i++)
    {
        QFile scriptFile(scriptCandidates[i]);
        if(scriptFile.exists())
        {
            mainScript = scriptCandidates[i];
            break;
        }
    }
    if(mainScript.length()==0)
    {
        throw std::runtime_error("Algorithm python script not found");
    }
    QStringList arguments;
    if(0)
    {
        //Enable python profiling
        arguments.append("-m");
        arguments.append("cProfile");
        arguments.append("-o");
        arguments.append("prof");
    }
    arguments.append(mainScript);

    this->start(program, arguments);
    this->stopping = 0;
    this->paused = 1;
    this->pausing = 0;
    this->initDone = 1;
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
    this->eventReceiver->Stop();
    this->SendCommand("QUIT\n");
    this->waitForFinished();
    return 1;
}

void AlgorithmProcess::StopNonBlocking()
{
    assert(this->initDone);
    this->pausing = 0;
    this->stopping = 1;
    this->eventReceiver->Stop();
    this->SendCommand("QUIT\n");
}

int AlgorithmProcess::Start()
{
    Init();
    if(!this->paused) return 0;
    this->pausing = 0;
    this->stopping = 0;
    this->Unpause(); //Start process in running state

    return 1;
}

int AlgorithmProcess::IsStopFlagged()
{
    return this->stopping;
}

AlgorithmProcess::ProcessState AlgorithmProcess::GetState()
{
    QProcess::ProcessState qstate = this->state();

    if(qstate == QProcess::Starting)
        return AlgorithmProcess::STARTING;
    if(qstate == QProcess::NotRunning)
    {
        this->stopping = 0;
        this->paused = 1;
        return AlgorithmProcess::STOPPED;
    }
    assert(qstate == QProcess::Running);
    if(!this->dataLoaded) return AlgorithmProcess::RUNNING_PREPARING;
    if(this->stopping) return AlgorithmProcess::RUNNING_STOPPING;
    if(this->progress >= 1.) return AlgorithmProcess::READY;
    if(this->paused) return AlgorithmProcess::PAUSED;
    if(this->pausing) return AlgorithmProcess::RUNNING_PAUSING;

    return AlgorithmProcess::RUNNING;
}

QByteArray AlgorithmProcess::ReadLineFromBuffer(QByteArray &buff, int popLine, int skipLines)
{
    if(popLine==1) assert(skipLines==0);//Cannot skip lines if we are modifying buffer

    //Count lines to skip
    int skipCount = 0, skipPos = 0;
    while(skipCount < skipLines)
    {
        if(buff[skipPos]=='\n')
        {
            skipCount++;
        }
        skipPos++;
    }

    //Check if a new line has occured
    int newLinePos = -1;
    for(unsigned i=skipPos;i<buff.length();i++)
    {
        if(buff[i]=='\n' && newLinePos == -1)
        {
            newLinePos = i;
        }
        if(newLinePos>=0) break;
    }

    //If no new line found, so return
    if(newLinePos==-1)
    {
        QByteArray empty;
        return empty;
    }

    //Extract a string for recent command
    QByteArray cmd = buff.mid(skipPos,newLinePos-skipPos);
    if(popLine) buff = buff.mid(newLinePos+1);

    //if(cmd.length()>0)
    //{
    //    this->algOutLog->write(cmd.constData());
    //    this->algOutLog->write("\n");
    //    this->algOutLog->flush();
    //}

    return cmd;
}

void AlgorithmProcess::Update()
{
    //cout << "AlgorithmProcess::Update()" << endl;

    //Do periodic update of keep alive message
    if(this->keepAliveTimer.elapsed()>1000)
    {
        this->keepAliveTimer.restart();
        this->SendCommand("KEEPALIVE\n");
    }

    AlgorithmProcess::ProcessState state = this->GetState();

    //Process events from application
    int flushEvents = 1;
    while(flushEvents)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();

        //Only process events addressed to this algorithm
        if(ev->toUuid.isNull() || ev->toUuid == this->GetUid())
            this->HandleEvent(ev);
    }
    catch(std::runtime_error e)
    {
        flushEvents = 0;
    }

    //Get standard output from algorithm process
    /*QByteArray ret = this->readAllStandardOutput();
    this->algOutBuffer.append(ret);
    this->ProcessAlgOutput();*/

    //Get errors from console error out
    QByteArray ret = this->readAllStandardError();
    this->algErrBuffer.append(ret);

    while(true)
    {
        QString err = ReadLineFromBuffer(this->algErrBuffer);
        if(err.length() == 0) break;
        cout << "Algorithm Error: " << err.toLocal8Bit().constData() << endl;
    }

}

void AlgorithmProcess::StdOutReady()
{
    //Get standard output from algorithm process
    QByteArray ret = this->readAllStandardOutput();
    this->algOutBuffer.append(ret);
    this->ProcessAlgOutput();
}

void AlgorithmProcess::StdErrReady()
{
    //Get errors from console error out
    QByteArray ret = this->readAllStandardError();
    this->algErrBuffer.append(ret);

    while(true)
    {
        QString err = ReadLineFromBuffer(this->algErrBuffer);
        if(err.length() == 0) break;
        cout << "Algorithm Error: " << err.toLocal8Bit().constData() << endl;
    }
}

void AlgorithmProcess::ProcessStateChanged(QProcess::ProcessState newState)
{
    if(newState == QProcess::NotRunning)
    {
        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        openEv->data = "stopped";
        openEv->fromUuid = this->uid;
        this->eventLoop->SendEvent(openEv);

    }

}

void AlgorithmProcess::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type == "PREDICT_FRAME_REQUEST")
    {
        //Encode request event into a serial data and send to process
        class ProcessingRequestOrResponse *req = (class ProcessingRequestOrResponse *)ev->raw;
        assert(req!=NULL);
        QSharedPointer<QImage> img = req->img;
        std::vector<std::vector<std::vector<float> > > &pos = req->pos;

        QString xml="<predict>\n";
        for(unsigned int i=0;i<pos.size();i++)
        {
            std::vector<std::vector<float> > &model = pos[i];
            xml+=" <model>\n";
            for(unsigned j=0;j<model.size();j++)
            {
                std::vector<float> &pt = model[j];
                assert(pt.size() == 2);
                xml+=QString("  <pt x=\"%1\" y=\"%2\" />\n").arg(pt[0]).arg(pt[1]);
            }
            xml+=" </model>\n";
        }

        xml+="</predict>\n";
        QByteArray xmlBytes(xml.toUtf8().constData());
        QByteArray imgRaw((const char *)img->bits(), img->byteCount());
        QByteArray combinedRaw = imgRaw;
        combinedRaw.append(xmlBytes);

        QString imgPreamble2 = QString("RGB_IMAGE_AND_XML HEIGHT=%1 WIDTH=%2 IMGBYTES=%3 XMLBYTES=%4 ID=%5\n").
                arg(img->height()).
                arg(img->width()).
                arg(img->byteCount()).
                arg(xmlBytes.length()).
                arg(ev->id);
        this->SendRawDataBlock(imgPreamble2, combinedRaw);
    }

    if(ev->type == "TRAINING_IMG_FOUND")
    {
        class BinaryData *data = (class BinaryData *)ev->raw;
        assert(data!=NULL);
        QByteArray imgRaw((const char *)data->raw, data->size);
        this->SendRawDataBlock(ev->data, imgRaw);
    }

    if(ev->type == "TRAINING_POS_FOUND")
    {
        QString preamble2 = QString("XML_DATA\n");
        QByteArray posRaw(ev->data.toUtf8().constData(), ev->data.size());
        this->SendRawDataBlock(preamble2, posRaw);
    }

    if(ev->type == "TRAINING_DATA_FINISH")
    {
        this->SendCommand("TRAINING_DATA_FINISH\n");
        this->dataLoaded = 1;
    }

    if(ev->type == "GET_PROGRESS")
    {
        this->SendCommand("GET_PROGRESS\n");
    }

    if(ev->type == "GET_MODEL")
    {
        this->saveModelRequestIds.push_back(ev->id);
        this->GetModel();
    }

    if(ev->type == "SET_MODEL")
    {
        cout << "Sending model with format " << ev->buffer.left(3).constData() << endl;
        cout << "Raw length" << ev->buffer.size() << endl;
        QByteArray b64bin = ev->buffer.toBase64();
        cout << "Base64 length" << b64bin.size() << endl;
        this->SendRawDataBlock("MODEL\n", b64bin);

        this->dataLoaded = 1;
    }

    if(ev->type == "PAUSE_ALGORITHM")
    {
        this->Pause();
    }

    if(ev->type == "RUN_ALGORITHM")
    {
        this->Unpause();
    }

    if(ev->type == "GET_STATE")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("ALG_STATE"));
        responseEv->id = ev->id;
        responseEv->data = QString::number((int)(this->GetState()), 'd', 0);
        responseEv->fromUuid = this->uid;
        this->eventLoop->SendEvent(responseEv);
    }
}

void AlgorithmProcess::ProcessAlgOutput()
{
    if(this->algOutBuffer.length()==0) return;
    class EventLoop &el = *this->eventLoop;

    //Get first line of console output without modifying the buffer
    QByteArray cmd = ReadLineFromBuffer(this->algOutBuffer,0);
    if(cmd.length()==0) return;
    cmd = cmd.trimmed(); //Remove whitespace from command

    if(cmd.left(9)=="PROGRESS=")
    {
        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_PROGRESS_UPDATE"));
        std::ostringstream tmp;
        tmp << cmd.mid(9).constData();
        openEv->data = tmp.str().c_str();
        openEv->fromUuid = this->uid;
        el.SendEvent(openEv);
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line

        this->progress = cmd.mid(9).toDouble();
        return;
    }

    if(cmd=="NOW_PAUSED")
    {
        this->paused = 1;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        openEv->data = "paused";
        openEv->fromUuid = this->uid;
        el.SendEvent(openEv);
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
        return;
    }

    if(cmd=="NOW_RUNNING")
    {
        this->paused = 0;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        openEv->data = "running";
        openEv->fromUuid = this->uid;
        el.SendEvent(openEv);
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
        return;
    }

    if(cmd=="FINISHED")
    {
        this->SendCommand("BYE\n");
        this->waitForFinished(); //Just wait for final finishing of process
        this->initDone = 0;

        std::tr1::shared_ptr<class Event> openEv(new Event("THREAD_STATUS_CHANGED"));
        openEv->data = "finished";
        openEv->fromUuid = this->uid;
        el.SendEvent(openEv);
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
        return;
    }

    if(cmd=="INTERNAL_ERROR")
    {

        this->SendCommand("QUIT\n");
        this->waitForFinished(1000);
        if(this->state() != QProcess::NotRunning)
        {
            this->terminate();
        }
    }

    if(cmd.left(11)=="DATA_BLOCK=")
    {
        QByteArray blockArg = this->ReadLineFromBuffer(this->algOutBuffer,0,1);
        QString blockLenStr = cmd.mid(11);
        int blockLen = blockLenStr.toInt();
        cout << "Expected block len " << blockLen << endl;

        //Wait for process to write the entire data block to standard output
        if(this->algOutBuffer.length() < cmd.length() + blockArg.length() + blockLen + 2)
            return;

        QByteArray blockData = this->algOutBuffer.mid(cmd.length() + blockArg.length() + 2, blockLen);
        //Remove used data from buffer
        this->algOutBuffer = this->algOutBuffer.mid(cmd.length() + blockArg.length() + blockLen + 2);

        //Return model as event
        std::tr1::shared_ptr<class Event> responseEv(new Event("SAVED_MODEL_BINARY"));
        cout << "Base64 length" << blockData.size() << endl;
        responseEv->buffer = QByteArray::fromBase64(blockData);
        cout << "Raw length" << responseEv->buffer.size() << endl;
        responseEv->fromUuid = this->uid;
        if(this->saveModelRequestIds.size()>=1)
        {
            responseEv->id = this->saveModelRequestIds[0];
            this->saveModelRequestIds.pop_front();
        }
        else
            responseEv->id = 0;
        el.SendEvent(responseEv);


        return;
    }

    if(cmd.left(10)=="XML_BLOCK=")
    {
        QByteArray blockArg = this->ReadLineFromBuffer(this->algOutBuffer,0,1);
        QString blockLenStr = cmd.mid(10);
        int blockLen = blockLenStr.toInt();
        std::vector<std::string> splitArgs = split(blockArg.constData(),' ');
        unsigned long long responseId = 0;
        for(unsigned int i=0;i<splitArgs.size();i++)
        {
            std::vector<std::string> splitArgPairs = split(splitArgs[i],'=');
            if(splitArgPairs[0]=="ID")
            {
                QByteArray pairArgBytes(splitArgPairs[1].c_str());
                responseId = STR_TO_ULL(pairArgBytes.constBegin(),
                                                           0,10);
            }
        }

        //Wait for process to write the entire data block to standard output
        if(this->algOutBuffer.length() < cmd.length() + blockArg.length() + blockLen + 2)
            return;

        QByteArray blockDataB64 = this->algOutBuffer.mid(cmd.length() + blockArg.length() + 2, blockLen);
        QByteArray blockData = QByteArray::fromBase64(blockDataB64);

        QTextStream dec(blockData);
        dec.setCodec("UTF-8");
        QString xmlBlock = dec.readAll();
        //cout << xmlBlock.toLocal8Bit().constData() << endl;

        //Remove used data from buffer
        this->algOutBuffer = this->algOutBuffer.mid(cmd.length() + blockArg.length() + blockLen + 2);

        //Parse XML to DOM
        QDomDocument doc("algxml");
        QString errorMsg;
        if (!doc.setContent(xmlBlock, 0, &errorMsg))
        {
            cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
            return;
        }

        //Iterate over result
        std::vector<std::vector<std::vector<float> > > entireResponse;
        QDomElement rootElem = doc.documentElement();
        QDomNode n = rootElem.firstChild();
        while(!n.isNull())
        {
            std::vector<std::vector<float> > model;

            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull())
            {
                if(e.tagName() != "model") continue;
                QDomNode ptNode = e.firstChild();
                while(!ptNode.isNull())
                {
                    QDomElement ptEl = ptNode.toElement(); // try to convert the node to an element.
                    if(!ptEl.isNull())
                    {
                        if(ptEl.tagName()!="pt") continue;
                        std::vector<float> pt;
                        QString xStr = ptEl.attribute("x");
                        QString yStr = ptEl.attribute("y");
                        //cout << "pt" << xStr.toFloat() << "," << yStr.toFloat() << endl;
                        pt.push_back(xStr.toFloat());
                        pt.push_back(yStr.toFloat());
                        model.push_back(pt);
                    }
                    ptNode = ptNode.nextSiblingElement();
                }
            }

            entireResponse.push_back(model);
            n = n.nextSiblingElement();
        }

        std::tr1::shared_ptr<class Event> resultEv(new Event("PREDICTION_RESULT"));
        class ProcessingRequestOrResponse *response = new class ProcessingRequestOrResponse;
        response->pos.clear();
        response->pos = entireResponse;
        resultEv->raw = response;
        resultEv->id = responseId;
        this->eventLoop->SendEvent(resultEv);

        return;

    }

    if(cmd.length()>0)
    {
        cout << "Algorithm: " << cmd.constData() << endl;
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
        return;
    }

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

void AlgorithmProcess::SendRawDataBlock(QString args, QByteArray data)
{
    int running = (this->state() == QProcess::Running);
    if(!running) return;
    assert(this->initDone);

    QString imgPreamble1 = QString("DATA_BLOCK=%1\n").arg(data.length());
    this->SendCommand(imgPreamble1);
    this->SendCommand(args);
    this->write(data);
    //this->waitForBytesWritten();
}

unsigned int AlgorithmProcess::EncodedLength(QString cmd)
{
    QString encodedCmd;
    QTextStream enc(&encodedCmd);
    enc.setCodec("UTF-8");
    enc << cmd;
    return encodedCmd.length();
}

void AlgorithmProcess::GetModel()
{
    //Send request to algorithm process
    assert(this->paused);
    this->SendCommand("SAVE_MODEL\n");
}

QUuid AlgorithmProcess::GetUid()
{
    return this->uid;
}

void AlgorithmProcess::SetUid(QUuid newUid)
{
    this->uid = newUid;
}

int AlgorithmProcess::IsBlockingShutdown()
{
    AlgorithmProcess::ProcessState state = AlgorithmProcess::GetState();
    if(state == AlgorithmProcess::STARTING) return 1;
    if(state == AlgorithmProcess::RUNNING_PREPARING) return 1;
    if(state == AlgorithmProcess::RUNNING) return 1;
    if(state == AlgorithmProcess::RUNNING_PAUSING) return 1;
    if(state == AlgorithmProcess::RUNNING_STOPPING) return 1;
    return 0;
}

//******************************************************

int AlgorithmProcess::PredictFrame(QSharedPointer<QImage> img,
                    std::vector<std::vector<float> > &model,
                    QUuid algUuid,
                    class EventLoop *eventLoop,
                    class EventReceiver *eventReceiver,
                    std::vector<std::vector<float> > &out)
{
    out.clear();

    //Ask alg process to make a prediction
    std::tr1::shared_ptr<class Event> requestEv(new Event("PREDICT_FRAME_REQUEST"));
    class ProcessingRequestOrResponse *req = new class ProcessingRequestOrResponse;
    req->img = img;
    req->pos.clear();
    req->pos.push_back(model);
    requestEv->raw = req;
    requestEv->id = eventLoop->GetId();
    requestEv->data = algUuid.toString();

    eventLoop->SendEvent(requestEv);

    //Wait for response
    try
    {
        std::tr1::shared_ptr<class Event> ev = eventReceiver->WaitForEventId(requestEv->id);

        if(ev->type!="PREDICTION_RESULT") return 0;
        class ProcessingRequestOrResponse *response = (class ProcessingRequestOrResponse *)ev->raw;

        out = response->pos[0];
        return 1;

    }
    catch(std::runtime_error e)
    {
        cout << "Warning: Prediction timed out" << endl;
    }
    return 0;
}

AlgorithmProcess::ProcessState AlgorithmProcess::GetState(QUuid algUuid,
                      class EventLoop *eventLoop,
                      class EventReceiver *eventReceiver)
{
    //Ask alg process for state
    std::tr1::shared_ptr<class Event> requestEv(new Event("GET_STATE"));
    requestEv->id = eventLoop->GetId();
    requestEv->toUuid = algUuid;
    eventLoop->SendEvent(requestEv);

    //Wait for response
    std::tr1::shared_ptr<class Event> resp = eventReceiver->WaitForEventId(requestEv->id);
    assert(resp->type=="ALG_STATE");
    return (AlgorithmProcess::ProcessState)resp->data.toInt();
}
