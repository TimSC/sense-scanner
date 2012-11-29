/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu Nov 29 19:40:59 2012
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
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "videowidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew_Workspace;
    QAction *actionLoad_Workspace;
    QAction *actionSave_Workspace;
    QAction *actionImport_Video;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
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
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new VideoWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));

        verticalLayout->addWidget(widget);

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
        menuFile->addSeparator();
        menuFile->addAction(actionImport_Video);

        retranslateUi(MainWindow);
        QObject::connect(actionImport_Video, SIGNAL(activated()), MainWindow, SLOT(ImportVideo()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionNew_Workspace->setText(QApplication::translate("MainWindow", "New Workspace", 0, QApplication::UnicodeUTF8));
        actionLoad_Workspace->setText(QApplication::translate("MainWindow", "Load Workspace", 0, QApplication::UnicodeUTF8));
        actionSave_Workspace->setText(QApplication::translate("MainWindow", "Save Workspace", 0, QApplication::UnicodeUTF8));
        actionImport_Video->setText(QApplication::translate("MainWindow", "Import Video", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
