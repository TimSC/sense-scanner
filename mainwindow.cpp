#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>

ImageSequence::ImageSequence(QString targetDir)
{

}

ImageSequence::~ImageSequence()
{

}

QSharedPointer<QImage> ImageSequence::Get(long long unsigned ti) //in milliseconds
{
    QImage *image = new QImage("/home/tim/dev/QtMedia/test.png");
    QSharedPointer<QImage> out(image);
    return out;
}

long long unsigned ImageSequence::Length() //Get length
{
    return 1;

}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->scene = new QGraphicsScene(this);

    ImageSequence seq(".");
    QSharedPointer<QImage> image = seq.Get(0);
    assert(!image->isNull());

    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    this->scene->addItem(item);
    this->ui->graphicsView->setScene(this->scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}
