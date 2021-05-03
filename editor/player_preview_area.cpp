#include "player_preview_area.h"
#include <QPainter>
#include <QString>
#include <QImage>
#include <QBitmap>
#include <QPixmap>

#include "mediator.h"

#define PREVIEW_SCALE 2

player_preview_area::player_preview_area(QWidget *parent) :
QWidget(parent), selected_player(0)
{
    myParent = parent;
}

void player_preview_area::update_sprites()
{
    QString filename = QString(FILEPATH.c_str()) + QString("/images/sprites/") + QString(Mediator::get_instance()->player_graphics_data.graphics_filename.c_str());
    QPixmap sprites = QPixmap(filename);
    if (sprites.isNull() == true || sprites.width() <= 0) {
        return;
    }

    QBitmap mask = sprites.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
    sprites.setMask(mask);
    _original_sprites = sprites.toImage();
    _original_sprites = _original_sprites.scaled(_original_sprites.width()*PREVIEW_SCALE, _original_sprites.height()*PREVIEW_SCALE+1);

    _colored_sprites = _original_sprites.copy();

    repaint();
}



void player_preview_area::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QLineF line;
    QRectF target, source;
    int i;

    if (_original_sprites.isNull()) {
        return;
    }

    int player_n = Mediator::get_instance()->current_player;
    int weapon_n = Mediator::get_instance()->current_weapon;

    if (weapon_n < 0) {
        std::cout << ">>>>>>>>>>> ERROR #1 - weapon_n[" << weapon_n << "]" << std::endl;
        return;
    }

    this->resize(_colored_sprites.size().width(), _colored_sprites.size().height()*2);
    myParent->adjustSize();

    target = QRectF(QPoint(0, 0), QSize(_colored_sprites.width(), _colored_sprites.height()));
    source = QRectF(QPoint(0, 0), QSize(_colored_sprites.width(), _colored_sprites.height()));
    painter.drawImage(target, _colored_sprites, source);

    // colorized copy under the first one
    // COLOR_KEY_GREEN, COLOR_KEY_PURPLE, COLOR_KEY_CYAN

    QColor weaponColor1 = QColor(Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color1.r, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color1.g, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color1.b);
    QColor weaponColor2 = QColor(Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color2.r, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color2.g, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color2.b);
    QColor weaponColor3 = QColor(Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color3.r, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color3.g, Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color3.b);

    QImage cloned = _colored_sprites.copy();
    source = QRectF(QPoint(0, 0), QSize(cloned.width(), cloned.height()));
    target = QRectF(QPoint(0, cloned.height()), QSize(cloned.width(), cloned.height()));
    for(int y = 0; y < cloned.height(); y++) {
      for(int x= 0; x < cloned.width(); x++) {
          QColor pixelColor = cloned.pixelColor(x, y);
          if (pixelColor == colorKey1) {
              cloned.setPixelColor(x, y, weaponColor1);
          } else if (pixelColor == colorKey2) {
              cloned.setPixelColor(x, y, weaponColor2);
          } else if (pixelColor == colorKey3) {
              cloned.setPixelColor(x, y, weaponColor3);
          }
      }
    }
    painter.drawImage(target, cloned, source);


    painter.setPen(QColor(0, 120, 0));
    int inc = Mediator::get_instance()->player_graphics_data.frame_size.width*2;
    if (inc > 0) {
       for (i=0; i<=this->width(); i=i+inc) {
          // linhas verticais
          line = QLineF(i, 0, i, this->height());
          painter.drawLine(line);
       }
    }
    inc = Mediator::get_instance()->player_graphics_data.frame_size.height*2;
    if (inc > 0) {
       for (i=0; i<this->height()*2; i=i+inc) {
          // linhas horizontais
          line = QLineF(0, i, this->width(), i);
          painter.drawLine(line);
       }
    }
}


void player_preview_area::change_player(int player_n)
{
	selected_player = player_n;
}
