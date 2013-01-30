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
