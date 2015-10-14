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

int player_preview_area::get_colorn_from_rgb(QImage &image, int r, int g, int b)
{
	QColor temp_key_color1(r, g, b);
	for (int i=0; i<image.colorCount(); i++) {
		if (temp_key_color1.rgb() == image.color(i)) {
			return i;
		}
	}
	return 0;
}

void player_preview_area::paintEvent(QPaintEvent *) {
	QPainter painter(this);
    QLineF line;
    QRectF target, source;
	int i;
	//printf("DEBUG - EditorArea.paintEvent\n");
	// draw the picked tile in the tileset
    //printf("DEBUG.npcPreviewArea::paintEvent - filename: %s\n", Mediator::get_instance()->addNpcFilename);
    QString filename = QString(FILEPATH.c_str()) + QString("/images/sprites/") + QString(Mediator::get_instance()->player_graphics_data.graphics_filename.c_str());
	QImage image(filename);
    if (image.isNull() == true || image.width() <= 0) {
		return;
	}
	image = image.scaled(image.width()*2, image.height()*2+1);

	/// @TODO: player_n
	/// @TODO: get color_n from rgb
    int key_n1 = get_colorn_from_rgb(image, COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
    int key_n2 = get_colorn_from_rgb(image, COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
    int key_n3 = get_colorn_from_rgb(image, COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

    if (Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r != -1) {
        QColor temp_color1(Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b);
		image.setColor(key_n1, temp_color1.rgb());
	}

    if (Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r != -1) {
        QColor temp_color2(Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b);
		image.setColor(key_n2, temp_color2.rgb());
	}

    if (Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r != -1) {
        QColor temp_color3(Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g, Mediator::get_instance()->game_data.players[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b);
		image.setColor(key_n3, temp_color3.rgb());
	}

	this->resize(image.size());
	myParent->adjustSize();

    target = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    painter.drawImage(target, image, source);

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
