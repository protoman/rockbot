#ifndef PROJECTILEPREVIEWAREA_H
#define PROJECTILEPREVIEWAREA_H

#include <QWidget>
#include <QPainter>
#include <stdio.h>
#include <QMouseEvent>
#include "mediator.h"

class projectilePreviewArea : public QWidget
{
  Q_OBJECT

public:
  // methods
  projectilePreviewArea(QWidget *parent = 0);

  // variables
  //QWidget *myParent;
private:
  int editor_selectedTileX;
  int editor_selectedTileY;
  int image_w;
  int image_h;
  int zoom = 4;

protected:
  void paintEvent(QPaintEvent *event);
  void wheelEvent(QWheelEvent *event);

  QWidget *myParent;

signals:
	 void clickedIn();

public slots:
   //void changeTile();
};


#endif // PROJECTILEPREVIEWAREA_H
