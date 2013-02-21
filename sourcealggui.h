#ifndef SOURCEALGGUI_H
#define SOURCEALGGUI_H

#include <QtGui/QWidget>
#include <QtCore/QModelIndex>

namespace Ui {
class SourceAlgGui;
}

class SourceAlgGui : public QWidget
{
    Q_OBJECT
    friend class MainWindow;
    
public:
    explicit SourceAlgGui(QWidget *parent = 0);
    ~SourceAlgGui();

public slots:
    void ImportVideo();
    void RemoveVideo();
    void TrainModelPressed();
    void ApplyModelPressed();
    void PauseProcessPressed();
    void RunProcessPressed();
    void RemoveProcessPressed();

    void SelectedSourceChanged(const QModelIndex current);
    void DeselectCurrentSource();
    
protected:
    Ui::SourceAlgGui *ui;
    class MainWindow *mainWindow;
};

#endif // SOURCEALGGUI_H
