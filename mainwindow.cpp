#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->scene = new QGraphicsScene(this);

    QImage image("/home/tim/dev/QtMedia/test.png");
    assert(!image.isNull());
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    this->scene->addItem(item);
    this->ui->graphicsView->setScene(this->scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}
