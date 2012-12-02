#ifndef SCENECONTROLLER_H
#define SCENECONTROLLER_H

#include <QtGui/QGraphicsScene>
#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>

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

    void VideoImageChanged(QImage &fr);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);
    int NearestPoint(float x, float y);
    int NearestLink(float x, float y);
    void Redraw();

    QWidget *ControlsFactory(QWidget *parent);
    void RemovePoint(int index);

    int GetMouseOver();
    void MouseEnterEvent();
    void MouseLeaveEvent();
    unsigned long long GetSeekFowardTime();
    unsigned long long GetSeekBackTime();

    QSharedPointer<MouseGraphicsScene> scene;

public slots:
    void MarkFramePressed();
    void MovePressed();
    void AddPointPressed();
    void RemovePointPressed();
    void AddLinkPressed();
    void RemoveLinkPressed();

protected:
    int activePoint;
    unsigned int imgHeight, imgWidth;
    float markerSize;
    int leftDrag;
    QString mode;
    QImage img;
    float mousex, mousey;
    QSharedPointer<QGraphicsPixmapItem> item;
    std::vector<std::vector<float> > pos;
    std::vector<std::vector<int> > links;
    int mouseOver;
};

#endif // SCENECONTROLLER_H
