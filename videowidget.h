#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>

class ImageSequence
{
public:
    ImageSequence(QString targetDir, float frameRate = 25.);
    virtual ~ImageSequence();
    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned GetNumFrames();
    long long unsigned Length(); //Get length (ms)

protected:
    long long unsigned minIndex, maxIndex;
    int numPackedChars;
    QString maxPrefix, maxExt, targetDir;
    float frameRate; //Hz
};

namespace Ui {
class VideoWidget;
}

class VideoWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
public slots:
    void SliderMoved(int newValue);

protected:
    void SetVisibleAtTime(long long unsigned ti);

    QSharedPointer<QGraphicsScene> scene;
    QSharedPointer<QGraphicsPixmapItem> item;
    QSharedPointer<ImageSequence> seq;

private:
    Ui::VideoWidget *ui;
};

#endif // VIDEOWIDGET_H
