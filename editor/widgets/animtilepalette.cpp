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
    for (int i=0; i<Mediator::get_instance()->anim_block_list.size(); i++) {
        std::string filename = FILEPATH + "/images/tilesets/anim/" + std::string(Mediator::get_instance()->anim_block_list.at(i).filename);
        if (filename.find(".png") == std::string::npos) {
            continue;
        }
        QPixmap image(QString(filename.c_str()));
        image = image.scaled(image.width()*2, image.height()*2);
        image_list.push_back(image);
    }
}

void animTilePalette::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    for (int i=0; i<image_list.size(); i++) {
        if (image_list.at(i).isNull() == false) {
            QRectF target(QPoint(i*TILESIZE*2, 0), QSize(TILESIZE*2, TILESIZE*2));
            QRectF source(QPoint(0, 0), QSize(TILESIZE*2, TILESIZE*2));
            painter.drawPixmap(target, image_list.at(i), source);
            this->resize(image_list.at(i).size());
            myParent->adjustSize();
        }
    }
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
    //printf("DEBUG.EditorTilePallete::mousePressEvent - PalleteX: %d, palleteY: %d\n", selectedTileX, selectedTileY);

    Mediator::get_instance()->selectedAnimTileset = selectedTileX;

    repaint();
}

void animTilePalette::changeTileSet(const QString &tileset)
{

}

