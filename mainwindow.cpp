#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videowidget.h"
#include "mediabuffer.h"
#include "imagesequence.h"
#include "avbinmedia.h"
#include "eventloop.h"
#include "localsleep.h"
#include "scenecontroller.h"
#include <QtGui/QFileDialog>
#include <QtCore/QThread>
#include <QtGui/QDialogButtonBox>
#include <iostream>
#ifndef _MSC_VER
#include <unistd.h>
#endif
using namespace std;

//*********************************************

CheckDiscardDataDialog::CheckDiscardDataDialog(QWidget *parent, QString discardMsg) : QObject(parent)
{
    this->shutdownDialog = new QDialog(parent);
    QVBoxLayout topLayout(this->shutdownDialog);
    QDialogButtonBox buttonbox;
    QLabel question("This workspace has unsaved changes.");
    this->shutdownDialog->setLayout(&topLayout);
    topLayout.addWidget(&question);
    topLayout.addWidget(&buttonbox);
    QPushButton *buttonClose = new QPushButton(discardMsg);
    QPushButton *buttonCancel = new QPushButton("Cancel");
    QPushButton *buttonSaveAs = new QPushButton("Save as..");
    buttonbox.addButton(buttonClose, QDialogButtonBox::DestructiveRole);
    buttonbox.addButton(buttonCancel, QDialogButtonBox::RejectRole);
    buttonbox.addButton(buttonSaveAs, QDialogButtonBox::ActionRole);
    buttonSaveAs->setDefault(true);
    QObject::connect(buttonClose,SIGNAL(pressed()), this, SLOT(ShutdownWithoutSave()));
    QObject::connect(buttonCancel,SIGNAL(pressed()), this, SLOT(ShutdownCancel()));
    QObject::connect(buttonSaveAs,SIGNAL(pressed()), this, SLOT(ShutdownSaveAs()));

    //Run the dialog
    //The variable this->shutdownUserSelection is modified at this stage!
    this->shutdownUserSelection = "CANCEL";
    this->shutdownDialog->exec();
}

CheckDiscardDataDialog::~CheckDiscardDataDialog()
{
    this->shutdownDialog = NULL;
}

QString CheckDiscardDataDialog::GetUserChoice()
{
    return this->shutdownUserSelection;
}

void CheckDiscardDataDialog::ShutdownSaveAs()
{
    this->shutdownUserSelection = "SAVEAS";
    assert(this->shutdownDialog != NULL);
    this->shutdownDialog->close();
}

void CheckDiscardDataDialog::ShutdownWithoutSave()
{
    this->shutdownUserSelection = "NOSAVE";
    assert(this->shutdownDialog != NULL);
    this->shutdownDialog->close();
}

void CheckDiscardDataDialog::ShutdownCancel()
{
    this->shutdownUserSelection = "CANCEL";
    assert(this->shutdownDialog != NULL);
    this->shutdownDialog->close();
}


//********************************

SourcesList::SourcesList(QWidget * parent) : QListView(parent)
{

}

SourcesList::~SourcesList()
{

}

void SourcesList::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    this->UpdateSources(current);
}

//**************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->threadCount = 0;
    this->annotationMenu = NULL;

    //Create inter thread message system
    this->eventLoop = new class EventLoop();

    //Create event listener
    this->eventReceiver = new class EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("THREAD_STARTING",*eventReceiver);
    this->eventLoop->AddListener("THREAD_STOPPING",*eventReceiver);
    this->eventLoop->AddListener("AVBIN_OPEN_RESULT",*eventReceiver);

    //Create file reader worker thread
    this->mediaThread = new AvBinThread(this->eventLoop);
    this->mediaThread->start();

    this->mediaInterface = new class AvBinMedia();
    this->mediaInterface->SetEventLoop(this->eventLoop);
    this->mediaInterface->SetActive(1);

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec

    ui->setupUi(this);
    this->setWindowTitle("Video Cognition System");
    this->ui->widget->SetSource(this->mediaInterface);

    this->sourcesModel = new QStandardItemModel(4, 1);
    this->ui->dataSources->setModel(this->sourcesModel);
    this->RegenerateSourcesList();

    this->workspace.Load(tr("/home/tim/test.work"));
    this->workspaceAsStored = this->workspace;
    this->ui->dataSources->setSelectionMode(QListView::SelectionMode::ExtendedSelection);
    this->RegenerateSourcesList();
}

MainWindow::~MainWindow()
{
    this->workspace.Clear();

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

    delete this->sourcesModel;
    this->sourcesModel = NULL;

    delete ui;
}

QString MainWindow::CheckIfDataShouldBeDiscarded(QString discardMsg)
{
    //Check if the workspace has been saved, if not
    //prompt the user
    if(this->workspace != this->workspaceAsStored)
    {
        //Create a dialog to find what the user wants
        class CheckDiscardDataDialog dialog(this, discardMsg);

        //Process the users choice
        QString shutdownUserSelection = dialog.GetUserChoice();

        return shutdownUserSelection;
    }
    return "NO_CHANGE";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QString shutdownUserSelection = this->CheckIfDataShouldBeDiscarded("Close without saving");

    if(shutdownUserSelection == "CANCEL")
    {
        event->setAccepted(false);
        return;
    }

    if(shutdownUserSelection == "SAVEAS")
    {
        event->setAccepted(false);
        this->SaveAsWorkspace();
        return;
    }

    event->setAccepted(true);
    assert(this->threadCount >= 1);

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
        LocalSleep::usleep(10000); //microsec
    }

    //If threads still running, terminate them
    if(this->mediaThread->isRunning())
    {
        cout << "Warning: terminating media tread" << endl;
        this->mediaThread->terminate();
    }

    //Continu shut down in parent object
    cout << "Continuing shut down of QT framework" << endl;
    QMainWindow::closeEvent(event);
}

void MainWindow::RegenerateSourcesList()
{
    assert(this->sourcesModel);

    QIcon icon("icons/media-eject.png");
    this->sourcesModel->setColumnCount(1);
    this->sourcesModel->setRowCount(this->workspace.GetNumSources());
    for (int row = 0; row < this->workspace.GetNumSources(); ++row) {
        for (int column = 0; column < 1; ++column) {
            QString fina = this->workspace.GetSourceName(row);
            QFileInfo finaInfo(fina);

            QStandardItem *item = new QStandardItem(icon, finaInfo.fileName());
            this->sourcesModel->setItem(row, column, item);
        }
    }
}

void MainWindow::ImportVideo()
{
    //Get filename from user
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Import Video"), "", tr("Video Files (*.avi *.mov *.mkv *.wmf *.webm *.flv *.mp4 *.rm *.asf *.wmv)"));
    if(fileName.length() == 0) return;

    unsigned int sourceId = this->workspace.AddSource(fileName);


    this->RegenerateSourcesList();
}

void MainWindow::RemoveVideo()
{
    cout << "remove" << endl;
    QItemSelectionModel *sourceSelected = this->ui->dataSources->selectionModel();
    assert(sourceSelected!=NULL);

    QModelIndexList rowList = sourceSelected->selectedRows();
    for(unsigned int i=0;i<rowList.size();i++)
    {
        QModelIndex &ind = rowList[i];
        cout << ind.row() << endl;
        this->workspace.RemoveSource(ind.row());
    }

    this->RegenerateSourcesList();
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
        if(ev->type=="AVBIN_OPEN_RESULT")
        {
            cout << "Open result: " << ev->data << endl;
        }
    }
    catch(std::runtime_error e) {flushing = 0;}
}

void MainWindow::NewWorkspace()
{
    QString shutdownUserSelection = this->CheckIfDataShouldBeDiscarded("Continue without saving");

    if(shutdownUserSelection == "CANCEL")
    {
        return;
    }

    if(shutdownUserSelection == "SAVEAS")
    {
        this->SaveAsWorkspace();
        return;
    }

    this->workspace.Clear();
    this->workspaceAsStored = this->workspace;
    this->RegenerateSourcesList();
}

void MainWindow::LoadWorkspace()
{
    QString shutdownUserSelection = this->CheckIfDataShouldBeDiscarded("Continue without saving");

    if(shutdownUserSelection == "CANCEL")
    {
        return;
    }

    if(shutdownUserSelection == "SAVEAS")
    {
        this->SaveAsWorkspace();
        return;
    }

    //Get filename from user
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Load Workspace"), "", tr("Workspaces (*.work)"));
    if(fileName.length() == 0) return;

    this->workspace.Load(fileName);
    this->workspaceAsStored = this->workspace;
    this->RegenerateSourcesList();
}

void MainWindow::SaveWorkspace()
{
    int ret = this->workspace.Save();
    if(ret == 0) this->SaveAsWorkspace();
    else this->workspaceAsStored = this->workspace;
}

void MainWindow::SaveAsWorkspace()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Workspace"), "", tr("Workspaces (*.work)"));
    if(fileName.length() == 0) return;

    this->workspace.SaveAs(fileName);
    this->workspaceAsStored = this->workspace;
}

void MainWindow::SelectedSourceChanged(const QModelIndex ind)
{
    unsigned int selectedRow = ind.row();
    if(selectedRow < 0 && selectedRow >= this->workspace.GetNumSources())
        return;

    QString fina = this->workspace.GetSourceName(selectedRow);

    if(this->annotationMenu)
    {
        this->menuBar()->removeAction(this->annotationMenu->menuAction());
    }

    //Pause video
    this->ui->widget->Pause();

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

    cout << "Opening " << fina.toLocal8Bit().constData() << endl;
    this->mediaInterface->OpenFile(fina.toLocal8Bit().constData());

    //Update scene controller
    SimpleSceneController *scene = this->workspace.GetTrack(selectedRow);
    this->ui->widget->SetSceneControl(scene);

    this->annotationMenu = scene->MenuFactory(this->menuBar());

    //Set widget to use this source
    this->ui->widget->SetSource(this->mediaInterface);

}

void MainWindow::TrainModelPressed()
{
    cout << "TrainModelPressed" << endl;
    QItemSelectionModel *selection = this->ui->dataSources->selectionModel();

}

void MainWindow::ApplyModelPressed()
{
    cout << "ApplyModelPressed" << endl;
    QItemSelectionModel *selection = this->ui->dataSources->selectionModel();

}
