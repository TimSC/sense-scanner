#ifndef VIDEOAREA_H
#define VIDEOAREA_H
#include <QtGui/QGraphicsView>

class VideoArea : public QGraphicsView
{
public:
    explicit VideoArea(QWidget * parent = 0);
};

#endif // VIDEOAREA_H
