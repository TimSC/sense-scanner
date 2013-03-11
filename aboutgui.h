#ifndef ABOUTGUI_H
#define ABOUTGUI_H

#include <QtGui/QWidget>
#include <QtWebKit/QtWebKit>

namespace Ui {
class AboutGui;
}

class AboutGui : public QWidget
{
    Q_OBJECT
    
public:
    explicit AboutGui(QWidget *parent = 0);
    ~AboutGui();
    
private:
    Ui::AboutGui *ui;
};

class WebViewErrCheck : public QWebView
{
    Q_OBJECT;

public:
    WebViewErrCheck(QWidget * parent = 0);
    virtual ~WebViewErrCheck();

public slots:
    void LoadingResult(bool ok);
};


#endif // ABOUTGUI_H
