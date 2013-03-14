/********************************************************************************
** Form generated from reading UI file 'videowidget.ui'
**
** Created: Thu Mar 14 00:11:38 2013
**      by: Qt User Interface Compiler version 4.8.3
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
#include <QtGui/QTimeEdit>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "videowidget.h"

QT_BEGIN_NAMESPACE

class Ui_VideoWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *annotationTools;
    ZoomGraphicsView *graphicsView;
    QHBoxLayout *horizontalLayout;
    QToolButton *seekBackButton;
    QToolButton *pauseButton;
    QToolButton *playButton;
    QToolButton *seekForwardButton;
    QScrollBar *horizontalScrollBar;
    QTimeEdit *timeEdit;

    void setupUi(QWidget *VideoWidget)
    {
        if (VideoWidget->objectName().isEmpty())
            VideoWidget->setObjectName(QString::fromUtf8("VideoWidget"));
        VideoWidget->resize(507, 417);
        VideoWidget->setMinimumSize(QSize(400, 400));
        verticalLayout_2 = new QVBoxLayout(VideoWidget);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        annotationTools = new QHBoxLayout();
        annotationTools->setObjectName(QString::fromUtf8("annotationTools"));

        verticalLayout_2->addLayout(annotationTools);

        graphicsView = new ZoomGraphicsView(VideoWidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CrossCursor)));

        verticalLayout_2->addWidget(graphicsView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        seekBackButton = new QToolButton(VideoWidget);
        seekBackButton->setObjectName(QString::fromUtf8("seekBackButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("icons/media-seek-backward.png"), QSize(), QIcon::Normal, QIcon::Off);
        seekBackButton->setIcon(icon);

        horizontalLayout->addWidget(seekBackButton);

        pauseButton = new QToolButton(VideoWidget);
        pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("icons/media-playback-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        pauseButton->setIcon(icon1);

        horizontalLayout->addWidget(pauseButton);

        playButton = new QToolButton(VideoWidget);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("icons/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
        playButton->setIcon(icon2);

        horizontalLayout->addWidget(playButton);

        seekForwardButton = new QToolButton(VideoWidget);
        seekForwardButton->setObjectName(QString::fromUtf8("seekForwardButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("icons/media-seek-forward.png"), QSize(), QIcon::Normal, QIcon::Off);
        seekForwardButton->setIcon(icon3);

        horizontalLayout->addWidget(seekForwardButton);

        horizontalScrollBar = new QScrollBar(VideoWidget);
        horizontalScrollBar->setObjectName(QString::fromUtf8("horizontalScrollBar"));
        horizontalScrollBar->setPageStep(1000);
        horizontalScrollBar->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(horizontalScrollBar);

        timeEdit = new QTimeEdit(VideoWidget);
        timeEdit->setObjectName(QString::fromUtf8("timeEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(timeEdit->sizePolicy().hasHeightForWidth());
        timeEdit->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(timeEdit);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(VideoWidget);
        QObject::connect(horizontalScrollBar, SIGNAL(valueChanged(int)), VideoWidget, SLOT(SliderMoved(int)));
        QObject::connect(playButton, SIGNAL(pressed()), VideoWidget, SLOT(Play()));
        QObject::connect(pauseButton, SIGNAL(pressed()), VideoWidget, SLOT(Pause()));
        QObject::connect(seekBackButton, SIGNAL(pressed()), VideoWidget, SLOT(SeekBack()));
        QObject::connect(seekForwardButton, SIGNAL(pressed()), VideoWidget, SLOT(SeekForward()));
        QObject::connect(timeEdit, SIGNAL(timeChanged(QTime)), VideoWidget, SLOT(TimeChanged(QTime)));

        QMetaObject::connectSlotsByName(VideoWidget);
    } // setupUi

    void retranslateUi(QWidget *VideoWidget)
    {
        VideoWidget->setWindowTitle(QApplication::translate("VideoWidget", "Form", 0, QApplication::UnicodeUTF8));
        seekBackButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
        pauseButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
        playButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
        seekForwardButton->setText(QApplication::translate("VideoWidget", "...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoWidget: public Ui_VideoWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOWIDGET_H
