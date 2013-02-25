#include <assert.h>
#include "mainwindow.h"
#include "sourcealggui.h"
#include "ui_sourcealggui.h"
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

WaitPopUpDialog::WaitPopUpDialog(QWidget *parent)
{
    this->workerTaskDone = 0;
    this->resultCode = 0;
    this->count = 0;

    this->dialog = new QDialog(parent);
    QVBoxLayout topLayout(this->dialog);
    QLabel txt("Waiting for complete");
    this->dialog->setLayout(&topLayout);
    topLayout.addWidget(&txt);

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec
}

WaitPopUpDialog::~WaitPopUpDialog()
{

}

void WaitPopUpDialog::Exec()
{
    //Run the dialog
    this->dialog->exec();
}

void WaitPopUpDialog::WorkerTaskDone(int resultCode)
{
    this->lock.lock();
    this->workerTaskDone = 1;
    this->resultCode = resultCode;
    this->lock.unlock();
}

void WaitPopUpDialog::Update()
{
    this->lock.lock();
    int done = this->workerTaskDone;
    this->lock.unlock();

    this->count += 1;
    //cout << "test " << count << endl;

    if(done) this->dialog->close();
}

int WaitPopUpDialog::GetResultCode()
{
    this->lock.lock();
    int out = this->resultCode;
    this->lock.unlock();
    return out;
}

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

//*************************************************

StopProcessingDialog::StopProcessingDialog(QWidget *parent) : QObject(parent)
{
    this->dialog = new QDialog(parent);
    QVBoxLayout topLayout(this->dialog);
    QDialogButtonBox buttonbox;
    QLabel question("Processing is currently running and should be stopped first.");
    this->dialog->setLayout(&topLayout);
    topLayout.addWidget(&question);
    topLayout.addWidget(&buttonbox);
    QPushButton *buttonStop = new QPushButton("Stop Processing");
    QPushButton *buttonCancel = new QPushButton("Cancel");
    buttonbox.addButton(buttonCancel, QDialogButtonBox::RejectRole);
    buttonbox.addButton(buttonStop, QDialogButtonBox::ActionRole);
    buttonCancel->setDefault(true);
    QObject::connect(buttonStop,SIGNAL(pressed()), this, SLOT(AnswerStop()));
    QObject::connect(buttonCancel,SIGNAL(pressed()), this, SLOT(AnswerCancel()));

    //Run the dialog
    //The variable this->shutdownUserSelection is modified at this stage!
    this->userSelection = "CANCEL";
    this->dialog->exec();
}

StopProcessingDialog::~StopProcessingDialog()
{
    this->dialog = NULL;
}

QString StopProcessingDialog::GetUserChoice()
{
    return this->userSelection;
}

void StopProcessingDialog::AnswerStop()
{
    this->userSelection = "STOP";
    assert(this->dialog != NULL);
    this->dialog->close();
}

void StopProcessingDialog::AnswerCancel()
{
    this->userSelection = "CANCEL";
    assert(this->dialog != NULL);
    this->dialog->close();
}

//********************************

ClickableQTreeView::ClickableQTreeView(QWidget * parent) : QTreeView(parent)
{

}

ClickableQTreeView::~ClickableQTreeView()
{

}

void ClickableQTreeView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
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
    this->errMsg = NULL;
    this->avbinVerChecked = 0;

    //Set the window icon
    QIcon windowIcon("icons/Kinatomic-Icon50.png");
    if(windowIcon.isNull()) cout << "Warning: Window icon not found" << endl;
    this->setWindowIcon(windowIcon);

    //Create inter thread message system
    this->eventLoop = new class EventLoop();
    this->workspace.SetEventLoop(*this->eventLoop);

    //Create event listener
    this->eventReceiver = new class EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("THREAD_STARTING",*eventReceiver);
    this->eventLoop->AddListener("THREAD_STOPPING",*eventReceiver);
    this->eventLoop->AddListener("AVBIN_OPEN_RESULT",*eventReceiver);
    this->eventLoop->AddListener("THREAD_PROGRESS_UPDATE",*eventReceiver);
    this->eventLoop->AddListener("THREAD_STATUS_CHANGED",*eventReceiver);
    this->eventLoop->AddListener("ALG_DATA_BLOCK",*eventReceiver);
    this->eventLoop->AddListener("ANNOTATION_THREAD_PROGRESS",*eventReceiver);
    this->eventLoop->AddListener("AVBIN_VERSION",*eventReceiver);
    this->eventLoop->AddListener("SOURCE_FILENAME",*eventReceiver);

    //Create file reader worker thread
    this->mediaInterfaceFront = new class AvBinMedia(0, this->eventLoop);
    this->mediaInterfaceBack = new class AvBinMedia(1, this->eventLoop);

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec

    ui->setupUi(this);
    this->setWindowTitle("Kinatomic Sense Scanner");
    this->ui->widget->SetSource(this->mediaInterfaceFront,"");

    QStringList horLabelsAnn;
    horLabelsAnn.push_back("Sources");
    horLabelsAnn.push_back("Status");
    this->sourcesModel.setHorizontalHeaderLabels(horLabelsAnn);
    this->ui->sourcesAlgGui->ui->dataSources->setModel(&this->sourcesModel);
    this->RegenerateSourcesList();

    QStringList horLabels;
    horLabels.push_back("Models");
    horLabels.push_back("Status");
    this->processingModel.setHorizontalHeaderLabels(horLabels);
    this->ui->sourcesAlgGui->ui->processingView->setModel(&this->processingModel);
    this->RegenerateProcessingList();

    //this->workspace.Load(tr("/home/tim/test.work"), this->mediaInterfaceBack);
    this->workspaceAsStored = this->workspace;
    this->ui->sourcesAlgGui->ui->dataSources->setSelectionMode(QListView::SelectionMode::ExtendedSelection);
    this->RegenerateSourcesList();

    //Set visibility to show about box
    //this->ui->workspaceLayout->hide();
    //this->ui->webViewLayout->hide();
    this->ui->sourcesAlgGui->mainWindow = this;

}

MainWindow::~MainWindow()
{
    this->workspace.ClearAnnotation();
    this->workspace.ClearProcessing();

    delete this->timer;
    this->timer = NULL;

    delete this->eventReceiver;
    this->eventReceiver = NULL;

    if(this->errMsg) delete this->errMsg;
    this->errMsg = NULL;

    delete this->mediaInterfaceFront;
    this->mediaInterfaceFront = NULL;

    delete this->mediaInterfaceBack;
    this->mediaInterfaceBack = NULL;

	delete this->eventLoop;
    this->eventLoop = NULL;

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
    //Prevent shutdown if processes running
    int numRunningBlockingShutdown = this->workspace.NumProcessesBlockingShutdown();
    if(numRunningBlockingShutdown>0)
    {
        cout << "Cannot shut down while running processing" << endl;

        class StopProcessingDialog dialog(this);
        QString userSelection = dialog.GetUserChoice();

        if(userSelection=="STOP")
        {
            QList<QUuid> uuids = this->workspace.GetProcessingUuids();
            for(unsigned int i=0;i<uuids.size();i++)
            {
                std::tr1::shared_ptr<class Event> pauseEvent(new Event("PAUSE_ALGORITHM"));
                pauseEvent->toUuid = uuids[i];
                this->eventLoop->SendEvent(pauseEvent);
            }
        }
        event->setAccepted(false);
        return;
    }

    //Ask user if they want to save changes, if changes are present
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
    this->ui->widget->SetSource(nullSrc,"");

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
        LocalSleep::msleep(10); //millisec
    }

    //If threads still running, terminate them
    this->mediaInterfaceFront->TerminateThread();
    this->mediaInterfaceBack->TerminateThread();

    this->workspace.TerminateThreads();


    //Continu shut down in parent object
    cout << "Continuing shut down of QT framework" << endl;
    QMainWindow::closeEvent(event);
}

void MainWindow::RegenerateSourcesList()
{
    //Update GUI list
    QIcon icon("icons/media-eject.png");
    if(this->sourcesModel.columnCount()!= 2)
        this->sourcesModel.setColumnCount(2);
    QList<QUuid> annotationUuids = this->workspace.GetAnnotationUuids();

    if(this->sourcesModel.rowCount() != annotationUuids.size())
        this->sourcesModel.setRowCount(annotationUuids.size());
    for (int row = 0; row < annotationUuids.size(); ++row)
    {
        for (int column = 0; column < 1; ++column)
        {
            std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
            getSourceNameEv->toUuid = annotationUuids[row];
            getSourceNameEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getSourceNameEv);

            std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
            QString fina = sourceName->data.c_str();
            QFileInfo finaInfo(fina);

            QStandardItem *item = this->sourcesModel.item(row, column);
            if(item!=NULL)
                continue;

            item = new QStandardItem(icon, finaInfo.fileName());
            this->sourcesModel.setItem(row, column, item);
        }

        for (int column = 1; column < 2; ++column)
        {
            std::ostringstream displayLine;
            //float progress = this->workspace.GetProgress(row);
            QUuid annotId = annotationUuids[row];
            //cout << annotId.toString().toLocal8Bit().constData() << endl;

            std::map<QUuid, float>::iterator it = this->annotProgress.find(annotId);
            if(it != this->annotProgress.end())
            {
                displayLine << it->second;
            }
            else
            {
                displayLine << "Unknown";
            }

            QStandardItem *item = this->sourcesModel.item(row, column);
            if(item!=NULL)
            {
                item->setText(displayLine.str().c_str());
                continue;
            }
            else
            {
                item = new QStandardItem(displayLine.str().c_str());
                this->sourcesModel.setItem(row, column, item);
            }
        }
    }
}

void MainWindow::RegenerateProcessingList()
{
    QItemSelectionModel *sourceSelected = this->ui->sourcesAlgGui->ui->dataSources->selectionModel();

    QIcon icon("icons/media-eject.png");
    if(this->processingModel.columnCount()!= 2)
        this->processingModel.setColumnCount(2);
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();
    if(this->processingModel.rowCount() != algUuids.size())
        this->processingModel.setRowCount(algUuids.size());
    for (int row = 0; row < algUuids.size(); ++row)
    {
        for (int column = 0; column < 1; ++column)
        {
            QStandardItem *item = this->processingModel.item(row, column);
            if(item!=NULL)
            {
                continue;
            }

            QString fina = "Relative Tracker";
            QFileInfo finaInfo(fina);
            std::ostringstream displayLine;
            displayLine << finaInfo.fileName().toLocal8Bit().constData();

            QString displayLineQString;
            displayLineQString = displayLine.str().c_str();
            item = new QStandardItem(icon, displayLineQString);
            this->processingModel.setItem(row, column, item);
        }

        for (int column = 1; column < 2; ++column)
        {
            std::ostringstream displayLine;
            float progress = this->workspace.GetProcessingProgress(algUuids[row]);
            if(this->workspace.GetProcessingState(algUuids[row])!=AlgorithmProcess::STOPPED)
            {
                AlgorithmProcess::ProcessState state = this->workspace.GetProcessingState(algUuids[row]);
                if(state == AlgorithmProcess::STARTING) displayLine << "Starting " << progress;
                if(state == AlgorithmProcess::RUNNING) displayLine << "Running " << progress;
                if(state == AlgorithmProcess::RUNNING_PAUSING) displayLine << "Pausing... " << progress;
                if(state == AlgorithmProcess::RUNNING_STOPPING) displayLine << "Stopping... " << progress;
                if(state == AlgorithmProcess::PAUSED)
                {
                    if(progress<1.f) displayLine << "Paused " << progress;
                    else displayLine << "Done " << progress;
                }
            }
            else
            {
                if(progress < 1.f)
                {
                    displayLine << "Stopped " <<progress;
                }
                else
                    displayLine << "Done";
            }

            QStandardItem *item = this->processingModel.item(row, column);
            if(item!=NULL)
            {
                item->setText(displayLine.str().c_str());
                continue;
            }
            else
            {
                item = new QStandardItem(displayLine.str().c_str());
                this->processingModel.setItem(row, column, item);
            }
        }

    }

}

void MainWindow::ImportVideo()
{
    //Get filename from user
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Import Video"), "", tr("Video Files (*.avi *.mov *.mkv *.wmf *.webm *.flv *.mp4 *.rm *.asf *.wmv)"));
    if(fileName.length() == 0) return;

    QUuid uid = QUuid::createUuid();
    unsigned int sourceId = this->workspace.AddSource(fileName, uid.toString(), this->mediaInterfaceBack);

    this->RegenerateSourcesList();
}

void MainWindow::RemoveVideo()
{
    cout << "remove" << endl;
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();
    QItemSelectionModel *sourceSelected = this->ui->sourcesAlgGui->ui->dataSources->selectionModel();
    assert(sourceSelected!=NULL);

    //Disable this source UI controls
    this->DeselectCurrentSource();

    QModelIndexList rowList = sourceSelected->selectedRows();
    for(unsigned int i=0;i<rowList.size();i++)
    {
        QModelIndex &ind = rowList[i];
        //cout << ind.row() << endl;
        this->workspace.RemoveSource(algUuids[ind.row()]);
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
            this->RegenerateProcessingList();
        }
        if(ev->type=="THREAD_STOPPING")
        {
            assert(this->threadCount > 0);
            this->threadCount --;
            this->RegenerateProcessingList();
        }
        if(ev->type=="AVBIN_OPEN_RESULT")
        {
            cout << "Open result: " << ev->data << endl;
        }
        if(ev->type=="THREAD_PROGRESS_UPDATE")
        {
            std::vector<std::string> args = split(ev->data.c_str(),',');
            this->workspace.ProcessingProgressChanged(QUuid(args[1].c_str()), atof(args[0].c_str()));
            this->RegenerateProcessingList();
        }
        if(ev->type=="THREAD_STATUS_CHANGED")
        {
            this->RegenerateProcessingList();
        }

        if(ev->type=="ANNOTATION_THREAD_PROGRESS")
        {
            assert(ev->data.length()>=40);
            QUuid annId(ev->data.substr(0, 38).c_str());
            QString progStr(ev->data.substr(39).c_str());
            this->annotProgress[annId] = progStr.toFloat();
            this->RegenerateSourcesList();
        }
        if(ev->type=="AVBIN_VERSION")
        {
            cout << ev->type <<" "<< ev->data<< endl;
            //The software does not function properly for versions before 11
            //so display a warning if an old avbin is detected
            if(atoi(ev->data.c_str())<11 && !this->avbinVerChecked)
            {
                if(this->errMsg == NULL)
                    this->errMsg = new QMessageBox(this);
                this->errMsg->setWindowTitle("Warning: Old Avbin version detected");
                QString errTxt = QString("You have Avbin version %1 but at least version %2 is recommended")
                    .arg(ev->data.c_str()).arg(11);
                this->errMsg->setText(errTxt);
                this->errMsg->exec();
            }
            this->avbinVerChecked = 1;
        }
    }
    catch(std::runtime_error e) {flushing = 0;}

    this->workspace.Update();
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

    this->workspace.ClearAnnotation();
    this->workspace.ClearProcessing();
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

    this->Load(fileName, this->mediaInterfaceBack);
    this->workspaceAsStored = this->workspace;
    this->RegenerateSourcesList();
}

void MainWindow::SaveWorkspace()
{
    WaitPopUpDialog *waitDlg = new WaitPopUpDialog(this);
    this->Save();

    waitDlg->Exec();
    int ret = waitDlg->GetResultCode();
    delete waitDlg;
    waitDlg = NULL;

    if(ret == 0) this->SaveAsWorkspace();
    else this->workspaceAsStored = this->workspace;

}

void MainWindow::SaveAsWorkspace()
{
    //Get output filename from user
    QString fileName = QFileDialog::getSaveFileName(0,
      tr("Save Workspace"), "", tr("Workspaces (*.work)"));
    if(fileName.length() == 0) return;

    WaitPopUpDialog *waitDlg = new WaitPopUpDialog(this);

    this->SaveAs(fileName);

    waitDlg->Exec();
    delete waitDlg;
    waitDlg = NULL;

    this->workspaceAsStored = this->workspace;
}

void MainWindow::SelectedSourceChanged(const QModelIndex ind)
{
    int selectedRow = ind.row();
    this->SelectedSourceChanged(selectedRow);
}

void MainWindow::SelectedSourceChanged(int selectedRow)
{
    QList<QUuid> annotationUuids = this->workspace.GetAnnotationUuids();
    if(selectedRow < 0 && selectedRow >= annotationUuids.size())
        return;

    //Get video file name from source
    std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
    getSourceNameEv->toUuid = annotationUuids[selectedRow];
    getSourceNameEv->id = this->eventLoop->GetId();
    this->eventLoop->SendEvent(getSourceNameEv);

    std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
    QString fina = sourceName->data.c_str();

    this->DeselectCurrentSource();

    //Update scene controller
    TrackingSceneController *sceneController = new TrackingSceneController(this);
    sceneController->SetEventLoop(this->eventLoop);
    this->ui->widget->SetSceneControl(sceneController);
    this->ui->widget->SetAnnotationTrack(annotationUuids[selectedRow]);
    //Update window menus
    assert(sceneController!=NULL);
    this->annotationMenu = sceneController->MenuFactory(this->menuBar());

    //Set widget to use this source
    this->ui->widget->SetSource(this->mediaInterfaceFront, fina);

}

void MainWindow::DeselectCurrentSource()
{
    if(this->annotationMenu)
    {
        this->menuBar()->removeAction(this->annotationMenu->menuAction());
    }

    //Pause video
    this->ui->widget->Pause();
    this->ui->widget->SetSceneControl(NULL);

}

void MainWindow::TrainModelPressed()
{
    cout << "TrainModelPressed" << endl;
    QItemSelectionModel *selection = this->ui->sourcesAlgGui->ui->dataSources->selectionModel();
    QList<QUuid> annotationUuids = this->workspace.GetAnnotationUuids();

    //Count frames, because at least one is needed to train
    int countMarkedFrames = 0;
    QModelIndexList selectList = selection->selectedRows(0);
    QList<std::vector<std::string> > seqMarked;
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];
        //For each annotated frame
        std::tr1::shared_ptr<class Event> getMarkedEv(new Event("GET_MARKED_LIST"));
        getMarkedEv->toUuid = annotationUuids[ind.row()];
        getMarkedEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(getMarkedEv);

        std::tr1::shared_ptr<class Event> markedEv = this->eventReceiver->WaitForEventId(getMarkedEv->id);
        std::vector<std::string> splitMarked = split(markedEv->data.c_str(),',');
        seqMarked.push_back(splitMarked);
        countMarkedFrames += splitMarked.size();
    }
    if(countMarkedFrames==0)
    {
        if(this->errMsg == NULL)
            this->errMsg = new QMessageBox(this);
        this->errMsg->setWindowTitle("Error: No training data");
        this->errMsg->setText("Annotate at least one frame before trying to train a model.");
        this->errMsg->exec();
        return;
    }

    //Suspend access from annot threads to media interface (for now)
    this->workspace.SetAnnotThreadsInactive();

    //Create worker process
    std::tr1::shared_ptr<class AlgorithmProcess> alg(new class AlgorithmProcess(this->eventLoop, this));
    QUuid newUuid = QUuid::createUuid();
    alg->SetUid(newUuid);

    //Add process to workspace
    this->workspace.AddProcessing(alg);

    //Configure worker process
    selectList = selection->selectedRows(0);
    QList<QSharedPointer<QImage> > imgs;
    for(unsigned int i=0;i<selectList.size();i++)
    {
        //Get filename from annotation source
        QModelIndex &ind = selectList[i];
        std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
        getSourceNameEv->toUuid = annotationUuids[ind.row()];
        getSourceNameEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(getSourceNameEv);

        std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
        QString fina = sourceName->data.c_str();
        std::vector<std::string> marked = seqMarked[i];
        //For each annotated frame

        for(unsigned int fr=0;fr<marked.size();fr++)
        {
            countMarkedFrames ++;

            //Get image data and send to process
            cout << marked[fr] << endl;
            unsigned long long startTimestamp = 0, endTimestamp = 0;
            unsigned long long annotTimestamp = atoi(marked[fr].c_str()); //TODO make this use long long properly
            QSharedPointer<QImage> img;
            try
            {
                img = this->mediaInterfaceBack->Get(fina,
                        annotTimestamp, startTimestamp, endTimestamp);
            }
            catch (std::runtime_error &err)
            {
                cout << "Timeout getting frame " << annotTimestamp << endl;
                continue;
            }

            if(annotTimestamp * 1000 < startTimestamp || annotTimestamp * 1000 > endTimestamp)
            {
                cout << "Warning: found a frame but it does not cover requested time" << endl;
                cout << "Requested: " << annotTimestamp << endl;
                cout << "Found: " << startTimestamp << " to " << endTimestamp << endl;
                continue;
            }

            imgs.append(img);

            int len = img->byteCount();

            //Send image to algorithm module
            assert(img->format() == QImage::Format_RGB888);
            std::tr1::shared_ptr<class Event> foundImgEv(new Event("TRAINING_IMG_FOUND"));
            QString imgPreamble2 = QString("RGB_IMAGE_DATA TIMESTAMP=%1 HEIGHT=%2 WIDTH=%3\n").
                                arg(annotTimestamp).
                                arg(img->height()).
                                arg(img->width());
            foundImgEv->data = imgPreamble2.toLocal8Bit().constData();
            class BinaryData *imgRaw = new BinaryData();
            imgRaw->Copy((const unsigned char *)img->bits(), len);
            foundImgEv->raw = imgRaw;
            foundImgEv->toUuid = newUuid;
            this->eventLoop->SendEvent(foundImgEv);

            //Get annotation data and sent it to the algorithm
            std::tr1::shared_ptr<class Event> getAnnotEv(new Event("GET_ANNOTATION_XML"));
            getAnnotEv->toUuid = annotationUuids[ind.row()];
            getAnnotEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAnnotEv);

            std::tr1::shared_ptr<class Event> annotXmlRet = this->eventReceiver->WaitForEventId(getAnnotEv->id);

            QString annotXml;
            QTextStream test(&annotXml);
            test << annotXmlRet->data.c_str();
            assert(annotXml.mid(annotXml.length()-1).toLocal8Bit().constData()[0]=='\n');

            std::tr1::shared_ptr<class Event> foundPosEv(new Event("TRAINING_POS_FOUND"));
            foundPosEv->data = annotXml.toUtf8().constData();
            foundPosEv->toUuid = newUuid;
            this->eventLoop->SendEvent(foundPosEv);
        }
    }

    std::tr1::shared_ptr<class Event> trainingFinishEv(new Event("TRAINING_DATA_FINISH"));
    trainingFinishEv->toUuid = newUuid;
    this->eventLoop->SendEvent(trainingFinishEv);

    //Update GUI to reflect changed workspace
    this->RegenerateProcessingList();
}

void MainWindow::ApplyModelPressed()
{
    cout << "ApplyModelPressed" << endl;
    QItemSelectionModel *modelSelection = this->ui->sourcesAlgGui->ui->processingView->selectionModel();
    QItemSelectionModel *srcSelection = this->ui->sourcesAlgGui->ui->dataSources->selectionModel();
    assert(modelSelection!=NULL);
    assert(srcSelection!=NULL);
    QModelIndexList modelSelList = modelSelection->selectedRows(0);
    QModelIndexList srcSelList = srcSelection->selectedRows(0);
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();
    QList<QUuid> annotationUuids = this->workspace.GetAnnotationUuids();

    for(unsigned int i=0;i<srcSelList.size();i++)
    {
        QModelIndex &ind = srcSelList[i];
        cout << "src "<< ind.row() << "," << ind.column() << endl;

        //Load appropriate video
        //QString fina = this->workspace.GetSourceName(ind.row());
        QUuid uid = annotationUuids[ind.row()];
        //ChangeVidSource(&this->mediaThreadBack,this->mediaInterfaceBack,this->eventLoop,fina);

        //Apply models to selected video
        for(unsigned int i=0;i<modelSelList.size();i++)
        {
            QModelIndex &mind = modelSelList[i];
            cout << "model "<< mind.row() << "," << mind.column() << endl;
            this->workspace.AddAutoAnnot(uid.toString(), algUuids[i], this->mediaInterfaceBack);
        }
    }
    this->RegenerateSourcesList();

}

void MainWindow::PauseProcessPressed()
{
    cout << "MainWindow::PauseProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->sourcesAlgGui->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];

        std::tr1::shared_ptr<class Event> pauseEvent(new Event("PAUSE_ALGORITHM"));
        pauseEvent->toUuid = algUuids[ind.row()];
        this->eventLoop->SendEvent(pauseEvent);
    }
    this->RegenerateProcessingList();
}

void MainWindow::RunProcessPressed()
{
    cout << "MainWindow::RunProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->sourcesAlgGui->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];

        std::tr1::shared_ptr<class Event> pauseEvent(new Event("RUN_ALGORITHM"));
        pauseEvent->toUuid = algUuids[ind.row()];
        this->eventLoop->SendEvent(pauseEvent);

    }
    this->RegenerateProcessingList();
}

void MainWindow::RemoveProcessPressed()
{
    cout << "MainWindow::RemoveProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->sourcesAlgGui->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    QList<QUuid> algUuids = this->workspace.GetProcessingUuids();

    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];
        AlgorithmProcess::ProcessState state = this->workspace.GetProcessingState(algUuids[ind.row()]);

        //Check if alg state is ready for removal
        if(state!= AlgorithmProcess::PAUSED && state!=AlgorithmProcess::STOPPED)
        {
            if(this->errMsg == NULL)
                this->errMsg = new QMessageBox(this);
            this->errMsg->setWindowTitle("Error: Process is still running");
            this->errMsg->setText("Stop the process before trying to remove it.");
            this->errMsg->exec();

            return;
        }

        int ret = this->workspace.RemoveProcessing(algUuids[ind.row()]);

    }
    this->RegenerateProcessingList();
}

void MainWindow::AboutPressed()
{
    this->ui->aboutDock->show();
}

void MainWindow::ShowVideoPressed()
{
    this->ui->videoDock->show();
}

//**********************************


int MainWindow::Save()
{
    int finaLen = this->defaultFilename.length();
    if(finaLen==0)
    {
        return 0;
    }

    QString tmpFina = this->defaultFilename;
    tmpFina.append(".tmp");
    QFileInfo pathInfo(this->defaultFilename);
    QDir dir(pathInfo.absoluteDir());

    //Save data to file
    QFile f(tmpFina);
    f.open( QIODevice::WriteOnly );
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<workspace>" << endl;
    out << "<sources>" << endl;
    QList<QUuid> annotationUuids = this->workspace.GetAnnotationUuids();
    for(unsigned int i=0;i<annotationUuids.size();i++)
    {
        try
        {
            //Get source filename for annotation
            std::tr1::shared_ptr<class Event> getSourceNameEv(new Event("GET_SOURCE_FILENAME"));
            getSourceNameEv->toUuid = annotationUuids[i];
            getSourceNameEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getSourceNameEv);

            std::tr1::shared_ptr<class Event> sourceName = this->eventReceiver->WaitForEventId(getSourceNameEv->id);
            QString fina = sourceName->data.c_str();

            //Get algorithm Uuid for this annotation track
            std::tr1::shared_ptr<class Event> getAlgUuidEv(new Event("GET_ALG_UUID"));
            getAlgUuidEv->toUuid = annotationUuids[i];
            getAlgUuidEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAlgUuidEv);

            std::tr1::shared_ptr<class Event> algUuidEv = this->eventReceiver->WaitForEventId(getAlgUuidEv->id);
            QUuid algUuid(algUuidEv->data.c_str());

            //Get annotation data
            std::tr1::shared_ptr<class Event> getAnnotEv(new Event("GET_ALL_ANNOTATION_XML"));
            getAnnotEv->toUuid = annotationUuids[i];
            getAnnotEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getAnnotEv);

            std::tr1::shared_ptr<class Event> annotXmlRet = this->eventReceiver->WaitForEventId(getAnnotEv->id);

            //Format as XML
            out << "\t<source id=\""<<i<<"\" uid=\""<<Qt::escape(annotationUuids[i].toString())<<"\" file=\""<<
                   Qt::escape(dir.relativeFilePath(fina))<<"\"";

            if(!algUuid.isNull())
                out << " alg=\"" << algUuid.toString().toLocal8Bit().constData() << "\"";;

            out << ">" << endl;
            out << annotXmlRet->data.c_str();
            out << "\t</source>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
    }


    out << "</sources>" << endl;
    out << "<models>" << endl;
    QList<QUuid> processingUuids = this->workspace.GetProcessingUuids();
    for(unsigned int i=0;i<processingUuids.size();i++)
    {
        try
        {
            //Get model
            std::tr1::shared_ptr<class Event> getModelEv(new Event("GET_MODEL"));
            getModelEv->toUuid = processingUuids[i];
            getModelEv->id = this->eventLoop->GetId();
            this->eventLoop->SendEvent(getModelEv);

            std::tr1::shared_ptr<class Event> ev = this->eventReceiver->WaitForEventId(getModelEv->id);
            class BinaryData *modelBin = (class BinaryData *)ev->raw;
            QByteArray modelArr((const char *)modelBin->raw, modelBin->size);

            //Encode as XML binary blob
            QByteArray modelBase64 = modelArr.toBase64();
            out << "<model uid=\""<< processingUuids[i] <<"\">" << endl;
            for(unsigned int pos=0;pos<modelBase64.length();pos+=512)
            {
                out << modelBase64.mid(pos, 512) << endl;
            }
            out << "</model>" << endl;
        }
        catch(std::runtime_error err)
        {
            cout << err.what() << endl;
        }
    }
    out << "</models>" << endl;
    out << "</workspace>" << endl;
    f.close();

    //Rename temporary file to final name
    QFile targetFina(this->defaultFilename);
    if(targetFina.exists())
        QFile::remove(this->defaultFilename);
    QFile::rename(tmpFina, this->defaultFilename);

    return 1;
}

void MainWindow::SaveAs(QString &fina)
{
    this->defaultFilename = fina;
    this->Save();
}


void MainWindow::Load(QString fina, class AvBinMedia* mediaInterface)
{
    this->workspace.ClearAnnotation();
    this->workspace.ClearProcessing();
    this->defaultFilename = fina;

    //Parse XML to DOM
    QFile f(fina);
    QDomDocument doc("mydocument");
    QString errorMsg;
    if (!doc.setContent(&f, &errorMsg))
    {
        cout << "Xml Error: "<< errorMsg.toLocal8Bit().constData() << endl;
        f.close();
        return;
    }
    f.close();

    //Get dir of data file
    QFileInfo pathInfo(fina);
    QDir dir(pathInfo.absoluteDir());

    //Load points and links into memory
    QDomElement rootElem = doc.documentElement();
    QDomNode n = rootElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {

            if(e.tagName() == "sources")
            {
                QDomNode sourceNode = e.firstChild();
                while(!sourceNode.isNull())
                {
                    QDomElement sourceEle = sourceNode.toElement(); // try to convert the node to an element.
                    if(sourceEle.tagName() != "source") {sourceNode = sourceNode.nextSibling(); continue;}

                    QString sourceFiNa = sourceEle.attribute("file");
                    QString sourceFiNaAbs = dir.absoluteFilePath(sourceFiNa);
                    QFileInfo fileInfo(sourceFiNaAbs);
                    std::tr1::shared_ptr<class Annotation> ann(new class Annotation);
                    ann->SetSource(fileInfo.absoluteFilePath());

                    //Set source UID
                    QString uidStr = sourceEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();

                    //Set alg Uid
                    QString algStr = sourceEle.attribute("alg");
                    QUuid alg(algStr);
                    ann->SetAlgUid(alg);

                    //Start annot worker thread
                    std::tr1::shared_ptr<class AnnotThread> annotThread(new class AnnotThread(&*ann, mediaInterface));
                    annotThread->SetEventLoop(this->eventLoop);
                    annotThread->Start();
                    ann->annotThread = annotThread;

                    TrackingAnnotationData *track =
                            new TrackingAnnotationData();

                    QDomNode trackData = sourceNode.firstChild();
                    while(!trackData.isNull())
                    {
                        QDomElement et = trackData.toElement(); // try to convert the node to an element.
                        if(et.isNull()) continue;
                        if(et.tagName() != "tracking") {trackData = trackData.nextSibling(); continue;}

                        track->ReadAnnotationXml(et);

                        trackData = trackData.nextSibling();

                    }

                    ann->SetTrack(track);
                    this->workspace.AddSource(ann, mediaInterface, uid);
                    sourceNode = sourceNode.nextSibling();
                }

            }

            if(e.tagName() == "models")
            {
                QDomNode modelNode = e.firstChild();
                while(!modelNode.isNull())
                {
                    QDomElement modelEle = modelNode.toElement(); // try to convert the node to an element.
                    if(modelEle.tagName() != "model") {modelNode = modelNode.nextSibling(); continue;}

                    QByteArray modelData = QByteArray::fromBase64(modelEle.text().toLocal8Bit().constData());
                    std::tr1::shared_ptr<class AlgorithmProcess> alg(
                                new class AlgorithmProcess(this->eventLoop, this));
                    alg->Init();

                    QString uidStr = modelEle.attribute("uid");
                    QUuid uid(uidStr);
                    if(uid.isNull()) uid = uid.createUuid();
                    alg->SetUid(uid);
                    this->workspace.AddProcessing(alg);

                    //Send data to algorithm process
                    std::tr1::shared_ptr<class Event> foundModelEv(new Event("ALG_MODEL_FOUND"));
                    QString imgPreamble2 = QString("MODEL\n");
                    foundModelEv->data = imgPreamble2.toLocal8Bit().constData();
                    class BinaryData *modelRaw = new BinaryData();
                    modelRaw->Copy((const unsigned char *)modelData.constData(), modelData.size());
                    foundModelEv->raw = modelRaw;
                    foundModelEv->toUuid = uid;
                    this->eventLoop->SendEvent(foundModelEv);

                    //Continue to train if needed
                    std::tr1::shared_ptr<class Event> trainingFinishEv(new Event("TRAINING_DATA_FINISH"));
                    trainingFinishEv->toUuid = uid;
                    this->eventLoop->SendEvent(trainingFinishEv);

                    //Ask process to provide progress update
                    std::tr1::shared_ptr<class Event> getProgressEv(new Event("GET_PROGRESS"));
                    getProgressEv->toUuid = uid;
                    this->eventLoop->SendEvent(getProgressEv);

                    modelNode = modelNode.nextSibling();
                }

            }
        }
        n = n.nextSibling();
    }
}
