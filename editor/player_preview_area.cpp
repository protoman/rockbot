#include "player_preview_area.h"
#include <QPainter>
#include <QString>
#include <QImage>
#include <QBitmap>

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
    _original_sprites = QImage(filename);
    if (_original_sprites.isNull() == true || _original_sprites.width() <= 0) {
        return;
    }
    std::cout << ">>>>>>>>>>>>>>>>> count[" << _original_sprites.colorCount() << "]" << std::endl;
    _original_sprites = _original_sprites.scaled(_original_sprites.width()*PREVIEW_SCALE, _original_sprites.height()*PREVIEW_SCALE+1);

    replace_colors();

}

void player_preview_area::replace_colors()
{

    std::cout << "### PLAYERPREVIEW::PAINT::START ###" << std::endl;

    _colored_sprites = _original_sprites.copy();


    /*
    QRgb color1 = qRgb(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b);
    QRgb color2 = qRgb(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b);
    QRgb color3 = qRgb(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b);

    QRgb key1 = qRgb(COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
    QRgb key2 = qRgb(COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
    QRgb key3 = qRgb(COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);


    std::cout << "key1: " << key1 << ", key2: " << key2 << ", key3: " << key3 << ", w: " << _colored_sprites.width() << ", h: " << _colored_sprites.height() << std::endl;

    for (int x=0; x<_colored_sprites.width(); x++) {
        for (int y=0; y<_colored_sprites.height(); y++) {
            QRgb pixel_color = _colored_sprites.pixel(x, y);
            std::cout << "pixel_color[" << x << "][" << y << "]: [" << pixel_color << "]" << std::endl;

            if (pixel_color == key1 && Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r != -1) {
                _colored_sprites.setPixel(x, y, color1);
            }
            if (pixel_color == key2 && Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r != -1) {
                _colored_sprites.setPixel(x, y, color2);
            }
            if (pixel_color == key3 && Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r != -1) {
                _colored_sprites.setPixel(x, y, color3);
            }
        }
    }

    std::cout << "### PLAYERPREVIEW::PAINT::END ###" << std::endl;
    */

    int player_n = Mediator::get_instance()->current_player;
    int weapon_n = Mediator::get_instance()->current_weapon;

    if (weapon_n < 0) {
        return;
    }

    st_color color1 = Mediator::get_instance()->player_list[player_n].weapon_colors[weapon_n].color1;
    st_color color2 = Mediator::get_instance()->player_list[player_n].weapon_colors[weapon_n].color2;
    st_color color3 = Mediator::get_instance()->player_list[player_n].weapon_colors[weapon_n].color3;

    QColor replace_color1((int)color1.r, (int)color1.g, (int)color1.b);
    QColor replace_color2((int)color2.r, (int)color2.g, (int)color2.b);
    QColor replace_color3((int)color3.r, (int)color3.g, (int)color3.b);

    QColor key_color1(55, 255, 0);
    QColor key_color2(255, 0, 255);
    QColor key_color3(0, 255, 255);



    // convert qpixmap to qimage, so we can manipulate colors

    //temp_image.setColor(64, replace_color1.rgb());
    //temp_image.setColor(65, replace_color2.rgb());
    //temp_image.setColor(66, replace_color3.rgb());


    int color_count = _original_sprites.colorCount();
    //std::cout << "replace1[" << replace_color1.rgb() << "], replace2[" << replace_color2.rgb() << "], replace3[" << replace_color3.rgb() << "]" << std::endl;
    for (int i=0; i<color_count; i++) {
        QRgb table_color = _colored_sprites.color(i);
        //std::cout << "table_color[" << i << "][" << table_color << "]" << std::endl;


        if (table_color == key_color1.rgb()) {
            //std::cout << "replace-color#1[" << i << "]" << std::endl;
            _colored_sprites.setColor(i, replace_color1.rgb());
        } else if (table_color == key_color2.rgb()) {
            //std::cout << "replace-color#2[" << i << "]" << std::endl;
            _colored_sprites.setColor(i, replace_color2.rgb());
        } else if (table_color == key_color3.rgb()) {
            //std::cout << "replace-color#3[" << i << "]" << std::endl;
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

    //printf("npcPreviewArea::paintEvent - gSize: %d\n", Mediator::get_instance()->npcGraphicSize_w);

   painter.setPen(QColor(0, 120, 0));
   int inc = Mediator::get_instance()->player_graphics_data.frame_size.width*2;
   //std::cout << "PLAYYERPREVIEWAREA::PAINTEVENT - inc: " << inc << ", limit: " << this->width() << std::endl;
   if (inc > 0) {
       for (i=0; i<=this->width(); i=i+inc) {
          //std::cout << "PLAYYERPREVIEWAREA::PAINTEVENT - LOOP #1 - inc: " << inc << ", limit: " << this->width() << std::endl;
          // linhas verticais
          line = QLineF(i, 0, i, this->height());
          painter.drawLine(line);
       }
   }
   inc = Mediator::get_instance()->player_graphics_data.frame_size.height*2;
   if (inc > 0) {
       for (i=0; i<this->height(); i=i+inc) {
          // linhas horizontais
          //std::cout << "PLAYYERPREVIEWAREA::PAINTEVENT - LOOP #2" << std::endl;
          line = QLineF(0, i, this->width(), i);
          painter.drawLine(line);
       }
   }
}


void player_preview_area::change_player(int player_n)
{
	selected_player = player_n;
}
