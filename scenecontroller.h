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

    void SetSceneControl(class BaseSceneController *sceneControlIn);

protected:
    class BaseSceneController *sceneControl;
};

//**************************************************

class BaseSceneController : public QObject
{
    Q_OBJECT
public:
    BaseSceneController(QObject *parent);
    virtual ~BaseSceneController();

    virtual QWidget *ControlsFactory(QWidget *parent);
    virtual QMenu *MenuFactory(QMenuBar *menuBar);
    virtual MouseGraphicsScene *GetScene();

    virtual int GetMouseOver();
    virtual void MouseEnterEvent();
    virtual void MouseLeaveEvent();
    virtual unsigned long long GetSeekForwardTime();
    virtual unsigned long long GetSeekBackTime();

    virtual void VideoImageChanged(QImage &fr, unsigned long long startTime,
                           unsigned long long endTime,
                           unsigned long long requestedTime);
    virtual void SetAnnotationTrack(QUuid srcUuid);

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);
    virtual void Redraw();

protected:
    MouseGraphicsScene *scene;
    int mouseOver;
};

//*******************************************************

class TrackingSceneController : public BaseSceneController
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

    unsigned long long GetSeekForwardTime();
    unsigned long long GetSeekBackTime();
    MouseGraphicsScene *GetScene();
    QTimer timer;

public slots:
    void MarkFramePressed(bool val);
    void MovePressed();
    void MoveAllPressed();
    void AddPointPressed();
    void RemovePointPressed();
    void AddLinkPressed();
    void RemoveLinkPressed();

    void LoadShape();
    void LoadShape(QString fina);
    void SaveShape();
    void SetShapeFromCurentFrame();
    void ResetCurentFrameShape();
    void LoadAnnotation();
    void SaveAnnotation();

    void SaveAnnotationCsv();
    void SaveAnnotationMatlab();
    void SaveAnnotationExcel();
    void ExportAnnotation(QString type, QString ext);

    void SetAnnotationTrack(QUuid srcUuid);

    void SetEventLoop(class EventLoop *eventLoopIn);
    //virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void RefreshCurrentPos();
    void RefreshLinks();
    void Update();
    void SetDemoMode(int mode);

protected:

    int GetAnnotationBetweenTimestamps(unsigned long long startTime,
        unsigned long long endTime,
        unsigned long long requestedTime,
        std::vector<std::vector<float> > &annot,
        unsigned long long &annotationTime);

    unsigned long long GetSeekForwardTimeFromAnnot(unsigned long long queryTime);
    unsigned long long GetSeekBackwardTimeFromAnnot(unsigned long long queryTime);

    void RemoveAnnotationAtTime(unsigned long long time);
    void AddAnnotationAtTime(unsigned long long time);
    void RemovePoint(int index);

    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;

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
    //QPushButton *markFrameButton;
    std::vector<std::vector<float> > currentShape; //contains the default shape
    std::vector<std::vector<float> > defaultShape; //contains the default shape
    //QWidget *annotationControls;
    QPushButton *markFrameButton;
    QWidget *annotationControls;

    QUuid annotationUuid;
    int demoMode;

    QAction *saveAnnotationCsv;
    QAction *saveAnnotationMatlab;
    QAction *saveAnnotationExcel;
};


class LogoSceneController : public BaseSceneController
{
    Q_OBJECT
public:
    LogoSceneController(QObject *parent);
    virtual ~LogoSceneController();

    virtual void Redraw();
protected:

};

#endif // SCENECONTROLLER_H
