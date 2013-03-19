#include "useractions.h"
#include "annotation.h"
#include "algorithm.h"
#include "avbinmedia.h"
#include "workspace.h"
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
    if(ev->type=="NEW_WORKSPACE")
    {
        this->NewWorkspace();
    }

    if(ev->type=="SAVE_WORKSPACE_AS")
    {
        this->SaveAs(ev->data);
    }

    if(ev->type=="LOAD_WORKSPACE")
    {
        this->Load(ev->data);
    }

    if(ev->type=="TRAIN_MODEL")
    {
        std::vector<std::string> splitAnnot = split(ev->data.toLocal8Bit().constData(),',');
        QList<QUuid> annotationUuids;
        for(unsigned int i=0;i<splitAnnot.size();i++)
            annotationUuids.append(QUuid(splitAnnot[i].c_str()));

        this->TrainModel(annotationUuids);
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
    this->eventLoop->AddListener("NEW_WORKSPACE", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_ADDED", *this->eventReceiver);
    this->eventLoop->AddListener("PROCESSING_ADDED", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_UUIDS", *this->eventReceiver);
    this->eventLoop->AddListener("PROCESSING_UUIDS", *this->eventReceiver);
    this->eventLoop->AddListener("SOURCE_FILENAME", *this->eventReceiver);
    this->eventLoop->AddListener("ALG_UUID_FOR_ANNOTATION", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_DATA", *this->eventReceiver);
    this->eventLoop->AddListener("SAVED_MODEL_BINARY", *this->eventReceiver);
    this->eventLoop->AddListener("TRAIN_MODEL", *this->eventReceiver);
    this->eventLoop->AddListener("MARKED_LIST_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("MEDIA_FRAME_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_AT_TIME", *this->eventReceiver);


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

    //Signal that save has started
    std::tr1::shared_ptr<class Event> saveEv(new Event("SAVE_STARTED"));
    this->eventLoop->SendEvent(saveEv);

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
            QString fina = Annotation::GetSourceFilename(annotationUuids[i],
                                                             this->eventLoop,
                                                             this->eventReceiver);

            //Get algorithm Uuid for this annotation track
            QUuid algUuid = Annotation::GetAlgUuid(annotationUuids[i],
                                                   this->eventLoop,
                                                   this->eventReceiver);

            //Get annotation data
            QString xml = Annotation::GetAllAnnotationByXml(annotationUuids[i],
                                                   this->eventLoop,
                                                   this->eventReceiver);

            //Format as XML
            out << "\t<source id=\""<<i<<"\" uid=\""<<Qt::escape(annotationUuids[i].toString())<<"\" file=\""<<
                   Qt::escape(dir.absoluteFilePath(fina))<<"\"";

            if(!algUuid.isNull())
                out << " alg=\"" << algUuid.toString().toLocal8Bit().constData() << "\"";;

            out << ">" << endl;
            out << xml;
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

            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(getModelEv->id, -1);

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

    //Signal that save has finished
    std::tr1::shared_ptr<class Event> saveDoneEv(new Event("SAVE_FINISHED"));
    this->eventLoop->SendEvent(saveDoneEv);

    return 1;
}

void UserActions::Load(QString fina)
{
    //Signal that load has started
    std::tr1::shared_ptr<class Event> loadEv(new Event("LOAD_STARTED"));
    this->eventLoop->SendEvent(loadEv);

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
    QMap<QUuid, QUuid> annotToAlgMap;

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
                    assert(!uid.isNull());
                    annotToAlgMap[uid] = alg;

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
                        newAnnEv4->data = alg.toString();
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
                    Workspace::AddProcessing(uid, this->eventLoop,
                                                  this->eventReceiver);

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

    //Set annotation to mappings
    QMap<QUuid, QUuid>::iterator it;
    for(it = annotToAlgMap.begin();it!=annotToAlgMap.end();it++)
    {
        std::tr1::shared_ptr<class Event> loadEv(new Event("ANNOT_USING_ALG"));
        QString dataStr = QString("%0,%1").arg(it.key()).arg(it.value());
        loadEv->data = dataStr;
        this->eventLoop->SendEvent(loadEv);
    }

    //Signal that load has finished
    std::tr1::shared_ptr<class Event> loadDoneEv(new Event("LOAD_FINISHED"));
    this->eventLoop->SendEvent(loadDoneEv);

}

void UserActions::TrainModel(QList<QUuid> annotationUuids)
{
    //Count frames, because at least one is needed to train
    int countMarkedFrames = 0;
    QList<std::vector<std::string> > seqMarked;
    for(unsigned int i=0;i<annotationUuids.size();i++)
    {
        QUuid annotUuid = annotationUuids[i];
        //For each annotated frame
        std::tr1::shared_ptr<class Event> getMarkedEv(new Event("GET_MARKED_LIST"));
        getMarkedEv->toUuid = annotUuid;
        getMarkedEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(getMarkedEv);

        std::tr1::shared_ptr<class Event> markedEv = this->eventReceiver->WaitForEventId(getMarkedEv->id);
        std::vector<std::string> splitMarked = split(markedEv->data.toLocal8Bit().constData(),',');
        seqMarked.push_back(splitMarked);
        countMarkedFrames += splitMarked.size();
    }
    assert(countMarkedFrames>0);

    //Create processing module and add to workspace
    QUuid newUuid = QUuid::createUuid();
    Workspace::AddProcessing(newUuid, this->eventLoop,
                                  this->eventReceiver);

    //Configure worker process
    QList<QSharedPointer<QImage> > imgs;
    for(unsigned int i=0;i<annotationUuids.size();i++)
    {
        //Get filename from annotation source
        std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
        getSourceNameEv->toUuid = annotationUuids[i];
        getSourceNameEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(getSourceNameEv);

        std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
        QString fina = sourceName->data;
        std::vector<std::string> marked = seqMarked[i];

        //For each annotated frame
        for(unsigned int fr=0;fr<marked.size();fr++)
        {
            countMarkedFrames ++;

            //Get image data and send to process
            cout << marked[fr] << endl;
            unsigned long long startTimestamp = 0, endTimestamp = 0;
            unsigned long long annotTimestamp = STR_TO_ULL_SIMPLE(marked[fr].c_str());
            QSharedPointer<QImage> img;
            try
            {
                std::tr1::shared_ptr<class Event> reqEv(new Event("GET_MEDIA_FRAME"));
                reqEv->toUuid = this->mediaUuid;
                reqEv->data = fina;
                QString tiStr = QString("%1").arg(annotTimestamp);
                reqEv->buffer = tiStr.toLocal8Bit().constData();
                reqEv->id = this->eventLoop->GetId();
                this->eventLoop->SendEvent(reqEv);

                std::tr1::shared_ptr<class Event> resp = this->eventReceiver->WaitForEventId(reqEv->id);
                assert(resp->type=="MEDIA_FRAME_RESPONSE");

                MediaResponseFrame processedImg(resp);
                img = QSharedPointer<QImage>(new QImage(processedImg.img));
                //annotTimestamp = processedImg.req;
                startTimestamp = processedImg.start;
                endTimestamp = processedImg.end;

            }
            catch (std::runtime_error &err)
            {
                cout << "Timeout getting frame " << annotTimestamp << endl;
                continue;
            }

            if(annotTimestamp < startTimestamp || annotTimestamp > endTimestamp)
            {
                cout << "Warning: found a frame but it does not cover requested time" << endl;
                cout << "Requested: " << annotTimestamp << endl;
                cout << "Found: " << startTimestamp << " to " << endTimestamp << endl;
                continue;
            }

            imgs.append(img);

            int len = img->byteCount();

            //Send image to algorithm module
            assert(img->format() == QImage::Format_RGB888);
            std::tr1::shared_ptr<class Event> foundImgEv(new Event("TRAINING_IMG_FOUND"));
            QString imgPreamble2 = QString("RGB_IMAGE_DATA TIMESTAMP=%1 HEIGHT=%2 WIDTH=%3\n").
                                arg(annotTimestamp).
                                arg(img->height()).
                                arg(img->width());
            foundImgEv->data = imgPreamble2.toLocal8Bit().constData();
            class BinaryData *imgRaw = new BinaryData();
            imgRaw->Copy((const unsigned char *)img->bits(), len);
            foundImgEv->raw = imgRaw;
            foundImgEv->toUuid = newUuid;
            this->eventLoop->SendEvent(foundImgEv);

            //Get annotation data and sent it to the algorithm
            std::tr1::shared_ptr<class Event> getAnnotEv(new Event("GET_ANNOTATION_AT_TIME"));
            getAnnotEv->toUuid = annotationUuids[i];
            QString reqDataStr = QString("%1").arg(annotTimestamp);
            getAnnotEv->data = reqDataStr.toLocal8Bit().constData();
            getAnnotEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAnnotEv);

            std::tr1::shared_ptr<class Event> annotXmlRet = this->eventReceiver->WaitForEventId(getAnnotEv->id);

            QString annotXml;
            QTextStream test(&annotXml);
            test << qPrintable(annotXmlRet->data);
            assert(annotXml.mid(annotXml.length()-1).toLocal8Bit().constData()[0]=='\n');

            std::tr1::shared_ptr<class Event> foundPosEv(new Event("TRAINING_POS_FOUND"));
            foundPosEv->data = annotXml.toUtf8().constData();
            foundPosEv->toUuid = newUuid;
            this->eventLoop->SendEvent(foundPosEv);
        }
    }

    std::tr1::shared_ptr<class Event> trainingFinishEv(new Event("TRAINING_DATA_FINISH"));
    trainingFinishEv->toUuid = newUuid;
    this->eventLoop->SendEvent(trainingFinishEv);

}

void UserActions::SetMediaInterface(QUuid mediaUuidIn)
{

    this->mediaUuid = mediaUuidIn;
}

void UserActions::NewWorkspace()
{
    int debug = 1;
    /*
    this->applyModelPool.Clear(); //Clear this first to stop interconnected behaviour
    this->workspace.ClearAnnotation();
    this->workspace.ClearProcessing();
    this->workspaceAsStored = this->workspace;
    this->RegenerateSourcesList();
    this->RegenerateProcessingList();
    */
}
