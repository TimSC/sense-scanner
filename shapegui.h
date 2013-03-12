#ifndef SHAPEGUI_H
#define SHAPEGUI_H

#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtGui/QStandardItemModel>
#include <QtGui/QListView>

namespace Ui {
class ShapeGui;
}

class QListViewWithChanges : public QListView
{
    Q_OBJECT
public:
    QListViewWithChanges(QWidget *parent = 0);
    virtual ~QListViewWithChanges();

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    QString selectedValue;
};

//************************************************

class ShapeGui : public QDialog
{
    Q_OBJECT
    
public:
    explicit ShapeGui(QWidget *parent = 0);
    ~ShapeGui();
    
    int usePresetSelected;
    int useCustomShapeSelected;
    int loadShapeSelected;

private:
    Ui::ShapeGui *ui;

public slots:
    void UsePresetPressed();
    void UseCustomPressed();
    void LoadShapePressed();
    QString GetCustomFilename();
};

#endif // SHAPEGUI_H
