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
#include <unistd.h>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->threadCount = 0;

    //QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(
    //    new MediaBuffer(this, QSharedPointer<AbstractMedia>(
    //        new ImageSequence(this,"/home/tim/dev/QtMedia/testseq"))));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia(this,
            "/home/tim/Downloads/Massive Attack Mezzanine Live.mp4"));

    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);
    this->ui->widget->SetSource(buff);

    //Create inter thread message system
    this->eventLoop = QSharedPointer<class EventLoop>(new class EventLoop);
    this->eventLoop->AddListener(Event::EVENT_THREAD_STARTING,eventReceiver);
    this->eventLoop->AddListener(Event::EVENT_THREAD_STOPPING,eventReceiver);

    //Create file reader worker thread
    this->readInputThread = new AvBinThread(this->eventLoop);
    this->readInputThread->start();

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec
}

MainWindow::~MainWindow()
{
    delete this->timer;
    this->timer = NULL;

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //Signal worker threads to stop
    this->eventLoop->SendEvent(Event::EVENT_STOP_THREADS);

    //Stop the timer and handle messages in this function
    this->timer->stop();

    //Wait for threads to stop
    for(int i=0;i<500;i++)
    {
        this->Update();
        if(this->threadCount == 0) break;
        usleep(10000); //microsec
    }

    //If threads still running, terminate them
    if(this->readInputThread->isRunning())
        this->readInputThread->terminate();

    //Continue shut down in parent object
    QMainWindow::closeEvent(event);
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

void MainWindow::Update()
{
    //Check for and handle events
    int flushing = 1;
    while(flushing)
    try
    {
        class Event ev = this->eventReceiver.PopEvent();
        cout << "Event type" << ev.type << endl;

        if(ev.type==Event::EVENT_THREAD_STARTING)
        {
            this->threadCount ++;
        }
        if(ev.type==Event::EVENT_THREAD_STOPPING)
        {
            assert(this->threadCount > 0);
            this->threadCount --;
        }
    }
    catch(std::runtime_error e) {flushing = 0;}
}
