#ifndef MEDIABUFFER_H
#define MEDIABUFFER_H

#include <QObject>
#include <QSharedPointer>
#include <QImage>
#include <QMap>

class AbstractMedia : public QObject
{
    Q_OBJECT
public:
    explicit AbstractMedia(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractMedia() {}

public slots:
    virtual QSharedPointer<QImage> Get(long long unsigned ti)=0; //in milliseconds
    virtual long long unsigned GetNumFrames()=0;
    virtual long long unsigned Length()=0; //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti)=0; //in milliseconds

};

class MediaBuffer: public AbstractMedia
{
    Q_OBJECT
public:
    explicit MediaBuffer(QObject *parent, QSharedPointer<AbstractMedia> src);
    virtual ~MediaBuffer();

    void SetSource(QSharedPointer<AbstractMedia> src);

    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned GetNumFrames();
    long long unsigned Length(); //Get length (ms)
    long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds

signals:
    
protected:
    QSharedPointer<AbstractMedia> seq;
    QMap<unsigned long long, QSharedPointer<QImage> > buffer;
public slots:
    
};

#endif // MEDIABUFFER_H
