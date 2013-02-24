#ifndef SCENECONTROLLER_H
#define SCENECONTROLLER_H

#include <QtGui/QGraphicsScene>
#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QtXml>
#ifdef _MSC_VER
    #include <memory>
#else
    #include <tr1/memory>
#endif

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

    void SetSceneControl(class TrackingSceneController *sceneControlIn);

protected:
    class TrackingSceneController *sceneControl;
};

class TrackingSceneController : public QObject
{
    /*!
    * TrackingAnnotation contains the handles the GUI for tracking data.
    */

    Q_OBJECT
public:
    TrackingSceneController(QObject *parent);
    virtual ~TrackingSceneController();

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
    QMenu *MenuFactory(QMenuBar *menuBar);

    int GetMouseOver();
    void MouseEnterEvent();
    void MouseLeaveEvent();
    unsigned long long GetSeekFowardTime();
    unsigned long long GetSeekBackTime();
    static void WriteShapeToStream(
            std::vector<std::vector<int> > links,
            std::vector<std::vector<float> > shape,
            QTextStream &out);
    static std::vector<std::vector<float> > ProcessXmlDomFrame(QDomElement &rootElem,
        std::vector<std::vector<int> > linksOut);
    QSharedPointer<MouseGraphicsScene> GetScene();

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
    void LoadAnnotation();
    void SaveAnnotation();

    void SetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        std::vector<std::vector<float> > annot);

    void SetAnnotationTrack(QUuid srcUuid);

    void SetEventLoop(class EventLoop *eventLoopIn);
    //virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void RefreshCurrentPos();
    std::vector<std::vector<float> > GetShape();
    void SetShape(std::vector<std::vector<float> > shape);
    void RefreshLinks();
protected:

    int GetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        unsigned long long requestedTime,
        std::vector<std::vector<float> > &annot,
        unsigned long long &annotationTime);

    unsigned long long GetSeekFowardTimeFromAnnot(unsigned long long queryTime);
    unsigned long long GetSeekBackwardTimeFromAnnot(unsigned long long queryTime);

    void RemoveAnnotationAtTime(unsigned long long time);
    void AddAnnotationAtTime(unsigned long long time);
    void RemovePoint(int index);

    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

    QSharedPointer<MouseGraphicsScene> scene;
    int activePoint; //which point is selected
    unsigned int imgHeight, imgWidth;
    float markerSize;
    int leftDrag;
    QString mode; //current mode of the gui
    QImage img;
    float mousex, mousey;
    QSharedPointer<QGraphicsPixmapItem> item;

    std::vector<std::vector<int> > links;
    unsigned long long frameStartTime, frameEndTime, frameRequestTime;
    unsigned long long annotationTime;
    int annotationTimeSet;
    int mouseOver;
    //QPushButton *markFrameButton;
    std::vector<std::vector<float> > currentShape; //contains the default shape
    std::vector<std::vector<float> > defaultShape; //contains the default shape
    //QWidget *annotationControls;
    QPushButton *markFrameButton;
    QWidget *annotationControls;
    int isShapeSet;

    QUuid annotationUuid;
};


#endif // SCENECONTROLLER_H
