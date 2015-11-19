#include "animtitle.h"
#include <QPainter>
#include <QString>
#include <QImage>

#include "mediator.h"


animTitle::animTitle(QWidget *parent) : QWidget(parent)
{
    std::cout << "animTitle::CONSTRUCTOR" << std::endl;
    myParent = parent;
    _timer = new QTimer(this);
    _sprite_n = 0;
    connect(_timer, SIGNAL(timeout()), this, SLOT(updateBG()));
    int delay = 100;
    if (Mediator::get_instance()->anim_block_list.size() != 0) {
        delay = Mediator::get_instance()->anim_block_list.at(Mediator::get_instance()->selectedAnimTileset).frame_delay[_sprite_n];
    }
    if (delay < 10) {
        delay = 10;
    }
    _timer->start(delay);

    update_properties();
}

void animTitle::update_properties()
{
    if (Mediator::get_instance()->anim_block_list.size() == 0) {
        return;
    }
    QString filename = QString(FILEPATH.c_str()) + QString("images/tilesets/anim/") + QString(Mediator::get_instance()->anim_block_list.at(Mediator::get_instance()->selectedAnimTileset).filename);

    std::cout << "animTitle::update_properties::filename: " << filename.toStdString() << ", selectedAnimTileset: " << Mediator::get_instance()->selectedAnimTileset << std::endl;

    image = QImage(filename);
    if (image.isNull()) {
        _timer->stop();
        repaint();
        return;
    }
    image = image.scaled(image.width()*2, image.height()*2);

    this->resize(TILESIZE*2, TILESIZE*2);
    myParent->adjustSize();
    max_frames = image.width()/(TILESIZE*2);
    //std::cout << "animTitle::update_properties::max_frames: " << max_frames << std::endl;

    _sprite_n = 0;
    _timer->stop();
    _timer->start(Mediator::get_instance()->anim_block_list.at(Mediator::get_instance()->selectedAnimTileset).frame_delay[_sprite_n]);
    repaint();
}


void animTitle::updateBG()
{
    if (Mediator::get_instance()->anim_block_list.size() == 0) {
        return;
    }
    _sprite_n++;
    if (_sprite_n > ANIM_FRAMES_COUNT-1) {
        _sprite_n = 0;
    }

    if (image.isNull()) {
        _timer->stop();
        return;
    }
    if (_sprite_n >= max_frames) {
        _sprite_n = 0;
    }
    _timer->stop();
    _timer->start(Mediator::get_instance()->anim_block_list.at(Mediator::get_instance()->selectedAnimTileset).frame_delay[_sprite_n]);
    repaint();
}


void animTitle::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QRectF target, source;

    if (image.isNull() == true || image.width() <= 0) {
        painter.fillRect(0, 0, this->width(), this->height(), QColor(0, 0, 0));
        return;
    }

    source = QRectF(QPoint(_sprite_n*(TILESIZE*2), 0), QSize(TILESIZE*2, TILESIZE*2));
    target = QRectF(QPoint(0, 0), QSize(TILESIZE*2, TILESIZE*2));
    painter.drawImage(target, image, source);
}
