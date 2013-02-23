#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtGui/QGraphicsScene>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include "avbinmedia.h"
#include "workspace.h"
#include "localmutex.h"

namespace Ui {
class MainWindow;
}

//***********************************************

class BackgroundActionThread : public MessagableThread
{
    /*!
    * Does processing that would otherwise freeze the GUI such
    * as loading and saving.
    */

public:
    BackgroundActionThread(class MainWindow *mainWindowIn);
    virtual ~BackgroundActionThread();

    void Update();
    void Finished();

    void Save(class WaitPopUpDialog *dialog);
    void SaveAs(class WaitPopUpDialog *dialog, QString filename);
protected:
    class MainWindow *mainWindow;
    QList<QString> cmds;
    QList<QString> args;
    QList<class WaitPopUpDialog *> dialogs;
    Mutex lock;
};

//**************************************************

class WaitPopUpDialog : public QObject
{
    /*!
    * QT Dialog pop up while GUI is busy doing something else
    */

    Q_OBJECT
public:
    WaitPopUpDialog(QWidget *parent);
    virtual ~WaitPopUpDialog();
    void Exec();
    void WorkerTaskDone(int resultCode);
    int GetResultCode();
public slots:
    void Update();
protected:
    QDialog *dialog;
    QTimer *timer;
    int workerTaskDone;
    Mutex lock;
    int resultCode;
    int count;
};

//*************************************************

class CheckDiscardDataDialog : public QObject
{
    /*!
    * QT Dialog pop up to ask user if unsaved changes should be discarded.
    */

    Q_OBJECT
public:
    CheckDiscardDataDialog(QWidget *parent, QString discardMsg);
    virtual ~CheckDiscardDataDialog();

    QString GetUserChoice();
public slots:
    void ShutdownSaveAs();
    void ShutdownWithoutSave();
    void ShutdownCancel();
protected:
    QDialog *shutdownDialog;
    QString shutdownUserSelection;
};

//**********************************

class StopProcessingDialog : public QObject
{
    /*!
    * QT Dialog pop up to ask user if currently running processing tasks
    * should be stopped (paused).
    */

    Q_OBJECT
public:
    StopProcessingDialog(QWidget *parent);
    virtual ~StopProcessingDialog();

    QString GetUserChoice();
public slots:
    void AnswerStop();
    void AnswerCancel();
protected:
    QDialog *dialog;
    QString userSelection;
};

//*****************************************

class ClickableQTreeView : public QTreeView
{
    /*!
    * A QListView with currentChanged overridden to detect when
    * this selection changes.
    */

    Q_OBJECT
public:
    ClickableQTreeView(QWidget * parent = 0);
    virtual ~ClickableQTreeView();

    void currentChanged(const QModelIndex & current, const QModelIndex & previous);

signals:
    void UpdateSources(const QModelIndex current);

};

//***************************************

class MainWindow : public QMainWindow
{
    /*!
    * The main GUI window.
    */

    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    AvBinMedia *mediaInterfaceFront;
    AvBinMedia *mediaInterfaceBack;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;
    QTimer *timer;
    int threadCount;
    class Workspace workspace;
    class Workspace workspaceAsStored;
    QStandardItemModel sourcesModel;
    QStandardItemModel processingModel;
    QMenu *annotationMenu;
    QMessageBox *errMsg;
    std::map<QUuid, float> annotProgress;
    int avbinVerChecked;
    int timeUpdatesEnabled;

public slots:
    void ImportVideo();
    void RemoveVideo();
    void TrainModelPressed();
    void ApplyModelPressed();
    void PauseProcessPressed();
    void RunProcessPressed();
    void RemoveProcessPressed();

    void Update();
    void closeEvent(QCloseEvent *event);
    void RegenerateSourcesList();
    void RegenerateProcessingList();

    void NewWorkspace();
    void LoadWorkspace();
    void SaveWorkspace();
    void SaveAsWorkspace();

    void SelectedSourceChanged(const QModelIndex current);
    void SelectedSourceChanged(int selectedRow);
    void DeselectCurrentSource();
    QString CheckIfDataShouldBeDiscarded(QString discardMsg);

    void AboutPressed();
    void ShowVideoPressed();
};

#endif // MAINWINDOW_H
