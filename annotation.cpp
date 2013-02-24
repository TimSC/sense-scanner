#include "annotation.h"
#include "scenecontroller.h"
#include "avbinmedia.h"
#include "localsleep.h"
#include <assert.h>
#include <iostream>
using namespace std;
#include "qblowfish/src/qblowfish.h"

#define TO_MILLISEC(x) (unsigned long long)(x / 1000. + 0.5)
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)

//****************************************************************************

TrackingAnnotationData::TrackingAnnotationData()
{

}

TrackingAnnotationData::TrackingAnnotationData(const TrackingAnnotationData &other)
{
    this->operator=(other);
}

TrackingAnnotationData::~TrackingAnnotationData()
{

}

TrackingAnnotationData& TrackingAnnotationData::operator= (const TrackingAnnotationData &other)
{
    this->pos = other.pos;
    this->links = other.links;
    this->shape = other.shape;
    return *this;
}

bool TrackingAnnotationData::operator!= (const TrackingAnnotationData &other)
{
    bool ret = false;
    if(this->pos != other.pos) ret = true;
    if(this->shape != other.shape) ret = true;
    if(this->links != other.links) ret = true;
    return ret;
}

int TrackingAnnotationData::GetAnnotationAtTime(unsigned long long time,
    std::vector<std::vector<float> > &annot)
{

    annot.clear();
    //Check if there is annotation
    //at the requested time
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(time);
    if(it != this->pos.end())
    {
        annot = it->second;
        return 1;
    }
    return 0;
}

int TrackingAnnotationData::GetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          unsigned long long requestedTime,
                                                          std::vector<std::vector<float> > &annot,
                                                          unsigned long long &outAnnotationTime)
{
    //Try to find annotation within the duration of the frame
    outAnnotationTime = 0;
    annot.clear();
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            outAnnotationTime = it->first;
            annot = it->second;
            return 1;
        }
    }

    //If the above did not find a frame, check if there is annotation
    //at the requested time
    int found = GetAnnotationAtTime(requestedTime, annot);
    if(found)
    {
        outAnnotationTime = requestedTime;
        return 1;
    }

    //Failed to find annotation
    return 0;
}

vector<unsigned long long> TrackingAnnotationData::GetAnnotationTimesBetweenTimestamps(unsigned long long startTime,
                                                                                      unsigned long long endTime)
{

    vector<unsigned long long> out;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            out.push_back(t);
        }
    }

    return out;
}

void TrackingAnnotationData::DeleteAnnotationAtTimestamp(unsigned long long annotationTimeIn)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(annotationTimeIn);
    if(it != this->pos.end())
    {
        this->pos.erase(it);
    }
}

void TrackingAnnotationData::SetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          std::vector<std::vector<float> > annot)
{

    if(annot.size() != this->shape.size())
    {
        cout << "Error: Cannot set annotation to mismatched size" << endl;
        return;
    }

    //Set annotation for preset frames
    int found = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            it->second = annot;
            found = 1;
        }
    }
    if(found)
    {
        return;
    }

    //No annotation data set, so create a new annotation entry
    assert(endTime >= startTime);
    unsigned long long midTime = ROUND_TIMESTAMP(0.5*(startTime + endTime));
    this->pos[midTime] = annot;
}


//*********************************************************

void TrackingAnnotationData::ReadAnnotationXml(QDomElement &elem)
{
    this->shape.clear();
    this->links.clear();
    this->pos.clear();
    QDomNode n = elem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if(e.tagName() == "shape")
            {
                std::vector<std::vector<float> > shapeData = ProcessXmlDomFrame(e);
                this->shape = shapeData;
            }

            //Obsolete format loading code here
            if(e.tagName() == "frame")
            {
                std::vector<std::vector<float> > frame = ProcessXmlDomFrame(e);
                //cout << e.attribute("time").toFloat() << endl;
                float timeSec = e.attribute("time").toFloat();
                assert(timeSec > 0.f);
                assert(frame.size() == this->shape.size());
                this->pos[(unsigned long long)(timeSec * 1000.f + 0.5)] = frame;
            }

            //Newer frame XML format
            if(e.tagName() == "frameset")
            {
                ReadFramesXml(e);
            }

            if(e.tagName() == "demoframe")
            {
                ReadDemoFramesXml(e);
            }
            if(e.tagName() == "available")
            {
                this->frameTimes.clear();
                this->frameTimesEnd = e.attribute("to").toULong();

                QDomElement frEl = e.firstChildElement();
                while(!frEl.isNull())
                {
                    if(frEl.tagName() != "f") continue;
                    unsigned long s = frEl.attribute("s").toULong();
                    unsigned long e = frEl.attribute("e").toULong();
                    this->frameTimes[s] = e;
                    frEl = frEl.nextSiblingElement();
                }
            }
        }
        n = n.nextSibling();
    }
    this->lock.unlock();
}

void TrackingAnnotationData::ReadFramesXml(QDomElement &elem)
{
    QDomElement e = elem.firstChildElement();
    while(!e.isNull())
    {
        if(e.tagName() != "frame") continue;

        std::vector<std::vector<float> > frame = ProcessXmlDomFrame(e);
        //cout << e.attribute("time").toFloat() << endl;
        float timeSec = e.attribute("time").toFloat();
        assert(timeSec > 0.f);
        assert(frame.size() == this->shape.size());
        this->pos[(unsigned long long)(timeSec * 1000.f + 0.5)] = frame;

        e = e.nextSiblingElement();
    }
}

void TrackingAnnotationData::ReadDemoFramesXml(QDomElement &elem)
{
    QString content = elem.text();
    int test = content.length();
    QString test2 = elem.tagName();

    QByteArray encData = QByteArray::fromBase64(content.toLocal8Bit().constData());
    int test3 = encData.length();

    QByteArray secretKey(SECRET_KEY);
    QBlowfish bf(secretKey);
    bf.setPaddingEnabled(true);

    QByteArray encryptedBa = bf.decrypted(encData);
    QString framesXml = QString::fromUtf8(encryptedBa);

    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(framesXml, &errorMsg))
    {
        throw runtime_error(errorMsg.toLocal8Bit().constData());
    }

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    this->ReadFramesXml(rootElem);
}

void TrackingAnnotationData::WriteAnnotationXml(QTextStream &out)
{
    out << "\t<tracking>" << endl;
    this->WriteShapeToStream(out);

    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;

    //Save annotated frames
    QString frameXmlStr;
    QTextStream frameXml(&frameXmlStr);
    frameXml << "\t<frameset>" << endl;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        assert(frame.size() == this->shape.size());
        frameXml << "\t<frame time='"<<(it->first/1000.f)<<"'>\n";
        for(unsigned int i=0; i < frame.size(); i++)
        {
            frameXml << "\t\t<point id='"<<i<<"' x='"<<frame[i][0]<<"' y='"<<frame[i][1]<<"'/>\n";
        }
        frameXml << "\t</frame>" << endl;
    }
    frameXml << "\t</frameset>" << endl;

#ifndef DEMO_MODE
    out << frameXml.string();
#else
    out << "<demoframe>" << endl;
    QByteArray secretKey(SECRET_KEY);

    QBlowfish bf(secretKey);
    bf.setPaddingEnabled(true);
    QByteArray encryptedBa = bf.encrypted(frameXmlStr.toUtf8());
    QByteArray encBase64 = encryptedBa.toBase64();
    for(int pos=0;pos<encBase64.length();pos+=512)
        out << encBase64.mid(pos,512) << endl;
    //out << encBase64 << endl;

    out << "</demoframe>" << endl;

#endif //DEMO_MODE

    //Save frame start and end times
    out << "\t<available to=\""<< this->frameTimesEnd << "\">" << endl;
    for(std::map<unsigned long, unsigned long>::iterator it = this->frameTimes.begin();
        it != this->frameTimes.end();
        it++)
    {
        unsigned long st = it->first;
        out << "\t<f s=\""<<st<<"\" e=\""<<this->frameTimes[st]<<"\"/>" << endl;
    }

    out << "\t</available>" << endl;
    out << "\t</tracking>" << endl;
}


void TrackingAnnotationData::LoadAnnotation()
{
    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Annotation"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    //Parse XML to DOM
    QFile f(fileName);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        return;
    }
    f.close();

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    this->ReadAnnotationXml(rootElem);
}

void TrackingAnnotationData::SaveAnnotation()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Annotation Track"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    this->WriteAnnotationXml(out);
    f.close();
}

void TrackingAnnotationData::RemovePoint(int index)
{
    assert(index >=0);
    assert(index < this->shape.size());

    //Remove from existing annotaiton frames
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        frame.erase(frame.begin()+index);
        assert(frame.size() == this->shape.size() - 1);
    }

    //Remove from points list
    this->shape.erase(this->shape.begin()+index);

    //Update links with a higher index number
    vector<vector<int> > filteredLinks;
    for(unsigned int i=0;i<this->links.size();i++)
    {
        int broken = 0;
        vector<int> &link = this->links[i];
        if(link[0]==index) broken = 1;
        if(link[1]==index) broken = 1;
        if(link[0]>index) link[0] --;
        if(link[1]>index) link[1] --;
        if(!broken) filteredLinks.push_back(link);
    }
    this->links = filteredLinks;
}

void TrackingAnnotationData::AddPoint(std::vector<float> p)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        frame.push_back(p);
        assert(frame.size() == this->shape.size() + 1);
    }

    //Apply to currunt shape template
    this->shape.push_back(p);

}


unsigned long long TrackingAnnotationData::GetSeekFowardTime(unsigned long long queryTime)
{
    assert(this!=NULL);

    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti <= queryTime) continue; //Ignore frames in the past
        unsigned long long diff = AbsDiff(ti, queryTime);
        if(!bestSet || diff < bestDiff)
        {
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
            cout << bestFrame << "," << bestDiff << endl;
        }
    }
    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

unsigned long long TrackingAnnotationData::GetSeekBackTime(unsigned long long queryTime)
{
    assert(this!=NULL);

    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti >= queryTime) continue; //Ignore frames in the future
        unsigned long long diff = AbsDiff(ti, queryTime);
        if(!bestSet || diff < bestDiff)
        {
            cout << bestFrame << "," << bestDiff << endl;
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
        }
    }

    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

void TrackingAnnotationData::SetShape(std::vector<std::vector<float> > shapeIn)
{
    this->shape = shapeIn;

    //Check existing data to see if has the correct number of points
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        while(frame.size() > shape.size())
        {
            frame.pop_back();
        }
        while(frame.size() < shape.size())
        {
            frame.push_back(shape[frame.size()]);
        }
    }
}

void TrackingAnnotationData::WriteShapeToStream(
        std::vector<std::vector<int> > links,
        std::vector<std::vector<float> > shape,
        QTextStream &out)
{
    out << "\t<shape>" << endl;

    for(unsigned int i=0; i < shape.size(); i++)
    {
        out << "\t\t<point id='"<<i<<"' x='"<<shape[i][0]<<"' y='"<<shape[i][1]<<"'/>" << endl;
    }
    for(unsigned int i=0;i < links.size();i++)
    {
        out << "\t\t<link from='"<<links[i][0]<<"' to='"<<links[i][1]<<"'/>" << endl;
    }

    out << "\t</shape>" << endl;
}

void TrackingAnnotationData::SaveShape()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0) return;

    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;;
    this->WriteShapeToStream(this->links,this->shape,out);
    f.close();
}

//****************************************************

AnnotThread::AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterfaceIn)
{
    this->parentAnn = annIn;
    this->srcDurationSet = 0;
    this->srcDuration = 0;
    this->mediaInterface = mediaInterfaceIn;

    this->currentStartTimestamp = 0;
    this->currentEndTimestamp = 0;
    this->currentTimeSet = 0;
    this->currentModelSet = 0;

    this->frameTimesEnd = 0;
    this->frameTimesSet = false;
}

AnnotThread::~AnnotThread()
{

}

void AnnotThread::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);
    this->eventLoop->AddListener("PREDICTION_RESULT", *this->eventReceiver);
    this->eventLoop->AddListener("STOP_SPECIFIC_THREAD", *this->eventReceiver);
}

void AnnotThread::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type=="STOP_SPECIFIC_THREAD")
    {
        QUuid algUid = this->parentAnn->GetAlgUid();
        QUuid reqUid(ev->data.c_str());
        if(algUid == reqUid)
        {
            this->stopThreads = true;
        }
    }

    MessagableThread::HandleEvent(ev);
}

void AnnotThread::Update()
{
    //Check if this thread should be active and access the video
    assert(this->parentAnn != NULL);
    QUuid algUid = this->parentAnn->GetAlgUid();
    QString src = this->parentAnn->GetSource();
    int activeStateDesired = this->parentAnn->GetActiveStateDesired();
    if(!activeStateDesired)
    {
        this->msleep(100);
        return;
    }

    if(algUid.isNull())
    {
        //cout << this->parentAnn->GetSource().toLocal8Bit().constData() << endl;
        //cout << algUid.toString().toLocal8Bit().constData() << endl;
        if(this->parentAnn->GetActive())
            this->parentAnn->SetActiveStateDesired(0);
        else
            this->msleep(100);
        return;
    }

    int isActive = this->parentAnn->GetActive();
    if(!isActive)
    {
        this->msleep(100);
        return;
    }

    class TrackingAnnotation *track = this->parentAnn->GetTrack();
    assert(track!=NULL);

    if(!this->srcDurationSet)
    {
        int err = 0;
        try
        {
            this->srcDuration = this->mediaInterface->Length(src);
            cout << "Annot thread found length " << this->srcDuration << endl;
        }
        catch (std::runtime_error &errMsg)
        {
            err = 1;
        }
        if(!err) this->srcDurationSet = 1;
        return;
    }

    //Get list of avilable frames
    if(!this->frameTimesSet)
    {
        track->GetFramesAvailable(this->frameTimes, this->frameTimesEnd);
        this->frameTimesSet = true;
        return;
    }

    unsigned long long frameDuration = 0; //microsec
    unsigned long long avTi = 0; //microsec
    unsigned long long nextTi = 0; //microsec

    //Check algorithm is ready to work
    //TODO

    if(this->frameTimesSet && this->currentTimeSet==false && this->frameTimes.size() > 0)
    {
        std::map<unsigned long, unsigned long>::iterator it = this->frameTimes.begin();
        assert(it!=this->frameTimes.end());
        unsigned long start = it->first;
        unsigned long end = it->second;
        avTi = (unsigned long long)(0.5 * (start + end) + 0.5); //microsec

        std::vector<std::vector<float> > foundAnnot;
        unsigned long long foundAnnotationTime=0;
        int found = track->GetAnnotationBetweenTimestamps(TO_MILLISEC(start),
                                              TO_MILLISEC(end),
                                              TO_MILLISEC(avTi),
                                              foundAnnot,
                                              foundAnnotationTime);

        //Check if first frame has already been annotated
        if(found)
        {
            this->currentTimeSet = true;
            this->currentStartTimestamp = start;
            this->currentEndTimestamp = end;
        }
    }

    //If needed, get the first frame from the video
    if(this->currentTimeSet==false)
    {
        //Get first frame
        QSharedPointer<QImage> img;
        try
        {
            img = this->mediaInterface->Get(src,
                    0, this->currentStartTimestamp, this->currentEndTimestamp);
            cout << "startTimestamp " << this->currentStartTimestamp << endl;
            cout << "endTimestamp " << this->currentEndTimestamp << endl;

            //Update annotation with frame that has been found
            track->FoundFrame(this->currentStartTimestamp, this->currentEndTimestamp);
            avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //microsec

            //Check if annotation is in this frame
            std::vector<std::vector<float> > foundAnnot;
            unsigned long long foundAnnotationTime=0;
            int found = track->GetAnnotationBetweenTimestamps(0,
                                                  TO_MILLISEC(this->currentEndTimestamp),
                                                  TO_MILLISEC(avTi),
                                                  foundAnnot,
                                                  foundAnnotationTime);

            if(found)
            {
                //Update current model from annotation
                this->currentModel = foundAnnot;
                this->currentModelSet = 1;
            }
            else
            {

                //If not annotation here, make a prediction
                if(this->currentModelSet == true)
                this->ImageToProcess(TO_MILLISEC(this->currentStartTimestamp),
                                     TO_MILLISEC(this->currentEndTimestamp),
                                     img, this->currentModel);
            }


        }
        catch (std::runtime_error &err)
        {
            cout << "Timeout getting frame 0" << endl;
            return;
        }
        this->currentTimeSet = true;
        return;
    }

    //If the list of frames has not been set, assume it is blank
    this->frameTimesSet = true;

    //Estimate mid time of next frame
    assert(this->currentTimeSet==true);
    frameDuration = this->currentEndTimestamp - this->currentStartTimestamp; //microsec
    avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //microsec
    nextTi = avTi + frameDuration; //microsec
    assert(nextTi > 0);

    //Check if known frames can satisfy iterations
    int knownFrame = 1;
    int countKnown = 0;
    if(nextTi < srcDuration * 1000) while(knownFrame)
    {
        unsigned long long milsec = TO_MILLISEC(nextTi);

        //Check if the next frame start and end is already known
        std::map<unsigned long, unsigned long>::iterator fit = this->frameTimes.find(this->currentEndTimestamp);
        if(fit != this->frameTimes.end())
        {
            //Frame duration is already known
            //Now check if annotation already exists
            std::vector<std::vector<float> > foundAnnot;
            unsigned long long foundAnnotationTime;

            //Get frame at expected time (fast)
            int found = track->GetAnnotationAtTime(milsec,
                                                  foundAnnot);
            if(found)
            {
                foundAnnotationTime = milsec;
            }
            else
            {
                //Get frames anywhere in frame iterval (slow)
                found = track->GetAnnotationBetweenTimestamps(
                    TO_MILLISEC(this->currentStartTimestamp),
                    TO_MILLISEC(this->currentEndTimestamp),
                    milsec,
                    foundAnnot,
                    foundAnnotationTime);
            }

            if(found)
            {
                //Update current model from annotation
                this->currentModel = foundAnnot;
                this->currentModelSet = 1;
                countKnown ++;

                //This frame is done, go to next frame
                this->currentStartTimestamp = fit->first;
                this->currentEndTimestamp = fit->second;
                frameDuration = this->currentEndTimestamp - this->currentStartTimestamp;
                avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5);
                nextTi = avTi + frameDuration;
            }
            else
                knownFrame = 0;

            //After 100 frames, return to allow thread messages to be processed
            if(countKnown>1000)
            {
                //Estimate progress and generate an event
                double progress = double(milsec) / this->srcDuration;
                std::tr1::shared_ptr<class Event> requestEv(new Event("ANNOTATION_THREAD_PROGRESS"));
                QString progressStr = QString("%0 %1").arg(this->parentAnn->GetAnnotUid()).arg(progress);
                requestEv->data = progressStr.toLocal8Bit().constData();
                this->eventLoop->SendEvent(requestEv);

                return;
            }

        }
        else
            knownFrame = 0;
    }

    //Get subsequent frames
    if(nextTi < srcDuration * 1000)
    {
        unsigned long long milsec = TO_MILLISEC(nextTi);

        QSharedPointer<QImage> img;

        //If needed, get the next frame from video

        try
        {
            cout << "Current time " << milsec << "," << src.toLocal8Bit().constData() << endl;
            img = this->mediaInterface->Get(src,
                    milsec,
                    this->currentStartTimestamp,
                    this->currentEndTimestamp);

            //Update annotation with frame that has been found
            track->FoundFrame(this->currentStartTimestamp, this->currentEndTimestamp);
        }
        catch (std::runtime_error &err)
        {
            this->parentAnn->SetActiveStateDesired(0);
            this->currentTimeSet = false;
            return;
        }

        if(this->currentEndTimestamp < milsec)
        {
            this->parentAnn->SetActiveStateDesired(0);
            this->currentTimeSet = false;
            throw runtime_error("Earlier frame found than was requested");
        }

        //Check if annotation is in this frame
        std::vector<std::vector<float> > foundAnnot;
        unsigned long long foundAnnotationTime;
        int found = track->GetAnnotationBetweenTimestamps(TO_MILLISEC(this->currentStartTimestamp),
                                              TO_MILLISEC(this->currentEndTimestamp),
                                              milsec,
                                              foundAnnot,
                                              foundAnnotationTime);

        if(found)
        {
            //Update current model from annotation
            this->currentModel = foundAnnot;
            this->currentModelSet = 1;
        }
        else
        {

            //If not annotation here, make a prediction
            if(this->currentModelSet != 0)
            this->ImageToProcess(TO_MILLISEC(this->currentStartTimestamp),
                                 TO_MILLISEC(this->currentEndTimestamp),
                                 img, this->currentModel);
        }

        //Estimate progress and generate an event
        double progress = double(milsec) / this->srcDuration;
        std::tr1::shared_ptr<class Event> requestEv(new Event("ANNOTATION_THREAD_PROGRESS"));
        QString progressStr = QString("%0 %1").arg(this->parentAnn->GetAnnotUid()).arg(progress);
        requestEv->data = progressStr.toLocal8Bit().constData();
        this->eventLoop->SendEvent(requestEv);

        //Estimate mid time of next frame
        frameDuration = this->currentEndTimestamp - this->currentStartTimestamp;
        avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5);
        nextTi = avTi + frameDuration;
        return;
    }
    else
    {
        this->parentAnn->SetActiveStateDesired(0);
    }

    this->msleep(100);
}

void AnnotThread::Finished()
{
    QString src = this->parentAnn->GetSource();
    cout << "AnnotThread::Finished()" << src.toLocal8Bit().constData() << endl;

}

void AnnotThread::ImageToProcess(unsigned long long startTi,
                                 unsigned long long endTi,
                                 QSharedPointer<QImage> img,
                                 std::vector<std::vector<float> > &model)
{
    QUuid algUid = this->parentAnn->GetAlgUid();
    assert(img->format() == QImage::Format_RGB888);

    assert(this->eventLoop!=NULL);
    int reqId = this->eventLoop->GetId();

    //Ask alg process to make a prediction
    std::tr1::shared_ptr<class Event> requestEv(new Event("PREDICT_FRAME_REQUEST"));
    class ProcessingRequestOrResponse *req = new class ProcessingRequestOrResponse;
    req->img = img;
    req->pos.clear();
    req->pos.push_back(model);
    requestEv->raw = req;
    requestEv->id = reqId;
    requestEv->data = algUid.toString().toLocal8Bit().constData();

    this->eventLoop->SendEvent(requestEv);

    //Wait for response
    try
    {
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(reqId,80000000);
        class TrackingAnnotation *track = this->parentAnn->GetTrack();
        assert(track!=NULL);

        if(ev->type!="PREDICTION_RESULT") return;
        class ProcessingRequestOrResponse *response = (class ProcessingRequestOrResponse *)ev->raw;

        cout << "Rx prediction for " << response->pos[0].size() << endl;
        cout << "Expected " << track->GetShapeNumPoints() << endl;

        if(response->pos[0].size() == track->GetShapeNumPoints())
            track->SetAnnotationBetweenTimestamps(startTi, endTi, response->pos[0]);
        this->currentModel = response->pos[0];
        this->currentModelSet = true;
    }
    catch(std::runtime_error e)
    {
        cout << "Warning: Prediction timed out" << endl;
    }
}

//*****************************************************

Annotation::Annotation()
{
    this->track = NULL;
    this->visible = true;
    this->active = 0;
    this->activeStateDesired = 1;
}

Annotation::~Annotation()
{
    if(this->annotThread)
    {
        annotThread->Stop();
    }
    this->SetTrack(NULL);
}

Annotation& Annotation::operator= (const Annotation &other)
{
    source = other.source;
    uid = other.uid;
    visible = other.visible;
    algUid = other.algUid;
    if(this->track) delete this->track;
    this->track = NULL;

    QObject *par = other.track->parent();
    this->SetTrack(new TrackingAnnotation(par));
    *this->track = *other.track;
    return *this;
}

bool Annotation::operator!= (const Annotation &other)
{
    if(source != other.source) return true;
    if(visible != other.visible) return true;
    if(uid != other.uid) return true;
    if(*track != *other.track) return true;
    if(algUid != other.algUid) return true;
    return false;
}

void Annotation::Clear()
{
    this->SetTrack(NULL);
    this->visible = true;
    QUuid uidBlank;
    this->SetAnnotUid(uidBlank);
    this->algUid = algUid;
    this->source = "";
    std::tr1::shared_ptr<class AnnotThread> thd;
    this->annotThread = thd;
}

void Annotation::SetTrack(class TrackingAnnotation *trackIn)
{
    if(this->track != NULL) delete this->track;
    this->track = trackIn;
}

void Annotation::Clone(class QUuid parentUuid)
{
    assert(0); //This is not thread safe
    this->SetTrack(NULL);
    this->track = new class TrackingAnnotationData(annIn->track);
    this->SetSource(annIn->GetSource());
    *this->track = *trackIn;
}

class TrackingAnnotation *Annotation::GetTrack()
{
    this->lock.lock();
    class TrackingAnnotation *out = this->track;
    this->lock.unlock();
    return out;
}

void Annotation::SetAlgUid(QUuid uidIn)
{
    this->lock.lock();
    this->algUid = uidIn;
    if(this->annotThread!=NULL)
        this->annotThread->SetThreadId(uidIn);
    this->lock.unlock();
}

QUuid Annotation::GetAlgUid()
{
    this->lock.lock();
    QUuid out = this->algUid;
    this->lock.unlock();
    return out;
}

void Annotation::SetAnnotUid(QUuid uidIn)
{
    this->lock.lock();
    this->uid = uidIn;
    if(this->annotThread != NULL)
        this->annotThread->SetThreadId(uidIn);
    this->lock.unlock();
}

QUuid Annotation::GetAnnotUid()
{
    this->lock.lock();
    QUuid out = this->uid;
    this->lock.unlock();
    return out;
}

void Annotation::SetSource(QString sourceIn)
{
    this->lock.lock();
    this->source = sourceIn;
    this->lock.unlock();
}

QString Annotation::GetSource()
{
    this->lock.lock();
    QString out = this->source;
    this->lock.unlock();
    return out;
}

void Annotation::SetActive(int activeIn)
{
    this->lock.lock();
    this->active = activeIn;
    this->lock.unlock();
}

int Annotation::GetActive()
{
    this->lock.lock();
    int out = this->active;
    this->lock.unlock();
    return out;
}

void Annotation::SetActiveStateDesired(int desiredIn)
{
    this->lock.lock();
    this->activeStateDesired = desiredIn;
    this->lock.unlock();
}

int Annotation::GetActiveStateDesired()
{
    this->lock.lock();
    int out = this->activeStateDesired;
    this->lock.unlock();
    return out;
}


void Annotation::Terminate()
{
    if(this->annotThread && this->annotThread->isRunning())
    {
        cout << "Warning: terminating annot thread " << this->source.toLocal8Bit().constData() << endl;
        this->annotThread->terminate();
    }
}

void Annotation::FoundFrame(unsigned long startTi, unsigned long endTi)
{
    if(this->track != NULL)
    {
        this->track->FoundFrame(startTi, endTi);
    }
}

void Annotation::PreDelete()
{

}
