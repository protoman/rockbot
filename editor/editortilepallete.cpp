#include "editortilepallete.h"
#include <stdio.h>

EditorTilePallete::EditorTilePallete(QWidget *parent) : QWidget(parent) {
   selectedTileX=0;
   selectedTileY=0;
   myParent = parent;
   parent->resize(QSize(250, 200));
   this->show();
}

void EditorTilePallete::paintEvent(QPaintEvent *) {
   QPixmap *image;

   QString filename;
   if (Mediator::get_instance()->getPallete().length() < 1) {
        filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/") + QString("default.png");
   } else {
        filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/") + QString(Mediator::get_instance()->getPallete().c_str());
   }

   QPainter painter(this);
   image = new QPixmap(filename);
   if (image->isNull()) {
      return;
   }
   QRectF target(QPoint(0, 0), QSize(image->size().width()*2, image->size().height()*2));
   QRectF source(QPoint(0, 0), image->size());
   painter.drawPixmap(target, *image, source);
   this->resize(image->size().width()*2, image->size().height()*2);
   myParent->adjustSize();
   // draw the selection marker
   painter.setPen(QColor(255, 0, 0));
   QRectF select(QPoint((selectedTileX*tilesize), (selectedTileY*tilesize)), QSize(tilesize, tilesize));
   painter.drawRect(select);
}

void EditorTilePallete::changeTileSet(const QString &tileset) {
   signalPalleteChanged();
   repaint();
}

void EditorTilePallete::mousePressEvent(QMouseEvent *event) {
   QPoint pnt = event->pos();
   selectedTileX = pnt.x()/tilesize;
   selectedTileY = pnt.y()/tilesize;
   Mediator::get_instance()->setPalleteX(selectedTileX);
   Mediator::get_instance()->setPalleteY(selectedTileY);
   repaint();
}




QString EditorTilePallete::getPallete() {
    QString res(Mediator::get_instance()->getPallete().c_str());
    return res;
}

//void EditorTilePallete::signalPalleteChanged() {
//   return;
//}


