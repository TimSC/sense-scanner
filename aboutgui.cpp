#include "aboutgui.h"
#include "ui_aboutgui.h"

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

}

WebViewErrCheck::~WebViewErrCheck()
{

}

void WebViewErrCheck::LoadingResult(bool ok)
{
    if(!ok) this->load(QUrl("about.html"));
}
