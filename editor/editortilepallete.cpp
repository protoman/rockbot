#include "editortilepallete.h"
#include <stdio.h>

extern Mediator dataExchange;
extern char EDITOR_FILEPATH[512];

EditorTilePallete::EditorTilePallete(QWidget *parent) : QWidget(parent) {
   printf("DEBUG.EditorTilePallete - constructor\n");
   selectedTileX=0;
   selectedTileY=0;
   currentPallete.clear();
   myParent = parent;
   parent->resize(QSize(250, 200));
   this->show();
}

void EditorTilePallete::paintEvent(QPaintEvent *) {
   QString *filename;
   QPixmap *image;
   //printf("DEBUG.EditorTilePallete - paintEvent, selectedTileX: %d, selectedTileY: %d\n", selectedTileX, selectedTileY);
   QPainter painter(this);
   if (currentPallete.length() <= 0) {
      std::string tempstr = dataExchanger->getPallete();
      filename = new QString(tempstr.c_str());
   } else {
	  filename = new QString(QString(EDITOR_FILEPATH)+"/data/images/tilesets/");
      filename->append(currentPallete);
   }
   image = new QPixmap(*filename);
   if (image->isNull()) {
      printf("DEBUG.Tile - Could not load image file '%s'\n", qPrintable(*filename));
   }
   QRectF target(QPoint(0, 0), image->size());
   QRectF source(QPoint(0, 0), image->size());
   painter.drawPixmap(target, *image, source);
   this->resize(image->size());
   myParent->adjustSize();
   // draw the selection marker
   painter.setPen(QColor(255, 0, 0));
   QRectF select(QPoint((selectedTileX*16), (selectedTileY*16)), QSize(16, 16));
   painter.drawRect(select);
}

void EditorTilePallete::changeTileSet(const QString &tileset) {
   printf("mudando paleta para %s\n", qPrintable(tileset));
   currentPallete=QString();
   currentPallete.append(tileset);
   signalPalleteChanged();
   repaint();
}

void EditorTilePallete::mousePressEvent(QMouseEvent *event) {
   QPoint pnt = event->pos();
   selectedTileX = pnt.x()/16;
   selectedTileY = pnt.y()/16;
   dataExchanger->setPalleteX(selectedTileX);
   dataExchanger->setPalleteY(selectedTileY);
   printf("DEBUG.EditorTilePallete::mousePressEvent - PalleteX: %d, palleteY: %d\n", selectedTileX, selectedTileY);
   repaint();
}




QString EditorTilePallete::getPallete() {
    return QString(dataExchanger->getPallete().c_str());
}

//void EditorTilePallete::signalPalleteChanged() {
//   return;
//}


