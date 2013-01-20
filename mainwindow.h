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

namespace Ui {
class MainWindow;
}

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

};

#endif // MAINWINDOW_H
