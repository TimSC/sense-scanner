/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu Mar 14 11:04:54 2013
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
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
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
    QAction *actionKnowledge_Base;
    QAction *actionGetSupport;
    QAction *actionAbout;
    QAction *actionVideo;
    QAction *actionShow;
    QAction *actionKinatomic_Home_Page;
    QAction *actionShow_Sources;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    VideoWidget *videoWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuVideo;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *aboutDock;
    AboutGui *aboutWidget;
    QDockWidget *sourcesAlgDock;
    SourceAlgGui *sourcesAlgGui;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(699, 496);
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
        actionKnowledge_Base = new QAction(MainWindow);
        actionKnowledge_Base->setObjectName(QString::fromUtf8("actionKnowledge_Base"));
        actionGetSupport = new QAction(MainWindow);
        actionGetSupport->setObjectName(QString::fromUtf8("actionGetSupport"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionVideo = new QAction(MainWindow);
        actionVideo->setObjectName(QString::fromUtf8("actionVideo"));
        actionShow = new QAction(MainWindow);
        actionShow->setObjectName(QString::fromUtf8("actionShow"));
        actionKinatomic_Home_Page = new QAction(MainWindow);
        actionKinatomic_Home_Page->setObjectName(QString::fromUtf8("actionKinatomic_Home_Page"));
        actionShow_Sources = new QAction(MainWindow);
        actionShow_Sources->setObjectName(QString::fromUtf8("actionShow_Sources"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        videoWidget = new VideoWidget(centralWidget);
        videoWidget->setObjectName(QString::fromUtf8("videoWidget"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(videoWidget->sizePolicy().hasHeightForWidth());
        videoWidget->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(videoWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 699, 23));
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
        aboutDock = new QDockWidget(MainWindow);
        aboutDock->setObjectName(QString::fromUtf8("aboutDock"));
        aboutDock->setMinimumSize(QSize(79, 100));
        aboutDock->setFloating(true);
        aboutWidget = new AboutGui();
        aboutWidget->setObjectName(QString::fromUtf8("aboutWidget"));
        aboutDock->setWidget(aboutWidget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), aboutDock);
        sourcesAlgDock = new QDockWidget(MainWindow);
        sourcesAlgDock->setObjectName(QString::fromUtf8("sourcesAlgDock"));
        sourcesAlgGui = new SourceAlgGui();
        sourcesAlgGui->setObjectName(QString::fromUtf8("sourcesAlgGui"));
        sourcesAlgDock->setWidget(sourcesAlgGui);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), sourcesAlgDock);
        aboutDock->raise();
        sourcesAlgDock->raise();

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
        menuVideo->addAction(actionShow_Sources);
        menuHelp->addAction(actionKnowledge_Base);
        menuHelp->addAction(actionGetSupport);
        menuHelp->addAction(actionKinatomic_Home_Page);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);
        QObject::connect(actionImport_Video, SIGNAL(activated()), MainWindow, SLOT(ImportVideo()));
        QObject::connect(actionNew_Workspace, SIGNAL(activated()), MainWindow, SLOT(NewWorkspace()));
        QObject::connect(actionLoad_Workspace, SIGNAL(activated()), MainWindow, SLOT(LoadWorkspace()));
        QObject::connect(actionSave_Workspace, SIGNAL(activated()), MainWindow, SLOT(SaveWorkspace()));
        QObject::connect(actionSave_Workspace_As, SIGNAL(activated()), MainWindow, SLOT(SaveAsWorkspace()));
        QObject::connect(actionAbout, SIGNAL(activated()), MainWindow, SLOT(AboutPressed()));
        QObject::connect(actionFit_to_Window, SIGNAL(activated()), MainWindow, SLOT(FitVideoToWindow()));
        QObject::connect(actionKnowledge_Base, SIGNAL(activated()), MainWindow, SLOT(GetKnowledgeBase()));
        QObject::connect(actionGetSupport, SIGNAL(activated()), MainWindow, SLOT(GetSupport()));
        QObject::connect(actionKinatomic_Home_Page, SIGNAL(activated()), MainWindow, SLOT(GetKinatomicHomePage()));
        QObject::connect(actionShow_Sources, SIGNAL(activated()), MainWindow, SLOT(ShowSourcesPressed()));

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
        actionKnowledge_Base->setText(QApplication::translate("MainWindow", "Knowledge Base", 0, QApplication::UnicodeUTF8));
        actionGetSupport->setText(QApplication::translate("MainWindow", "Get Support", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("MainWindow", "About", 0, QApplication::UnicodeUTF8));
        actionVideo->setText(QApplication::translate("MainWindow", "Video", 0, QApplication::UnicodeUTF8));
        actionShow->setText(QApplication::translate("MainWindow", "Show", 0, QApplication::UnicodeUTF8));
        actionKinatomic_Home_Page->setText(QApplication::translate("MainWindow", "Kinatomic Home Page", 0, QApplication::UnicodeUTF8));
        actionShow_Sources->setText(QApplication::translate("MainWindow", "Show Sources", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuVideo->setTitle(QApplication::translate("MainWindow", "Video", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
        aboutDock->setWindowTitle(QApplication::translate("MainWindow", "Welcome to Sense Scanner", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
