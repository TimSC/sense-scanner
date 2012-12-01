#include "scenecontroller.h"
#include <iostream>
#include <vector>
#include <QtGui/QPixmap>
#include "assert.h"
using namespace::std;

//Custom graphics scene to catch mouse move and press

MouseGraphicsScene::MouseGraphicsScene(QWidget *parent) : QGraphicsScene(parent)
{
    this->sceneControl = NULL;
}

MouseGraphicsScene::~MouseGraphicsScene()
{

}

void MouseGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mouseMoveEvent(mouseEvent);
}

void MouseGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mousePressEvent(mouseEvent);
}

void MouseGraphicsScene::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mouseReleaseEvent(mouseEvent);
}

void MouseGraphicsScene::SetSceneControl(SimpleSceneController *sceneControlIn)
{
    this->sceneControl = sceneControlIn;
}

//********************************************************************

SimpleSceneController::SimpleSceneController(QWidget *parent)
{
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(parent));
    this->scene->SetSceneControl(this);
    for(int i=0;i<50;i++)
    {
        std::vector<float> p;
        p.push_back(rand() % 500);
        p.push_back(rand() % 500);
        this->pos.push_back(p);
    }
    activePoint = -1;
    this->imgWidth = 0;
    this->imgHeight = 0;
    this->markerSize = 2.;
    this->leftDrag = 0;
    this->mode = "MOVE";
}

SimpleSceneController::~SimpleSceneController()
{
    this->item = QSharedPointer<QGraphicsPixmapItem>(NULL);
    if(!this->scene.isNull())
    {
        this->scene->clear();
        this->scene->SetSceneControl(NULL);
        this->scene = QSharedPointer<MouseGraphicsScene>(NULL);
    }

}

void SimpleSceneController::VideoImageChanged(QImage &fr)
{
    this->img = fr;
    //this->item =
    //        QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(fr)));
    this->imgWidth = fr.width();
    this->imgHeight = fr.height();
    this->Redraw();
}

void SimpleSceneController::Redraw()
{

    this->scene->clear();
    if(this->imgWidth > 0 && this->imgHeight>0 && !this->img.isNull())
    {
        QGraphicsPixmapItem *tmp = new QGraphicsPixmapItem(QPixmap::fromImage(this->img));
        this->scene->addItem(tmp); //I love pointers
        this->scene->setSceneRect ( 0, 0, this->imgWidth, this->imgHeight );
    }

    QPen penRed(QColor(255,0,0));
    QBrush brushTransparent(QColor(0,0,0,0));
    QBrush brushRed(QColor(255,0,0));

    for(unsigned int i=0;i<this->pos.size();i++)
    {
        //cout << this->activePoint << endl;
        if(i!=this->activePoint)
            this->scene->addEllipse(this->pos[i][0]-this->markerSize/2,
                                    this->pos[i][1]-this->markerSize/2,
                                    this->markerSize, this->markerSize,
                                    penRed, brushRed);
        else
            this->scene->addEllipse(this->pos[i][0]-this->markerSize/2,
                                    this->pos[i][1]-this->markerSize/2,
                                    this->markerSize, this->markerSize,
                                    penRed, brushTransparent);
    }
}

void SimpleSceneController::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    assert(mouseEvent);
    QPointF pos = mouseEvent->scenePos();
    cout << "mouseMoveEvent, " << pos.x() << "," << pos.y () << endl;

    if(this->mode == "MOVE")
    {
    if(this->activePoint >= 0 && this->leftDrag)
    {
        this->pos[this->activePoint][0] = pos.x();
        this->pos[this->activePoint][1] = pos.y();
        this->Redraw();
    }
    }

}

void SimpleSceneController::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mousePressEvent" << endl;
    assert(mouseEvent);
    QPointF pos = mouseEvent->buttonDownScenePos(mouseEvent->button());

    if(this->mode == "MOVE")
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y());
        this->activePoint = nearestPoint;
        this->Redraw();
    }

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 1;
}

void SimpleSceneController::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mouseReleaseEvent" << endl;
    assert(mouseEvent);

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 0;
}

int SimpleSceneController::NearestPoint(float x, float y)
{
    int best = -1;
    float bestDist = -1;
    for(unsigned int i=0;i<this->pos.size();i++)
    {
        float dx = this->pos[i][0] - x;
        float dy = this->pos[i][1] - y;
        float dist = pow(dx*dx + dy*dy, 0.5f);
        if(bestDist < 0. || dist < bestDist)
        {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

QWidget *SimpleSceneController::ControlsFactory(QWidget *parent)
{
    QWidget *layoutW = new QWidget();
    assert(layoutW->layout() == NULL);
    QHBoxLayout *layout = new QHBoxLayout();

    QPushButton *button = new QPushButton("Move");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MovePressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    button->setChecked(true);
    layout->addWidget(button);

    button = new QPushButton("Add");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddPointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemovePointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Add Link");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove Link");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemoveLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);
    layoutW->setLayout(layout);
    return layoutW;
}

void SimpleSceneController::MovePressed()
{
    cout << "Move pressed"<< endl;
    this->mode = "MOVE";
}

void SimpleSceneController::AddPointPressed()
{
    cout << "AddPointPressed"<< endl;
    this->mode = "ADD_POINT";
}

void SimpleSceneController::RemovePointPressed()
{
    cout << "RemovePointPressed"<< endl;
    this->mode = "REMOVE_POINT";
}

void SimpleSceneController::AddLinkPressed()
{
    cout << "AddLinkPressed"<< endl;
    this->mode = "ADD_LINK";
}

void SimpleSceneController::RemoveLinkPressed()
{
    cout << "RemoveLinkPressed"<< endl;
    this->mode = "REMOVE_LINK";
}
