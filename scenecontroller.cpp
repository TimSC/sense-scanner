#include "scenecontroller.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <QtGui/QPixmap>
#include "assert.h"
#include "vectors.h"
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
    this->mode = "MOVE";
    this->mouseOver = false;
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(parent));
    this->scene->SetSceneControl(this);
    for(int i=0;i<50;i++)
    {
        std::vector<float> p;
        p.push_back(rand() % 500);
        p.push_back(rand() % 500);
        this->pos.push_back(p);
    }
    this->activePoint = -1;
    this->imgWidth = 0;
    this->imgHeight = 0;
    this->markerSize = 2.;
    this->leftDrag = 0;
    this->mousex = 0.f;
    this->mousey = 0.f;



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
    QPen penBlue(QColor(0,0,255));

    //If in add link mode
    if(this->mode == "ADD_LINK" && this->activePoint >= 0 && this->mouseOver)
    {
        //draw a link to encourage user to click on the next point
        this->scene->addLine(this->mousex, this->mousey,
                             this->pos[this->activePoint][0], this->pos[this->activePoint][1],
                             penBlue);
    }

    //Draw links
    for(unsigned int i=0;i<this->links.size();i++)
    {
        vector<int> &link = this->links[i];
        assert(link.size()==2);
        this->scene->addLine(this->pos[link[0]][0],this->pos[link[0]][1],
                             this->pos[link[1]][0],this->pos[link[1]][1], penBlue);
    }

    //Draw marker points
    for(unsigned int i=0;i<this->pos.size();i++)
    {
        assert(this->pos[i].size()==2);
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
    //assert(mouseEvent);
    QPointF pos = mouseEvent->scenePos();
    this->mousex = pos.x();
    this->mousey = pos.y();
    //cout << "mouseMoveEvent, " << pos.x() << "," << pos.y () << endl;

    if(this->mode == "MOVE")
    {
    if(this->activePoint >= 0 && this->leftDrag)
    {
        this->pos[this->activePoint][0] = pos.x();
        this->pos[this->activePoint][1] = pos.y();
        this->Redraw();
    }
    }

    //Update prompt line
    if(this->mode == "ADD_LINK" && this->activePoint >= 0)
        this->Redraw();
}

void SimpleSceneController::RemovePoint(int index)
{
    assert(index >=0);
    assert(index < this->pos.size());

    //Remove from points list
    this->pos.erase(this->pos.begin()+index);

    //Update links with a higher index number
    vector<vector<int> > filteredLinks;
    for(unsigned int i=0;i<this->links.size();i++)
    {
        int broken = 0;
        vector<int> &link = this->links[i];
        if(link[0]==index) broken = 1;
        if(link[1]==index) broken = 1;
        if(link[0]>index) link[0] --;
        if(link[1]>index) link[1] --;
        if(!broken) filteredLinks.push_back(link);
    }
    this->links = filteredLinks;

}

int SimpleSceneController::NearestLink(float x, float y)
{
    cout << x << "," << y << endl;
    vector<float> pc;
    pc.push_back(x);
    pc.push_back(y);

    int bestInd = -1;
    float bestDist = -1.f;
    for(unsigned int i=0;i<this->links.size();i++)
    {
        //Calculate angle between clicked point and link direction
        vector<int> &link = this->links[i];
        vector<float> pa = this->pos[link[0]];
        vector<float> pb = this->pos[link[1]];
        vector<float> pac = SubVec(pc,pa);
        vector<float> pab = SubVec(pb,pa);
        vector<float> pacn = NormVec(pac);
        vector<float> pabn = NormVec(pab);
        float dot = DotVec(pacn, pabn);
        float angA = acos(dot);

        //Calculate click distance from link and how far along
        float alongLineDist = dot * MagVec(pac);
        float alongLineFrac = alongLineDist / MagVec(pab);
        float fromLine = sin(angA) * MagVec(pac);

        //Only consider clicks that are between nodes
        if(alongLineFrac < 0.f || alongLineFrac > 1.f) continue;

        if(bestDist < 0.f || fromLine < bestDist)
        {
            bestDist = fromLine;
            bestInd = i;
        }
    }
    return bestInd;
}

void SimpleSceneController::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mousePressEvent" << endl;
    assert(mouseEvent);
    QPointF pos = mouseEvent->buttonDownScenePos(mouseEvent->button());
    Qt::MouseButton button = mouseEvent->button();

    if(this->mode == "MOVE" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y());
        this->activePoint = nearestPoint;
        this->Redraw();
    }

    if(this->mode == "ADD_POINT" && button==Qt::LeftButton)
    {
        std::vector<float> p;
        p.push_back(pos.x());
        p.push_back(pos.y());
        this->pos.push_back(p);
        this->Redraw();
    }

    if(this->mode == "REMOVE_POINT" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y());
        if(nearestPoint>=0) this->RemovePoint(nearestPoint);
        this->activePoint = -1;
        this->Redraw();
    }

    if(this->mode == "ADD_LINK" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y());

        //Join previously selected point with nearest point
        if(this->activePoint >= 0)
        {
            std::vector<int> link;
            link.push_back(this->activePoint);
            link.push_back(nearestPoint);
            this->links.push_back(link);
            this->activePoint = nearestPoint;
            this->Redraw();
        }

        //Nothing is selected, so select nearest point
        if(this->activePoint == -1)
        {
            this->activePoint = nearestPoint;
            this->Redraw();
        }

    }

    if(this->mode == "ADD_LINK" && button==Qt::RightButton)
    {
        //Right click deselects point in this mode
        this->activePoint = -1;
        this->Redraw();
    }

    if(this->mode == "REMOVE_LINK" && button==Qt::LeftButton)
    {
        this->activePoint = -1;
        int nearestLink = this->NearestLink(pos.x(), pos.y());
        if(nearestLink>=0)
        {
            this->links.erase(this->links.begin() + nearestLink);
            this->Redraw();
        }
    }

    //Update dragging flag
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
    this->mode = "MOVE";
}

void SimpleSceneController::AddPointPressed()
{
    this->mode = "ADD_POINT";
}

void SimpleSceneController::RemovePointPressed()
{
    this->mode = "REMOVE_POINT";
}

void SimpleSceneController::AddLinkPressed()
{
    this->mode = "ADD_LINK";
    this->activePoint = -1; //Start with nothing selected
    this->Redraw();
}

void SimpleSceneController::RemoveLinkPressed()
{
    this->mode = "REMOVE_LINK";
}

int SimpleSceneController::GetMouseOver()
{
    return this->mouseOver;
}

void SimpleSceneController::MouseEnterEvent()
{
    this->mouseOver = true;
    this->Redraw();
}

void SimpleSceneController::MouseLeaveEvent()
{
    this->mouseOver = false;
    this->Redraw();
}
