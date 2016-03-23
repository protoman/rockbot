#include "editortilepallete.h"
#include <stdio.h>

EditorTilePallete::EditorTilePallete(QWidget *parent) : QWidget(parent) {
   printf("DEBUG.EditorTilePallete - constructor\n");
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
      printf("DEBUG.Tile - Could not load image file '%s'\n", qPrintable(filename));
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
   signalPalleteChanged();
   repaint();
}

void EditorTilePallete::mousePressEvent(QMouseEvent *event) {
   QPoint pnt = event->pos();
   selectedTileX = pnt.x()/16;
   selectedTileY = pnt.y()/16;
   Mediator::get_instance()->setPalleteX(selectedTileX);
   Mediator::get_instance()->setPalleteY(selectedTileY);
   printf("DEBUG.EditorTilePallete::mousePressEvent - PalleteX: %d, palleteY: %d\n", selectedTileX, selectedTileY);
   repaint();
}




QString EditorTilePallete::getPallete() {
    QString res(Mediator::get_instance()->getPallete().c_str());
    std::cout << "EditorTilePallete::getPallete - res: " << res.toStdString() << std::endl;
    return res;
}

//void EditorTilePallete::signalPalleteChanged() {
//   return;
//}


