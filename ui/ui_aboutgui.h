/********************************************************************************
** Form generated from reading UI file 'aboutgui.ui'
**
** Created: Mon Apr 8 19:25:41 2013
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
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>
#include "aboutgui.h"

QT_BEGIN_NAMESPACE

class Ui_AboutGui
{
public:
    QVBoxLayout *verticalLayout;
    QTextBrowser *licenseInfo;
    WebViewErrCheck *webView;

    void setupUi(QWidget *AboutGui)
    {
        if (AboutGui->objectName().isEmpty())
            AboutGui->setObjectName(QString::fromUtf8("AboutGui"));
        AboutGui->resize(384, 409);
        verticalLayout = new QVBoxLayout(AboutGui);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        licenseInfo = new QTextBrowser(AboutGui);
        licenseInfo->setObjectName(QString::fromUtf8("licenseInfo"));
        licenseInfo->setMaximumSize(QSize(16777215, 40));

        verticalLayout->addWidget(licenseInfo);

        webView = new WebViewErrCheck(AboutGui);
        webView->setObjectName(QString::fromUtf8("webView"));
        webView->setUrl(QUrl(QString::fromUtf8("about:blank")));
        licenseInfo->raise();

        verticalLayout->addWidget(webView);


        retranslateUi(AboutGui);
        QObject::connect(webView, SIGNAL(loadFinished(bool)), webView, SLOT(LoadingResult(bool)));

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
