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
#include "eventloop.h"
#include "annotation.h"
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

void MouseGraphicsScene::SetSceneControl(TrackingSceneController *sceneControlIn)
{
    this->sceneControl = sceneControlIn;
}

//********************************************************************

TrackingSceneController::TrackingSceneController(QObject *parent)
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
    this->isShapeSet = 0;

    this->annotationControls = NULL;
    this->eventLoop = NULL;
    this->eventReceiver = NULL;
}

TrackingSceneController::~TrackingSceneController()
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

        std::tr1::shared_ptr<class Event> addEv(new Event("ADD_POINT"));
        addEv->toUuid = this->annotationUuid;
        QString args = QString("%1,%2").arg(pos.x()).arg(pos.y());
        addEv->data = args.toLocal8Bit().constData();
        this->eventLoop->SendEvent(addEv);

        this->RefreshCurrentPos();

        this->Redraw();
    }

    if(this->mode == "REMOVE_POINT" && button==Qt::LeftButton)
    {
        int nearestPoint = this->NearestPoint(pos.x(), pos.y(), currentFrame);
        if(nearestPoint>=0) this->RemovePoint(nearestPoint);
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

unsigned long long TrackingSceneController::GetSeekFowardTime()
{
    assert(this!=NULL);
    unsigned long long queryTime = this->annotationTime;
    if(!this->annotationTimeSet)
        queryTime = this->frameRequestTime;

    return this->GetSeekFowardTimeFromAnnot(queryTime);
}

unsigned long long TrackingSceneController::GetSeekBackTime()
{
    assert(this!=NULL);
    unsigned long long queryTime = this->annotationTime;
    if(!this->annotationTimeSet)
        queryTime = this->frameRequestTime;

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
        if(!isShapeSet)
        {
            this->LoadShape();
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

int TrackingSceneController::GetMouseOver()
{
    return this->mouseOver;
}

void TrackingSceneController::MouseEnterEvent()
{
    this->mouseOver = true;
    this->Redraw();
}

void TrackingSceneController::MouseLeaveEvent()
{
    this->mouseOver = false;
    this->Redraw();
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

void TrackingSceneController::LoadShape()
{

    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0)
    {
        return;
    }
    QFile shapeFile(fileName);
    shapeFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream shapeStream(&shapeFile);

    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_SHAPE"));
    reqEv->toUuid = this->annotationUuid;
    QString xml = shapeStream.readAll();
    reqEv->data = xml.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);

    this->isShapeSet = true;
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
    this->SetShape(annotShape);
}

void TrackingSceneController::ResetCurentFrameShape()
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
    assert(0); //TODO

    this->RefreshCurrentPos();
    this->Redraw();
}

QSharedPointer<MouseGraphicsScene> TrackingSceneController::GetScene()
{
    return this->scene;
}

//***************************************************

void TrackingSceneController::SetEventLoop(class EventLoop *eventLoopIn)
{
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventLoop = eventLoopIn;
    this->eventReceiver = new EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("ANNOTATION_FRAME",*eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_SHAPE",*eventReceiver);


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
    unsigned long long &annotationTime)
{
    assert(this->eventLoop!=NULL);
    annot.clear();
    annotationTime = 0;

    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_ANNOTATION_BETWEEN_TIMES"));
    reqEv->id = this->eventLoop->GetId();
    QString arg = QString("%1,%2,%3").arg(startTime).arg(endTime).arg(requestedTime);
    reqEv->data = arg.toLocal8Bit().constData();
    reqEv->toUuid = this->annotationUuid;
    this->eventLoop->SendEvent(reqEv);

    assert(this->eventReceiver!=NULL);
    std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(reqEv->id);
    if(response->data == "FRAME_NOT_FOUND")
    {
        return 0;
    }
    else
    {
        double ti = 0.;
        QString xml = response->data.c_str();
        int ret = TrackingAnnotationData::FrameFromXml(xml, annot, ti);
        if(ret==0) return 0; //Xml error

        annotationTime = (unsigned long long)(ti * 1000. + 0.5);
        return 1;
    }
}

void TrackingSceneController::SetAnnotationBetweenTimestamps(unsigned long long startTime,
    unsigned long long endTime,
    std::vector<std::vector<float> > annot)
{
    assert(this->eventLoop!=NULL);

    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_ANNOTATION_BETWEEN_TIMES"));
    reqEv->id = this->eventLoop->GetId();
    reqEv->toUuid = this->annotationUuid;
    QString xml;
    QTextStream xmlStr(&xml);
    TrackingAnnotationData::FrameToXml(annot, 0., xmlStr);
    QString data = QString("%1,%2,%3").arg(startTime).arg(endTime).arg(xml);
    reqEv->data = data.toLocal8Bit().constData();

    this->eventLoop->SendEvent(reqEv);
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

std::vector<std::vector<float> > TrackingSceneController::GetShape()
{
    assert(0);

}

void TrackingSceneController::SetShape(std::vector<std::vector<float> > shape)
{
    assert(0);

}

void TrackingSceneController::RefreshLinks()
{
    assert(this->eventLoop!=NULL);

    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_SHAPE"));
    reqEv->id = this->eventLoop->GetId();
    reqEv->toUuid = this->annotationUuid;
    this->eventLoop->SendEvent(reqEv);

    assert(this->eventReceiver!=NULL);
    std::tr1::shared_ptr<class Event> response = this->eventReceiver->WaitForEventId(reqEv->id);

    QDomDocument doc("mydocument");
    QString errorMsg;
    QString xmlStr(response->data.c_str());
    if (!doc.setContent(xmlStr, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        return;
    }

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    TrackingAnnotationData::ProcessXmlDomFrame(rootElem, this->links);
    return;
}

unsigned long long TrackingSceneController::GetSeekFowardTimeFromAnnot(unsigned long long queryTime)
{
    assert(0);
}

unsigned long long TrackingSceneController::GetSeekBackwardTimeFromAnnot(unsigned long long queryTime)
{
    assert(0);
}

void TrackingSceneController::RemoveAnnotationAtTime(unsigned long long time)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("REMOVE_ANNOTATION_AT_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = QString("%1").arg(time).toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::AddAnnotationAtTime(unsigned long long time)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("ADD_ANNOTATION_AT_TIME"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = QString("%1").arg(time).toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::LoadAnnotation()
{
    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Annotation"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    std::tr1::shared_ptr<class Event> reqEv(new Event("LOAD_ANNOTATION"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = fileName.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::SaveAnnotation()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Annotation Track"), "", tr("Annotation (*.annot)"));
    if(fileName.length() == 0) return;

    std::tr1::shared_ptr<class Event> reqEv(new Event("SAVE_ANNOTATION"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = fileName.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::SaveShape()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0) return;

    std::tr1::shared_ptr<class Event> reqEv(new Event("SAVE_SHAPE"));
    reqEv->toUuid = this->annotationUuid;
    reqEv->data = fileName.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::RemovePoint(int index)
{
    std::tr1::shared_ptr<class Event> reqEv(new Event("REMOVE_POINT"));
    reqEv->toUuid = this->annotationUuid;
    QString ind = QString("%1").arg(index);
    reqEv->data = ind.toLocal8Bit().constData();
    this->eventLoop->SendEvent(reqEv);
}
