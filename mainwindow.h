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
    QStandardItemModel *sourcesModel;

public slots:
    void ImportVideo();
    void Update();
    void closeEvent(QCloseEvent *event);
    void RegenerateSourcesList();
};

#endif // MAINWINDOW_H
