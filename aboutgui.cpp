#include "aboutgui.h"
#include "ui_aboutgui.h"
#include "version.h"

AboutGui::AboutGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutGui)
{
    ui->setupUi(this);
}

AboutGui::~AboutGui()
{
    delete ui;
}

//***********************************

WebViewErrCheck::WebViewErrCheck(QWidget * parent) : QWebView(parent)
{
    QTimer::singleShot(10, this, SLOT(LoadInitialPage()));
}

WebViewErrCheck::~WebViewErrCheck()
{

}

void WebViewErrCheck::LoadingResult(bool ok)
{
    if(!ok) this->load(QUrl("about.html"));
}

void WebViewErrCheck::LoadInitialPage()
{
    QString urlStr = QString("http://www.kinatomic.com/progurl/about.php?version=").arg(VERSION_URL);
    this->load(QUrl(urlStr));
}
