#include "sourcealggui.h"
#include "ui_sourcealggui.h"
#include <assert.h>

SourceAlgGui::SourceAlgGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SourceAlgGui)
{
    ui->setupUi(this);
    this->mainWindow = NULL;
}

SourceAlgGui::~SourceAlgGui()
{
    delete ui;
}

void SourceAlgGui::ImportVideo()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->ImportVideo();
}

void SourceAlgGui::RemoveVideo()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->RemoveVideo();
}

void SourceAlgGui::TrainModelPressed()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->TrainModelPressed();
}

void SourceAlgGui::ApplyModelPressed()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->ApplyModelPressed();
}

void SourceAlgGui::PauseProcessPressed()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->PauseProcessPressed();
}

void SourceAlgGui::RunProcessPressed()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->RunProcessPressed();
}

void SourceAlgGui::RemoveProcessPressed()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->RemoveProcessPressed();
}

void SourceAlgGui::SelectedSourceChanged(const QModelIndex current)
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->SelectedSourceChanged(current);
}

void SourceAlgGui::DeselectCurrentSource()
{
    assert(this->mainWindow!=NULL);
    this->mainWindow->DeselectCurrentSource();
}
