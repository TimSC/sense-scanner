#include "applymodel.h"
#include "annotation.h"
#include "avbinmedia.h"
#include <iostream>
using namespace std;

ApplyModel::ApplyModel(QUuid annotUuidIn) : MessagableThread()
{
    this->annotUuid = annotUuidIn;
    this->algUuidSet = 0;
    this->srcFinaSet = 0;
    this->srcDurationSet = 0;
    this->srcDuration = 0;
}

ApplyModel::~ApplyModel()
{

}

void ApplyModel::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);
    this->eventLoop->AddListener("SOURCE_FILENAME", *this->eventReceiver);
    this->eventLoop->AddListener("ALG_UUID_FOR_ANNOTATION", *this->eventReceiver);
    this->eventLoop->AddListener("MEDIA_DURATION_RESPONSE", *this->eventReceiver);
}

void ApplyModel::SetMediaInterface(QUuid mediaInterfaceIn)
{
    this->mediaInterface = mediaInterfaceIn;
}

void ApplyModel::Update()
{

    //Get source filename for annotation
    if(!srcFinaSet)
    {
        this->srcFina = Annotation::GetSourceFilename(this->annotUuid,
                                                     this->eventLoop,
                                                     this->eventReceiver);
        this->srcFinaSet = 1;
        this->msleep(5);
        return;
    }
    cout << qPrintable(srcFina) << endl;

    //Get algorithm Uuid for this annotation track
    if(!this->algUuidSet)
    {
        this->algUuid = Annotation::GetAlgUuid(this->annotUuid,
                                           this->eventLoop,
                                           this->eventReceiver);

        this->algUuidSet = 1;
        this->msleep(5);
        return;
    }
    cout << qPrintable(algUuid) << endl;

    /*

    //Check if this thread should be active and access the video
    assert(this->parentAnn != NULL);
    QUuid algUid = this->parentAnn->GetAlgUid();

    QString src = this->parentAnn->GetSource();
    int activeStateDesired = this->parentAnn->GetActiveStateDesired();
    if(!activeStateDesired)
    {
        this->msleep(5);
        return;
    }

    if(algUid.isNull())
    {
        //cout << this->parentAnn->GetSource().toLocal8Bit().constData() << endl;
        //cout << algUid.toString().toLocal8Bit().constData() << endl;
        if(this->parentAnn->GetActive())
            this->parentAnn->SetActiveStateDesired(0);
        else
            this->msleep(5);
        return;
    }

    int isActive = this->parentAnn->GetActive();
    if(!isActive)
    {
        this->msleep(5);
        return;
    }
*/

    if(!this->srcDurationSet)
    {
        this->srcDuration = AvBinMedia::GetMediaDuration(this->srcFina,
                                            this->mediaInterface,
                                            this->eventLoop,
                                            this->eventReceiver);
        this->srcDurationSet = 1;
        this->msleep(5);
        return;
    }

/*
    //Get list of avilable frames
    if(!this->frameTimesSet)
    {
        track->GetFramesAvailable(this->frameTimes, this->frameTimesEnd);
        this->frameTimesSet = true;
        this->msleep(5);
        return;
    }

    unsigned long long frameDuration = 0; //millisec
    unsigned long long avTi = 0; //millisec
    unsigned long long nextTi = 0; //millisec

    //Check algorithm is ready to work
    //TODO

    //WHAT is this for? Getting the start frame time from cache?
    if(this->frameTimesSet && this->currentTimeSet==false && this->frameTimes.size() > 0)
    {
        std::map<unsigned long, unsigned long>::iterator it = this->frameTimes.begin();
        assert(it!=this->frameTimes.end());
        unsigned long start = it->first;
        unsigned long end = it->second;
        avTi = (unsigned long long)(0.5 * (start + end) + 0.5); //millisec

        std::vector<std::vector<float> > foundAnnot;
        unsigned long long foundAnnotationTime=0;
        int found = track->GetAnnotationBetweenTimestamps(start,
                                              end,
                                              avTi,
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
            std::tr1::shared_ptr<class Event> reqEv(new Event("GET_MEDIA_FRAME"));
            reqEv->toUuid = this->mediaInterface;
            reqEv->data = src;
            QString tiStr = QString("0");
            reqEv->buffer = tiStr.toLocal8Bit().constData();
            reqEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(reqEv);

            std::tr1::shared_ptr<class Event> resp = this->eventReceiver->WaitForEventId(reqEv->id);
            assert(resp->type=="MEDIA_FRAME_RESPONSE");

            MediaResponseFrame processedImg(resp);
            img = QSharedPointer<QImage>(new QImage(processedImg.img));
            //annotTimestamp = processedImg.req;
            this->currentStartTimestamp = processedImg.start;
            this->currentEndTimestamp = processedImg.end;

            cout << "startTimestamp " << this->currentStartTimestamp << endl;
            cout << "endTimestamp " << this->currentEndTimestamp << endl;

            //Update annotation with frame that has been found
            track->FoundFrame(this->currentStartTimestamp, this->currentEndTimestamp);
            avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //millisec

            //Check if annotation is in this frame
            std::vector<std::vector<float> > foundAnnot;
            unsigned long long foundAnnotationTime=0;
            int found = track->GetAnnotationBetweenTimestamps(0,
                                                  this->currentEndTimestamp,
                                                  avTi,
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
                this->ImageToProcess(this->currentStartTimestamp,
                                     this->currentEndTimestamp,
                                     img, this->currentModel);
            }


        }
        catch (std::runtime_error &err)
        {
            cout << "Timeout getting frame 0" << endl;
            this->msleep(5);
            return;
        }
        this->currentTimeSet = true;
        this->msleep(5);
        return;
    }

    //If the list of frames has not been set, assume it is blank
    this->frameTimesSet = true;

    //Estimate mid time of next frame
    assert(this->currentTimeSet==true);
    frameDuration = this->currentEndTimestamp - this->currentStartTimestamp; //millisec
    avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //millisec
    nextTi = avTi + frameDuration; //millisec
    assert(nextTi > 0);

    //Check if known frames can satisfy iterations
    int knownFrame = 1;
    int countKnown = 0;
    if(nextTi < this->srcDuration) while(knownFrame)
    {
        unsigned long long milsec = nextTi;

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
                    this->currentStartTimestamp,
                    this->currentEndTimestamp,
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

            //After 1 frame, return to allow thread messages to be processed
            if(countKnown>0)
            {
                //Estimate progress and generate an event
                double progress = double(milsec) / this->srcDuration;
                std::tr1::shared_ptr<class Event> requestEv(new Event("THREAD_PROGRESS_UPDATE"));
                requestEv->fromUuid = this->parentAnn->GetAnnotUid();
                QString progressStr = QString("%1").arg(progress);
                requestEv->data = progressStr.toLocal8Bit().constData();
                this->eventLoop->SendEvent(requestEv);
                this->msleep(5);
                return;
            }

        }
        else
            knownFrame = 0;
    }

    //Get subsequent frames
    if(nextTi < srcDuration)
    {
        unsigned long long milsec = nextTi;

        QSharedPointer<QImage> img;

        //If needed, get the next frame from video

        try
        {

            std::tr1::shared_ptr<class Event> reqEv(new Event("GET_MEDIA_FRAME"));
            reqEv->toUuid = this->mediaInterface;
            reqEv->data = src;
            QString tiStr = QString("%1").arg(milsec);
            reqEv->buffer = tiStr.toLocal8Bit().constData();
            reqEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(reqEv);

            std::tr1::shared_ptr<class Event> resp = this->eventReceiver->WaitForEventId(reqEv->id);
            assert(resp->type=="MEDIA_FRAME_RESPONSE");

            MediaResponseFrame processedImg(resp);
            img = QSharedPointer<QImage>(new QImage(processedImg.img));
            //annotTimestamp = processedImg.req;
            this->currentStartTimestamp = processedImg.start;
            this->currentEndTimestamp = processedImg.end;

            //cout << "Current time " << milsec << "," << src.toLocal8Bit().constData() << endl;
            //img = this->mediaInterface->Get(src,
            //        milsec,
            //        this->currentStartTimestamp,
            //        this->currentEndTimestamp);

            //Update annotation with frame that has been found
            track->FoundFrame(this->currentStartTimestamp, this->currentEndTimestamp);
        }
        catch (std::runtime_error &err)
        {
            this->parentAnn->SetActiveStateDesired(0);
            this->currentTimeSet = false;
            this->msleep(5);
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
        int found = track->GetAnnotationBetweenTimestamps(this->currentStartTimestamp,
                                              this->currentEndTimestamp,
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
            this->ImageToProcess(this->currentStartTimestamp,
                                 this->currentEndTimestamp,
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
        this->msleep(5);
        return;
    }
    else
    {
        this->parentAnn->SetActiveStateDesired(0);
    }
*/

    this->msleep(1000);
}

void ApplyModel::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{

    MessagableThread::HandleEvent(ev);
}

//**********************************************************

ApplyModelPool::ApplyModelPool()
{

}

ApplyModelPool::~ApplyModelPool()
{
    this->Clear();
}

void ApplyModelPool::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
}

void ApplyModelPool::Add(QUuid uuid, QUuid annotUuid, QUuid mediaInterface)
{
    std::tr1::shared_ptr<class ApplyModel> am(new ApplyModel(annotUuid));
    this->pool[uuid] = am;
    this->pool[uuid]->SetEventLoop(this->eventLoop);
    this->pool[uuid]->SetThreadId(uuid);
    this->pool[uuid]->SetMediaInterface(mediaInterface);
    this->pool[uuid]->Start();
}

void ApplyModelPool::Remove(QUuid uuid)
{
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> >::iterator it;
    it = this->pool.find(uuid);
    if(it!=this->pool.end())
    {
        it.value()->Stop();
        this->pool.remove(it.key());
    }
}

void ApplyModelPool::Clear()
{
    QMap<QUuid, std::tr1::shared_ptr<class ApplyModel> >::iterator it;
    for(it = this->pool.begin(); it!=this->pool.end();it++)
    {
        it.value()->Stop();
    }
    this->pool.clear();
}

