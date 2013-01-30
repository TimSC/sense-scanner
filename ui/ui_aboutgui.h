/********************************************************************************
** Form generated from reading UI file 'aboutgui.ui'
**
** Created: Wed Jan 30 22:14:38 2013
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTGUI_H
#define UI_ABOUTGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE

class Ui_AboutGui
{
public:
    QVBoxLayout *verticalLayout;
    QWebView *webView;

    void setupUi(QWidget *AboutGui)
    {
        if (AboutGui->objectName().isEmpty())
            AboutGui->setObjectName(QString::fromUtf8("AboutGui"));
        AboutGui->resize(400, 300);
        verticalLayout = new QVBoxLayout(AboutGui);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        webView = new QWebView(AboutGui);
        webView->setObjectName(QString::fromUtf8("webView"));
        webView->setUrl(QUrl(QString::fromUtf8("http://www.sheerman-chase.org.uk/vcs/about.html?version=alpha1")));

        verticalLayout->addWidget(webView);


        retranslateUi(AboutGui);

        QMetaObject::connectSlotsByName(AboutGui);
    } // setupUi

    void retranslateUi(QWidget *AboutGui)
    {
        AboutGui->setWindowTitle(QApplication::translate("AboutGui", "Form", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AboutGui: public Ui_AboutGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTGUI_H
