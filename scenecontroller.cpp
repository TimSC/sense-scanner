#include "scenecontroller.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <stdexcept>
#include <QtGui/QPixmap>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>
#include <QtXml/QtXml>
#include <assert.h>
#include <stdexcept>
#include "vectors.h"
#include "eventloop.h"
#include "annotation.h"
#include "shapegui.h"
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

void MouseGraphicsScene::SetSceneControl(BaseSceneController *sceneControlIn)
{
    assert(this!=NULL);
    this->sceneControl = sceneControlIn;
}

//****************************************************************************

BaseSceneController::BaseSceneController(QObject *parent) : QObject(parent)
{
    this->scene = new MouseGraphicsScene(parent);
    //this->scene->SetSceneControl(this);
}

BaseSceneController::~BaseSceneController()
{
    if(this->scene != NULL)
    {
        this->scene->clear();
        this->scene->SetSceneControl(NULL);
        delete this->scene;
    }
}

QWidget *BaseSceneController::ControlsFactory(QWidget *parent)
{
    return NULL;

}

QMenu *BaseSceneController::MenuFactory(QMenuBar *menuBar)
{
    return NULL;

}

MouseGraphicsScene *BaseSceneController::GetScene()
{
    return this->scene;
}

int BaseSceneController::GetMouseOver()
{
    return this->mouseOver;
}

void BaseSceneController::MouseEnterEvent()
{
    this->mouseOver = true;
    this->Redraw();
}

void BaseSceneController::MouseLeaveEvent()
{
    this->mouseOver = false;
    this->Redraw();
}

unsigned long long BaseSceneController::GetSeekForwardTime()
{
    return 0;
}

unsigned long long BaseSceneController::GetSeekBackTime()
{
    return 0;
}

void BaseSceneController::VideoImageChanged(QImage &fr, unsigned long long startTime,
                       unsigned long long endTime,
                       unsigned long long requestedTime)
{

}

void BaseSceneController::SetAnnotationTrack(QUuid srcUuid)
{

}

void BaseSceneController::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

}

void BaseSceneController::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

}

void BaseSceneController::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{

}

void BaseSceneController::Redraw()
{

}

//********************************************************************

TrackingSceneController::TrackingSceneController(QObject *parent) : BaseSceneController(parent)
{
    this->mode = "MOVE";
    this->mouseOver = false;
    this->frameStartTime = 0;
    this->frameEndTime = 0;
    this->frameRequestTime = 0;
    this->annotationTime = 0;
    this->annotationTimeSet = false;
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
    this->eventLoop = NULL;
    this->eventReceiver = NULL;

    QObject::connect(&this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer.start(10); //in millisec
}

TrackingSceneController::~TrackingSceneController()
{
    cout << "TrackingAnnotation::~TrackingAnnotation()" << endl;
    this->item = QSharedPointer<QGraphicsPixmapItem>(NULL);

    if(this->annotationControls != NULL)
        delete annotationControls;
    annotationControls = NULL;

}

void TrackingSceneController::VideoImageChanged(QImage &fr, unsigned long long startTime,
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

void TrackingSceneController::Redraw()
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

void TrackingSceneController::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    //assert(mouseEvent);
    QPointF pos = mouseEvent->scenePos();
    //cout << "mouseMoveEvent, " << pos.x() << "," << pos.y () << endl;

    //Get current frame
    std::vector<std::vector<float> > currentFrame;
    unsigned long long getAnnotationTime = 0;
    int isUsed = 0;
    try
    {
        isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentFrame,
                                                      getAnnotationTime);

    }
    catch(std::runtime_error err)
    {
        //Error getting annotation
        return;
    }

    if(!isUsed) return;

    if(this->mode == "MOVE" && this->activePoint >= 0 && this->leftDrag)
    {
        currentFrame[this->activePoint][0] = pos.x();
        currentFrame[this->activePoint][1] = pos.y();
        Annotation::SetAnnotationBetweenTimestamps(this->frameStartTime,
                                             this->frameEndTime,
                                             currentFrame,
                                             this->annotationUuid,
                                             this->eventLoop);
        this->Redraw();
    }

    if(this->mode == "MOVE_ALL" && this->leftDrag)
    {
        for(unsigned int i=0;i<currentFrame.size();i++)
        {
            currentFrame[i][0] += pos.x()-this->mousex;
            currentFrame[i][1] += pos.y()-this->mousey;
        }
        Annotation::SetAnnotationBetweenTimestamps(this->frameStartTime,
                                                   this->frameEndTime,
                                                   currentFrame,
                                                   this->annotationUuid,
                                                   this->eventLoop);
        this->Redraw();
    }

    //Update prompt line
    if(this->mode == "ADD_LINK" && this->activePoint >= 0)
        this->Redraw();

    this->mousex = pos.x();
    this->mousey = pos.y();
}

int TrackingSceneController::NearestLink(float x, float y, std::vector<std::vector<float> > &currentFrame)
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

void TrackingSceneController::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
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
        //Apply change to existing annotation frames

        assert(this->eventLoop!=NULL);

        std::vector<float> pt;
        pt.push_back(pos.x());
        pt.push_back(pos.y());
        this->currentShape.push_back(pt);

        QString xml;
        QTextStream xmlStr(&xml);
        TrackingAnnotationData::WriteShapeToStream(this->links, this->currentShape, xmlStr);

        std::tr1::shared_ptr<class Event> reqEv(new Event("SET_SHAPE"));
        reqEv->toUuid = this->annotationUuid;
        reqEv->data = xml.toLocal8Bit().constData();
        this->eventLoop->SendEvent(reqEv);

        //Get a local copy of shape information
        this->defaultShape = Annotation::GetShape(this->annotationUuid, this->eventLoop,
                                                      this->eventReceiver, this->links);
        this->RefreshCurrentPos();

        this->Redraw();
    }

    if(this->mode == "REMOVE_POINT" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y(), currentFrame);
        if(nearestPoint>=0) this->RemovePoint(nearestPoint);

        //Get a local copy of shape information
        this->defaultShape = Annotation::GetShape(this->annotationUuid, this->eventLoop,
                                                      this->eventReceiver, this->links);

        this->RefreshCurrentPos();
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

            //Update annotation track with new link
            static std::vector<std::vector<float> > shape = Annotation::GetShape(this->annotationUuid,
                this->eventLoop,
                this->eventReceiver,
                this->links);
            this->links.push_back(link);

            Annotation::SetShape(this->annotationUuid,
                                 shape,
                                 this->links,
                                 this->eventLoop,
                                 this->eventReceiver);



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

void TrackingSceneController::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mouseReleaseEvent" << endl;
    assert(mouseEvent);

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 0;
}

int TrackingSceneController::NearestPoint(float x, float y, std::vector<std::vector<float> > &currentFrame)
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

unsigned long long TrackingSceneController::GetSeekForwardTime()
{
    assert(this!=NULL);
    unsigned long long queryTime = 0.5*(this->frameStartTime+this->frameEndTime);
    //if(!this->annotationTimeSet)
    //    queryTime = this->frameRequestTime;

    return this->GetSeekForwardTimeFromAnnot(queryTime);
}

unsigned long long TrackingSceneController::GetSeekBackTime()
{
    assert(this!=NULL);
    unsigned long long queryTime = 0.5*(this->frameStartTime+this->frameEndTime);
    //if(!this->annotationTimeSet)
    //    queryTime = this->frameRequestTime;

    return this->GetSeekBackwardTimeFromAnnot(queryTime);
}

//********************************************************************

QWidget *TrackingSceneController::ControlsFactory(QWidget *parent)
{
    QWidget *annotationControls = new QWidget();
    assert(annotationControls->layout() == NULL);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);

    assert(this->markFrameButton == NULL);
    this->markFrameButton = new QPushButton("Mark Frame", annotationControls);
    QObject::connect(this->markFrameButton, SIGNAL(toggled(bool)), this, SLOT(MarkFramePressed(bool)));
    this->markFrameButton->setCheckable(true);
    //this->markFrameButton->setChecked(true);
    layout->addWidget(this->markFrameButton);

    QPushButton *button = new QPushButton("Move", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MovePressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    button->setChecked(true);
    layout->addWidget(button);

    button = new QPushButton("Move All", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(MoveAllPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Add", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddPointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemovePointPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Add Link", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);

    button = new QPushButton("Remove Link", annotationControls);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemoveLinkPressed()));
    button->setAutoExclusive(true);
    button->setCheckable(true);
    layout->addWidget(button);
    annotationControls->setLayout(layout);
    return annotationControls;
}

void TrackingSceneController::MarkFramePressed(bool val)
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
        this->RemoveAnnotationAtTime(getAnnotationTime);
        this->RefreshCurrentPos();
    }

    if(val==1 && !isUsed) //Enable frame annotation
    {
        if(this->defaultShape.size()==0)
        {
            class ShapeGui *shapeGui = new class ShapeGui();
            shapeGui->exec();

            if(shapeGui->loadShapeSelected)
            {
                this->LoadShape();
            }
            if(shapeGui->usePresetSelected)
            {
                QString fina = shapeGui->GetCustomFilename();
                this->LoadShape(fina);
            }

            delete shapeGui;
        }

        assert(this->frameEndTime >= this->frameStartTime);
        unsigned long long frameMidpoint = ROUND_TIMESTAMP(0.5f*(this->frameStartTime + this->frameEndTime));
        this->AddAnnotationAtTime(frameMidpoint);

        this->RefreshCurrentPos();
    }

    this->Redraw();
}

void TrackingSceneController::MovePressed()
{
    this->mode = "MOVE";
}

void TrackingSceneController::MoveAllPressed()
{
    this->mode = "MOVE_ALL";
}

void TrackingSceneController::AddPointPressed()
{
    this->mode = "ADD_POINT";
}

void TrackingSceneController::RemovePointPressed()
{
    this->mode = "REMOVE_POINT";
}

void TrackingSceneController::AddLinkPressed()
{
    this->mode = "ADD_LINK";
    this->activePoint = -1; //Start with nothing selected
    this->Redraw();
}

void TrackingSceneController::RemoveLinkPressed()
{
    this->mode = "REMOVE_LINK";
}

//************************************************************

QMenu *TrackingSceneController::MenuFactory(QMenuBar *menuBar)
{
    assert(this!=NULL);
    assert(menuBar != NULL);
    QAction *loadShape = new QAction(tr("&Load Shape from File"), menuBar);
    QAction *saveShape = new QAction(tr("&Save Shape to File"), menuBar);
    QAction *setShape = new QAction(tr("Set Shape from &Current Frame"), menuBar);
    QAction *resetShape = new QAction(tr("&Reset Shape"), menuBar);

    QAction *loadAnnotation = new QAction(tr("L&oad Annotation (XML)"), menuBar);
    QAction *saveAnnotation = new QAction(tr("S&ave Annotation (XML)"), menuBar);

    QAction *saveAnnotationCsv = new QAction(tr("Export Annotation as CSV"), menuBar);
    QAction *saveAnnotationMatlab = new QAction(tr("Export Annotation for Matlab"), menuBar);
    QAction *saveAnnotationMM = new QAction(tr("Export Annotation as Matrix Market"), menuBar);

#ifdef DEMO_MODE
    saveAnnotationCsv->setDisabled(1);
    saveAnnotationMatlab->setDisabled(1);
    saveAnnotationMM->setDisabled(1);
#endif //DEMO_MODE

    QMenu *newMenu = menuBar->addMenu(tr("&Annotate"));
    newMenu->addAction(loadShape);
    newMenu->addAction(saveShape);
    newMenu->addAction(setShape);
    newMenu->addAction(resetShape);
    newMenu->addSeparator();
    newMenu->addAction(loadAnnotation);
    newMenu->addAction(saveAnnotation);

    newMenu->addAction(saveAnnotationCsv);
    newMenu->addAction(saveAnnotationMatlab);
    newMenu->addAction(saveAnnotationMM);

    QObject::connect(loadShape, SIGNAL(triggered()), this, SLOT(LoadShape()));
    QObject::connect(saveShape, SIGNAL(triggered()), this, SLOT(SaveShape()));
    QObject::connect(setShape, SIGNAL(triggered()), this, SLOT(SetShapeFromCurentFrame()));
    QObject::connect(resetShape, SIGNAL(triggered()), this, SLOT(ResetCurentFrameShape()));
    QObject::connect(loadAnnotation, SIGNAL(triggered()), this, SLOT(LoadAnnotation()));
    QObject::connect(saveAnnotation, SIGNAL(triggered()), this, SLOT(SaveAnnotation()));

    QObject::connect(saveAnnotationCsv, SIGNAL(triggered()), this, SLOT(SaveAnnotationCsv()));
    QObject::connect(saveAnnotationMatlab, SIGNAL(triggered()), this, SLOT(SaveAnnotationMatlab()));
    QObject::connect(saveAnnotationMM, SIGNAL(triggered()), this, SLOT(SaveAnnotationMM()));
    return newMenu;
}

void TrackingSceneController::LoadShape()
{
    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0)
        return;

    this->LoadShape(fileName);


}

void TrackingSceneController::LoadShape(QString fileName)
{
    QFile shapeFile(fileName);
    if(!shapeFile.exists())
        throw std::runtime_error("Shape file does not exist");
    shapeFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream shapeStream(&shapeFile);

    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_SHAPE"));
    reqEv->toUuid = this->annotationUuid;
    QString xml = shapeStream.readAll();
    reqEv->data = xml.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);

    //Get a local copy of shape information
    this->defaultShape = Annotation::GetShape(this->annotationUuid, this->eventLoop,
                                                  this->eventReceiver, this->links);

}

void TrackingSceneController::SetShapeFromCurentFrame()
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
    this->defaultShape = annotShape;
    Annotation::SetShape(this->annotationUuid, annotShape, this->links,
                         this->eventLoop, this->eventReceiver);
}

void TrackingSceneController::ResetCurentFrameShape()
{
    //Get default shape
    this->defaultShape = Annotation::GetShape(this->annotationUuid, this->eventLoop,
                                                  this->eventReceiver, this->links);


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
    this->currentShape = this->defaultShape;
    Annotation::SetAnnotationBetweenTimestamps(this->frameStartTime,
                                               this->frameEndTime,
                                               this->currentShape,
                                               this->annotationUuid,
                                               this->eventLoop);

    this->RefreshCurrentPos();
    this->Redraw();
}

MouseGraphicsScene *TrackingSceneController::GetScene()
{
    return this->scene;
}

//***************************************************

void TrackingSceneController::SetEventLoop(class EventLoop *eventLoopIn)
{
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventLoop = eventLoopIn;
    this->eventReceiver = new EventReceiver(this->eventLoop,__FILE__,__LINE__);

    this->eventLoop->AddListener("ANNOTATION_FRAME",*eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_SHAPE",*eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_DATA",*eventReceiver);
    this->eventLoop->AddListener("SEEK_RESULT",*eventReceiver);
    this->eventLoop->AddListener("STOP_THREADS",*eventReceiver);
    this->eventLoop->AddListener("SET_ANNOTATION_DONE",*eventReceiver);

}

void TrackingSceneController::SetAnnotationTrack(QUuid srcUuid)
{
    this->annotationUuid = srcUuid;
    this->RefreshCurrentPos();
    this->RefreshLinks();
}

int TrackingSceneController::GetAnnotationBetweenTimestamps(unsigned long long startTime,
    unsigned long long endTime,
    unsigned long long requestedTime,
    std::vector<std::vector<float> > &annot,
    unsigned long long &annotationTimeOut)
{
    double ti = 0.;
    int ret = Annotation::GetAnnotationBetweenFrames(startTime,
                                               endTime,
                                               requestedTime,
                                               annotationUuid,
                                               this->eventLoop,
                                               this->eventReceiver,
                                               annot,
                                               ti);

    annotationTimeOut = (unsigned long long)(ti * 1000. + 0.5);
    return ret;
}

void TrackingSceneController::RefreshCurrentPos()
{
    unsigned long long annotationTimeTmp;
    std::vector<std::vector<float> > currentShapeTmp;

    int isUsed = this->GetAnnotationBetweenTimestamps(this->frameStartTime,
                                                      this->frameEndTime,
                                                      this->frameRequestTime,
                                                      currentShapeTmp,
                                                      annotationTimeTmp);
    if(isUsed)
    {
        this->annotationTimeSet = 1;
        this->currentShape = currentShapeTmp;
        this->annotationTime = annotationTimeTmp;
    }
}

void TrackingSceneController::RefreshLinks()
{
    assert(this->eventLoop!=NULL);
    this->defaultShape = Annotation::GetShape(this->annotationUuid, this->eventLoop,
                                              this->eventReceiver, this->links);
    return;
}

unsigned long long TrackingSceneController::GetSeekForwardTimeFromAnnot(unsigned long long queryTime)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_SEEK_FORWARD_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->id = this->eventLoop->GetId();
    reqEv->data = QString("%1").arg(queryTime).toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);

    std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(reqEv->id);
    if(response->data=="NOT_FOUND")
    {
        throw runtime_error("Not found");
    }
    return response->data.toULongLong();
}

unsigned long long TrackingSceneController::GetSeekBackwardTimeFromAnnot(unsigned long long queryTime)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_SEEK_BACKWARD_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->id = this->eventLoop->GetId();
    reqEv->data = QString("%1").arg(queryTime);
    this->eventLoop->SendEvent(reqEv);

    std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(reqEv->id);
    if(response->data=="NOT_FOUND")
    {
        throw runtime_error("Not found");
    }
    return response->data.toULongLong();
}

void TrackingSceneController::RemoveAnnotationAtTime(unsigned long long time)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("REMOVE_ANNOTATION_AT_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = QString("%1").arg(time);
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::AddAnnotationAtTime(unsigned long long time)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("ADD_ANNOTATION_AT_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = QString("%1").arg(time);
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::LoadAnnotation()
{
    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Annotation"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    QFile inFile(fileName);
    inFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream fileStream(&inFile);

    //Set annotation data
    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_ANNOTATION_BY_XML"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = fileStream.readAll();
    reqEv->id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(reqEv);

    //Wait for this to complete
    std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(reqEv->id);
    assert(response->type=="SET_ANNOTATION_DONE");

    //Read back current shape from annotation
    this->RefreshLinks();
}

void TrackingSceneController::SaveAnnotation()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Annotation Track"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    //If no file extension is set, use .annot as the extension
    QFileInfo fi(fileName);
    QString csuffix = fi.completeSuffix();
    if(csuffix.size()==0)
    {
        fileName.append(".annot");
    }

    QString xml = Annotation::GetAllAnnotationByXml(this->annotationUuid,
                                           this->eventLoop,
                                           this->eventReceiver);

    //Write to file
    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream fileStream(&outFile);
    fileStream << xml.toUtf8().constData();
    fileStream.flush();
}

void TrackingSceneController::SaveAnnotationCsv()
{
#ifndef DEMO_MODE
    this->ExportAnnotation("CSV","csv");
#endif
}

void TrackingSceneController::SaveAnnotationMatlab()
{
#ifndef DEMO_MODE
    this->ExportAnnotation("Matlab","mat");
#endif
}

void TrackingSceneController::SaveAnnotationMM()
{
#ifndef DEMO_MODE
    this->ExportAnnotation("Matrix Market","mm");
#endif
}

void TrackingSceneController::ExportAnnotation(QString type, QString ext)
{
#ifndef DEMO_MODE
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
                                                    QString("Save Annotation Track as %1").arg(type),
                                                    "",
                                                    QString("%1 (*.%2)").arg(type).arg(ext));
    if(fileName.length() == 0) return;

    //If no file extension is set, use .csv as the extension
    QFileInfo fi(fileName);
    QString csuffix = fi.completeSuffix();
    if(csuffix.size()==0)
    {
        fileName.append(".");
        fileName.append(ext);
    }

    //Trigger export
    std::tr1::shared_ptr<class Event> reqEv(new Event("EXPORT_ANNOTATION"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = fileName;
    reqEv->buffer = ext.toLocal8Bit();
    reqEv->id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(reqEv);
#endif
}

void TrackingSceneController::SaveShape()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0) return;

    //If no file extension is set, use .shape as the extension
    QFileInfo fi(fileName);
    QString csuffix = fi.completeSuffix();
    if(csuffix.size()==0)
    {
        fileName.append(".shape");
    }

    //Retrieve shape
    std::vector<std::vector<int> > links;
    std::vector<std::vector<float> > shape = Annotation::GetShape(this->annotationUuid,
                                                          this->eventLoop,
                                                          this->eventReceiver,
                                                          links);
    //Encode as XML
    QString xml;
    QTextStream xmlStr(&xml);
    TrackingAnnotationData::WriteShapeToStream(links, shape, xmlStr);

    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;;
    out << xml;
    f.close();
}

void TrackingSceneController::RemovePoint(int index)
{
    assert(this->eventLoop!=NULL);

    //Remove specified point
    std::tr1::shared_ptr<class Event> reqEv(new Event("REMOVE_POINT"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = QString::number(index,'d',0);
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::Update()
{
    //This is mainly needed to flush the event queue of unnecessary messages
    //Process events from application
    int flushEvents = 1;
    while(flushEvents)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();

        //Discard event
    }
    catch(std::runtime_error e)
    {
        flushEvents = 0;
    }
}

//*****************************************************************************

LogoSceneController::LogoSceneController(QObject *parent) : BaseSceneController(parent)
{

}

LogoSceneController::~LogoSceneController()
{

}

void LogoSceneController::Redraw()
{
    this->scene->clear();
    QPixmap pixmap("Kinatomic-Logo.jpg");
    this->scene->addPixmap(pixmap);
}

