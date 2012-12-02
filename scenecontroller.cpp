#include "scenecontroller.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <stdexcept>
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
    this->currentTime = 0;
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(parent));
    this->scene->SetSceneControl(this);
    std::vector<std::vector<float> > exampleFrame;
    for(int i=0;i<50;i++)
    {
        std::vector<float> p;
        p.push_back(rand() % 500);
        p.push_back(rand() % 500);
        exampleFrame.push_back(p);
    }
    this->pos[0] = exampleFrame;
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

void SimpleSceneController::VideoImageChanged(QImage &fr, unsigned long long ti)
{
    this->img = fr;
    this->currentTime = ti;
    //this->item =
    //        QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(fr)));
    this->imgWidth = fr.width();
    this->imgHeight = fr.height();
    this->Redraw();
}

void SimpleSceneController::Redraw()
{
    //Get positions for current frame
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(this->currentTime);
    std::vector<std::vector<float> > currentFrame;
    if(it != this->pos.end())
    {
        currentFrame = it->second;
    }

    //Recreate scene and convert image
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
    if(this->mode == "ADD_LINK" && this->activePoint >= 0 && this->mouseOver && currentFrame.size() > 0)
    {
        //draw a link to encourage user to click on the next point
        this->scene->addLine(this->mousex, this->mousey,
                             currentFrame[this->activePoint][0], currentFrame[this->activePoint][1],
                             penBlue);
    }

    //Draw links
    if(currentFrame.size() > 0)
    for(unsigned int i=0;i<this->links.size();i++)
    {
        vector<int> &link = this->links[i];
        assert(link.size()==2);
        this->scene->addLine(currentFrame[link[0]][0],currentFrame[link[0]][1],
                             currentFrame[link[1]][0],currentFrame[link[1]][1], penBlue);
    }

    //Draw marker points
    if(currentFrame.size() > 0)
    for(unsigned int i=0;i<currentFrame.size();i++)
    {
        assert(currentFrame[i].size()==2);
        //cout << this->activePoint << endl;
        if(i!=this->activePoint)
            this->scene->addEllipse(currentFrame[i][0]-this->markerSize/2,
                                    currentFrame[i][1]-this->markerSize/2,
                                    this->markerSize, this->markerSize,
                                    penRed, brushRed);
        else
            this->scene->addEllipse(currentFrame[i][0]-this->markerSize/2,
                                    currentFrame[i][1]-this->markerSize/2,
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

    //Get current frame
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(this->currentTime);
    if(it == this->pos.end()) return;
    std::vector<std::vector<float> > &currentFrame = it->second;

    if(this->mode == "MOVE")
    {
    if(this->activePoint >= 0 && this->leftDrag)
    {
        currentFrame[this->activePoint][0] = pos.x();
        currentFrame[this->activePoint][1] = pos.y();
        this->Redraw();
    }
    }

    //Update prompt line
    if(this->mode == "ADD_LINK" && this->activePoint >= 0)
        this->Redraw();
}

void SimpleSceneController::RemovePoint(int index)
{
    //Get current frame
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(this->currentTime);
    if(it == this->pos.end()) return;
    std::vector<std::vector<float> > &currentFrame = it->second;

    assert(index >=0);
    assert(index < currentFrame.size());

    //Remove from points list
    currentFrame.erase(currentFrame.begin()+index);

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

int SimpleSceneController::NearestLink(float x, float y, std::vector<std::vector<float> > &currentFrame)
{
    //cout << x << "," << y << endl;
    vector<float> pc;
    pc.push_back(x);
    pc.push_back(y);

    int bestInd = -1;
    float bestDist = -1.f;
    for(unsigned int i=0;i<this->links.size();i++)
    {
        //Calculate angle between clicked point and link direction
        vector<int> &link = this->links[i];
        vector<float> pa = currentFrame[link[0]];
        vector<float> pb = currentFrame[link[1]];
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

    //Get current frame
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(this->currentTime);
    if(it == this->pos.end()) return;
    std::vector<std::vector<float> > &currentFrame = it->second;

    QPointF pos = mouseEvent->buttonDownScenePos(mouseEvent->button());
    Qt::MouseButton button = mouseEvent->button();

    if(this->mode == "MOVE" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y(), currentFrame);
        this->activePoint = nearestPoint;
        this->Redraw();
    }

    if(this->mode == "ADD_POINT" && button==Qt::LeftButton)
    {
        std::vector<float> p;
        p.push_back(pos.x());
        p.push_back(pos.y());
        currentFrame.push_back(p);
        this->Redraw();
    }

    if(this->mode == "REMOVE_POINT" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y(), currentFrame);
        if(nearestPoint>=0) this->RemovePoint(nearestPoint);
        this->activePoint = -1;
        this->Redraw();
    }

    if(this->mode == "ADD_LINK" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y(), currentFrame);

        //Join previously selected point with nearest point
        if(this->activePoint >= 0 && nearestPoint >= 0 && this->activePoint != nearestPoint)
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
        int nearestLink = this->NearestLink(pos.x(), pos.y(), currentFrame);
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

int SimpleSceneController::NearestPoint(float x, float y, std::vector<std::vector<float> > &currentFrame)
{
    int best = -1;
    float bestDist = -1;
    for(unsigned int i=0;i<currentFrame.size();i++)
    {
        float dx = currentFrame[i][0] - x;
        float dy = currentFrame[i][1] - y;
        float dist = pow(dx*dx + dy*dy, 0.5f);
        if(bestDist < 0. || dist < bestDist)
        {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

unsigned long long SimpleSceneController::GetSeekFowardTime()
{
    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti <= this->currentTime) continue; //Ignore frames in the past
        unsigned long long diff = abs(ti - this->currentTime);
        if(!bestSet || diff < bestDiff)
        {
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
        }
    }
    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

unsigned long long SimpleSceneController::GetSeekBackTime()
{

    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti >= this->currentTime) continue; //Ignore frames in the future
        unsigned long long diff = abs(ti - this->currentTime);
        if(!bestSet || diff < bestDiff)
        {
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
        }
    }

    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

//********************************************************************

QWidget *SimpleSceneController::ControlsFactory(QWidget *parent)
{
    QWidget *layoutW = new QWidget();
    assert(layoutW->layout() == NULL);
    QHBoxLayout *layout = new QHBoxLayout();

    QPushButton *button = new QPushButton("Mark Frame");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MarkFramePressed()));
    button->setCheckable(true);
    button->setChecked(true);
    layout->addWidget(button);

    button = new QPushButton("Move");
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

void SimpleSceneController::MarkFramePressed()
{

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

//************************************************************


