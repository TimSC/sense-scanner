#include "algorithm.h"
#include "localsleep.h"
#include "mediabuffer.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
#include <QtCore/QTextStream>
#include <QtCore/QSharedPointer>
#include <QtXml/QtXml>
using namespace std;

//************************************************

AlgorithmProcess::AlgorithmProcess(class EventLoop *eventLoopIn, QObject *parent) : QProcess(parent)
{
    this->stopping = 0;
    this->paused = 1;
    this->pausing = 0;
    this->initDone = 0;
    this->dataBlockReceived = 0;

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

	//Determine which python executable to run
	QList<QString> programCandidates;
	programCandidates.append("python.exe");
	programCandidates.append("/usr/bin/python");
	programCandidates.append("c:\\dev\\Python27\\python.exe");

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
	scriptCandidates.append("../QtMedia/echosrv.py");
	scriptCandidates.append("echosrv.py");
	scriptCandidates.append("../echosrv.py");
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
    arguments.append(mainScript);

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
    this->Unpause(); //Start process in running state
    
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

	if(cmd.length()>0)
    {
        this->algOutLog->write(cmd.constData());
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
    this->ProcessAlgOutput();

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
    if(QString(ev->data.c_str()) != this->uid.toString()) return; //Check if this is the appropriate algorithm

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
        tmp << cmd.mid(9).constData() << "," << this->threadId;
        openEv->data = tmp.str();
        el.SendEvent(openEv);
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
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
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
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
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
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
        ReadLineFromBuffer(this->algOutBuffer,1);//Pop this line
        return;
    }

    if(cmd.left(11)=="DATA_BLOCK=")
    {
        QByteArray blockArg = this->ReadLineFromBuffer(this->algOutBuffer,0,1);
		QString blockLenStr = cmd.mid(11);
        int blockLen = blockLenStr.toInt();

        //Wait for process to write the entire data block to standard output
        if(this->algOutBuffer.length() < cmd.length() + blockArg.length() + blockLen)
            return;

        QByteArray blockData = this->algOutBuffer.mid(cmd.length() + blockArg.length() + 2, blockLen);
        //Remove used data from buffer
        this->algOutBuffer = this->algOutBuffer.mid(cmd.length() + blockArg.length() + blockLen + 2);

		this->dataBlock = QByteArray::fromBase64(blockData);
        this->dataBlockReceived = 1;

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
	//out.append(QByteArray::fromBase64(this->dataBlock));
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

