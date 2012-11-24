#ifndef AVBINMEDIA_H
#define AVBINMEDIA_H

#include <QWidget>
#include <QtCore>
#include <QtGui>

class AvBinMedia : public QObject
{
    Q_OBJECT
public:
    explicit AvBinMedia(QObject *parent = 0);
    virtual ~AvBinMedia();

public slots:
    virtual QSharedPointer<QImage> Get(long long unsigned ti)=0; //in milliseconds
    virtual long long unsigned GetNumFrames()=0;
    virtual long long unsigned Length()=0; //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti)=0; //in milliseconds

protected:
    class AvBinBackend *backend;
};


#endif // AVBINMEDIA_H
