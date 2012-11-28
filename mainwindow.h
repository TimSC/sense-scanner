#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>
#include <QThread>
#include <QTimer>
#include "avbinmedia.h"

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
    AvBinThread *readInputThread;
    class EventLoop *eventLoop;
    class EventReceiver *eventReceiver;
    QTimer *timer;
    int threadCount;

public slots:
    void ImportVideo();
    void Update();
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
