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
    int minIndex, maxIndex, maxPackedChars;
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

    void SliderMoved(int newValue);

protected:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *item;

private:
    Ui::VideoWidget *ui;
};

#endif // VIDEOWIDGET_H
