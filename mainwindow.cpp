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

    //Set the window icon
    QIcon windowIcon("icons/Charm.png");
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

    //Create file reader worker thread
    this->mediaInterfaceFront = new class AvBinMedia(0, this->eventLoop);
    this->mediaInterfaceBack = new class AvBinMedia(1, this->eventLoop);

    //Start event buffer timer
    this->timer = new QTimer();
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(Update()));
    this->timer->start(10); //in millisec

    ui->setupUi(this);
    this->setWindowTitle("Video Cognition System");
    this->ui->widget->SetSource(this->mediaInterfaceFront,"");

    QStringList horLabelsAnn;
    horLabelsAnn.push_back("Sources");
    horLabelsAnn.push_back("Status");
    this->sourcesModel.setHorizontalHeaderLabels(horLabelsAnn);
    this->ui->dataSources->setModel(&this->sourcesModel);
    this->RegenerateSourcesList();

    QStringList horLabels;
    horLabels.push_back("Models");
    horLabels.push_back("Status");
    this->processingModel.setHorizontalHeaderLabels(horLabels);
    this->ui->processingView->setModel(&this->processingModel);
    this->RegenerateProcessingList();

    this->workspace.Load(tr("/home/tim/test.work"), this->mediaInterfaceBack);
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

    if(this->errMsg) delete this->errMsg;
    this->errMsg = NULL;

    delete this->mediaInterfaceFront;
    this->mediaInterfaceFront = NULL;

    delete this->mediaInterfaceBack;
    this->mediaInterfaceBack = NULL;

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
            for(unsigned int i=0;i<this->workspace.GetNumProcessing();i++)
            {
                this->workspace.PauseProcessing(i);
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
    if(this->sourcesModel.rowCount() != this->workspace.GetNumSources())
        this->sourcesModel.setRowCount(this->workspace.GetNumSources());
    for (int row = 0; row < this->workspace.GetNumSources(); ++row)
    {
        for (int column = 0; column < 1; ++column)
        {
            QString fina = this->workspace.GetSourceName(row);
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
            QUuid annotId = this->workspace.GetAnnotUid(row);
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
    QItemSelectionModel *sourceSelected = this->ui->dataSources->selectionModel();

    QIcon icon("icons/media-eject.png");
    if(this->processingModel.columnCount()!= 2)
        this->processingModel.setColumnCount(2);
    if(this->processingModel.rowCount() != this->workspace.GetNumProcessing())
        this->processingModel.setRowCount(this->workspace.GetNumProcessing());
    for (int row = 0; row < this->workspace.GetNumProcessing(); ++row)
    {
        for (int column = 0; column < 1; ++column)
        {
            QStandardItem *item = this->processingModel.item(row, column);
            if(item!=NULL)
            {
                continue;
            }

            QString fina = this->workspace.GetProcessingName(row);
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
            float progress = this->workspace.GetProgress(row);
            if(this->workspace.GetState(row)!=AlgorithmProcess::STOPPED)
            {
                AlgorithmProcess::ProcessState state = this->workspace.GetState(row);
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
    QItemSelectionModel *sourceSelected = this->ui->dataSources->selectionModel();
    assert(sourceSelected!=NULL);

    //Disable this source UI controls
    this->DeselectCurrentSource();

    QModelIndexList rowList = sourceSelected->selectedRows();
    for(unsigned int i=0;i<rowList.size();i++)
    {
        QModelIndex &ind = rowList[i];
        //cout << ind.row() << endl;
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
            this->workspace.ProcessingUpdate(atoi(args[1].c_str()), atof(args[0].c_str()));
            for(unsigned int i=0;i<this->workspace.GetNumProcessing();i++)
            {
                //cout << this->workspace.GetProgress(i) << endl;
            }
            this->RegenerateProcessingList();
        }
        if(ev->type=="THREAD_STATUS_CHANGED")
        {
            this->RegenerateProcessingList();
        }

        if(ev->type=="ANNOTATION_THREAD_PROGRESS")
        {
            assert(ev->data.length()>40);
            QUuid annId(ev->data.substr(0, 38).c_str());
            QString progStr(ev->data.substr(39).c_str());
            this->annotProgress[annId] = progStr.toFloat();
            this->RegenerateSourcesList();
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

    this->workspace.Load(fileName, this->mediaInterfaceBack);
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
    int selectedRow = ind.row();
    this->SelectedSourceChanged(selectedRow);
}

void MainWindow::SelectedSourceChanged(int selectedRow)
{
    if(selectedRow < 0 && selectedRow >= this->workspace.GetNumSources())
        return;

    QString fina = this->workspace.GetSourceName(selectedRow);

    this->DeselectCurrentSource();

    //Update scene controller
    TrackingAnnotation *scene = this->workspace.GetTrack(selectedRow);
    this->ui->widget->SetSceneControl(scene);

    assert(scene!=NULL);
    this->annotationMenu = scene->MenuFactory(this->menuBar());

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
    QItemSelectionModel *selection = this->ui->dataSources->selectionModel();

    //Count frames, because at least one is needed to train
    int countMarkedFrames = 0;
    QModelIndexList selectList = selection->selectedRows(0);
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];
        //For each annotated frame
        TrackingAnnotation *annot = this->workspace.GetTrack(ind.row());
        assert(annot!=0);
        for(unsigned int fr=0;fr<annot->NumMarkedFrames();fr++)
        {
            countMarkedFrames ++;
        }
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
    alg->SetUid(QUuid::createUuid());

    alg->Init();

    //Start worker process
    alg->Start();

    //Configure worker process
    selectList = selection->selectedRows(0);
    for(unsigned int i=0;i<selectList.size();i++)
    {

        QModelIndex &ind = selectList[i];
        QString fina = this->workspace.GetSourceName(ind.row());

        //For each annotated frame
        TrackingAnnotation *annot = this->workspace.GetTrack(ind.row());
        assert(annot!=0);
        for(unsigned int fr=0;fr<annot->NumMarkedFrames();fr++)
        {
            countMarkedFrames ++;

            //Get image data and send to process
            this->ui->widget->Pause();

            cout << annot->GetIndexTimestamp(fr) << endl;
            unsigned long long startTimestamp = 0, endTimestamp = 0;
            unsigned long long annotTimestamp = annot->GetIndexTimestamp(fr);
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

            int len = img->byteCount();
            //cout << "Image bytes "<< len << endl;
            //int len = 10;

            assert(img->format() == QImage::Format_RGB888);
            QString imgPreamble1 = QString("DATA_BLOCK=%1\n").arg(len);
            QString imgPreamble2 = QString("RGB_IMAGE_DATA TIMESTAMP=%1 HEIGHT=%2 WIDTH=%3\n").
                    arg(annotTimestamp).
                    arg(img->height()).
                    arg(img->width());
            alg->SendCommand(imgPreamble1);
            alg->SendCommand(imgPreamble2);
            QByteArray imgRaw((const char *)img->bits(), len);
            alg->SendRawData(imgRaw);
            //for (int xx=0;xx<len;xx++) alg->SendCommand("x");

            //Get annotation data and sent it to the process
            QString annotXml;
            QTextStream test(&annotXml);
            annot->GetIndexAnnotationXml(fr, &test);
            assert(annotXml.mid(annotXml.length()-1).toLocal8Bit().constData()[0]=='\n');

            int xmlLen = alg->EncodedLength(annotXml);
            QString preamble1 = QString("DATA_BLOCK=%1\n").arg(xmlLen);
            QString preamble2 = QString("XML_DATA\n");
            alg->SendCommand(preamble1);
            alg->SendCommand(preamble2);
            alg->SendCommand(annotXml);
        }
    }

    alg->SendCommand("TRAIN\n");

    //Add process to workspace
    this->workspace.AddProcessing(alg);

    //Update GUI to reflect changed workspace
    this->RegenerateProcessingList();
}

void MainWindow::ApplyModelPressed()
{
    cout << "ApplyModelPressed" << endl;
    QItemSelectionModel *modelSelection = this->ui->processingView->selectionModel();
    QItemSelectionModel *srcSelection = this->ui->dataSources->selectionModel();
    assert(modelSelection!=NULL);
    assert(srcSelection!=NULL);
    QModelIndexList modelSelList = modelSelection->selectedRows(0);
    QModelIndexList srcSelList = srcSelection->selectedRows(0);

    for(unsigned int i=0;i<srcSelList.size();i++)
    {
        QModelIndex &ind = srcSelList[i];
        cout << "src "<< ind.row() << "," << ind.column() << endl;

        //Load appropriate video
        QString fina = this->workspace.GetSourceName(ind.row());
        QUuid uid = this->workspace.GetAnnotUid(ind.row());
        //ChangeVidSource(&this->mediaThreadBack,this->mediaInterfaceBack,this->eventLoop,fina);

        //Apply models to selected video
        for(unsigned int i=0;i<modelSelList.size();i++)
        {
            QModelIndex &mind = modelSelList[i];
            cout << "model "<< mind.row() << "," << mind.column() << endl;
            std::tr1::shared_ptr<class AlgorithmProcess> alg = this->workspace.GetProcessing(mind.row());

            this->workspace.AddAutoAnnot(uid.toString(), alg->GetUid(), this->mediaInterfaceBack);
        }
    }
    this->RegenerateSourcesList();

}

void MainWindow::PauseProcessPressed()
{
    cout << "MainWindow::PauseProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];
        this->workspace.PauseProcessing(ind.row());
    }
    this->RegenerateProcessingList();
}

void MainWindow::RunProcessPressed()
{
    cout << "MainWindow::RunProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];
        //cout << ind.row() << "," <<ind.column() << endl;
        this->workspace.StartProcessing(ind.row());
    }
    this->RegenerateProcessingList();
}

void MainWindow::RemoveProcessPressed()
{
    cout << "MainWindow::RemoveProcessPressed()" << endl;

    QItemSelectionModel *selection = this->ui->processingView->selectionModel();
    QModelIndexList selectList = selection->selectedRows(0);
    for(unsigned int i=0;i<selectList.size();i++)
    {
        QModelIndex &ind = selectList[i];

        int ret = this->workspace.RemoveProcessing(ind.row());
        if(ret == 0)
        {
            if(this->errMsg == NULL)
                this->errMsg = new QMessageBox(this);
            this->errMsg->setWindowTitle("Error: Process is still running");
            this->errMsg->setText("Stop the process before trying to remove it.");
            this->errMsg->exec();
        }
    }
    this->RegenerateProcessingList();
}

void MainWindow::AboutPressed()
{
    if(this->errMsg == NULL)
        this->errMsg = new QMessageBox(this);
    this->errMsg->setWindowTitle("About");
    this->errMsg->setText("About text");
    this->errMsg->exec();
}
