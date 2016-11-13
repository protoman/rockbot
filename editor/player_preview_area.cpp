#include "player_preview_area.h"
#include <QPainter>
#include <QString>
#include <QImage>

#include "mediator.h"

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
    //_original_sprites = _original_sprites.scaled(_original_sprites.width()*2, _original_sprites.height()*2+1);
    _colored_sprites = _original_sprites.copy();

    replace_colors();

}

void player_preview_area::replace_colors()
{

    std::cout << "### PLAYERPREVIEW::PAINT::START ###" << std::endl;

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

    int color_count = _colored_sprites.colorCount();
    std::cout << "color_count[" << color_count << "]" << std::endl;

    _colored_sprites.setColor(64, replace_color1.rgb());
    _colored_sprites.setColor(65, replace_color2.rgb());
    _colored_sprites.setColor(66, replace_color3.rgb());

    /*
    for (int i=0; i<color_count; i++) {
        QRgb table_color = _colored_sprites.color(i);
        std::cout << "table_color[" << i << "][" << table_color << "]" << std::endl;


        if (table_color == replace_color1.rgb()) {
            std::cout << "replace-color#1[" << i << "]" << std::endl;
            _colored_sprites.setColor(i, replace_color1.rgb());
        } else if (table_color == replace_color2.rgb()) {
            std::cout << "replace-color#2[" << i << "]" << std::endl;
            _colored_sprites.setColor(i, replace_color2.rgb());
        } else if (table_color == replace_color3.rgb()) {
            std::cout << "replace-color#3[" << i << "]" << std::endl;
            _colored_sprites.setColor(i, replace_color3.rgb());
        }

    }
    */

    /*
    for (int x=0; x<_colored_sprites.width(); x++) {
        for (int y=0; y<_colored_sprites.height(); y++) {
            QColor pixel_color(_colored_sprites.pixel(x, y));


            if (pixel_color.red() != 0 || pixel_color.green() != 0 || pixel_color.blue() != 0) {
                //std::cout << "pixel_color[" << x << "][" << y << "]: [" << pixel_color.red() << ", " << pixel_color.green() << ", " << pixel_color.blue() << "]" << std::endl;
            }

            if (color1.r != -1 && color1.g != -1 && color1.b != -1 && pixel_color.red() == COLORKEY1_R && pixel_color.green() == COLORKEY1_G && pixel_color.blue() == COLORKEY1_B) {
                std::cout << "SET-pixel COLOR1[" << x << "][" << y << "] with [" << (int)color1.r << "][" << (int)color1.g << "][" << (int)color1.b << "]" << std::endl;
                _colored_sprites.setPixel(x, y, replace_color1.rgb());
            }
            if (color2.r != -1 && color2.g != -1 && color2.b != -1 && pixel_color.red() == COLORKEY2_R && pixel_color.green() == COLORKEY2_G && pixel_color.blue() == COLORKEY2_B) {
                std::cout << "SET-pixel COLOR2[" << x << "][" << y << "] with [" << (int)color2.r << "][" << (int)color2.g << "][" << (int)color2.b << "]" << std::endl;
                _colored_sprites.setPixel(x, y, replace_color2.rgb());
            }
            if (color3.r != -1 && color3.g != -1 && color3.b != -1 && pixel_color.red() == COLORKEY3_R && pixel_color.green() == COLORKEY3_G && pixel_color.blue() == COLORKEY3_B) {
                std::cout << "SET-pixel COLOR3[" << x << "][" << y << "] with [" << (int)color3.r << "][" << (int)color3.g << "][" << (int)color3.b << "]" << std::endl;
                _colored_sprites.setPixel(x, y, replace_color3.rgb());
            }
        }
    }
    */



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


    /*
    int key_n1 = get_colorn_from_rgb(image, COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
    int key_n2 = get_colorn_from_rgb(image, COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
    int key_n3 = get_colorn_from_rgb(image, COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

    if (Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r != -1) {
        QColor temp_color1(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b);
		image.setColor(key_n1, temp_color1.rgb());
	}

    if (Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r != -1) {
        QColor temp_color2(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b);
		image.setColor(key_n2, temp_color2.rgb());
	}

    if (Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r != -1) {
        QColor temp_color3(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g, Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b);
		image.setColor(key_n3, temp_color3.rgb());
    }
    */

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
