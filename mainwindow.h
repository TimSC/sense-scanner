#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtGui/QGraphicsScene>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include "avbinmedia.h"

namespace Ui {
class MainWindow;
}

class SourcesModel : public QAbstractItemModel
{
public:
    SourcesModel(QObject *parent = 0);
    virtual ~SourcesModel();

    QModelIndex index(int row, int column, const QModelIndex &parent);
    QModelIndex parent(const QModelIndex &index);
    int rowCount(const QModelIndex &parent);
    int columnCount(const QModelIndex &parent);
    QVariant data( const QModelIndex &index, int role);
};

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

public slots:
    void ImportVideo();
    void Update();
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
