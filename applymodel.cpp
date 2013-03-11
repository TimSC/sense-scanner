#include "applymodel.h"
#include "annotation.h"
#include "avbinmedia.h"
#include "algorithm.h"
#include <assert.h>
#include <iostream>
using namespace std;

ApplyModel::ApplyModel(QUuid annotUuidIn) : MessagableThread()
{
    this->annotUuid = annotUuidIn;
    this->algUuidSet = 0;
    this->srcFinaSet = 0;
    this->srcDurationSet = 0;
    this->srcDuration = 0;
    this->currentTimeSet = 0;
    this->currentStartTimestamp = 0;
    this->currentEndTimestamp = 0;
    this->issueEnountered = 0;
    this->currentModelSet = 0;
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
    this->eventLoop->AddListener("MEDIA_FRAME_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_FRAME", *this->eventReceiver);
    this->eventLoop->AddListener("PREDICTION_RESULT", *this->eventReceiver);
    this->eventLoop->AddListener("AUTO_LABELED_END", *this->eventReceiver);
    this->eventLoop->AddListener("ALG_STATE", *this->eventReceiver);

}

void ApplyModel::SetMediaInterface(QUuid mediaInterfaceIn)
{
    this->mediaInterface = mediaInterfaceIn;
}

void ApplyModel::Update()
{
    if(issueEnountered)
    {
        this->msleep(1);
        return;
    }

    //Get source filename for annotation
    if(!srcFinaSet)
    {
        this->srcFina = Annotation::GetSourceFilename(this->annotUuid,
                                                     this->eventLoop,
                                                     this->eventReceiver);
        this->srcFinaSet = 1;
        this->msleep(1);
        return;
    }

    //Get algorithm Uuid for this annotation track
    if(!this->algUuidSet)
    {
        this->algUuid = Annotation::GetAlgUuid(this->annotUuid,
                                           this->eventLoop,
                                           this->eventReceiver);

        this->algUuidSet = 1;
        this->msleep(1);
        return;
    }

    //Get algorithm progress so far, in case we are resuming from a partly complete computation run
    if(!this->currentModelSet)
    {
        unsigned long long autoLabeledEnd = Annotation::GetAutoLabeledEnd(this->annotUuid,
                                        this->eventLoop,
                                        this->eventReceiver);
        if(autoLabeledEnd>0)
        {
            std::vector<std::vector<float> > annot;
            double ti=0;

            //Initialise current model to closest frame
            int found = Annotation::GetAnnotationBeforeTime(autoLabeledEnd,
                                                            this->annotUuid,
                                                             this->eventLoop,
                                                             this->eventReceiver,
                                                                   annot,
                                                                   ti);
            if(found)
            {
                //Get frame timings to start computation
                this->currentModel = annot;
                this->currentModelSet = true;
                MediaResponseFrame processedImg;
                AvBinMedia::GetMediaFrame(this->srcFina,
                                     ti * 1000.,
                                     this->mediaInterface,
                                     this->eventLoop,
                                     this->eventReceiver,
                                     processedImg);
                this->currentStartTimestamp = processedImg.start;
                this->currentEndTimestamp = processedImg.end;
                this->currentTimeSet = 1;
            }
        }
    }

    if(!this->srcDurationSet)
    {
        this->srcDuration = AvBinMedia::GetMediaDuration(this->srcFina,
                                            this->mediaInterface,
                                            this->eventLoop,
                                            this->eventReceiver);
        this->srcDurationSet = 1;
        this->msleep(1);
        return;
    }

    unsigned long long frameDuration = 0; //millisec
    unsigned long long avTi = 0; //millisec
    unsigned long long nextTi = 0; //millisec

    //Check algorithm is ready to work
    AlgorithmProcess::ProcessState state = AlgorithmProcess::GetState(this->algUuid,
                          this->eventLoop,
                          this->eventReceiver);


    //If needed, get the first frame from the video
    if(this->currentTimeSet==false)
    {
        //Get first frame
        QSharedPointer<QImage> img;
        try
        {

            MediaResponseFrame processedImg;

            AvBinMedia::GetMediaFrame(this->srcFina,
                                 0,
                                 this->mediaInterface,
                                 this->eventLoop,
                                 this->eventReceiver,
                                 processedImg);

            img = QSharedPointer<QImage>(new QImage(processedImg.img));
            //annotTimestamp = processedImg.req;
            this->currentStartTimestamp = processedImg.start;
            this->currentEndTimestamp = processedImg.end;

            //Update annotation with frame that has been found
            Annotation::FoundFrameEvent(this->currentStartTimestamp,
                                                    this->currentEndTimestamp,
                                                    this->algUuid,
                                                    this->annotUuid,
                                                    this->eventLoop);

            //Check if annotation is in this frame
            std::vector<std::vector<float> > foundAnnot;
            double foundAnnotationTime=0;
            int found = Annotation::GetAnnotationBetweenFrames(0,
                                                  0,
                                                  0,
                                                  this->annotUuid,
                                                  this->eventLoop,
                                                  this->eventReceiver,
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
            this->msleep(1);
            return;
        }
        this->currentTimeSet = true;
        this->msleep(1);
        return;
    }

    //Estimate mid time of next frame
    assert(this->currentTimeSet==true);
    frameDuration = this->currentEndTimestamp - this->currentStartTimestamp; //millisec
    avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //millisec
    nextTi = avTi + frameDuration; //millisec
    assert(nextTi > 0);

    //Get subsequent frames
    if(nextTi < this->srcDuration)
    {
        QSharedPointer<QImage> img;

        //If needed, get the next frame from video

        try
        {

            MediaResponseFrame processedImg;

            AvBinMedia::GetMediaFrame(this->srcFina,
                                 nextTi,
                                 this->mediaInterface,
                                 this->eventLoop,
                                 this->eventReceiver,
                                 processedImg);

            img = QSharedPointer<QImage>(new QImage(processedImg.img));
            this->currentStartTimestamp = processedImg.start;
            this->currentEndTimestamp = processedImg.end;

            //Update annotation with frame that has been found
            Annotation::FoundFrameEvent(this->currentStartTimestamp,
                                                    this->currentEndTimestamp,
                                                    this->algUuid,
                                                    this->annotUuid,
                                                    this->eventLoop);
        }
        catch (std::runtime_error &err)
        {
            this->issueEnountered = true;

            this->currentTimeSet = false;
            this->msleep(1);
            return;
        }

        if(this->currentEndTimestamp < nextTi)
        {
            this->issueEnountered = true;

            this->currentTimeSet = false;
            throw runtime_error("Earlier frame found than was requested");
        }


        //Check if annotation is in this frame       
        std::vector<std::vector<float> > foundAnnot;
        double foundAnnotationTime=0;
        int found = Annotation::GetAnnotationBetweenFrames(this->currentStartTimestamp,
                                                          this->currentEndTimestamp,
                                                          nextTi,
                                                          this->annotUuid,
                                                          this->eventLoop,
                                                          this->eventReceiver,
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
        Annotation::SetAutoLabelTimeRange(0, nextTi,
                                            this->annotUuid,
                                            this->eventLoop);

        //Estimate mid time of next frame
        frameDuration = this->currentEndTimestamp - this->currentStartTimestamp;
        avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5);
        nextTi = avTi + frameDuration;
        this->msleep(1);
        return;
    }
    else
    {
        //All done, stop work
        this->issueEnountered = true;
    }

    this->msleep(1000);
}

void ApplyModel::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(this->eventReceiver->BufferSize()>50)
    {
        int debug = this->eventReceiver->BufferSize();
        cout << "debug" << endl;
    }


    MessagableThread::HandleEvent(ev);
}

void ApplyModel::ImageToProcess(unsigned long long startTi,
                                 unsigned long long endTi,
                                 QSharedPointer<QImage> img,
                                 std::vector<std::vector<float> > &model)
{
    std::vector<std::vector<float> > out;

    int ret = AlgorithmProcess::PredictFrame(img,
                            model,
                            this->annotUuid,
                            this->eventLoop,
                            this->eventReceiver,
                            out);

    if(ret == 1)
    {
        this->currentModel = out;
        this->currentModelSet = true;

        Annotation::SetAnnotationBetweenTimestamps(startTi,
                                                   endTi,
                                                   out,
                                                   this->annotUuid,
                                                   this->eventLoop);
    }
    else
    {
        cout << "Warning: Prediction timed out" << endl;
    }

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

