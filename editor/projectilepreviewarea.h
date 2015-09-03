#ifndef PROJECTILEPREVIEWAREA_H
#define PROJECTILEPREVIEWAREA_H

#include <QWidget>
#include <QPainter>
#include <stdio.h>
#include <QMouseEvent>
#include "mediator.h"

extern CURRENT_FILE_FORMAT::file_game game_data;

extern Mediator *dataExchanger;

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

protected:
  void paintEvent(QPaintEvent *event);

  QWidget *myParent;

signals:
	 void clickedIn();

public slots:
   //void changeTile();
};


#endif // PROJECTILEPREVIEWAREA_H
