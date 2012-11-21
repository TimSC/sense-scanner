#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>

namespace Ui {
class MainWindow;
}

class ImageSequence
{
public:
    ImageSequence(QString targetDir);
    virtual ~ImageSequence();
    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned Length(); //Get length
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *item;
};

#endif // MAINWINDOW_H
