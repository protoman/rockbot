#include "animtilepalette.h"
#include "mediator.h"

#include <QPainter>
#include <QMouseEvent>


animTilePalette::animTilePalette(QWidget *parent) : QWidget(parent)
{
    myParent = parent;
    selectedTileX = 0;
    selectedTileY = 0;
}

QString animTilePalette::getPallete()
{

}

void animTilePalette::reload()
{
    image_list.clear();
    int max = Mediator::get_instance()->anim_block_list.size();
    std::cout << "ANIMPALETTE::reload::max: " << max << std::endl;
    for (int i=0; i<max; i++) {
        std::string filename = FILEPATH + "/images/tilesets/anim/" + std::string(Mediator::get_instance()->anim_block_list.at(i).filename);
        if (filename.find(".png") == std::string::npos) {
            continue;
        }
        QPixmap image(QString(filename.c_str()));
        image = image.scaled(image.width()*2, image.height()*2);
        image_list.push_back(image);
    }
    this->resize(QSize(max*TILESIZE*2, TILESIZE*2));
    myParent->adjustSize();
}

void animTilePalette::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int row = 0;
    int col = 0;
    for (int i=0; i<image_list.size(); i++) {
        if (image_list.at(i).isNull() == false) {
            QRectF target(QPoint(col*TILESIZE*2, row*TILESIZE*2), QSize(TILESIZE*2, TILESIZE*2));
            QRectF source(QPoint(0, 0), QSize(TILESIZE*2, TILESIZE*2));
            painter.drawPixmap(target, image_list.at(i), source);
            col++;
            if (col > EDITOR_ANIM_PALETE_MAX_COL) {
                row++;
                col = 0;
            }
        }
    }
    this->resize(this->width(), (row+1)*TILESIZE*2);
    // draw the selection marker
    painter.setPen(QColor(255, 0, 0));
    QRectF select(QPoint((selectedTileX*TILESIZE*2), (selectedTileY*TILESIZE*2)), QSize(TILESIZE*2, TILESIZE*2-1));
    painter.drawRect(select);
}

void animTilePalette::mousePressEvent(QMouseEvent *event)
{
    QPoint pnt = event->pos();
    selectedTileX = pnt.x()/(TILESIZE*2);
    selectedTileY = pnt.y()/(TILESIZE*2);
    Mediator::get_instance()->setPalleteX(selectedTileX);
    Mediator::get_instance()->setPalleteY(selectedTileY);

    std::cout << ">>>>>>>>>>>>> animTilePalette::mousePressEvent - x: " << selectedTileX << ", y: " << selectedTileY << std::endl;

    Mediator::get_instance()->selectedAnimTileset = selectedTileX + (selectedTileY * EDITOR_ANIM_PALETE_MAX_COL);
    if (selectedTileY > 0) {
        Mediator::get_instance()->selectedAnimTileset++;
    }

    repaint();
}

void animTilePalette::changeTileSet(const QString &tileset)
{

}

