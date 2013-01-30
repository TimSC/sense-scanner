/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Jan 30 22:18:13 2013
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
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "aboutgui.h"
#include "sourcealggui.h"
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
    QAction *actionVideo;
    QAction *actionShow;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    SourceAlgGui *sourcesAlgGui;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuVideo;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *videoDock;
    VideoWidget *widget;
    QDockWidget *aboutDock;
    AboutGui *aboutWidget;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(525, 375);
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
        actionVideo = new QAction(MainWindow);
        actionVideo->setObjectName(QString::fromUtf8("actionVideo"));
        actionShow = new QAction(MainWindow);
        actionShow->setObjectName(QString::fromUtf8("actionShow"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        sourcesAlgGui = new SourceAlgGui(centralWidget);
        sourcesAlgGui->setObjectName(QString::fromUtf8("sourcesAlgGui"));
        sourcesAlgGui->setMinimumSize(QSize(200, 200));

        verticalLayout->addWidget(sourcesAlgGui);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 525, 23));
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
        videoDock = new QDockWidget(MainWindow);
        videoDock->setObjectName(QString::fromUtf8("videoDock"));
        videoDock->setCursor(QCursor(Qt::ArrowCursor));
        widget = new VideoWidget();
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setCursor(QCursor(Qt::ArrowCursor));
        videoDock->setWidget(widget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), videoDock);
        aboutDock = new QDockWidget(MainWindow);
        aboutDock->setObjectName(QString::fromUtf8("aboutDock"));
        aboutDock->setMinimumSize(QSize(76, 100));
        aboutDock->setBaseSize(QSize(0, 0));
        aboutWidget = new AboutGui();
        aboutWidget->setObjectName(QString::fromUtf8("aboutWidget"));
        aboutDock->setWidget(aboutWidget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(8), aboutDock);

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
        menuVideo->addAction(actionShow);
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
        QObject::connect(actionAbout, SIGNAL(triggered()), MainWindow, SLOT(AboutPressed()));
        QObject::connect(actionShow, SIGNAL(triggered()), MainWindow, SLOT(ShowVideoPressed()));

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
        actionVideo->setText(QApplication::translate("MainWindow", "Video", 0, QApplication::UnicodeUTF8));
        actionShow->setText(QApplication::translate("MainWindow", "Show", 0, QApplication::UnicodeUTF8));
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
