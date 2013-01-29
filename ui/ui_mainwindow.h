/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue Jan 29 02:11:31 2013
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "mainwindow.h"
#include "videowidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew_Workspace;
    QAction *actionLoad_Workspace;
    QAction *actionSave_Workspace;
    QAction *actionImport_Video;
    QAction *actionSave_Workspace_As;
    QAction *actionFit_to_Window;
    QAction *actionWeb_Help_Pages;
    QAction *actionSupport_Forum;
    QAction *actionAbout;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *sourceButtons;
    QToolButton *addSourceButton;
    QToolButton *removeSourceButton;
    QPushButton *trainModelButton;
    QPushButton *applyModelButton;
    QSplitter *splitter;
    ClickableQTreeView *dataSources;
    ClickableQTreeView *processingView;
    QHBoxLayout *processingButtons;
    QToolButton *pauseProcessButton;
    QToolButton *runProcessButton;
    QToolButton *removeProcessButton;
    VideoWidget *widget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuVideo;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 323);
        actionNew_Workspace = new QAction(MainWindow);
        actionNew_Workspace->setObjectName(QString::fromUtf8("actionNew_Workspace"));
        actionLoad_Workspace = new QAction(MainWindow);
        actionLoad_Workspace->setObjectName(QString::fromUtf8("actionLoad_Workspace"));
        actionSave_Workspace = new QAction(MainWindow);
        actionSave_Workspace->setObjectName(QString::fromUtf8("actionSave_Workspace"));
        actionImport_Video = new QAction(MainWindow);
        actionImport_Video->setObjectName(QString::fromUtf8("actionImport_Video"));
        actionSave_Workspace_As = new QAction(MainWindow);
        actionSave_Workspace_As->setObjectName(QString::fromUtf8("actionSave_Workspace_As"));
        actionFit_to_Window = new QAction(MainWindow);
        actionFit_to_Window->setObjectName(QString::fromUtf8("actionFit_to_Window"));
        actionWeb_Help_Pages = new QAction(MainWindow);
        actionWeb_Help_Pages->setObjectName(QString::fromUtf8("actionWeb_Help_Pages"));
        actionSupport_Forum = new QAction(MainWindow);
        actionSupport_Forum->setObjectName(QString::fromUtf8("actionSupport_Forum"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        sourceButtons = new QHBoxLayout();
        sourceButtons->setSpacing(6);
        sourceButtons->setObjectName(QString::fromUtf8("sourceButtons"));
        addSourceButton = new QToolButton(centralWidget);
        addSourceButton->setObjectName(QString::fromUtf8("addSourceButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("icons/archive-insert.png"), QSize(), QIcon::Normal, QIcon::Off);
        addSourceButton->setIcon(icon);

        sourceButtons->addWidget(addSourceButton);

        removeSourceButton = new QToolButton(centralWidget);
        removeSourceButton->setObjectName(QString::fromUtf8("removeSourceButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("icons/archive-remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        removeSourceButton->setIcon(icon1);

        sourceButtons->addWidget(removeSourceButton);

        trainModelButton = new QPushButton(centralWidget);
        trainModelButton->setObjectName(QString::fromUtf8("trainModelButton"));

        sourceButtons->addWidget(trainModelButton);

        applyModelButton = new QPushButton(centralWidget);
        applyModelButton->setObjectName(QString::fromUtf8("applyModelButton"));

        sourceButtons->addWidget(applyModelButton);


        verticalLayout->addLayout(sourceButtons);

        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Vertical);
        dataSources = new ClickableQTreeView(splitter);
        dataSources->setObjectName(QString::fromUtf8("dataSources"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(dataSources->sizePolicy().hasHeightForWidth());
        dataSources->setSizePolicy(sizePolicy);
        splitter->addWidget(dataSources);
        processingView = new ClickableQTreeView(splitter);
        processingView->setObjectName(QString::fromUtf8("processingView"));
        splitter->addWidget(processingView);

        verticalLayout->addWidget(splitter);

        processingButtons = new QHBoxLayout();
        processingButtons->setSpacing(6);
        processingButtons->setObjectName(QString::fromUtf8("processingButtons"));
        pauseProcessButton = new QToolButton(centralWidget);
        pauseProcessButton->setObjectName(QString::fromUtf8("pauseProcessButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("icons/media-playback-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        pauseProcessButton->setIcon(icon2);

        processingButtons->addWidget(pauseProcessButton);

        runProcessButton = new QToolButton(centralWidget);
        runProcessButton->setObjectName(QString::fromUtf8("runProcessButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("icons/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
        runProcessButton->setIcon(icon3);

        processingButtons->addWidget(runProcessButton);

        removeProcessButton = new QToolButton(centralWidget);
        removeProcessButton->setObjectName(QString::fromUtf8("removeProcessButton"));
        removeProcessButton->setIcon(icon1);

        processingButtons->addWidget(removeProcessButton);


        verticalLayout->addLayout(processingButtons);


        horizontalLayout->addLayout(verticalLayout);

        widget = new VideoWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));

        horizontalLayout->addWidget(widget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuVideo = new QMenu(menuBar);
        menuVideo->setObjectName(QString::fromUtf8("menuVideo"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuVideo->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew_Workspace);
        menuFile->addAction(actionLoad_Workspace);
        menuFile->addAction(actionSave_Workspace);
        menuFile->addAction(actionSave_Workspace_As);
        menuFile->addSeparator();
        menuFile->addAction(actionImport_Video);
        menuVideo->addAction(actionFit_to_Window);
        menuHelp->addAction(actionWeb_Help_Pages);
        menuHelp->addAction(actionSupport_Forum);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);
        QObject::connect(actionImport_Video, SIGNAL(activated()), MainWindow, SLOT(ImportVideo()));
        QObject::connect(actionNew_Workspace, SIGNAL(activated()), MainWindow, SLOT(NewWorkspace()));
        QObject::connect(actionLoad_Workspace, SIGNAL(activated()), MainWindow, SLOT(LoadWorkspace()));
        QObject::connect(actionSave_Workspace, SIGNAL(activated()), MainWindow, SLOT(SaveWorkspace()));
        QObject::connect(actionSave_Workspace_As, SIGNAL(activated()), MainWindow, SLOT(SaveAsWorkspace()));
        QObject::connect(addSourceButton, SIGNAL(pressed()), MainWindow, SLOT(ImportVideo()));
        QObject::connect(removeSourceButton, SIGNAL(pressed()), MainWindow, SLOT(RemoveVideo()));
        QObject::connect(actionFit_to_Window, SIGNAL(activated()), widget, SLOT(FitToWindow()));
        QObject::connect(trainModelButton, SIGNAL(pressed()), MainWindow, SLOT(TrainModelPressed()));
        QObject::connect(applyModelButton, SIGNAL(pressed()), MainWindow, SLOT(ApplyModelPressed()));
        QObject::connect(pauseProcessButton, SIGNAL(clicked()), MainWindow, SLOT(PauseProcessPressed()));
        QObject::connect(runProcessButton, SIGNAL(clicked()), MainWindow, SLOT(RunProcessPressed()));
        QObject::connect(removeProcessButton, SIGNAL(clicked()), MainWindow, SLOT(RemoveProcessPressed()));
        QObject::connect(actionAbout, SIGNAL(triggered()), MainWindow, SLOT(AboutPressed()));
        QObject::connect(dataSources, SIGNAL(UpdateSources(QModelIndex)), MainWindow, SLOT(SelectedSourceChanged(QModelIndex)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionNew_Workspace->setText(QApplication::translate("MainWindow", "New Workspace", 0, QApplication::UnicodeUTF8));
        actionLoad_Workspace->setText(QApplication::translate("MainWindow", "Load Workspace", 0, QApplication::UnicodeUTF8));
        actionSave_Workspace->setText(QApplication::translate("MainWindow", "Save Workspace", 0, QApplication::UnicodeUTF8));
        actionImport_Video->setText(QApplication::translate("MainWindow", "Import Video", 0, QApplication::UnicodeUTF8));
        actionSave_Workspace_As->setText(QApplication::translate("MainWindow", "Save Workspace As...", 0, QApplication::UnicodeUTF8));
        actionFit_to_Window->setText(QApplication::translate("MainWindow", "Fit to Window", 0, QApplication::UnicodeUTF8));
        actionWeb_Help_Pages->setText(QApplication::translate("MainWindow", "Web Help Pages", 0, QApplication::UnicodeUTF8));
        actionSupport_Forum->setText(QApplication::translate("MainWindow", "Support Forum", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("MainWindow", "About", 0, QApplication::UnicodeUTF8));
        addSourceButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        removeSourceButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        trainModelButton->setText(QApplication::translate("MainWindow", "Train", 0, QApplication::UnicodeUTF8));
        applyModelButton->setText(QApplication::translate("MainWindow", "Apply", 0, QApplication::UnicodeUTF8));
        pauseProcessButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        runProcessButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        removeProcessButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuVideo->setTitle(QApplication::translate("MainWindow", "Video", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
