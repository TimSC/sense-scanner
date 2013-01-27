#include "annotation.h"
#include "scenecontroller.h"
#include "avbinmedia.h"
#include "localsleep.h"
#include <assert.h>
#include <iostream>
using namespace std;

#define TO_MILLISEC(x) (unsigned long long)(x / 1000. + 0.5)

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

                //This frame is done, go to next frame
                this->currentStartTimestamp = fit->first;
                this->currentEndTimestamp = fit->second;
                frameDuration = this->currentEndTimestamp - this->currentStartTimestamp;
                avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5);
                nextTi = avTi + frameDuration;

                //Estimate progress and generate an event
                double progress = double(milsec) / this->srcDuration;
                std::tr1::shared_ptr<class Event> requestEv(new Event("ANNOTATION_THREAD_PROGRESS"));
                QString progressStr = QString("%0 %1").arg(this->parentAnn->GetAnnotUid()).arg(progress);
                requestEv->data = progressStr.toLocal8Bit().constData();
                this->eventLoop->SendEvent(requestEv);
            }
            else
                knownFrame = 0;

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
            return;
        }

        if(this->currentEndTimestamp < milsec)
        {
            this->parentAnn->SetActiveStateDesired(0);
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

void Annotation::CloneTrack(class TrackingAnnotation *trackIn)
{
    this->SetTrack(NULL);
    this->track = new class TrackingAnnotation(trackIn->parent());
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
