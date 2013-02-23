#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QImage>
#include "eventloop.h"

//**********************************************************************

class TrackingAnnotationData
{
public:
    /*!
     * This class contains the tracking annotation for a single video.
     */

    TrackingAnnotationData();
    TrackingAnnotationData(const TrackingAnnotationData &other);
    virtual ~TrackingAnnotationData();
    TrackingAnnotationData& operator= (const TrackingAnnotationData &other);
    bool operator!= (const TrackingAnnotationData &other);

    int GetAnnotationAtTime(unsigned long long time,
        std::vector<std::vector<float> > &annot);
    int GetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        unsigned long long requestedTime,
        std::vector<std::vector<float> > &annot,
        unsigned long long &annotationTime);
    void DeleteAnnotationAtTimestamp(unsigned long long annotationTime);
    std::vector<unsigned long long> GetAnnotationTimesBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime);
    void SetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        std::vector<std::vector<float> > annot);

    //Read individual frames
    unsigned int NumMarkedFrames();
    void GetIndexAnnotationXml(unsigned int index, QTextStream *out);
    unsigned long long GetIndexTimestamp(unsigned int index);

    void ReadAnnotationXml(QDomElement &elem);
    void WriteAnnotationXml(QTextStream &out);
    void ReadFramesXml(QDomElement &elem);
    void ReadDemoFramesXml(QDomElement &elem);

protected:
    std::map<unsigned long long, std::vector<std::vector<float> > > pos; //contains annotation positions
    std::vector<std::vector<int> > links;
    std::vector<std::vector<float> > shape; //contains the default shape
};

//****************************************************

class AnnotThread : public MessagableThread
{
    /*!
    * AnnotThread is a worker thread that allows annotations to create events
    * and continue processing without interupting the GUI thread. It also
    * manages the current tracker positions and frame iteration during tracking
    * of new videos.
    */

public:
    AnnotThread(class Annotation *annIn, class AvBinMedia* mediaInterface);
    virtual ~AnnotThread();

    void SetEventLoop(class EventLoop *eventLoopIn);
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void Update();
    void Finished();

protected:

    void ImageToProcess(unsigned long long startTi,
                        unsigned long long endTi,
                        QSharedPointer<QImage> img,
                        std::vector<std::vector<float> > &model);

    class Annotation *parentAnn;
    int srcDurationSet;
    long long unsigned srcDuration;
    class AvBinMedia* mediaInterface;

    unsigned long long currentStartTimestamp, currentEndTimestamp;
    int currentTimeSet;
    std::vector<std::vector<float> > currentModel;
    int currentModelSet;

    std::map<unsigned long, unsigned long> frameTimes;
    unsigned long frameTimesEnd;
    bool frameTimesSet;
};

class Annotation
{
    /*!
    * Annotation contains various shared state information between
    * the AnnotThread and the GUI. Some of the methods are thread safe
    * to enable iteraction between the two.
    */

public:
    Annotation();
    virtual ~Annotation();
    Annotation& operator= (const Annotation &other);
    bool operator!= (const Annotation &other);
    void Clear();
    void SetTrack(class TrackingAnnotationData *trackIn);
    void CloneTrack(class TrackingAnnotationData *trackIn);
    class TrackingAnnotation *GetTrack();

    void SetAlgUid(QUuid uidIn); //Thread safe
    QUuid GetAlgUid(); //Thread safe

    void SetAnnotUid(QUuid uidIn); //Thread safe
    QUuid GetAnnotUid(); //Thread safe

    void SetSource(QString uidIn); //Thread safe
    QString GetSource(); //Thread safe

    void SetActive(int activeIn);
    int GetActive();

    void SetActiveStateDesired(int desiredIn);
    int GetActiveStateDesired();

    void FoundFrame(unsigned long startTi, unsigned long endTi);

    void Terminate();
    void PreDelete();

    bool visible;

    std::tr1::shared_ptr<class AnnotThread> annotThread;

protected:
    QMutex lock;
    class TrackingAnnotationData *track;
    QUuid algUid, uid;
    QString source;
    int active;
    int activeStateDesired;

};


#endif // ANNOTATION_H
