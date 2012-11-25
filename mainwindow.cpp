#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videowidget.h"
#include "mediabuffer.h"
#include "imagesequence.h"
#include "avbinmedia.h"
#include <QFileDialog>
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(
    //    new MediaBuffer(this, QSharedPointer<AbstractMedia>(
    //        new ImageSequence(this,"/home/tim/dev/QtMedia/testseq"))));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia(this,
            "/home/tim/Desktop/SurreyHeadPoseDatabase/SANY0012.MP4"));

    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);

    this->ui->widget->SetSource(buff);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ImportVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this,
      tr("Import Video"), "", tr("Video Files (*.avi *.mov *.mkv *.wmf, *.webm, *.flv, *.mp4, *.rm, *.asf)"));
    QSharedPointer<AvBinMedia> avbin (new class AvBinMedia(this,
            fileName.toLocal8Bit().constData()));
    cout << "Opening " << fileName.toLocal8Bit().constData() << endl;
    QSharedPointer<AbstractMedia> buff = QSharedPointer<AbstractMedia>(avbin);

    this->ui->widget->SetSource(buff);
}
