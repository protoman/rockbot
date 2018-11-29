#ifndef CASTLE_POINTS_EDITORAREA_H
#define CASTLE_POINTS_EDITORAREA_H

#include <QWidget>
#include <QPainter>
#include "mediator.h"

class castle_points_editorarea : public QWidget
{
    Q_OBJECT
public:
    castle_points_editorarea(QWidget *parent = nullptr);
    ~castle_points_editorarea();
    void set_current_point(int current);

    QWidget *myParent;
    QPixmap *image;


private:
    void load_image();

    int currentPoint;

protected:
  void paintEvent(QPaintEvent *event);

};

#endif // CASTLE_POINTS_EDITORAREA_H
