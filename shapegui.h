#ifndef SHAPEGUI_H
#define SHAPEGUI_H

#include <QDialog>

namespace Ui {
class ShapeGui;
}

class ShapeGui : public QDialog
{
    Q_OBJECT
    
public:
    explicit ShapeGui(QWidget *parent = 0);
    ~ShapeGui();
    
private:
    Ui::ShapeGui *ui;
    int usePresetSelected;
    int useCustomShapeSelected;
    int loadShapeSelected;

public slots:
    void UsePresetPressed();
    void UseCustomPressed();
    void LoadShapePressed();
};

#endif // SHAPEGUI_H
