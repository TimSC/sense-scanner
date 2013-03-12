#include "shapegui.h"
#include "ui_shapegui.h"

ShapeGui::ShapeGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShapeGui)
{
    ui->setupUi(this);

    this->usePresetSelected = 0;
    this->useCustomShapeSelected = 0;
    this->loadShapeSelected = 0;
}

ShapeGui::~ShapeGui()
{
    delete ui;
}

void ShapeGui::UsePresetPressed()
{
    this->usePresetSelected = 1;
    this->close();
}

void ShapeGui::UseCustomPressed()
{
    this->useCustomShapeSelected = 1;
    this->close();
}

void ShapeGui::LoadShapePressed()
{
    this->loadShapeSelected = 1;
    this->close();
}
