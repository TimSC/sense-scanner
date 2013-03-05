#include "annotation.h"
#include "scenecontroller.h"
#include "avbinmedia.h"
#include "localsleep.h"
#include "videowidget.h"
#include <assert.h>
#include <iostream>
using namespace std;
#include "qblowfish/src/qblowfish.h"
#include "scenecontroller.h"

#define TO_MILLISEC(x) (unsigned long long)(x / 1000. + 0.5)
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)
#define DEMO_MODE
#define SECRET_KEY "This is a secret..."

unsigned long long AbsDiff(unsigned long long a, unsigned long long b)
{
    if(a>b) return a-b;
    return b-a;
}

//****************************************************************************

TrackingAnnotationData::TrackingAnnotationData()
{
    this->frameTimesEnd = 0;
    this->predictionEnd = 0;
}

TrackingAnnotationData::TrackingAnnotationData(const TrackingAnnotationData &other)
{
    this->operator=(other);
}

TrackingAnnotationData::~TrackingAnnotationData()
{

}

TrackingAnnotationData& TrackingAnnotationData::operator= (const TrackingAnnotationData &other)
{
    this->pos = other.pos;
    this->links = other.links;
    this->shape = other.shape;
    this->frameTimes = other.frameTimes;
    this->frameTimesEnd = other.frameTimesEnd;
    return *this;
}

bool TrackingAnnotationData::operator!= (const TrackingAnnotationData &other)
{
    bool ret = false;
    if(this->pos != other.pos) ret = true;
    if(this->shape != other.shape) ret = true;
    if(this->links != other.links) ret = true;
    return ret;
}

int TrackingAnnotationData::GetAnnotationAtTime(unsigned long long time,
    std::vector<std::vector<float> > &annot)
{

    annot.clear();
    //Check if there is annotation
    //at the requested time
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(time);
    if(it != this->pos.end())
    {
        annot = it->second;
        return 1;
    }
    return 0;
}

int TrackingAnnotationData::GetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          unsigned long long requestedTime,
                                                          std::vector<std::vector<float> > &annot,
                                                          unsigned long long &outAnnotationTime)
{
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
            return 1;
        }
    }

    //If the above did not find a frame, check if there is annotation
    //at the requested time
    int found = GetAnnotationAtTime(requestedTime, annot);
    if(found)
    {
        outAnnotationTime = requestedTime;
        return 1;
    }

    //Failed to find annotation
    return 0;
}

vector<unsigned long long> TrackingAnnotationData::GetAnnotationTimesBetweenTimestamps(unsigned long long startTime,
                                                                                      unsigned long long endTime)
{

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

    return out;
}

void TrackingAnnotationData::DeleteAnnotationAtTimestamp(unsigned long long annotationTimeIn)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    it = this->pos.find(annotationTimeIn);
    if(it != this->pos.end())
    {
        this->pos.erase(it);
    }
}

void TrackingAnnotationData::SetAnnotationBetweenTimestamps(unsigned long long startTime,
                                                          unsigned long long endTime,
                                                          std::vector<std::vector<float> > annot)
{

    if(annot.size() != this->shape.size())
    {
        cout << "Error: Cannot set annotation to mismatched size" << endl;
        return;
    }

    //Set annotation for annotated frames that already exist
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
        return;
    }

    //No annotation data set, so create a new annotation entry
    assert(endTime >= startTime);
    unsigned long long midTime = ROUND_TIMESTAMP(0.5*(startTime + endTime));
    this->pos[midTime] = annot;
}

std::vector<unsigned long long> TrackingAnnotationData::GetMarkTimes()
{
    std::vector<unsigned long long> out;
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it = this->pos.begin(); it != this->pos.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}

//*********************************************************

void TrackingAnnotationData::ReadAnnotationXml(QDomElement &elem)
{
    this->shape.clear();
    this->links.clear();
    this->pos.clear();
    QDomNode n = elem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if(e.tagName() == "shape")
            {
                std::vector<std::vector<float> > shapeData = this->ProcessXmlDomFrame(e,this->links);
                this->shape = shapeData;
            }

            //Obsolete format loading code here
            if(e.tagName() == "frame")
            {
                std::vector<std::vector<float> > frame = this->ProcessXmlDomFrame(e,this->links);
                //cout << e.attribute("time").toFloat() << endl;
                float timeSec = e.attribute("time").toFloat();
                assert(timeSec > 0.f);
                assert(frame.size() == this->shape.size());
                this->pos[(unsigned long long)(timeSec * 1000. + 0.5)] = frame;
            }

            //Newer frame XML format
            if(e.tagName() == "frameset")
            {
                ReadFramesXml(e);
            }

            if(e.tagName() == "demoframe")
            {
                ReadDemoFramesXml(e);
            }

            if(e.tagName() == "available")
            {
                this->frameTimes.clear();
                this->frameTimesEnd = e.attribute("to").toULong();

                QDomElement frEl = e.firstChildElement();
                while(!frEl.isNull())
                {
                    if(frEl.tagName() != "f") continue;
                    unsigned long s = frEl.attribute("s").toULong();
                    unsigned long e = frEl.attribute("e").toULong();
                    this->frameTimes[s] = e;
                    frEl = frEl.nextSiblingElement();
                }
            }
        }
        n = n.nextSibling();
    }
}

void TrackingAnnotationData::ReadFramesXml(QDomElement &elem)
{
    QDomElement e = elem.firstChildElement();
    while(!e.isNull())
    {
        if(e.tagName() != "frame") continue;

        std::vector<std::vector<float> > frame = this->ProcessXmlDomFrame(e,this->links);
        //cout << e.attribute("time").toFloat() << endl;
        float timeSec = e.attribute("time").toFloat();
        assert(frame.size() == this->shape.size());
        this->pos[(unsigned long long)(timeSec * 1000. + 0.5)] = frame;

        e = e.nextSiblingElement();
    }
}

void TrackingAnnotationData::ReadDemoFramesXml(QDomElement &elem)
{
    QString content = elem.text();
    int test = content.length();
    QString test2 = elem.tagName();

    QByteArray encData = QByteArray::fromBase64(content.toLocal8Bit().constData());
    int test3 = encData.length();

    QByteArray secretKey(SECRET_KEY);
    QBlowfish bf(secretKey);
    bf.setPaddingEnabled(true);

    QByteArray encryptedBa = bf.decrypted(encData);
    QString framesXml = QString::fromUtf8(encryptedBa);

    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(framesXml, &errorMsg))
    {
        throw runtime_error(errorMsg.toLocal8Bit().constData());
    }

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    this->ReadFramesXml(rootElem);
}

void TrackingAnnotationData::FrameToXml(std::vector<std::vector<float> > &frame,
                                        double ti, QTextStream &out)
{
    out << "\t<frame time='"<<ti<<"'>\n";
    for(unsigned int i=0; i < frame.size(); i++)
    {
        out << "\t\t<point id='"<<i<<"' x='"<<frame[i][0]<<"' y='"<<frame[i][1]<<"'/>\n";
    }
    out << "\t</frame>" << endl;
}

int TrackingAnnotationData::FrameFromXml(QString &xml,
    std::vector<std::vector<float> > &frame,
    double &tiOut)
{
    frame.clear();
    tiOut = 0;
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(xml, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        return 0;
    }

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    std::vector<std::vector<int> > links;
    frame = TrackingAnnotationData::ProcessXmlDomFrame(rootElem,links);
    tiOut = rootElem.attribute("time").toDouble();
    return 1;
}

void TrackingAnnotationData::WriteAnnotationXml(QTextStream &out)
{
    out << "\t<tracking>" << endl;
    this->WriteShapeToStream(this->links, this->shape, out);

    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;

    //Save annotated frames
    QString frameXmlStr;
    QTextStream frameXml(&frameXmlStr);
    frameXml << "\t<frameset>" << endl;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        assert(frame.size() == this->shape.size());
        this->FrameToXml(frame, (it->first/1000.), frameXml);
    }
    frameXml << "\t</frameset>" << endl;

#ifndef DEMO_MODE
    out << frameXml.string();
#else
    out << "<demoframe>" << endl;
    QByteArray secretKey(SECRET_KEY);

    QBlowfish bf(secretKey);
    bf.setPaddingEnabled(true);
    QByteArray encryptedBa = bf.encrypted(frameXmlStr.toUtf8());
    QByteArray encBase64 = encryptedBa.toBase64();
    for(int pos=0;pos<encBase64.length();pos+=512)
        out << encBase64.mid(pos,512) << endl;
    //out << encBase64 << endl;

    out << "</demoframe>" << endl;

#endif //DEMO_MODE

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

void TrackingAnnotationData::RemovePoint(int index)
{
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
}

void TrackingAnnotationData::AddPoint(std::vector<float> p)
{
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        frame.push_back(p);
        assert(frame.size() == this->shape.size() + 1);
    }

    //Apply to currunt shape template
    this->shape.push_back(p);

}


unsigned long long TrackingAnnotationData::GetSeekForwardTime(unsigned long long queryTime)
{
    assert(this!=NULL);

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
    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

unsigned long long TrackingAnnotationData::GetSeekBackTime(unsigned long long queryTime)
{
    assert(this!=NULL);

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

    if(bestSet)
        return bestFrame;
    throw std::runtime_error("No frame");
}

void TrackingAnnotationData::SetShape(std::vector<std::vector<float> > shapeIn)
{
    this->shape = shapeIn;

    //Check existing data to see if has the correct number of points
    std::map<unsigned long long, std::vector<std::vector<float> > >::iterator it;
    for(it=this->pos.begin(); it != this->pos.end();it++)
    {
        std::vector<std::vector<float> > &frame = it->second;
        while(frame.size() > shape.size())
        {
            frame.pop_back();
        }
        while(frame.size() < shape.size())
        {
            frame.push_back(shape[frame.size()]);
        }
    }
}

void TrackingAnnotationData::WriteShapeToStream(
        std::vector<std::vector<int> > links,
        std::vector<std::vector<float> > shape,
        QTextStream &out)
{
    out << "\t<shape>" << endl;

    for(unsigned int i=0; i < shape.size(); i++)
    {
        out << "\t\t<point id='"<<i<<"' x='"<<shape[i][0]<<"' y='"<<shape[i][1]<<"'/>" << endl;
    }
    for(unsigned int i=0;i < links.size();i++)
    {
        out << "\t\t<link from='"<<links[i][0]<<"' to='"<<links[i][1]<<"'/>" << endl;
    }

    out << "\t</shape>" << endl;
}

int TrackingAnnotationData::XmlToShape(QString xml,
                                        std::vector<std::vector<int> > &links,
                                        std::vector<std::vector<float> > &shapeOut)
{
    //Parse XML to DOM
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(xml, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        return 0;
    }

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();

    links.clear();
    shapeOut = TrackingAnnotationData::ProcessXmlDomFrame(rootElem, links);

    //Validate points
    int invalidShape = 0;
    for(unsigned int i=0;i < shapeOut.size();i++)
        if(shapeOut[i].size() != 2)
        {
            cout << "Error: missing point ID " << i << endl;
            invalidShape = 1;
        }

    //Validate links
    for(unsigned int i=0;i<links.size();i++)
    {
        if(links[i].size() != 2)
        {
            cout << "Error: Invalid link" << endl;
            invalidShape = 1;
        }
        if(links[i][0] < 0 || links[i][0] >= shapeOut.size())
        {
            cout << "Link refers to non-existent point " << links[i][0] << endl;
            invalidShape = 1;
        }
        if(links[i][1] < 0 || links[i][1] >= shapeOut.size())
        {
            cout << "Link refers to non-existent point " << links[i][1] << endl;
            invalidShape = 1;
        }
    }

    if(invalidShape)
    {
        shapeOut.clear();
        links.clear();
        return 0;
    }
    return 1;
}

std::vector<std::vector<float> > TrackingAnnotationData::ProcessXmlDomFrame(QDomElement &rootElem,
    std::vector<std::vector<int> > linksOut)
{
    linksOut.clear();
    std::vector<std::vector<float> > out;
    QDomNode n = rootElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(!e.isNull()) {
            QString tagName = e.tagName();
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
                linksOut.push_back(link);
            }
        }
    n = n.nextSibling();
    }

    return out;
}

void TrackingAnnotationData::SaveShape(QString fileName)
{
    //Save data to file
    QFile f(fileName);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;;
    this->WriteShapeToStream(this->links,this->shape,out);
    f.close();
}

void TrackingAnnotationData::FoundFrame(unsigned long startTi, unsigned long endTi)
{
    //Update store
    this->frameTimes[startTi] = endTi;
    if(endTi > this->frameTimesEnd)
        this->frameTimesEnd = endTi;
}

void TrackingAnnotationData::GetFramesAvailable(std::map<unsigned long, unsigned long> &frameTimesOut,
                        unsigned long &frameTimesEndOut)
{
    frameTimesOut = this->frameTimes;
    frameTimesEndOut = this->frameTimesEnd;
}

int TrackingAnnotationData::GetShapeNumPoints()
{
    return this->shape.size();
}

std::vector<std::vector<int> > TrackingAnnotationData::GetLinks()
{
    return this->links;
}

void TrackingAnnotationData::SetLinks(std::vector<std::vector<int> > linksIn)
{
    this->links = linksIn;
}

std::vector<std::vector<float> > TrackingAnnotationData::GetShapePositions()
{
    return this->shape;
}

void TrackingAnnotationData::AddAnnotationAtTime(unsigned long long ti)
{
    this->pos[ti] = this->shape;
}

void TrackingAnnotationData::RemoveAnnotationAtTime(unsigned long long ti)
{
    this->pos.erase(ti);
}

std::vector<std::vector<float> > TrackingAnnotationData::GetAnnotationAtTime(unsigned long long ti)
{
    return this->pos[ti];
}

//****************************************************

AnnotThread::AnnotThread(class Annotation *annIn)
{
    this->parentAnn = annIn;
    this->frameTimesEnd = 0;
    this->frameTimesSet = false;
}

AnnotThread::~AnnotThread()
{

}

void AnnotThread::SetEventLoop(class EventLoop *eventLoopIn)
{
    MessagableThread::SetEventLoop(eventLoopIn);
    this->eventLoop->AddListener("PREDICTION_RESULT", *this->eventReceiver);
    this->eventLoop->AddListener("STOP_SPECIFIC_THREAD", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ANNOTATION_BETWEEN_TIMES", *this->eventReceiver);
    this->eventLoop->AddListener("SET_ANNOTATION_BETWEEN_TIMES", *this->eventReceiver);

    this->eventLoop->AddListener("SET_SOURCE_FILENAME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_SOURCE_FILENAME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_SHAPE", *this->eventReceiver);
    this->eventLoop->AddListener("SET_SHAPE", *this->eventReceiver);
    this->eventLoop->AddListener("ADD_ANNOTATION_AT_TIME", *this->eventReceiver);
    this->eventLoop->AddListener("REMOVE_ANNOTATION_AT_TIME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ANNOTATION_AT_TIME", *this->eventReceiver);
    this->eventLoop->AddListener("SET_ALG_UUID", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ALG_UUID", *this->eventReceiver);
    this->eventLoop->AddListener("GET_ALL_ANNOTATION_XML", *this->eventReceiver);
    this->eventLoop->AddListener("SET_ANNOTATION_BY_XML", *this->eventReceiver);

    this->eventLoop->AddListener("GET_SEEK_BACKWARD_TIME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_SEEK_FORWARD_TIME", *this->eventReceiver);
    this->eventLoop->AddListener("GET_MARKED_LIST", *this->eventReceiver);

    this->eventLoop->AddListener("MEDIA_DURATION_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("MEDIA_FRAME_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_THREAD_PROGRESS", *this->eventReceiver);

    this->eventLoop->AddListener("GET_PREDICTION_END", *this->eventReceiver);
}

void AnnotThread::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{

    //cout << "AnnotThread " << (long long unsigned) this << endl;
    QUuid algUid = this->parentAnn->GetAnnotUid();

    if(ev->toUuid == algUid)
    {
    if(ev->type=="STOP_SPECIFIC_THREAD")
    {
        //TODO Is this redundant with the funtionality in the parent class?
        this->stopThreads = true;
    }

    if(ev->type=="GET_ANNOTATION_BETWEEN_TIMES")
    {
        //Decode request
        std::vector<std::string> args = split(ev->data.toLocal8Bit().constData(),',');

        unsigned long long startTime = STR_TO_ULL_SIMPLE(args[0].c_str());
        unsigned long long endTime = STR_TO_ULL_SIMPLE(args[1].c_str());
        unsigned long long requestedTime = STR_TO_ULL_SIMPLE(args[2].c_str());
        std::vector<std::vector<float> > annot;
        unsigned long long annotationTime;

        //Perform request
        assert(this->parentAnn!=NULL);
        assert(this->parentAnn->track!=NULL);
        int found = this->parentAnn->track->GetAnnotationBetweenTimestamps(startTime,
            endTime, requestedTime,
            annot, annotationTime);

        //Return response by event
        std::tr1::shared_ptr<class Event> responseEv(new Event("ANNOTATION_FRAME"));
        responseEv->fromUuid = algUid;
        if(found)
        {
            QString xmlStr;
            QTextStream xml(&xmlStr);
            TrackingAnnotationData::FrameToXml(annot, annotationTime / 1000., xml);
            responseEv->data = xmlStr.toLocal8Bit().constData();
        }
        else
        {
            responseEv->data = "FRAME_NOT_FOUND";
        }
        responseEv->id = ev->id;
        this->eventLoop->SendEvent(responseEv);
    }

    if(ev->type=="SET_ANNOTATION_BETWEEN_TIMES")
    {
        //Find first two commas and split the string to args
        std::string dataStr(ev->data.toLocal8Bit().constData());
        std::string::size_type firstComma = dataStr.find(",",0);
        std::string::size_type secondComma = dataStr.find(",",firstComma+1);

        QString startStr = dataStr.substr(0, firstComma).c_str();
        QString endStr = dataStr.substr(firstComma+1, secondComma-firstComma-1).c_str();
        QString xml = dataStr.substr(secondComma+1).c_str();
        QString qxml(xml);
        //unsigned long long startTime
        //unsigned long long endTime

        std::vector<std::vector<float> > frame;
        double ti;
        int ret = TrackingAnnotationData::FrameFromXml(qxml,
                                     frame,
                                     ti);

        assert(ret);
        assert(this->parentAnn!=NULL);
        assert(this->parentAnn->track!=NULL);
        this->parentAnn->track->SetAnnotationBetweenTimestamps(startStr.toULongLong(), endStr.toULongLong(), frame);
    }

    if(ev->type=="SET_SOURCE_FILENAME")
    {
        this->parentAnn->SetSource(ev->data);
        std::tr1::shared_ptr<class Event> changeEv(new Event("WORKSPACE_ANNOTATION_CHANGED"));
        this->eventLoop->SendEvent(changeEv);
    }

    if(ev->type=="GET_SOURCE_FILENAME")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("SOURCE_FILENAME"));
        responseEv->data = this->parentAnn->GetSource().toLocal8Bit().constData();
        responseEv->fromUuid = algUid;
        responseEv->id = ev->id;
        this->eventLoop->SendEvent(responseEv);
    }

    if(ev->type=="GET_SHAPE")
    {
        //Return response by event
        std::tr1::shared_ptr<class Event> responseEv(new Event("ANNOTATION_SHAPE"));
        responseEv->fromUuid = algUid;
        QString xmlStr;
        QTextStream xml(&xmlStr);
        TrackingAnnotationData::WriteShapeToStream(this->parentAnn->track->GetLinks(),
                                                   this->parentAnn->track->GetShapePositions(),
                                                   xml);
        responseEv->data = xmlStr.toLocal8Bit().constData();
        responseEv->id = ev->id;
        this->eventLoop->SendEvent(responseEv);
    }
    if(ev->type=="SET_SHAPE")
    {
        QString xml = ev->data;
        std::vector<std::vector<int> > links;
        std::vector<std::vector<float> > shape;
        int ret = TrackingAnnotationData::XmlToShape(xml, links, shape);

        if(ret)
        {
            this->parentAnn->track->SetShape(shape);
            this->parentAnn->track->SetLinks(links);
        }
    }
    if(ev->type=="ADD_ANNOTATION_AT_TIME")
    {
        unsigned long long ti = ev->data.toULongLong();
        assert(this->parentAnn!=NULL);
        assert(this->parentAnn->track!=NULL);
        this->parentAnn->track->AddAnnotationAtTime(ti);

    }
    if(ev->type=="REMOVE_ANNOTATION_AT_TIME")
    {
        unsigned long long ti = ev->data.toULongLong();
        assert(this->parentAnn!=NULL);
        assert(this->parentAnn->track!=NULL);
        this->parentAnn->track->RemoveAnnotationAtTime(ti);
    }
    if(ev->type=="GET_ANNOTATION_AT_TIME")
    {
        //Check for annotation at this time
        unsigned long long ti = ev->data.toULongLong();
        assert(this->parentAnn!=NULL);
        assert(this->parentAnn->track!=NULL);
        QString xml;

        try
        {
            std::vector<std::vector<float> > ann = this->parentAnn->track->GetAnnotationAtTime(ti);

            //Format as XML
            QTextStream xmlStr(&xml);
            TrackingAnnotationData::FrameToXml(ann, (ti / 1000.), xmlStr);
        }
        catch(exception &err)
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("ANNOTATION_AT_TIME"));
            responseEv->id = ev->id;
            responseEv->data = "NOT_FOUND";
            this->eventLoop->SendEvent(responseEv);
        }

        //Return data as event
        std::tr1::shared_ptr<class Event> responseEv(new Event("ANNOTATION_AT_TIME"));
        responseEv->id = ev->id;
        responseEv->data = xml.toLocal8Bit().constData();
        this->eventLoop->SendEvent(responseEv);

    }
    if(ev->type=="SET_ALG_UUID")
    {
        this->parentAnn->SetAlgUid(QUuid(ev->data));
    }

    if(ev->type=="GET_ALG_UUID")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("ALG_UUID_FOR_ANNOTATION"));
        responseEv->data = this->parentAnn->GetAlgUid().toString().toLocal8Bit().constData();
        responseEv->fromUuid = this->parentAnn->GetAnnotUid();
        responseEv->id = ev->id;
        this->eventLoop->SendEvent(responseEv);
    }
    if(ev->type=="GET_ALL_ANNOTATION_XML")
    {
        QString xml;
        QTextStream xmlStr(&xml);
        this->parentAnn->track->WriteAnnotationXml(xmlStr);

        std::tr1::shared_ptr<class Event> responseEv(new Event("ANNOTATION_DATA"));
        responseEv->fromUuid = this->parentAnn->GetAnnotUid();
        responseEv->id = ev->id;
        responseEv->data = xml.toLocal8Bit().constData();
        this->eventLoop->SendEvent(responseEv);
    }
    if(ev->type=="SET_ANNOTATION_BY_XML")
    {
        QDomDocument doc("mydocument");
        QString errorMsg;
        QString xml(ev->data);
        if (!doc.setContent(xml, &errorMsg))
        {
            //throw runtime_error(errorMsg.toLocal8Bit().constData());
            return;
        }

        //Load points and links into memory
        QDomElement rootElem = doc.documentElement();
        this->parentAnn->track->ReadAnnotationXml(rootElem);
    }
    if(ev->type=="GET_SEEK_BACKWARD_TIME")
    {
        unsigned long long ti = ev->data.toULongLong();

        try
        {
            unsigned long long ret = this->parentAnn->track->GetSeekBackTime(ti);
            std::tr1::shared_ptr<class Event> responseEv(new Event("SEEK_RESULT"));
            responseEv->id = ev->id;
            QString dataStr = QString("%1").arg(ret);
            responseEv->data = dataStr.toLocal8Bit().constData();
            this->eventLoop->SendEvent(responseEv);
        }
        catch(exception &err)
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("SEEK_RESULT"));
            responseEv->id = ev->id;
            responseEv->data = "NOT_FOUND";
            this->eventLoop->SendEvent(responseEv);
        }
    }
    if(ev->type=="GET_SEEK_FORWARD_TIME")
    {
        unsigned long long ti = ev->data.toULongLong();

        try
        {
            unsigned long long ret = this->parentAnn->track->GetSeekForwardTime(ti);
            std::tr1::shared_ptr<class Event> responseEv(new Event("SEEK_RESULT"));
            responseEv->id = ev->id;
            QString dataStr = QString("%1").arg(ret);
            responseEv->data = dataStr.toLocal8Bit().constData();
            this->eventLoop->SendEvent(responseEv);
        }
        catch(exception &err)
        {
            std::tr1::shared_ptr<class Event> responseEv(new Event("SEEK_RESULT"));
            responseEv->id = ev->id;
            responseEv->data = "NOT_FOUND";
            this->eventLoop->SendEvent(responseEv);
        }
    }
    if(ev->type=="GET_MARKED_LIST")
    {
        std::vector<unsigned long long> tis = this->parentAnn->track->GetMarkTimes();
        QString tisStr;
        for(unsigned int i=0;i<tis.size();i++)
        {
            if(i>0) tisStr.append(",");
            QString tmp = QString("%1").arg(tis[i]);
            tisStr.append(tmp);
        }

        std::tr1::shared_ptr<class Event> responseEv(new Event("MARKED_LIST_RESPONSE"));
        responseEv->id = ev->id;
        responseEv->data = tisStr.toLocal8Bit().constData();
        this->eventLoop->SendEvent(responseEv);
    }

    if(ev->type=="GET_PREDICTION_END")
    {
        std::tr1::shared_ptr<class Event> responseEv(new Event("PREDICTION_END"));
        QString dataStr = QString("%1").arg(this->parentAnn->track->predictionEnd);
        responseEv->data = dataStr;
        responseEv->fromUuid = this->parentAnn->GetAnnotUid();
        responseEv->id = ev->id;
        this->eventLoop->SendEvent(responseEv);
    }
    }

    //This uses a different field to filter uuids
    if(ev->type=="ANNOTATION_THREAD_PROGRESS")
    {
        QUuid msgAnnotUuid(ev->buffer);
        if(algUid == msgAnnotUuid)
        {
            this->parentAnn->track->predictionEnd = ev->data.toULongLong();
        }
    }



    this->msleep(5);
    MessagableThread::HandleEvent(ev);
}

void AnnotThread::Update()
{
    this->msleep(5);
}

void AnnotThread::Finished()
{
    QString src = this->parentAnn->GetSource();
    cout << "AnnotThread::Finished()" << src.toLocal8Bit().constData() << endl;

}

void AnnotThread::SendEvent(std::tr1::shared_ptr<class Event> event)
{
    assert(this->eventLoop!=NULL);
    this->eventLoop->SendEvent(event);

}

std::tr1::shared_ptr<class Event> AnnotThread::WaitForEventId(unsigned long long id,
                           unsigned timeOutMs)
{
    assert(this->eventReceiver!=NULL);
    return this->eventReceiver->WaitForEventId(id, timeOutMs);
}

unsigned long long AnnotThread::GetNewEventId()
{
    return this->eventLoop->GetId();
}

//*****************************************************

Annotation::Annotation()
{
    this->track = NULL;
}

Annotation::~Annotation()
{
    if(this->annotThread)
    {
        annotThread->Stop();
    }
    std::tr1::shared_ptr<class AnnotThread> empty;
    this->annotThread = empty;
    this->SetTrack(NULL);
}

Annotation& Annotation::operator= (const Annotation &other)
{
    source = other.source;
    uid = other.uid;
    algUid = other.algUid;
    if(this->track) delete this->track;
    this->track = NULL;

    this->SetTrack(new TrackingAnnotationData());
    *this->track = *other.track;
    return *this;
}

bool Annotation::operator!= (const Annotation &other)
{
    if(source != other.source) return true;
    if(uid != other.uid) return true;
    if(*track != *other.track) return true;
    if(algUid != other.algUid) return true;
    return false;
}

void Annotation::Clear()
{
    this->SetTrack(NULL);
    QUuid uidBlank;
    this->SetAnnotUid(uidBlank);
    this->algUid = algUid;
    this->source = "";
    std::tr1::shared_ptr<class AnnotThread> thd;
    this->annotThread = thd;
}

void Annotation::SetTrack(class TrackingAnnotationData *trackIn)
{
    if(this->track != NULL) delete this->track;
    this->track = trackIn;
}

class TrackingAnnotationData *Annotation::GetTrack()
{
    this->lock.lock();
    class TrackingAnnotationData *out = this->track;
    this->lock.unlock();
    return out;
}

void Annotation::SetAlgUid(QUuid uidIn)
{
    this->lock.lock();
    this->algUid = uidIn;
    this->lock.unlock();
}

QUuid Annotation::GetAlgUid()
{
    this->lock.lock();
    QUuid out = this->algUid;
    this->lock.unlock();
    return out;
}

void Annotation::SetAnnotUid(QUuid uidIn)
{
    this->lock.lock();
    this->uid = uidIn;
    if(this->annotThread != NULL)
        this->annotThread->SetThreadId(uidIn);
    this->lock.unlock();
}

QUuid Annotation::GetAnnotUid()
{
    this->lock.lock();
    QUuid out = this->uid;
    this->lock.unlock();
    return out;
}

void Annotation::SetSource(QString sourceIn)
{
    this->lock.lock();
    this->source = sourceIn;
    this->lock.unlock();
}

QString Annotation::GetSource()
{
    this->lock.lock();
    QString out = this->source;
    this->lock.unlock();
    return out;
}

void Annotation::Terminate()
{
    if(this->annotThread && this->annotThread->isRunning())
    {
        cout << "Warning: terminating annot thread " << this->source.toLocal8Bit().constData() << endl;
        this->annotThread->terminate();
    }
}

void Annotation::FoundFrame(unsigned long startTi, unsigned long endTi)
{
    if(this->track != NULL)
    {
        this->track->FoundFrame(startTi, endTi);
    }
}

void Annotation::PreDelete()
{

}

QString Annotation::GetSourceFilename(QUuid annotUuid,
                                      class EventLoop *eventLoop,
                                      class EventReceiver *eventReceiver)
{
    //Get source filename for annotation
    std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
    getSourceNameEv->toUuid = annotUuid;
    getSourceNameEv->id = eventLoop->GetId();
    eventLoop->SendEvent(getSourceNameEv);

    std::tr1::shared_ptr<class Event> sourceName = eventReceiver->WaitForEventId(getSourceNameEv->id);
    assert(sourceName->type=="SOURCE_FILENAME");
    return sourceName->data;
}

QUuid Annotation::GetAlgUuid(QUuid annotUuid,
                             class EventLoop *eventLoop,
                             class EventReceiver *eventReceiver)
{
    //Get algorithm Uuid for this annotation track
    std::tr1::shared_ptr<class Event> getAlgUuidEv(new Event("GET_ALG_UUID"));
    getAlgUuidEv->toUuid = annotUuid;
    getAlgUuidEv->id = eventLoop->GetId();
    eventLoop->SendEvent(getAlgUuidEv);

    std::tr1::shared_ptr<class Event> algUuidEv = eventReceiver->WaitForEventId(getAlgUuidEv->id);
    assert(algUuidEv->type=="ALG_UUID_FOR_ANNOTATION");
    return QUuid(algUuidEv->data);
}

int Annotation::GetAnnotationBetweenFrames(unsigned long long startTime,
                                           unsigned long long endTime,
                                           unsigned long long requestedTime,
                                           QUuid annotUuid,
                                           class EventLoop *eventLoop,
                                           class EventReceiver *eventReceiver,
                                           std::vector<std::vector<float> > &frameOut,
                                           double &tiOut)
{
    frameOut.clear();
    tiOut = 0.;
    std::tr1::shared_ptr<class Event> reqEv(new Event("GET_ANNOTATION_BETWEEN_TIMES"));
    reqEv->id = eventLoop->GetId();
    QString arg = QString("%1,%2,%3").arg(startTime).arg(endTime).arg(requestedTime);
    reqEv->data = arg.toLocal8Bit().constData();
    reqEv->toUuid = annotUuid;
    eventLoop->SendEvent(reqEv);

    assert(eventReceiver!=NULL);
    try
    {
        std::tr1::shared_ptr<class Event> response = eventReceiver->WaitForEventId(reqEv->id);
        assert(response->type=="ANNOTATION_FRAME");
        if(response->data == "FRAME_NOT_FOUND")
        {
            return 0;
        }
        else
        {
            double ti = 0.;
            QString xml = response->data;
            int ret = TrackingAnnotationData::FrameFromXml(xml, frameOut, tiOut);
            if(ret==0) return -1; //Xml error
            return 1;
        }
    }
    catch(std::runtime_error &err)
    {
        //Probably caused by aborted wait
    }
    return -2;
}

void Annotation::SetAnnotationBetweenTimestamps(unsigned long long startTime,
                                unsigned long long endTime,
                                std::vector<std::vector<float> > annot,
                                QUuid annotUuid,
                                class EventLoop *eventLoop)
{
    assert(endTime >= startTime);
    assert(eventLoop!=NULL);

    std::tr1::shared_ptr<class Event> reqEv(new Event("SET_ANNOTATION_BETWEEN_TIMES"));
    reqEv->id = eventLoop->GetId();
    reqEv->toUuid = annotUuid;
    QString xml;
    QTextStream xmlStr(&xml);
    TrackingAnnotationData::FrameToXml(annot, 0., xmlStr);
    QString data = QString("%1,%2,%3").arg(startTime).arg(endTime).arg(xml);
    reqEv->data = data.toLocal8Bit().constData();

    eventLoop->SendEvent(reqEv);
}

unsigned long long Annotation::GetPredictionEnd(QUuid annotUuid,
                                class EventLoop *eventLoop,
                                class EventReceiver *eventReceiver)
{
    //Get algorithm Uuid for this annotation track
    std::tr1::shared_ptr<class Event> getAlgUuidEv(new Event("GET_PREDICTION_END"));
    getAlgUuidEv->toUuid = annotUuid;
    getAlgUuidEv->id = eventLoop->GetId();
    eventLoop->SendEvent(getAlgUuidEv);

    std::tr1::shared_ptr<class Event> responseEV = eventReceiver->WaitForEventId(getAlgUuidEv->id);
    assert(responseEV->type=="PREDICTION_END");
    return responseEV->data.toULongLong();
}

void Annotation::UpdateAnnotationThreadProgress(unsigned long long progress,
                                    QUuid srcUuid,
                                    QUuid annotUuid,
                                    class EventLoop *eventLoop)
{
    //Estimate progress and generate an event
    std::tr1::shared_ptr<class Event> requestEv(new Event("ANNOTATION_THREAD_PROGRESS"));
    QString progressStr = QString("%0").arg(progress);
    requestEv->fromUuid = srcUuid;
    requestEv->data = progressStr;
    requestEv->buffer = annotUuid.toByteArray();
    eventLoop->SendEvent(requestEv);
}
