#include "player_preview_area.h"
#include <QPainter>
#include <QString>
#include <QImage>
extern std::string FILEPATH;

#include "mediator.h"
extern Mediator *dataExchanger;

extern CURRENT_FILE_FORMAT::file_game game_data;

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
	//printf("DEBUG.npcPreviewArea::paintEvent - filename: %s\n", dataExchanger->addNpcFilename);
	QString filename = QString(FILEPATH.c_str()) + QString("data/images/sprites/") + QString(dataExchanger->player_graphics_data.graphics_filename.c_str());
	QImage image(filename);
    if (image.isNull() == true || image.width() <= 0) {
		return;
	}
	image = image.scaled(image.width()*2, image.height()*2+1);

	/// @TODO: player_n
	/// @TODO: get color_n from rgb
	int key_n1 = get_colorn_from_rgb(image, game_data.players[dataExchanger->current_player].color_keys[0].r, game_data.players[dataExchanger->current_player].color_keys[0].g, game_data.players[dataExchanger->current_player].color_keys[0].b);
	int key_n2 = get_colorn_from_rgb(image, game_data.players[dataExchanger->current_player].color_keys[1].r, game_data.players[dataExchanger->current_player].color_keys[1].g, game_data.players[dataExchanger->current_player].color_keys[1].b);
	int key_n3 = get_colorn_from_rgb(image, game_data.players[dataExchanger->current_player].color_keys[2].r, game_data.players[dataExchanger->current_player].color_keys[2].g, game_data.players[dataExchanger->current_player].color_keys[2].b);

	if (game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r != -1) {
		QColor temp_color1(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.g, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.b);
		image.setColor(key_n1, temp_color1.rgb());
	}

	if (game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r != -1) {
		QColor temp_color2(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.g, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.b);
		image.setColor(key_n2, temp_color2.rgb());
	}

	if (game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r != -1) {
		QColor temp_color3(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.g, game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.b);
		image.setColor(key_n3, temp_color3.rgb());
	}

	this->resize(image.size());
	myParent->adjustSize();

    target = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    painter.drawImage(target, image, source);

	//printf("npcPreviewArea::paintEvent - gSize: %d\n", dataExchanger->npcGraphicSize_w);

   painter.setPen(QColor(0, 120, 0));
   int inc = dataExchanger->player_graphics_data.frame_size.width*2;
   //std::cout << "PLAYYERPREVIEWAREA::PAINTEVENT - inc: " << inc << ", limit: " << this->width() << std::endl;
   if (inc > 0) {
       for (i=0; i<=this->width(); i=i+inc) {
          //std::cout << "PLAYYERPREVIEWAREA::PAINTEVENT - LOOP #1 - inc: " << inc << ", limit: " << this->width() << std::endl;
          // linhas verticais
          line = QLineF(i, 0, i, this->height());
          painter.drawLine(line);
       }
   }
   inc = dataExchanger->player_graphics_data.frame_size.height*2;
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
