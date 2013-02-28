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
        this->SaveAs(ev->data.c_str());
    }

    if(ev->type=="LOAD_WORKSPACE")
    {
        this->Load(ev->data.c_str());
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
}

void UserActions::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);

    this->eventLoop->AddListener("SAVE_WORKSPACE_AS", *this->eventReceiver);
    this->eventLoop->AddListener("LOAD_WORKSPACE", *this->eventReceiver);
}

void UserActions::SetMediaInterface(class AvBinMedia* mediaInterfaceIn)
{
    this->mediaInterface = mediaInterfaceIn;
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
    QList<QUuid> annotationUuids;
    assert(0);

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
            QString fina = sourceName->data.c_str();

            //Get algorithm Uuid for this annotation track
            std::tr1::shared_ptr<class Event> getAlgUuidEv(new Event("GET_ALG_UUID"));
            getAlgUuidEv->toUuid = annotationUuids[i];
            getAlgUuidEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAlgUuidEv);

            std::tr1::shared_ptr<class Event> algUuidEv = this->eventReceiver->WaitForEventId(getAlgUuidEv->id);
            QUuid algUuid(algUuidEv->data.c_str());

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
            out << annotXmlRet->data.c_str();
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

    QList<QUuid> processingUuids;
    assert(0);

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
            class BinaryData *modelBin = (class BinaryData *)ev->raw;
            QByteArray modelArr((const char *)modelBin->raw, modelBin->size);

            //Encode as XML binary blob
            QByteArray modelBase64 = modelArr.toBase64();
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
                    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
                    ann->SetSource(fileInfo.absoluteFilePath());

                    //Set source UID
                    QString uidStr = sourceEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();

                    //Set alg Uid
                    QString algStr = sourceEle.attribute("alg");
                    QUuid alg(algStr);
                    ann->SetAlgUid(alg);

                    TrackingAnnotationData *track =
                            new TrackingAnnotationData();

                    QDomNode trackData = sourceNode.firstChild();
                    while(!trackData.isNull())
                    {
                        QDomElement et = trackData.toElement(); // try to convert the node to an element.
                        if(et.isNull()) continue;
                        if(et.tagName() != "tracking") {trackData = trackData.nextSibling(); continue;}

                        track->ReadAnnotationXml(et);

                        trackData = trackData.nextSibling();

                    }

                    ann->SetTrack(track);
                    //this->workspace.AddSource(ann, mediaInterface, uid);
                    assert(0);//TODO change to event basis
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
                    std::tr1::shared_ptr<class AlgorithmProcess> alg(
                                new class AlgorithmProcess(this->eventLoop, this));
                    alg->Init();

                    QString uidStr = modelEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();
                    alg->SetUid(uid);
                    //this->workspace.AddProcessing(alg);
                    assert(0);
                    //TODO change to event basis

                    //Send data to algorithm process
                    std::tr1::shared_ptr<class Event> foundModelEv(new Event("ALG_MODEL_FOUND"));
                    QString imgPreamble2 = QString("MODEL\n");
                    foundModelEv->data = imgPreamble2.toLocal8Bit().constData();
                    class BinaryData *modelRaw = new BinaryData();
                    modelRaw->Copy((const unsigned char *)modelData.constData(), modelData.size());
                    foundModelEv->raw = modelRaw;
                    foundModelEv->toUuid = uid;
                    this->eventLoop->SendEvent(foundModelEv);

                    //Continue to train if needed
                    std::tr1::shared_ptr<class Event> trainingFinishEv(new Event("TRAINING_DATA_FINISH"));
                    trainingFinishEv->toUuid = uid;
                    this->eventLoop->SendEvent(trainingFinishEv);

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
