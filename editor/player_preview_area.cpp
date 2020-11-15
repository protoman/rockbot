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

    replace_colors();

}

void player_preview_area::replace_colors()
{

    _colored_sprites = _original_sprites.copy();


    int player_n = Mediator::get_instance()->current_player;
    int weapon_n = Mediator::get_instance()->current_weapon;

    if (weapon_n < 0) {
        return;
    }

    st_color color1 = Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color1;
    st_color color2 = Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color2;
    st_color color3 = Mediator::get_instance()->player_list_v3_1[player_n].weapon_colors[weapon_n].color3;

    QColor replace_color1((int)color1.r, (int)color1.g, (int)color1.b);
    QColor replace_color2((int)color2.r, (int)color2.g, (int)color2.b);
    QColor replace_color3((int)color3.r, (int)color3.g, (int)color3.b);

    QColor key_color1(55, 255, 0);
    QColor key_color2(255, 0, 255);
    QColor key_color3(0, 255, 255);

    int color_count = _original_sprites.colorCount();
    for (int i=0; i<color_count; i++) {
        QRgb table_color = _colored_sprites.color(i);

        if (table_color == key_color1.rgb()) {
            _colored_sprites.setColor(i, replace_color1.rgb());
        } else if (table_color == key_color2.rgb()) {
            _colored_sprites.setColor(i, replace_color2.rgb());
        } else if (table_color == key_color3.rgb()) {
            _colored_sprites.setColor(i, replace_color3.rgb());
        }

    }

    // convert back from qimage to qpixmap
    QPixmap temp_pixmap = temp_pixmap.fromImage(_colored_sprites);
    QColor transparent_mask_color = QColor(75, 125, 125);
    QPixmap player_mask = temp_pixmap.createMaskFromColor(qRgb(75, 125, 125), Qt::MaskInColor);
    temp_pixmap.setMask(player_mask);
    _colored_sprites = temp_pixmap.toImage();


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


    this->resize(_colored_sprites.size());
	myParent->adjustSize();

    target = QRectF(QPoint(0, 0), QSize(_colored_sprites.width(), _colored_sprites.height()));
    source = QRectF(QPoint(0, 0), QSize(_colored_sprites.width(), _colored_sprites.height()));
    painter.drawImage(target, _colored_sprites, source);

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
       for (i=0; i<this->height(); i=i+inc) {
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
