/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Dec 5 15:08:30 2012
**      by: Qt User Interface Compiler version 4.8.1
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
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *addSourceButton;
    QToolButton *removeSourceButton;
    SourcesList *dataSources;
    VideoWidget *widget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 300);
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
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        addSourceButton = new QToolButton(centralWidget);
        addSourceButton->setObjectName(QString::fromUtf8("addSourceButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("icons/archive-insert.png"), QSize(), QIcon::Normal, QIcon::Off);
        addSourceButton->setIcon(icon);

        horizontalLayout_2->addWidget(addSourceButton);

        removeSourceButton = new QToolButton(centralWidget);
        removeSourceButton->setObjectName(QString::fromUtf8("removeSourceButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("icons/archive-remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        removeSourceButton->setIcon(icon1);

        horizontalLayout_2->addWidget(removeSourceButton);


        verticalLayout->addLayout(horizontalLayout_2);

        dataSources = new SourcesList(centralWidget);
        dataSources->setObjectName(QString::fromUtf8("dataSources"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(dataSources->sizePolicy().hasHeightForWidth());
        dataSources->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(dataSources);


        horizontalLayout->addLayout(verticalLayout);

        widget = new VideoWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));

        horizontalLayout->addWidget(widget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 23));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionNew_Workspace);
        menuFile->addAction(actionLoad_Workspace);
        menuFile->addAction(actionSave_Workspace);
        menuFile->addAction(actionSave_Workspace_As);
        menuFile->addSeparator();
        menuFile->addAction(actionImport_Video);

        retranslateUi(MainWindow);
        QObject::connect(actionImport_Video, SIGNAL(activated()), MainWindow, SLOT(ImportVideo()));
        QObject::connect(actionNew_Workspace, SIGNAL(activated()), MainWindow, SLOT(NewWorkspace()));
        QObject::connect(actionLoad_Workspace, SIGNAL(activated()), MainWindow, SLOT(LoadWorkspace()));
        QObject::connect(actionSave_Workspace, SIGNAL(activated()), MainWindow, SLOT(SaveWorkspace()));
        QObject::connect(actionSave_Workspace_As, SIGNAL(activated()), MainWindow, SLOT(SaveAsWorkspace()));
        QObject::connect(dataSources, SIGNAL(UpdateSources(QModelIndex)), MainWindow, SLOT(SelectedSourceChanged(QModelIndex)));
        QObject::connect(addSourceButton, SIGNAL(pressed()), MainWindow, SLOT(ImportVideo()));
        QObject::connect(removeSourceButton, SIGNAL(pressed()), MainWindow, SLOT(RemoveVideo()));

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
        addSourceButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        removeSourceButton->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
