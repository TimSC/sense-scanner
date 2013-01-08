#include "annotation.h"
#include "scenecontroller.h"
#include "avbinmedia.h"
#include <assert.h>
#include <iostream>
using namespace std;

AnnotThread::AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterfaceIn)
{
    this->parentAnn = annIn;
    this->srcDurationSet = 0;
    this->srcDuration = 0;
    this->mediaInterface = mediaInterfaceIn;

    this->currentStartTimestamp = 0;
    this->currentEndTimestamp = 0;
    this->currentTimeSet = 0;
}

AnnotThread::~AnnotThread()
{

}

void AnnotThread::Update()
{
    int isActive = this->parentAnn->GetActive();
    if(!isActive)
    {
        this->msleep(100);
        return;
    }

    //cout << "x" << (unsigned long)this << endl;
    assert(this->parentAnn != NULL);
    QUuid algUid = this->parentAnn->GetAlgUid();
    QString src = this->parentAnn->GetSource();
    if(!algUid.isNull())
    {
        //cout << this->parentAnn->GetSource().toLocal8Bit().constData() << endl;
        //cout << algUid.toString().toLocal8Bit().constData() << endl;

    }
    class SimpleSceneController *track = this->parentAnn->GetTrack();

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

    //Check algorithm is ready to work
    //TODO

    if(!currentTimeSet)
    {
        //Get first frame
        QSharedPointer<QImage> img;
        try
        {
            img = this->mediaInterface->Get(src,
                    0, this->currentStartTimestamp, this->currentEndTimestamp);
            cout << "startTimestamp " << this->currentStartTimestamp << endl;
            cout << "endTimestamp " << this->currentEndTimestamp << endl;
        }
        catch (std::runtime_error &err)
        {
            cout << "Timeout getting frame 0" << endl;
            return;
        }
        currentTimeSet = 1;
        return;
    }

    //Estimate mid time of next frame
    unsigned long long frameDuration = this->currentEndTimestamp - this->currentStartTimestamp; //microsec
    unsigned long long avTi = (unsigned long long)(0.5 * (this->currentStartTimestamp + this->currentEndTimestamp) + 0.5); //microsec
    unsigned long long nextTi = avTi + frameDuration; //microsec

    //Get subsequent frames
    if(nextTi < srcDuration * 1000)
    {
        QSharedPointer<QImage> img;
        unsigned long long milsec = (unsigned long long)(nextTi / 1000. + 0.5);
        try
        {
            cout << "Current time " << milsec << endl;
            img = this->mediaInterface->Get(src,
                    milsec,
                    this->currentStartTimestamp,
                    this->currentEndTimestamp);

            /*assert(img->format() == QImage::Format_RGB888);
            QString imgPreamble1 = QString("DATA_BLOCK=%1\n").arg(img->byteCount());
            QString imgPreamble2 = QString("RGB_IMAGE_DATA TIMESTAMP=%1 HEIGHT=%2 WIDTH=%3\n").
                    arg(milsec).
                    arg(img->height()).
                    arg(img->width());
            alg->SendCommand(imgPreamble1);
            alg->SendCommand(imgPreamble2);
            QByteArray imgRaw((const char *)img->bits(), img->byteCount());
            alg->SendRawData(imgRaw);

            //Wait for response
            for(int i=0;i<10;i++)
            {
                this->Update();
                LocalSleep::msleep(100);
            }*/

        }
        catch (std::runtime_error &err)
        {
            return;
        }

        if(this->currentEndTimestamp < milsec)
        {
            throw runtime_error("Earlier frame found than was requested");
        }

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
    this->SetTrack(new SimpleSceneController(par));
    *this->track = *other.track;
    return *this;
}

bool Annotation::operator!= (const Annotation &other)
{
    if(source != other.source) return true;
    if(visible != other.visible) return true;
    if(uid != other.uid) return true;
    if(track != other.track) return true;
    if(algUid != other.algUid) return true;
    return false;
}

void Annotation::Clear()
{
    this->SetTrack(NULL);
    this->visible = true;
    QUuid uidBlank;
    this->uid = uidBlank;
    this->algUid = algUid;
    this->source = "";
    std::tr1::shared_ptr<class AnnotThread> thd;
    this->annotThread = thd;
}

void Annotation::SetTrack(class SimpleSceneController *trackIn)
{
    if(this->track != NULL) delete this->track;
    this->track = trackIn;
}

void Annotation::CloneTrack(class SimpleSceneController *trackIn)
{
    this->SetTrack(NULL);
    this->track = new class SimpleSceneController(trackIn->parent());
    *this->track = *trackIn;
}

class SimpleSceneController *Annotation::GetTrack()
{
    this->lock.lock();
    class SimpleSceneController *out = this->track;
    this->lock.unlock();
    return out;
}

void Annotation::SetAlgUid(QUuid uidIn)
{
    this->lock.lock();
    this->algUid = uidIn;
    this->lock.unlock();
}

QUuid Annotation::GetAlgUid()
{
    this->lock.lock();
    QUuid out = this->algUid;
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



