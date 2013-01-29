#ifndef SCENECONTROLLER_H
#define SCENECONTROLLER_H

#include <QtGui/QGraphicsScene>
#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QtXml>

class MouseGraphicsScene : public QGraphicsScene
{
    /*!
    * This is a QGraphicsScene with various methods overridden
    * to monitor for special mouse events.
    */

public:
    MouseGraphicsScene(QObject *parent);
    virtual ~MouseGraphicsScene();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);

    void SetSceneControl(class TrackingAnnotation *sceneControlIn);

protected:
    class TrackingAnnotation *sceneControl;
};

class TrackingAnnotation : public QObject
{
    /*!
    * TrackingAnnotation contains the annotation data for a single video and
    * handles the GUI, loading and saving functionality of tracking.
    */

    Q_OBJECT
public:
    TrackingAnnotation(QObject *parent);
    TrackingAnnotation(const TrackingAnnotation &other);
    virtual ~TrackingAnnotation();
    TrackingAnnotation& operator= (const TrackingAnnotation &other);
    bool operator!= (const TrackingAnnotation &other);

    void VideoImageChanged(QImage &fr, unsigned long long startTime,
                           unsigned long long endTime,
                           unsigned long long requestedTime);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);
    int NearestPoint(float x, float y, std::vector<std::vector<float> > &currentFrame);
    int NearestLink(float x, float y, std::vector<std::vector<float> > &currentFrame);
    void Redraw();

    QWidget *ControlsFactory(QWidget *parent);
    void DestroyControls();

    QMenu *MenuFactory(QMenuBar *menuBar);

    void RemovePoint(int index);

    int GetMouseOver();
    void MouseEnterEvent();
    void MouseLeaveEvent();
    unsigned long long GetSeekFowardTime();
    unsigned long long GetSeekBackTime();
    void WriteShapeToStream(QTextStream &textStream);
    std::vector<std::vector<float> > ProcessXmlDomFrame(QDomElement &e);
    QSharedPointer<MouseGraphicsScene> GetScene();

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

public slots:
    void MarkFramePressed(bool val);
    void MovePressed();
    void MoveAllPressed();
    void AddPointPressed();
    void RemovePointPressed();
    void AddLinkPressed();
    void RemoveLinkPressed();

    void LoadShape();
    void SaveShape();
    void SetShapeFromCurentFrame();
    void ResetCurentFrameShape();
    int GetShapeNumPoints();
    void LoadAnnotation();
    void SaveAnnotation();

    void ReadAnnotationXml(QDomElement &elem);
    void WriteAnnotationXml(QTextStream &out);

    void FoundFrame(unsigned long startTi, unsigned long endTi);
    void GetFramesAvailable(std::map<unsigned long, unsigned long> &frameTimesOut,
                            unsigned long &frameTimesEndOut);

protected:
    QSharedPointer<MouseGraphicsScene> scene;
    int activePoint; //which point is selected
    unsigned int imgHeight, imgWidth;
    float markerSize;
    int leftDrag;
    QString mode; //current mode of the gui
    QImage img;
    float mousex, mousey;
    QSharedPointer<QGraphicsPixmapItem> item;
    std::map<unsigned long long, std::vector<std::vector<float> > > pos; //contains annotation positions
    std::vector<std::vector<int> > links;
    unsigned long long frameStartTime, frameEndTime, frameRequestTime;
    unsigned long long annotationTime;
    int annotationTimeSet;
    int mouseOver;
    QPushButton *markFrameButton;
    std::vector<std::vector<float> > shape; //contains the default shape
    QWidget *annotationControls;
    QMutex lock;

    //Keep track of frame times that are available
    std::map<unsigned long, unsigned long> frameTimes;
    unsigned long frameTimesEnd;
};

#endif // SCENECONTROLLER_H
