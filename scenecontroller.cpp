#include "scenecontroller.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <stdexcept>
#include <QtGui/QPixmap>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>
#include <QtXml/QtXml>
#include "assert.h"
#include "vectors.h"
using namespace::std;
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)

//Custom graphics scene to catch mouse move and press

MouseGraphicsScene::MouseGraphicsScene(QObject *parent) : QGraphicsScene(parent)
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

void MouseGraphicsScene::SetSceneControl(TrackingAnnotation *sceneControlIn)
{
    this->sceneControl = sceneControlIn;
}

//********************************************************************

TrackingAnnotation::TrackingAnnotation(QObject *parent)
{
    this->mode = "MOVE";
    this->mouseOver = false;
    this->frameStartTime = 0;
    this->frameEndTime = 0;
    this->frameRequestTime = 0;
    this->annotationTime = 0;
    this->annotationTimeSet = false;
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(parent));
    this->scene->SetSceneControl(this);
    this->activePoint = -1;
    this->imgWidth = 0;
    this->imgHeight = 0;
    this->markerSize = 2.;
    this->leftDrag = 0;
    this->mousex = 0.f;
    this->mousey = 0.f;
    this->markFrameButton = NULL;

    this->annotationControls = NULL;
    this->frameTimesEnd = 0;

}

TrackingAnnotation::TrackingAnnotation(const TrackingAnnotation &other)
{
    this->operator=(other);
}

TrackingAnnotation::~TrackingAnnotation()
{
    cout << "TrackingAnnotation::~TrackingAnnotation()" << endl;
    this->item = QSharedPointer<QGraphicsPixmapItem>(NULL);
    if(!this->scene.isNull())
    {
        this->scene->clear();
        this->scene->SetSceneControl(NULL);
        this->scene = QSharedPointer<MouseGraphicsScene>(NULL);
    }

    if(this->annotationControls != NULL)
        delete annotationControls;
    annotationControls = NULL;
}

TrackingAnnotation& TrackingAnnotation::operator= (const TrackingAnnotation &other)
{
    this->lock.lock();

    this->mode = other.mode;
    this->mouseOver = other.mouseOver;
    this->frameStartTime = other.frameStartTime;
    this->frameEndTime = other.frameEndTime;
    this->frameRequestTime = other.frameRequestTime;
    this->annotationTime = other.annotationTime;
    this->annotationTimeSet = other.annotationTimeSet;
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(other.parent()));
    this->scene->SetSceneControl(this);
    this->activePoint = other.activePoint;
    this->imgWidth = other.imgWidth;
    this->imgHeight = other.imgHeight;
    this->markerSize = other.markerSize;
    this->leftDrag = other.leftDrag;
    this->mousex = other.mousex;
    this->mousey = other.mousey;
    this->markFrameButton = NULL;
    this->annotationControls = NULL;
    this->pos = other.pos; //contains annotation positions
    this->shape = other.shape; //contains the default shape
    this->links = other.links;
    this->frameTimes = other.frameTimes;
    this->frameTimesEnd = other.frameTimesEnd;

    this->lock.unlock();
    return *this;

}

bool TrackingAnnotation::operator!= (const TrackingAnnotation &other)
{
    bool ret = false;
    this->lock.lock();

    if(this->pos != other.pos) ret = true;
    if(this->shape != other.shape) ret = true;

    this->lock.unlock();

    return ret;
}

int TrackingAnnotation::GetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          unsigned long long requestedTime,
                                                          std::vector<std::vector<float> > &annot,
                                                          unsigned long long &outAnnotationTime)
{
    this->lock.lock();

    //Try to find annotation within the duration of the frame
    outAnnotationTime = 0;
    annot.clear();
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            outAnnotationTime = it->first;
            annot = it->second;
            this->lock.unlock();
            return 1;
        }
    }

    //If the above did not find a frame, check if there is annotation
    //at the requested time
    it = this->pos.find(requestedTime);
    if(it != this->pos.end())
    {
        outAnnotationTime = requestedTime;
        annot = it->second;
        this->lock.unlock();
        return 1;
    }

    //Failed to find annotation
    this->lock.unlock();
    return 0;
}

vector<unsigned long long> TrackingAnnotation::GetAnnotationTimesBetweenTimestamps(unsigned long long startTime,
                                                                                      unsigned long long endTime)
{
    this->lock.lock();

    vector<unsigned long long> out;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            out.push_back(t);
        }
    }

    this->lock.unlock();
    return out;
}

void TrackingAnnotation::DeleteAnnotationAtTimestamp(unsigned long long annotationTimeIn)
{
    this->lock.lock();
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(annotationTimeIn);
    if(it != this->pos.end())
    {
        this->pos.erase(it);
    }
    this->lock.unlock();
}

void TrackingAnnotation::SetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          std::vector<std::vector<float> > annot)
{
    this->lock.lock();

    if(annot.size() != this->shape.size())
    {
        cout << "Error: Cannot set annotation to mismatched size" << endl;
        this->lock.unlock();
        return;
    }

    //Set annotation for preset frames
    int found = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        unsigned long long t = it->first;
        if(t >= startTime && t < endTime)
        {
            it->second = annot;
            found = 1;
        }
    }
    if(found)
    {
        this->lock.unlock();
        return;
    }

    //No annotation data set, so create a new annotation entry
    assert(endTime >= startTime);
    unsigned long long midTime = ROUND_TIMESTAMP(0.5*(startTime + endTime));
    this->pos[midTime] = annot;
    this->lock.unlock();
}

/*float TrackingAnnotation::GetProportionAnnotated()
{
    unsigned int numAnnot = this->pos.size();
    //Unfortunately, there is no easy way to determine the number of frames
}*/

void TrackingAnnotation::VideoImageChanged(QImage &fr, unsigned long long startTime,
                                              unsigned long long endTime,
                                              unsigned long long requestedTime)
{
    this->img = fr;
    this->frameStartTime = startTime;
    this->frameEndTime = endTime;
    this->frameRequestTime = requestedTime;
    this->imgWidth = fr.width();
    this->imgHeight = fr.height();

    //Check if this frame is used for annotation
    std::vector<std::vector<float> > annot;
    unsigned long long getAnnotationTime = 0;
    int isUsed = this->GetAnnotationBetweenTimestamps(startTime, endTime, requestedTime, annot, getAnnotationTime);
    if(this->markFrameButton!=NULL)
        this->markFrameButton->setChecked(isUsed);

    this->Redraw();
}

void TrackingAnnotation::Redraw()
{
    //Get positions for current frame
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int found = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                         this->frameEndTime,
                                         this->frameRequestTime,
                                         currentFrame,
                                         getAnnotationTime);
    this->annotationTime = getAnnotationTime;
    this->annotationTimeSet = found;

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

void TrackingAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    //assert(mouseEvent);
    QPointF pos = mouseEvent->scenePos();
    //cout << "mouseMoveEvent, " << pos.x() << "," << pos.y () << endl;

    //Get current frame
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentFrame,
                                                      getAnnotationTime);
    if(!isUsed) return;

    if(this->mode == "MOVE" && this->activePoint >= 0 && this->leftDrag)
    {
        currentFrame[this->activePoint][0] = pos.x();
        currentFrame[this->activePoint][1] = pos.y();
        this->SetAnnotationBetweenTimestamps(this->frameStartTime, this->frameEndTime, currentFrame);
        this->Redraw();
    }

    if(this->mode == "MOVE_ALL" && this->leftDrag)
    {
        for(unsigned int i=0;i<currentFrame.size();i++)
        {
            currentFrame[i][0] += pos.x()-this->mousex;
            currentFrame[i][1] += pos.y()-this->mousey;
        }
        this->SetAnnotationBetweenTimestamps(this->frameStartTime, this->frameEndTime, currentFrame);
        this->Redraw();
    }

    //Update prompt line
    if(this->mode == "ADD_LINK" && this->activePoint >= 0)
        this->Redraw();

    this->mousex = pos.x();
    this->mousey = pos.y();
}

void TrackingAnnotation::RemovePoint(int index)
{
    this->lock.lock();
    assert(index >=0);
    assert(index < this->shape.size());

    //Remove from existing annotaiton frames
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        frame.erase(frame.begin()+index);
        assert(frame.size() == this->shape.size() - 1);
    }

    //Remove from points list
    this->shape.erase(this->shape.begin()+index);

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
    this->lock.unlock();
}

int TrackingAnnotation::NearestLink(float x, float y, std::vector<std::vector<float> > &currentFrame)
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

void TrackingAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mousePressEvent" << endl;
    assert(mouseEvent);

    //Get current frame
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentFrame,
                                                      getAnnotationTime);

    if(!isUsed) return;

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

        //Apply change to existing annotation frames
        this->lock.lock();
        std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
        for(it=this->pos.begin(); it != this->pos.end();it++)
        {
            std::vector<std::vector<float> > &frame = it->second;
            frame.push_back(p);
            assert(frame.size() == this->shape.size() + 1);
        }
        this->lock.unlock();

        //Apply to currunt shape template
        this->shape.push_back(p);

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

void TrackingAnnotation::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mouseReleaseEvent" << endl;
    assert(mouseEvent);

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 0;
}

int TrackingAnnotation::NearestPoint(float x, float y, std::vector<std::vector<float> > &currentFrame)
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

unsigned long long AbsDiff(unsigned long long a, unsigned long long b)
{
    if(a>b) return a-b;
    return b-a;
}

unsigned long long TrackingAnnotation::GetSeekFowardTime()
{
    this->lock.lock();
    assert(this!=NULL);
    unsigned long long queryTime = this->annotationTime;
    if(!this->annotationTimeSet)
        queryTime = this->frameRequestTime;

    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti <= queryTime) continue; //Ignore frames in the past
        unsigned long long diff = AbsDiff(ti, queryTime);
        if(!bestSet || diff < bestDiff)
        {
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
            cout << bestFrame << "," << bestDiff << endl;
        }
    }
    this->lock.unlock();
    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

unsigned long long TrackingAnnotation::GetSeekBackTime()
{
    this->lock.lock();
    assert(this!=NULL);
    unsigned long long queryTime = this->annotationTime;
    if(!this->annotationTimeSet)
        queryTime = this->frameRequestTime;

    unsigned long long bestDiff = 0;
    unsigned long long bestFrame = 0;
    int bestSet = 0;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        const unsigned long long &ti = it->first;
        std::vector<std::vector<float> >&framePos = it->second;
        if(ti >= queryTime) continue; //Ignore frames in the future
        unsigned long long diff = AbsDiff(ti, queryTime);
        if(!bestSet || diff < bestDiff)
        {
            cout << bestFrame << "," << bestDiff << endl;
            bestDiff = diff;
            bestFrame = ti;
            bestSet = 1;
        }
    }

    this->lock.unlock();
    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

//********************************************************************

QWidget *TrackingAnnotation::ControlsFactory(QWidget *parent)
{
    assert(this->annotationControls==NULL);

    this->annotationControls = new QWidget();
    assert(this->annotationControls->layout() == NULL);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);

    assert(this->markFrameButton == NULL);
    this->markFrameButton = new QPushButton("Mark Frame", this->annotationControls);
    QObject::connect(this->markFrameButton, SIGNAL(toggled(bool)), this, SLOT(MarkFramePressed(bool)));
    this->markFrameButton->setCheckable(true);
    //this->markFrameButton->setChecked(true);
    layout->addWidget(this->markFrameButton);

    QPushButton *button = new QPushButton("Move", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MovePressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    button->setChecked(true);
    layout->addWidget(button);

    button = new QPushButton("Move All", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MoveAllPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Add", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddPointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemovePointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Add Link", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove Link", this->annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemoveLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);
    this->annotationControls->setLayout(layout);
    return this->annotationControls;
}

void TrackingAnnotation::DestroyControls()
{
    assert(this->markFrameButton != NULL);
    //delete this->markFrameButton;
    this->markFrameButton = NULL;

    assert(this->annotationControls != NULL);
    //delete this->annotationControls;
    this->annotationControls = NULL;
}

void TrackingAnnotation::MarkFramePressed(bool val)
{


    //Check if current frame already exists
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentFrame,
                                                      getAnnotationTime);

    if(val==0 && isUsed) //Deselect frame for marking
    {
        this->DeleteAnnotationAtTimestamp(getAnnotationTime);
    }
    if(val==1 && !isUsed) //Enable frame annotation
    {
        if(this->shape.size() == 0)
        {
            this->LoadShape();
        }

        assert(this->frameEndTime >= this->frameStartTime);
        unsigned long long frameMidpoint = ROUND_TIMESTAMP(0.5f*(this->frameStartTime + this->frameEndTime));
        this->lock.lock();
        this->pos[frameMidpoint] = this->shape;
        this->lock.unlock();
    }

    this->Redraw();
}

void TrackingAnnotation::MovePressed()
{
    this->mode = "MOVE";
}

void TrackingAnnotation::MoveAllPressed()
{
    this->mode = "MOVE_ALL";
}

void TrackingAnnotation::AddPointPressed()
{
    this->mode = "ADD_POINT";
}

void TrackingAnnotation::RemovePointPressed()
{
    this->mode = "REMOVE_POINT";
}

void TrackingAnnotation::AddLinkPressed()
{
    this->mode = "ADD_LINK";
    this->activePoint = -1; //Start with nothing selected
    this->Redraw();
}

void TrackingAnnotation::RemoveLinkPressed()
{
    this->mode = "REMOVE_LINK";
}

int TrackingAnnotation::GetMouseOver()
{
    return this->mouseOver;
}

void TrackingAnnotation::MouseEnterEvent()
{
    this->mouseOver = true;
    this->Redraw();
}

void TrackingAnnotation::MouseLeaveEvent()
{
    this->mouseOver = false;
    this->Redraw();
}

//************************************************************

QMenu *TrackingAnnotation::MenuFactory(QMenuBar *menuBar)
{
    assert(this!=NULL);
    assert(menuBar != NULL);
    QAction *loadShape = new QAction(tr("&Load Shape from File"), menuBar);
    QAction *saveShape = new QAction(tr("&Save Shape to File"), menuBar);
    QAction *setShape = new QAction(tr("Set Shape from &Current Frame"), menuBar);
    QAction *resetShape = new QAction(tr("&Reset Shape"), menuBar);

    QAction *loadAnnotation = new QAction(tr("L&oad Annotation"), menuBar);
    QAction *saveAnnotation = new QAction(tr("S&ave Annotation"), menuBar);

    QMenu *newMenu = menuBar->addMenu(tr("&Annotate"));
    newMenu->addAction(loadShape);
    newMenu->addAction(saveShape);
    newMenu->addAction(setShape);
    newMenu->addAction(resetShape);
    newMenu->addSeparator();
    newMenu->addAction(loadAnnotation);
    newMenu->addAction(saveAnnotation);

    QObject::connect(loadShape, SIGNAL(triggered()), this, SLOT(LoadShape()));
    QObject::connect(saveShape, SIGNAL(triggered()), this, SLOT(SaveShape()));
    QObject::connect(setShape, SIGNAL(triggered()), this, SLOT(SetShapeFromCurentFrame()));
    QObject::connect(resetShape, SIGNAL(triggered()), this, SLOT(ResetCurentFrameShape()));
    QObject::connect(loadAnnotation, SIGNAL(triggered()), this, SLOT(LoadAnnotation()));
    QObject::connect(saveAnnotation, SIGNAL(triggered()), this, SLOT(SaveAnnotation()));

    return newMenu;
}

std::vector<std::vector<float> > TrackingAnnotation::ProcessXmlDomFrame(QDomElement &rootElem)
{
    std::vector<std::vector<float> > out;
    QDomNode n = rootElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(!e.isNull()) {
            //cout << qPrintable(e.tagName()) << endl; // the node really is an element.
            if(e.tagName() == "point")
            {
                std::vector<float> p;
                int id = e.attribute("id").toInt();
                p.push_back(e.attribute("x").toFloat());
                p.push_back(e.attribute("y").toFloat());
                while(id >= out.size())
                {
                    std::vector<float> empty;
                    out.push_back(empty);
                }
                out[id] = p;
            }
            if(e.tagName() == "link")
            {
                std::vector<int> link;
                link.push_back(e.attribute("from").toInt());
                link.push_back(e.attribute("to").toInt());
                this->links.push_back(link);
            }
        }
    n = n.nextSibling();
    }

    return out;
}

void TrackingAnnotation::LoadShape()
{
    this->lock.lock();

    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0)
    {
        this->lock.unlock();
        return;
    }

    //Parse XML to DOM
    QFile f(fileName);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        this->lock.unlock();
        return;
    }
    f.close();

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    this->links.clear();
    this->shape = this->ProcessXmlDomFrame(rootElem);

    //Validate points
    int invalidShape = 0;
    for(unsigned int i=0;i<this->shape.size();i++)
        if(this->shape[i].size() != 2)
        {
            cout << "Error: missing point ID " << i << endl;
            invalidShape = 1;
        }

    //Validate links
    for(unsigned int i=0;i<this->links.size();i++)
    {
        if(this->links[i].size() != 2)
        {
            cout << "Error: Invalid link" << endl;
            invalidShape = 1;
        }
        if(this->links[i][0] < 0 || this->links[i][0] >= this->shape.size())
        {
            cout << "Link refers to non-existent point " << this->links[i][0] << endl;
            invalidShape = 1;
        }
        if(this->links[i][1] < 0 || this->links[i][1] >= this->shape.size())
        {
            cout << "Link refers to non-existent point " << this->links[i][1] << endl;
            invalidShape = 1;
        }
    }

    if(invalidShape)
    {
        this->shape.clear();
        this->links.clear();
        this->lock.unlock();
        return;
    }

    //Check existing data to see if has the correct number of points
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        while(frame.size() > this->shape.size())
        {
            frame.pop_back();
        }
        while(frame.size() < this->shape.size())
        {
            frame.push_back(this->shape[frame.size()]);
        }
    }
    this->lock.unlock();
}

void TrackingAnnotation::WriteShapeToStream(QTextStream &out)
{
    out << "\t<shape>" << endl;

    for(unsigned int i=0; i < this->shape.size(); i++)
    {
        out << "\t\t<point id='"<<i<<"' x='"<<this->shape[i][0]<<"' y='"<<this->shape[i][1]<<"'/>" << endl;
    }
    for(unsigned int i=0;i < this->links.size();i++)
    {
        out << "\t\t<link from='"<<this->links[i][0]<<"' to='"<<this->links[i][1]<<"'/>" << endl;
    }

    out << "\t</shape>" << endl;
}

void TrackingAnnotation::SaveShape()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0) return;

    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;;
    this->WriteShapeToStream(out);
    f.close();
}

void TrackingAnnotation::SetShapeFromCurentFrame()
{
    //Get annotated times(s) in current visible frame
    std::vector<std::vector<float> > annotShape;
    unsigned long long getAnnotationTime = 0;
    int found = GetAnnotationBetweenTimestamps(this->frameStartTime,
        this->frameEndTime,
        this->frameRequestTime,
        annotShape,
        getAnnotationTime);
    if(!found) return;

    //Set shape from current frame
    this->shape = annotShape;
}

void TrackingAnnotation::ResetCurentFrameShape()
{
    //Get current frame
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentFrame,
                                                      getAnnotationTime);
    if(!isUsed) return;

    //Set current frame to canonical shape
    this->SetAnnotationBetweenTimestamps(this->frameStartTime, this->frameEndTime, this->shape);
    this->Redraw();
}

int TrackingAnnotation::GetShapeNumPoints()
{
    this->lock.lock();
    int out = this->shape.size();
    this->lock.unlock();
    return out;
}

void TrackingAnnotation::LoadAnnotation()
{
    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Annotation"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    //Parse XML to DOM
    QFile f(fileName);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        return;
    }
    f.close();

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    this->ReadAnnotationXml(rootElem);
}

void TrackingAnnotation::SaveAnnotation()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Annotation Track"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    this->WriteAnnotationXml(out);
    f.close();
}

//*********************************************************

void TrackingAnnotation::ReadAnnotationXml(QDomElement &elem)
{
    this->lock.lock();
    this->shape.clear();
    this->links.clear();
    this->pos.clear();
    QDomNode n = elem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if(e.tagName() == "shape")
            {
                std::vector<std::vector<float> > shapeData = ProcessXmlDomFrame(e);
                this->shape = shapeData;
            }
            if(e.tagName() == "frame")
            {
                std::vector<std::vector<float> > frame = ProcessXmlDomFrame(e);
                cout << e.attribute("time").toFloat() << endl;
                float timeSec = e.attribute("time").toFloat();
                assert(timeSec > 0.f);
                assert(frame.size() == this->shape.size());
                this->pos[(unsigned long long)(timeSec * 1000.f + 0.5)] = frame;
            }
            if(e.tagName() == "available")
            {
                this->frameTimes.clear();
                this->frameTimesEnd = e.attribute("to").toULong();

                QDomElement frEl = e.firstChildElement();
                while(!frEl.isNull())
                {
                    if(frEl.tagName() != "f");
                    unsigned long s = frEl.attribute("s").toULong();
                    unsigned long e = frEl.attribute("e").toULong();
                    this->frameTimes[s] = e;
                    frEl = frEl.nextSiblingElement();
                }
            }
        }
        n = n.nextSibling();
    }
    this->lock.unlock();
}

void TrackingAnnotation::WriteAnnotationXml(QTextStream &out)
{
    out << "\t<tracking>" << endl;
    this->WriteShapeToStream(out);

    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;

    //Save annotated frames
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        assert(frame.size() == this->shape.size());
        out << "\t<frame time='"<<(it->first/1000.f)<<"'>" << endl;
        for(unsigned int i=0; i < frame.size(); i++)
        {
            out << "\t\t<point id='"<<i<<"' x='"<<frame[i][0]<<"' y='"<<frame[i][1]<<"'/>" << endl;
        }
        out << "\t</frame>" << endl;
    }

    //Save frame start and end times
    out << "\t<available to=\""<< this->frameTimesEnd << "\">" << endl;
    for(std::map<unsigned long, unsigned long>::iterator it = this->frameTimes.begin();
        it != this->frameTimes.end();
        it++)
    {
        unsigned long st = it->first;
        out << "\t<f s=\""<<st<<"\" e=\""<<this->frameTimes[st]<<"\"/>" << endl;
    }

    out << "\t</available>" << endl;
    out << "\t</tracking>" << endl;
}

QSharedPointer<MouseGraphicsScene> TrackingAnnotation::GetScene()
{
    return this->scene;
}

//***************************************************

unsigned int TrackingAnnotation::NumMarkedFrames()
{

    this->lock.lock();
    unsigned int out = this->pos.size();
    this->lock.unlock();
    return out;
}

void TrackingAnnotation::GetIndexAnnotationXml(unsigned int index, QTextStream *out)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it = this->pos.begin();
    for(unsigned int i=0;i<index;i++)
        it ++;
    std::vector<std::vector<float> > &frame = it->second;
    *out << "\t<frame time='"<<(it->first/1000.f)<<"'>" << endl;
    for(unsigned int i=0; i < frame.size(); i++)
    {
        *out << "\t\t<point id='"<<i<<"' x='"<<frame[i][0]<<"' y='"<<frame[i][1]<<"'/>" << endl;
    }
    *out << "\t</frame>" << endl;
}

unsigned long long TrackingAnnotation::GetIndexTimestamp(unsigned int index)
{
    this->lock.lock();
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it = this->pos.begin();
    for(unsigned int i=0;i<index;i++)
        it ++;
    unsigned long long out = it->first;
    this->lock.unlock();
    return out;
}

void TrackingAnnotation::FoundFrame(unsigned long startTi, unsigned long endTi)
{
    this->lock.lock();
    //Update store
    this->frameTimes[startTi] = endTi;
    if(endTi > this->frameTimesEnd)
        this->frameTimesEnd = endTi;
    this->lock.unlock();
}

void TrackingAnnotation::GetFramesAvailable(std::map<unsigned long, unsigned long> &frameTimesOut,
                        unsigned long &frameTimesEndOut)
{
    this->lock.lock();
    frameTimesOut = this->frameTimes;
    frameTimesEndOut = this->frameTimesEnd;
    this->lock.unlock();
}
