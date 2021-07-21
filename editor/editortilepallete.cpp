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
   QRectF target(QPoint(0, 0), QSize(image->size().width()*zoom, image->size().height()*zoom));
   QRectF source(QPoint(0, 0), image->size());
   painter.drawPixmap(target, *image, source);
   this->resize(image->size().width()*zoom, image->size().height()*zoom);
   myParent->adjustSize();

   // draw the selection marker
   painter.setPen(QColor(255, 0, 0));
   QRectF select(QPoint((selectedTileX*TILESIZE*zoom), (selectedTileY*TILESIZE*zoom)), QSize(TILESIZE*zoom, TILESIZE*zoom));
   painter.drawRect(select);

   // GRID
   QPen pen(QColor(160, 160, 160), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
   painter.setPen(pen);
   for (int i=0; i<image->size().height()/TILESIZE; i++) {
       int pos = i*TILESIZE*zoom-1;
       // linhas horizontais
       QLineF line = QLineF(0, pos, image->size().width()*zoom-1, pos);
       painter.drawLine(line);
   }
   for (int j=0; j<image->size().width()/TILESIZE; j++) {
       int pos = j*TILESIZE*zoom-1;
       // linhas verticais
       QLineF line = QLineF(pos, 0, pos, image->size().height()*zoom-1);
       painter.drawLine(line);
   }

}

void EditorTilePallete::changeTileSet(const QString &tileset) {
   signalPalleteChanged();
   repaint();
}

void EditorTilePallete::mousePressEvent(QMouseEvent *event) {
   QPoint pnt = event->pos();
   selectedTileX = pnt.x()/(TILESIZE*zoom);
   selectedTileY = pnt.y()/(TILESIZE*zoom);

   Mediator::get_instance()->setPalleteX(selectedTileX);
   Mediator::get_instance()->setPalleteY(selectedTileY);
   repaint();
}

void EditorTilePallete::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    if (numSteps > 0) {
        if (zoom < 4) {
            zoom++;
            repaint();
        }
    } else if (numSteps < 0) {
        if (zoom > 1) {
            zoom--;
            repaint();
        }
    }
}




QString EditorTilePallete::getPallete() {
    QString res(Mediator::get_instance()->getPallete().c_str());
    return res;
}

//void EditorTilePallete::signalPalleteChanged() {
//   return;
//}


