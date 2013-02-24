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
using namespace::std;
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)

#define DEMO_MODE
#define SECRET_KEY "This is a secret..."

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
    this->frameTimesEnd = 0;
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

unsigned long long AbsDiff(unsigned long long a, unsigned long long b)
{
    if(a>b) return a-b;
    return b-a;
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

std::vector<std::vector<float> > TrackingSceneController::ProcessXmlDomFrame(QDomElement &rootElem)
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

void TrackingSceneController::LoadShape()
{

    //Get input filename from user
    QString fileName = QFileDialog::getOpenFileName(0,
        tr("Load Shape"), "", tr("Shapes (*.shape)"));
    if(fileName.length() == 0)
    {
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
        return;
    }
    f.close();

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    this->links.clear();
    std::vector<std::vector<float> > shape = this->ProcessXmlDomFrame(rootElem);

    //Validate points
    int invalidShape = 0;
    for(unsigned int i=0;i < shape.size();i++)
        if(shape[i].size() != 2)
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
        if(this->links[i][0] < 0 || this->links[i][0] >= shape.size())
        {
            cout << "Link refers to non-existent point " << this->links[i][0] << endl;
            invalidShape = 1;
        }
        if(this->links[i][1] < 0 || this->links[i][1] >= shape.size())
        {
            cout << "Link refers to non-existent point " << this->links[i][1] << endl;
            invalidShape = 1;
        }
    }

    if(invalidShape)
    {
        shape.clear();
        this->links.clear();
        return;
    }

    //TODO

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

/*void TrackingSceneController::GetIndexAnnotationXml(unsigned int index, QTextStream *out)
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
}*/

/*unsigned long long TrackingSceneController::GetIndexTimestamp(unsigned int index)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it = this->pos.begin();
    for(unsigned int i=0;i<index;i++)
        it ++;
    unsigned long long out = it->first;
    return out;
}*/

void TrackingSceneController::FoundFrame(unsigned long startTi, unsigned long endTi)
{
    //Update store
    this->frameTimes[startTi] = endTi;
    if(endTi > this->frameTimesEnd)
        this->frameTimesEnd = endTi;
}

void TrackingSceneController::GetFramesAvailable(std::map<unsigned long, unsigned long> &frameTimesOut,
                        unsigned long &frameTimesEndOut)
{
    frameTimesOut = this->frameTimes;
    frameTimesEndOut = this->frameTimesEnd;
}

void TrackingSceneController::SetEventLoop(class EventLoop *eventLoopIn)
{
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventLoop = eventLoopIn;
    this->eventReceiver = new EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("FOUND_ANNOTATION",*eventReceiver);
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
    assert(0);//TODO decode response
}

void TrackingSceneController::SetAnnotationBetweenTimestamps(unsigned long long startTime,
    unsigned long long endTime,
    std::vector<std::vector<float> > annot)
{
    assert(this->eventLoop!=NULL);

    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_ANNOTATION_BETWEEN_TIMES"));
    reqEv->id = this->eventLoop->GetId();
    reqEv->toUuid = this->annotationUuid;
    this->eventLoop->SendEvent(reqEv);
}

void TrackingSceneController::RefreshCurrentPos()
{
    assert(0);

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
    assert(0);

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
    assert(0);

}

void TrackingSceneController::AddAnnotationAtTime(unsigned long long time)
{
    assert(0);
}
