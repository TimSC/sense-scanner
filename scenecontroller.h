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
public:
    MouseGraphicsScene(QObject *parent);
    virtual ~MouseGraphicsScene();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);

    void SetSceneControl(class SimpleSceneController *sceneControlIn);

protected:
    class SimpleSceneController *sceneControl;
};

class SimpleSceneController : public QObject
{
    Q_OBJECT

public:
    SimpleSceneController(QObject *parent);
    SimpleSceneController(const SimpleSceneController &other);
    virtual ~SimpleSceneController();
    SimpleSceneController& operator= (const SimpleSceneController &other);
    bool operator!= (const SimpleSceneController &other);

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
    void LoadAnnotation();
    void SaveAnnotation();

    void ReadAnnotationXml(QDomElement &elem);
    void WriteAnnotationXml(QTextStream &out);

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
    int mouseOver;
    QPushButton *markFrameButton;
    std::vector<std::vector<float> > shape; //contains the default shape
    QWidget *annotationControls;

};

#endif // SCENECONTROLLER_H
