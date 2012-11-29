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
    this->eventLoop = new class EventLoop();

    //Create event listener
    this->eventReceiver = new class EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("THREAD_STARTING",*eventReceiver);
    this->eventLoop->AddListener("THREAD_STOPPING",*eventReceiver);

    //Create file reader worker thread
    this->mediaThread = new AvBinThread(this->eventLoop);
    this->mediaThread->start();

    //QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(
    //    new MediaBuffer(this, QSharedPointer<AbstractMedia>(
    //        new ImageSequence(this,"/home/tim/dev/QtMedia/testseq"))));
    //QSharedPointer<AvBinMedia> avbin (new class AvBinMedia());
    this->mediaInterface = new class AvBinMedia();
    this->mediaInterface->SetEventLoop(this->eventLoop);
    this->mediaInterface->SetActive(1);
    this->mediaInterface->OpenFile("/home/tim/Desktop/SurreyHeadPoseDatabase/SANY0012.MP4");

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec

    ui->setupUi(this);
    this->ui->widget->SetSource(this->mediaInterface);
}

MainWindow::~MainWindow()
{
    delete this->timer;
    this->timer = NULL;

    delete this->eventLoop;
    this->eventLoop = NULL;

    delete this->eventReceiver;
    this->eventReceiver = NULL;

    if(this->mediaThread) delete this->mediaThread;
    this->mediaThread = NULL;

    delete this->mediaInterface;
    this->mediaInterface = NULL;

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    //Disconnect video widget from media source
    cout << "Disconnect video from source" << endl;
    AbstractMedia *nullSrc = NULL;
    this->ui->widget->SetSource(nullSrc);

    //Mark media interface as inactive
    this->mediaInterface->SetActive(0);

    //Signal worker threads to stop
    cout << "Signal worker threads to stop" << endl;
    std::tr1::shared_ptr<class Event> stopEvent(new Event("STOP_THREADS"));
    this->eventLoop->SendEvent(stopEvent);

    cout << "Stop timer" << endl;
    //Stop the timer and handle messages in this function
    this->timer->stop();

    //Wait for threads to stop
    cout << "Wait for threads to stop" << endl;
    for(int i=0;i<500;i++)
    {
        this->Update();
        if(this->threadCount == 0) break;
        usleep(10000); //microsec
    }

    //If threads still running, terminate them
    if(this->mediaThread->isRunning())
    {
        cout << "Warning: terminating media tread" << endl;
        this->mediaThread->terminate();
    }

    //Continue shut down in parent object
    cout << "Continue shut down in parent object" << endl;
    QMainWindow::closeEvent(event);
}

void MainWindow::ImportVideo()
{
    //Get filename from user
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Import Video"), "", tr("Video Files (*.avi *.mov *.mkv *.wmf, *.webm, *.flv, *.mp4, *.rm, *.asf)"));
    if(fileName.length() == 0) return;

    //Mark media interface as inactive
    this->mediaInterface->SetActive(0);

    //Shut down media thread and delete
    int result = this->mediaThread->StopThread();
    cout << "stop thread result=" << result << endl;
    delete(this->mediaThread);
    this->mediaThread = NULL;

    //Create a new source
    this->mediaThread = new AvBinThread(this->eventLoop);
    this->mediaThread->start();

    //Mark media interface as active
    this->mediaInterface->SetActive(1);

    //avbinNew->SetEventLoop(this->eventLoop);
    cout << "Opening " << fileName.toLocal8Bit().constData() << endl;
    this->mediaInterface->OpenFile(fileName.toLocal8Bit().constData());

    //Set widget to use this source
    this->ui->widget->SetSource(this->mediaInterface);
}

void MainWindow::Update()
{
    //Check for and handle events
    int flushing = 1;
    while(flushing)
    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
        assert(ev != NULL);
        cout << "Event type " << ev->type << endl;

        if(ev->type=="THREAD_STARTING")
        {
            this->threadCount ++;
        }
        if(ev->type=="THREAD_STOPPING")
        {
            assert(this->threadCount > 0);
            this->threadCount --;
        }
    }
    catch(std::runtime_error e) {flushing = 0;}
}
