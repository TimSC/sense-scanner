#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videowidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->ui->widget->SetSource(QSharedPointer<AbstractMedia>(new ImageSequence(this,"/home/tim/dev/QtMedia/testseq")));
}

MainWindow::~MainWindow()
{
    delete ui;
}
