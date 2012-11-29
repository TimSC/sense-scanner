/********************************************************************************
** Form generated from reading UI file 'videowidget.ui'
**
** Created: Thu Nov 29 16:38:55 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOWIDGET_H
#define UI_VIDEOWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "videowidget.h"

QT_BEGIN_NAMESPACE

class Ui_VideoWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    ZoomGraphicsView *graphicsView;
    QHBoxLayout *horizontalLayout;
    QToolButton *pauseButton;
    QToolButton *playButton;
    QScrollBar *horizontalScrollBar;

    void setupUi(QWidget *VideoWidget)
    {
        if (VideoWidget->objectName().isEmpty())
            VideoWidget->setObjectName(QString::fromUtf8("VideoWidget"));
        VideoWidget->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(VideoWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        graphicsView = new ZoomGraphicsView(VideoWidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));

        verticalLayout_2->addWidget(graphicsView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pauseButton = new QToolButton(VideoWidget);
        pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("../QtMedia/icons/media-playback-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        pauseButton->setIcon(icon);

        horizontalLayout->addWidget(pauseButton);

        playButton = new QToolButton(VideoWidget);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../QtMedia/icons/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
        playButton->setIcon(icon1);

        horizontalLayout->addWidget(playButton);

        horizontalScrollBar = new QScrollBar(VideoWidget);
        horizontalScrollBar->setObjectName(QString::fromUtf8("horizontalScrollBar"));
        horizontalScrollBar->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(horizontalScrollBar);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(VideoWidget);
        QObject::connect(horizontalScrollBar, SIGNAL(valueChanged(int)), VideoWidget, SLOT(SliderMoved(int)));
        QObject::connect(playButton, SIGNAL(pressed()), VideoWidget, SLOT(Play()));
        QObject::connect(pauseButton, SIGNAL(pressed()), VideoWidget, SLOT(Pause()));

        QMetaObject::connectSlotsByName(VideoWidget);
    } // setupUi

    void retranslateUi(QWidget *VideoWidget)
    {
        VideoWidget->setWindowTitle(QApplication::translate("VideoWidget", "Form", 0, QApplication::UnicodeUTF8));
        pauseButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
        playButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoWidget: public Ui_VideoWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOWIDGET_H
