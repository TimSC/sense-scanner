#include "shapegui.h"
#include "ui_shapegui.h"

ShapeGui::ShapeGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShapeGui)
{
    ui->setupUi(this);
}

ShapeGui::~ShapeGui()
{
    delete ui;
}
