#include "useractions.h"
#include "annotation.h"
#include "algorithm.h"
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <iostream>
#include <assert.h>

using namespace std;

UserActions::UserActions() : MessagableThread()
{

}

UserActions::~UserActions()
{

}

void UserActions::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type=="SAVE_WORKSPACE_AS")
    {
        this->SaveAs(ev->data);
    }

    if(ev->type=="LOAD_WORKSPACE")
    {
        this->Load(ev->data);
    }

    MessagableThread::HandleEvent(ev);
}

void UserActions::Update()
{
    //Process events from application
    int flushEvents = 1;
    while(flushEvents)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();

        //Only process events addressed to this algorithm
        //if(ev->toUuid.isNull() || ev->toUuid == this->GetUid())
        this->HandleEvent(ev);
    }
    catch(std::runtime_error e)
    {
        flushEvents = 0;
    }
    this->msleep(100);
}

void UserActions::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);

    this->eventLoop->AddListener("SAVE_WORKSPACE_AS", *this->eventReceiver);
    this->eventLoop->AddListener("LOAD_WORKSPACE", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_ADDED", *this->eventReceiver);
    this->eventLoop->AddListener("PROCESSING_ADDED", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_UUIDS", *this->eventReceiver);
    this->eventLoop->AddListener("PROCESSING_UUIDS", *this->eventReceiver);
    this->eventLoop->AddListener("SOURCE_FILENAME", *this->eventReceiver);
    this->eventLoop->AddListener("ALG_UUID_FOR_ANNOTATION", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_DATA", *this->eventReceiver);
    this->eventLoop->AddListener("SAVED_MODEL_BINARY", *this->eventReceiver);

}

int UserActions::SaveAs(QString fina)
{
    int finaLen = fina.length();
    if(finaLen==0)
    {
        return 0;
    }

    QString tmpFina = fina;
    tmpFina.append(".tmp");
    QFileInfo pathInfo(fina);
    QDir dir(pathInfo.absoluteDir());

    //Save data to file
    QFile f(tmpFina);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<workspace>" << endl;
    out << "<sources>" << endl;

    //Get list of annotation uuids
    std::tr1::shared_ptr<class Event> getAnnotUuids(new Event("GET_ANNOTATION_UUIDS"));
    getAnnotUuids->id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(getAnnotUuids);

    //Decode response
    std::tr1::shared_ptr<class Event> resp = this->eventReceiver->WaitForEventId(getAnnotUuids->id);
    QList<QUuid> annotationUuids;
    std::vector<std::string> splitUuids = split(resp->data.toLocal8Bit().constData(), ',');
    for(unsigned int i=0;i<splitUuids.size();i++)
        annotationUuids.push_back(QUuid(splitUuids[i].c_str()));

    for(unsigned int i=0;i<annotationUuids.size();i++)
    {
        try
        {
            //Get source filename for annotation
            std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
            getSourceNameEv->toUuid = annotationUuids[i];
            getSourceNameEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getSourceNameEv);

            std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
            QString fina = sourceName->data;

            //Get algorithm Uuid for this annotation track
            std::tr1::shared_ptr<class Event> getAlgUuidEv(new Event("GET_ALG_UUID"));
            getAlgUuidEv->toUuid = annotationUuids[i];
            getAlgUuidEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAlgUuidEv);

            std::tr1::shared_ptr<class Event> algUuidEv = this->eventReceiver->WaitForEventId(getAlgUuidEv->id);
            QUuid algUuid(algUuidEv->data);

            //Get annotation data
            std::tr1::shared_ptr<class Event> getAnnotEv(new Event("GET_ALL_ANNOTATION_XML"));
            getAnnotEv->toUuid = annotationUuids[i];
            getAnnotEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAnnotEv);

            std::tr1::shared_ptr<class Event> annotXmlRet = this->eventReceiver->WaitForEventId(getAnnotEv->id);

            //Format as XML
            out << "\t<source id=\""<<i<<"\" uid=\""<<Qt::escape(annotationUuids[i].toString())<<"\" file=\""<<
                   Qt::escape(dir.absoluteFilePath(fina))<<"\"";

            if(!algUuid.isNull())
                out << " alg=\"" << algUuid.toString().toLocal8Bit().constData() << "\"";;

            out << ">" << endl;
            out << annotXmlRet->data;
            out << "\t</source>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
    }


    out << "</sources>" << endl;
    out << "<models>" << endl;

    std::tr1::shared_ptr<class Event> getProcessingEv(new Event("GET_PROCESSING_UUIDS"));
    getProcessingEv->id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(getProcessingEv);

    //Decode response
    std::tr1::shared_ptr<class Event> resp2 = this->eventReceiver->WaitForEventId(getProcessingEv->id);
    QList<QUuid> processingUuids;
    std::vector<std::string> splitUuids2 = split(resp2->data.toLocal8Bit().constData(), ',');
    for(unsigned int i=0;i<splitUuids2.size();i++)
        processingUuids.push_back(QUuid(splitUuids2[i].c_str()));

    for(unsigned int i=0;i<processingUuids.size();i++)
    {
        try
        {
            //Get model
            std::tr1::shared_ptr<class Event> getModelEv(new Event("GET_MODEL"));
            getModelEv->toUuid = processingUuids[i];
            getModelEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getModelEv);

            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(getModelEv->id);

            //Encode as XML binary blob
            QByteArray modelBase64 = ev->buffer.toBase64();
            out << "<model uid=\""<< processingUuids[i] <<"\">" << endl;
            for(unsigned int pos=0;pos<modelBase64.length();pos+=512)
            {
                out << modelBase64.mid(pos, 512) << endl;
            }
            out << "</model>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
    }
    out << "</models>" << endl;
    out << "</workspace>" << endl;
    f.close();

    //Rename temporary file to final name
    QFile targetFina(fina);
    if(targetFina.exists())
        QFile::remove(fina);
    QFile::rename(tmpFina, fina);

    return 1;
}

void UserActions::Load(QString fina)
{
    //Parse XML to DOM
    QFile f(fina);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        return;
    }
    f.close();

    //Get dir of data file
    QFileInfo pathInfo(fina);
    QDir dir(pathInfo.absoluteDir());

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    QDomNode n = rootElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {

            if(e.tagName() == "sources")
            {
                QDomNode sourceNode = e.firstChild();
                while(!sourceNode.isNull())
                {
                    QDomElement sourceEle = sourceNode.toElement(); // try to convert the node to an element.
                    if(sourceEle.tagName() != "source") {sourceNode = sourceNode.nextSibling(); continue;}

                    QString sourceFiNa = sourceEle.attribute("file");
                    //QString sourceFiNaAbs = dir.absoluteFilePath(sourceFiNa);

                    QFileInfo fileInfo(sourceFiNa);
                    //std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
                    //ann->SetSource(fileInfo.absoluteFilePath());

                    //Set source UID
                    QString uidStr = sourceEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();

                    //Set alg Uid
                    QString algStr = sourceEle.attribute("alg");
                    QUuid alg(algStr);
                    //ann->SetAlgUid(alg);

                    QDomNode trackData = sourceNode.firstChild();
                    while(!trackData.isNull())
                    {
                        QDomElement et = trackData.toElement(); // try to convert the node to an element.
                        if(et.isNull()) {trackData = trackData.nextSibling(); continue;}
                        if(et.tagName() != "tracking") {trackData = trackData.nextSibling(); continue;}

                        //Get XML of this annotation track
                        QString xml;
                        QTextStream xmlStream(&xml);
                        et.save(xmlStream, 0);

                        std::tr1::shared_ptr<class Event> newAnnEv(new Event("NEW_ANNOTATION"));
                        QString dataStr = QString("%1").arg(uid.toString());
                        newAnnEv->data = dataStr.toLocal8Bit().constData();
                        newAnnEv->id = this->eventLoop->GetId();
                        this->eventLoop->SendEvent(newAnnEv);

                        //Wait for workspace to register this annotation
                        this->eventReceiver->WaitForEventId(newAnnEv->id);

                        //Set source
                        std::tr1::shared_ptr<class Event> newAnnEv2(new Event("SET_SOURCE_FILENAME"));
                        newAnnEv2->data = sourceFiNa.toLocal8Bit().constData();
                        newAnnEv2->toUuid = uid;
                        this->eventLoop->SendEvent(newAnnEv2);

                        //Set annotation
                        std::tr1::shared_ptr<class Event> newAnnEv3(new Event("SET_ANNOTATION_BY_XML"));
                        newAnnEv3->toUuid = uid;
                        newAnnEv3->data = xml.toLocal8Bit().constData();
                        this->eventLoop->SendEvent(newAnnEv3);

                        //Set alg uuid
                        std::tr1::shared_ptr<class Event> newAnnEv4(new Event("SET_ALG_UUID"));
                        newAnnEv4->toUuid = uid;
                        newAnnEv4->data = xml.toLocal8Bit().constData();
                        this->eventLoop->SendEvent(newAnnEv4);

                        trackData = trackData.nextSibling();
                    }

                    sourceNode = sourceNode.nextSibling();
                }

            }

            if(e.tagName() == "models")
            {
                QDomNode modelNode = e.firstChild();
                while(!modelNode.isNull())
                {
                    QDomElement modelEle = modelNode.toElement(); // try to convert the node to an element.
                    if(modelEle.tagName() != "model") {modelNode = modelNode.nextSibling(); continue;}

                    QByteArray modelData = QByteArray::fromBase64(modelEle.text().toLocal8Bit().constData());

                    QString uidStr = modelEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();

                    //Create processing module
                    std::tr1::shared_ptr<class Event> newAnnEv(new Event("NEW_PROCESSING"));
                    QString dataStr = QString("%1").arg(uid.toString());
                    newAnnEv->data = dataStr.toLocal8Bit().constData();
                    newAnnEv->id = this->eventLoop->GetId();
                    this->eventLoop->SendEvent(newAnnEv);

                    //Wait for workspace to register this annotation
                    this->eventReceiver->WaitForEventId(newAnnEv->id);

                    //Send data to algorithm process
                    std::tr1::shared_ptr<class Event> foundModelEv(new Event("SET_MODEL"));
                    foundModelEv->buffer = modelData;
                    foundModelEv->toUuid = uid;
                    this->eventLoop->SendEvent(foundModelEv);

                    //Ask process to provide progress update
                    std::tr1::shared_ptr<class Event> getProgressEv(new Event("GET_PROGRESS"));
                    getProgressEv->toUuid = uid;
                    this->eventLoop->SendEvent(getProgressEv);

                    modelNode = modelNode.nextSibling();
                }

            }
        }
        n = n.nextSibling();
    }
}

