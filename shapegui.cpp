#include "shapegui.h"
#include "ui_shapegui.h"
#include <QtCore/QDir>


QListViewWithChanges::QListViewWithChanges(QWidget *parent) : QListView(parent)
{

}

QListViewWithChanges::~QListViewWithChanges()
{

}

void QListViewWithChanges::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList inds = selected.indexes();
    if(inds.size()>0)
    {
        QModelIndex firstInd = inds[0];
        QAbstractItemModel *model = this->model();
        QVariant data = model->data(firstInd);
        this->selectedValue = data.toString();
    }
    else
        this->selectedValue = "";
}

//************************************************

ShapeGui::ShapeGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShapeGui)
{
    ui->setupUi(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    this->ui->shapePresets->setModel(model);

    QDir shapesDir("shapes");
    QStringList entries = shapesDir.entryList();
    QStringList validEntries;
    for(unsigned int i=0;i<entries.size();i++)
    {
        if(entries[i] == ".") continue;
        if(entries[i] == "..") continue;
        validEntries.append(entries[i]);
    }

    model->setColumnCount(1);
    model->setRowCount(validEntries.size());
    for(unsigned int i=0;i<validEntries.size();i++)
    {
        QString e = validEntries[i];
        QStandardItem *item = new QStandardItem(e);
        model->setItem(i, item);
    }

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

QString ShapeGui::GetCustomFilename()
{
    QString out("shapes/");
    out.append(this->ui->shapePresets->selectedValue);
    return out;
}
