#ifndef SPRITE_PREVIEW_AREA_H
#define SPRITE_PREVIEW_AREA_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

#include "../file/format.h"

class sprite_preview_area : public QWidget
{
    Q_OBJECT
public:
    explicit sprite_preview_area(QWidget *parent = 0);
    
signals:
    
public slots:
    void updateBG();

protected:
  void paintEvent(QPaintEvent *);

private:
  int _sprite_n;
  int _timer_next_frame;

  QTimer *_timer;

    
};

#endif // SPRITE_PREVIEW_AREA_H
