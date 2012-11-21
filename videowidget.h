#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>

class ImageSequence
{
public:
    ImageSequence(QString targetDir);
    virtual ~ImageSequence();
    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned Length(); //Get length

protected:
    long long unsigned minIndex, maxIndex;
    int numPackedChars;
    QString maxPrefix, maxExt, targetDir;
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
