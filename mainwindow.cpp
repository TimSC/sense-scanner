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
    this->threadCount = 0;

    //Create inter thread message system
    this->eventLoop = QSharedPointer<class EventLoop>(new class EventLoop);
    this->eventLoop->AddListener("THREAD_STARTING",eventReceiver);
    this->eventLoop->AddListener("THREAD_STOPPING",eventReceiver);

    //Create file reader worker thread
    this->readInputThread = new AvBinThread(&*this->eventLoop);
    this->readInputThread->start();

    //QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(
    //    new MediaBuffer(this, QSharedPointer<AbstractMedia>(
    //        new ImageSequence(this,"/home/tim/dev/QtMedia/testseq"))));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia());
    avbin->SetEventLoop(&*this->eventLoop);
    avbin->OpenFile("/home/tim/Downloads/Massive Attack Mezzanine Live.mp4");

    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec

    ui->setupUi(this);
    this->ui->widget->SetSource(buff);
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
    this->eventLoop->SendEvent(Event("STOP_THREADS"));

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
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia());
    avbin->OpenFile(fileName.toLocal8Bit().constData());

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
        cout << "Event type " << ev.type << endl;

        if(ev.type=="THREAD_STARTING")
        {
            this->threadCount ++;
        }
        if(ev.type=="THREAD_STOPPING")
        {
            assert(this->threadCount > 0);
            this->threadCount --;
        }
    }
    catch(std::runtime_error e) {flushing = 0;}
}
