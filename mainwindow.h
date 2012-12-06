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

//*****************************************

class SourcesList : public QListView
{
    Q_OBJECT
public:
    SourcesList(QWidget * parent = 0);
    virtual ~SourcesList();

    void currentChanged(const QModelIndex & current, const QModelIndex & previous);

signals:
    void UpdateSources(const QModelIndex current);

};

//***************************************

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    AvBinThread *mediaThread;
    AvBinMedia *mediaInterface;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;
    QTimer *timer;
    int threadCount;
    class Workspace workspace;
    class Workspace workspaceAsStored;
    QStandardItemModel *sourcesModel;
    QMenu *annotationMenu;

public slots:
    void ImportVideo();
    void RemoveVideo();

    void Update();
    void closeEvent(QCloseEvent *event);
    void RegenerateSourcesList();

    void NewWorkspace();
    void LoadWorkspace();
    void SaveWorkspace();
    void SaveAsWorkspace();
    void SelectedSourceChanged(const QModelIndex current);
    QString CheckIfDataShouldBeDiscarded(QString discardMsg);

};

#endif // MAINWINDOW_H
