#include "aboutgui.h"
#include "ui_aboutgui.h"
#include "version.h"

AboutGui::AboutGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutGui)
{
    ui->setupUi(this);
    this->licensee = tr("test");
    this->ui->licenseInfo->setText(this->licensee);
}

AboutGui::~AboutGui()
{
    delete ui;
}

void AboutGui::SetLicensee(QString text)
{
    this->licensee = text;
    this->ui->licenseInfo->setText(this->licensee);
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
    QString urlStr = QString("http://www.kinatomic.com/progurl/about.php?version=%1").arg(VERSION_URL);
    this->load(QUrl(urlStr));
}
