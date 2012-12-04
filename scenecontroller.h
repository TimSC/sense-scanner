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
    MouseGraphicsScene(QWidget *parent);
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
    SimpleSceneController(QWidget *parent);
    virtual ~SimpleSceneController();

    void VideoImageChanged(QImage &fr, unsigned long long ti);

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
    int HasChanged();

    QSharedPointer<MouseGraphicsScene> scene;

public slots:
    void MarkFramePressed(bool val);
    void MovePressed();
    void AddPointPressed();
    void RemovePointPressed();
    void AddLinkPressed();
    void RemoveLinkPressed();

    void LoadShape();
    void SaveShape();
    void SetShapeFromCurentFrame();
    void LoadAnnotation();
    void SaveAnnotation();

    void ReadAnnotationXml(QDomElement &elem);
    void WriteAnnotationXml(QTextStream &out);

protected:
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
    unsigned long long currentTime;
    int mouseOver;
    QPushButton *markFrameButton;
    std::vector<std::vector<float> > shape; //contains the default shape
    QWidget *annotationControls;

};

#endif // SCENECONTROLLER_H
