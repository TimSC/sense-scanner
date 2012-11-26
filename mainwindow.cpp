#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videowidget.h"
#include "mediabuffer.h"
#include "imagesequence.h"
#include "avbinmedia.h"
#include "eventloop.h"
#include <QFileDialog>
#include <QThread>
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(
    //    new MediaBuffer(this, QSharedPointer<AbstractMedia>(
    //        new ImageSequence(this,"/home/tim/dev/QtMedia/testseq"))));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia(this,
            "/home/tim/Downloads/Massive Attack Mezzanine Live.mp4"));

    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);
    this->eventLoop = QSharedPointer<class EventLoop>(new class EventLoop);

    this->ui->widget->SetSource(buff);

    this->readInputThread = new AvBinThread(this->eventLoop);
    this->readInputThread->start();
}

MainWindow::~MainWindow()
{
    class Event event;
    event.type = event.EVENT_STOP_THREADS;
    this->eventLoop->SendEvent(event);

    delete ui;
    sleep(1);
}

void MainWindow::ImportVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Import Video"), "", tr("Video Files (*.avi *.mov *.mkv *.wmf, *.webm, *.flv, *.mp4, *.rm, *.asf)"));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia(this,
            fileName.toLocal8Bit().constData()));
    cout << "Opening " << fileName.toLocal8Bit().constData() << endl;
    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);

    this->ui->widget->SetSource(buff);
}
