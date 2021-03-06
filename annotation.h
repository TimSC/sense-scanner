#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QImage>
#include <QtCore/QTextStream>
#include <QtXml/QtXml>

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

    int GetAnnotationBeforeTimestamps(unsigned long long ti,
        std::vector<std::vector<float> > &annot,
        unsigned long long &outAnnotationTime);

    void SetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        std::vector<std::vector<float> > annot);

    //Read individual frames
    unsigned int NumMarkedFrames();
    std::vector<unsigned long long> GetMarkTimes();
    //void GetIndexAnnotationXml(unsigned int index, QTextStream *out);
    //unsigned long long GetIndexTimestamp(unsigned int index);

    static void FrameToXml(std::vector<std::vector<float> > &frame,
                    double ti, QTextStream &out);
    static int FrameFromXml(QString &xml,
                             std::vector<std::vector<float> > &frame,
                             double &tiOut);

    void ReadAnnotationXml(QDomElement &elem);
    void WriteAnnotationXml(QTextStream &out, int demoMode);
    void ReadFramesXml(QDomElement &elem);
    void ReadDemoFramesXml(QDomElement &elem);

    void FoundFrame(unsigned long startTi, unsigned long endTi);
    void GetFramesAvailable(std::map<unsigned long, unsigned long> &frameTimesOut,
                            unsigned long &frameTimesEndOut);

    int GetShapeNumPoints();
    std::vector<std::vector<int> > GetLinks();
    void SetLinks(std::vector<std::vector<int> > linksIn);
    std::vector<std::vector<float> > GetShapePositions();
    void SetShape(std::vector<std::vector<float> > shapeIn);

    static void WriteShapeToStream(
            std::vector<std::vector<int> > links,
            std::vector<std::vector<float> > shape,
            QTextStream &out);
    static int XmlToShape(QString xml,
        std::vector<std::vector<int> > &links,
        std::vector<std::vector<float> > &shape);

    static std::vector<std::vector<float> > ProcessXmlDomFrame(QDomElement &rootElem,
        std::vector<std::vector<int> > &linksOut);

    void AddAnnotationAtTime(unsigned long long ti);
    void RemoveAnnotationAtTime(unsigned long long ti);
    std::vector<std::vector<float> > GetAnnotationAtTime(unsigned long long ti);

    unsigned long long GetSeekForwardTime(unsigned long long queryTime);
    unsigned long long GetSeekBackTime(unsigned long long queryTime);
    void RemovePoint(int index);

    void SaveAnnotationCsv(QString fileName);
    int SaveAnnotationMatlab(QString fileName);
    void SaveAnnotationExcel(QString fileName);

    //Keep track of automatically labeled data
    unsigned long long autoLabeledStart;
    unsigned long long autoLabeledEnd;

protected:

    void AddPoint(std::vector<float> p);
    void SaveShape(QString fileName);
    void LoadAnnotation(QString fileName);
    void SaveAnnotation(QString fileName);

    std::map<unsigned long long, std::vector<std::vector<float> > > pos; //contains annotation positions
    std::vector<std::vector<int> > links;
    std::vector<std::vector<float> > shape; //contains the default shape

    //Keep track of frame times that are available
    std::map<unsigned long, unsigned long> frameTimes;
    unsigned long long frameTimesEnd;
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
    AnnotThread(class Annotation *annIn);
    virtual ~AnnotThread();

    void SetEventLoop(class EventLoop *eventLoopIn);
    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void Update();
    void Finished();

    void SendEvent(std::tr1::shared_ptr<class Event> event);
    std::tr1::shared_ptr<class Event> WaitForEventId(unsigned long long id,
                               unsigned timeOutMs = 50000);
    unsigned long long GetNewEventId();

protected:

    class Annotation *parentAnn;

    std::map<unsigned long, unsigned long> frameTimes;
    unsigned long frameTimesEnd;
    bool frameTimesSet;
    int demoMode;
};

class Annotation
{
    /*!
    * Annotation contains various shared state information between
    * the AnnotThread and the GUI.
    */

public:
    Annotation();
    virtual ~Annotation();
    Annotation& operator= (const Annotation &other);
    bool operator!= (const Annotation &other);
    void Clear();
    void SetTrack(class TrackingAnnotationData *trackIn);
    class TrackingAnnotationData *GetTrack();

    void SetAlgUid(QUuid uidIn);
    QUuid GetAlgUid();

    void SetAnnotUid(QUuid uidIn);
    QUuid GetAnnotUid();

    void SetSource(QString uidIn);
    QString GetSource();

    void Terminate();
    void PreDelete();

    static QString GetSourceFilename(QUuid annotUuid,
                                                 class EventLoop *eventLoop,
                                                 class EventReceiver *eventReceiver);

    static QUuid GetAlgUuid(QUuid annotUuid,
                                 class EventLoop *eventLoop,
                                 class EventReceiver *eventReceiver);

    static int GetAnnotationBetweenFrames(unsigned long long startTime,
                                               unsigned long long endTime,
                                               unsigned long long requestedTime,
                                               QUuid annotUuid,
                                               class EventLoop *eventLoop,
                                               class EventReceiver *eventReceiver,
                                               std::vector<std::vector<float> > &frameOut,
                                               double &tiOut);

    static int GetAnnotationBeforeTime(unsigned long long ti,
                                               QUuid annotUuid,
                                               class EventLoop *eventLoop,
                                               class EventReceiver *eventReceiver,
                                               std::vector<std::vector<float> > &frameOut,
                                               double &tiOut);

    static void SetAnnotationBetweenTimestamps(unsigned long long startTime,
                                    unsigned long long endTime,
                                    std::vector<std::vector<float> > annot,
                                    QUuid annotUuid,
                                    class EventLoop *eventLoop);

    static unsigned long long GetAutoLabeledEnd(QUuid annotUuid,
                                    class EventLoop *eventLoop,
                                    class EventReceiver *eventReceiver);

    static void FoundFrameEvent(unsigned long long startTime,
                                     unsigned long long endTime,
                                        QUuid srcUuid,
                                        QUuid annotUuid,
                                     class EventLoop *eventLoop);

    static void SetAutoLabelTimeRange(unsigned long long startTime,
                                     unsigned long long endTime,
                                        QUuid annotUuid,
                                        class EventLoop *eventLoop);

    static QString GetAllAnnotationByXml(QUuid annotUuid,
                                           class EventLoop *eventLoop,
                                           class EventReceiver *eventReceiver);

    static std::vector<std::vector<float> > GetShape(QUuid annotUuid,
                                                          class EventLoop *eventLoop,
                                                          class EventReceiver *eventReceiver,
                                                          std::vector<std::vector<int> > &linksOut);

    static void SetShape(QUuid annotUuid,
                              std::vector<std::vector<float> > shape,
                              std::vector<std::vector<int> > links,
                              class EventLoop *eventLoop,
                              class EventReceiver *eventReceiver);

    std::tr1::shared_ptr<class AnnotThread> annotThread;
    class TrackingAnnotationData *track;

protected:
    QMutex lock;
    QUuid algUid, uid;
    QString source;

};


#endif // ANNOTATION_H
