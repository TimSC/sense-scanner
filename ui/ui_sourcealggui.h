/********************************************************************************
** Form generated from reading UI file 'sourcealggui.ui'
**
** Created: Mon Apr 8 19:25:41 2013
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCEALGGUI_H
#define UI_SOURCEALGGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "mainwindow.h"

QT_BEGIN_NAMESPACE

class Ui_SourceAlgGui
{
public:
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

    void setupUi(QWidget *SourceAlgGui)
    {
        if (SourceAlgGui->objectName().isEmpty())
            SourceAlgGui->setObjectName(QString::fromUtf8("SourceAlgGui"));
        SourceAlgGui->resize(400, 300);
        verticalLayout = new QVBoxLayout(SourceAlgGui);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        sourceButtons = new QHBoxLayout();
        sourceButtons->setObjectName(QString::fromUtf8("sourceButtons"));
        addSourceButton = new QToolButton(SourceAlgGui);
        addSourceButton->setObjectName(QString::fromUtf8("addSourceButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("icons/archive-insert.png"), QSize(), QIcon::Normal, QIcon::Off);
        addSourceButton->setIcon(icon);

        sourceButtons->addWidget(addSourceButton);

        removeSourceButton = new QToolButton(SourceAlgGui);
        removeSourceButton->setObjectName(QString::fromUtf8("removeSourceButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("icons/archive-remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        removeSourceButton->setIcon(icon1);

        sourceButtons->addWidget(removeSourceButton);

        trainModelButton = new QPushButton(SourceAlgGui);
        trainModelButton->setObjectName(QString::fromUtf8("trainModelButton"));

        sourceButtons->addWidget(trainModelButton);

        applyModelButton = new QPushButton(SourceAlgGui);
        applyModelButton->setObjectName(QString::fromUtf8("applyModelButton"));

        sourceButtons->addWidget(applyModelButton);


        verticalLayout->addLayout(sourceButtons);

        splitter = new QSplitter(SourceAlgGui);
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
        processingButtons->setObjectName(QString::fromUtf8("processingButtons"));
        pauseProcessButton = new QToolButton(SourceAlgGui);
        pauseProcessButton->setObjectName(QString::fromUtf8("pauseProcessButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("icons/media-playback-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        pauseProcessButton->setIcon(icon2);

        processingButtons->addWidget(pauseProcessButton);

        runProcessButton = new QToolButton(SourceAlgGui);
        runProcessButton->setObjectName(QString::fromUtf8("runProcessButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("icons/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
        runProcessButton->setIcon(icon3);

        processingButtons->addWidget(runProcessButton);

        removeProcessButton = new QToolButton(SourceAlgGui);
        removeProcessButton->setObjectName(QString::fromUtf8("removeProcessButton"));
        removeProcessButton->setIcon(icon1);

        processingButtons->addWidget(removeProcessButton);


        verticalLayout->addLayout(processingButtons);


        retranslateUi(SourceAlgGui);
        QObject::connect(addSourceButton, SIGNAL(pressed()), SourceAlgGui, SLOT(ImportVideo()));
        QObject::connect(removeSourceButton, SIGNAL(pressed()), SourceAlgGui, SLOT(RemoveVideo()));
        QObject::connect(trainModelButton, SIGNAL(pressed()), SourceAlgGui, SLOT(TrainModelPressed()));
        QObject::connect(applyModelButton, SIGNAL(pressed()), SourceAlgGui, SLOT(ApplyModelPressed()));
        QObject::connect(pauseProcessButton, SIGNAL(pressed()), SourceAlgGui, SLOT(PauseProcessPressed()));
        QObject::connect(runProcessButton, SIGNAL(pressed()), SourceAlgGui, SLOT(RunProcessPressed()));
        QObject::connect(removeProcessButton, SIGNAL(pressed()), SourceAlgGui, SLOT(RemoveProcessPressed()));
        QObject::connect(dataSources, SIGNAL(UpdateSources(QModelIndex)), SourceAlgGui, SLOT(SelectedSourceChanged(QModelIndex)));

        QMetaObject::connectSlotsByName(SourceAlgGui);
    } // setupUi

    void retranslateUi(QWidget *SourceAlgGui)
    {
        SourceAlgGui->setWindowTitle(QApplication::translate("SourceAlgGui", "Form", 0, QApplication::UnicodeUTF8));
        addSourceButton->setText(QApplication::translate("SourceAlgGui", "...", 0, QApplication::UnicodeUTF8));
        removeSourceButton->setText(QApplication::translate("SourceAlgGui", "...", 0, QApplication::UnicodeUTF8));
        trainModelButton->setText(QApplication::translate("SourceAlgGui", "Train", 0, QApplication::UnicodeUTF8));
        applyModelButton->setText(QApplication::translate("SourceAlgGui", "Apply", 0, QApplication::UnicodeUTF8));
        pauseProcessButton->setText(QApplication::translate("SourceAlgGui", "...", 0, QApplication::UnicodeUTF8));
        runProcessButton->setText(QApplication::translate("SourceAlgGui", "...", 0, QApplication::UnicodeUTF8));
        removeProcessButton->setText(QApplication::translate("SourceAlgGui", "...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SourceAlgGui: public Ui_SourceAlgGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCEALGGUI_H
