#include "colorcycle_map_preview.h"

#include <QBrush>
#include <QDir>
#include <QResource>
#include <QPainter>
#include <QDateTime>
#include <iostream>
#include "defines.h"

#include "../file/format.h"
#include "../file/file_io.h"

#define TIMER_DELAY 10

colorcycle_map_preview::colorcycle_map_preview(QWidget *parent) : QWidget(parent), _current_color1(0), _current_color2(0), _current_color3(0), _zoom(2)
{
	myParent = parent;

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(timer_check()));

    unsigned int now_time = QDateTime::currentMSecsSinceEpoch();
    _timer1 = now_time;
    _timer2 = now_time;
    _timer3 = now_time;

    _timer->start(TIMER_DELAY);
}

int colorcycle_map_preview::get_colorn_from_rgb(QImage &image, int r, int g, int b)
{
	QColor temp_key_color1(r, g, b);
	for (int i=0; i<image.colorCount(); i++) {
		if (temp_key_color1.rgb() == image.color(i)) {
			return i;
		}
	}
	return 0;
}



void colorcycle_map_preview::paintEvent(QPaintEvent *) {
	int i, j;
	QPainter painter(this);
	//QLineF *line;
    QString filename;

    filename = QString("");
    filename.append(QString(QString(FILEPATH.c_str())+"/images/tilesets/default.png"));
    QImage image(filename);
	if (image.isNull()) {
        printf("ERROR: EditorArea::paintEvent - Could not load image file '%s'\n", qPrintable(filename));
	}

    apply_colorcycle(image);

	// draw background color
    painter.fillRect(QRectF(0.0, 0.0, MAP_W*TILESIZE*_zoom, MAP_H*TILESIZE*_zoom), QColor(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].background_color.r, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].background_color.g, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].background_color.b, 255));
	// draw background1
    if (strlen(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[0].filename) > 0) {
        filename.clear();
        filename.append(QString(QString(FILEPATH.c_str())+"/images/map_backgrounds/"+QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[0].filename)));
        QImage bg1_image(filename);
		if (bg1_image.isNull()) {
            printf("ERROR: EditorArea::paintEvent - Could not load bg1 image file '%s'\n", qPrintable(filename));
		} else {
            apply_colorcycle(bg1_image);
			for (int k=0; k<((MAP_W*16)/bg1_image.width())+1; k++) {
				QRectF pos_source(QPoint(0, 0), QSize(bg1_image.width(), bg1_image.height()));
                QRectF pos_dest(QPoint(k*bg1_image.width(), Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[0].adjust_y*_zoom), QSize(bg1_image.width()*_zoom, bg1_image.height()*_zoom));
                painter.drawImage(pos_dest, bg1_image, pos_source);
			}
		}
	}
	// draw background2
    if (strlen(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[1].filename) > 0) {
        filename.clear();
        filename.append(QString(QString(FILEPATH.c_str())+"/images/map_backgrounds/"+QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[1].filename)));
        QImage bg2_image(filename);
		if (bg2_image.isNull()) {
            printf("ERROR: EditorArea::paintEvent - Could not load bg1 image file '%s'\n", qPrintable(filename));
		} else {
            apply_colorcycle(bg2_image);
			for (int k=0; k<((MAP_W*16)/bg2_image.width())+1; k++) {
				QRectF pos_source(QPoint(0, 0), QSize(bg2_image.width(), bg2_image.height()));
                QRectF pos_dest(QPoint(k*bg2_image.width(), Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].backgrounds[1].adjust_y*_zoom), QSize(bg2_image.width()*_zoom, bg2_image.height()*_zoom));
                painter.drawImage(pos_dest, bg2_image, pos_source);
			}
		}
	}

	// draw tiles
	for (i=0; i<MAP_W; i++) {
		for (j=0; j<MAP_H; j++) {
			// level one
            if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile1.x != -1 && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile1.y != -1) {
				QRectF target(QPoint(i*16*_zoom, j*16*_zoom), QSize(16*_zoom, 16*_zoom));
                QRectF source(QPoint((Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile1.x*16), (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile1.y*16)), QSize(16, 16));
				painter.drawImage(target, image, source);
			}
			// leval 3
            if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile3.x != -1 && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile3.y != -1) {
				QRectF target(QPoint(i*16*_zoom, j*16*_zoom), QSize(16*_zoom, 16*_zoom));
                QRectF source(QPoint((Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile3.x*16), (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].maps[Mediator::get_instance()->currentMap].tiles[i][j].tile3.y*16)), QSize(16, 16));
				painter.drawImage(target, image, source);
			}
		}
	}
	QSize resizeMe(MAP_W*16*_zoom, MAP_H*16*_zoom);
	this->resize(resizeMe);
	myParent->adjustSize();

}



bool colorcycle_map_preview::color1_next(unsigned int now_time)
{
    //std::cout << ">> colorcycle_map_preview::color1_next - now_time: " << now_time << ", _timer1: " << _timer1 << std::endl;
    if (now_time < _timer1) { // not reached timer1 yet
        return false;
    }
    if (Mediator::get_instance()->show_colorcycle1 == true && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color1+1][0].r != -1) {
		_current_color1++;
	} else {
		_current_color1 = 0;
	}
    _timer1 = _timer1 + Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.duration[_current_color1][0];
    return true;
}

bool colorcycle_map_preview::color2_next(unsigned int now_time)
{
    if (now_time < _timer2) { // not reached timer2 yet
        return false;
    }
    if (Mediator::get_instance()->show_colorcycle2 == true && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color2+1][1].r != -1) {
		_current_color2++;
	} else {
		_current_color2 = 0;
	}
    _timer2 = _timer2 + Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.duration[_current_color2][1];
    return true;
}

bool colorcycle_map_preview::color3_next(unsigned int now_time)
{
    if (now_time < _timer3) { // not reached timer3 yet
        return false;
    }
    if (Mediator::get_instance()->show_colorcycle3 == true && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color3+1][2].r != -1) {
		_current_color3++;
	} else {
		_current_color3 = 0;
	}
    _timer3 = _timer3 + Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.duration[_current_color3][2];
    return true;
}

void colorcycle_map_preview::apply_colorcycle(QImage& image)
{
    st_color key_color1(55, 255, 0);
    int key_n1 = get_colorn_from_rgb(image, key_color1.r, key_color1.g, key_color1.b);

    st_color key_color2(255, 0, 255);
    int key_n2 = get_colorn_from_rgb(image, key_color2.r, key_color2.g, key_color2.b);

    st_color key_color3(0, 255, 255);
    int key_n3 = get_colorn_from_rgb(image, key_color3.r, key_color3.g, key_color3.b);

    //std::cout << "key_n1: " << key_n1 << ", " << ", key_n2: " << key_n2 << ", key_n3: " << key_n3 << std::endl;

    st_color color1 = Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color1][0];
    if (color1.r != -1) {
        QColor temp_color1(color1.r, color1.g, color1.b);
        image.setColor(key_n1, temp_color1.rgb());
    }

    st_color color2 = Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color2][1];
    if (color2.r != -1) {
        QColor temp_color2(color2.r, color2.g, color2.b);
        image.setColor(key_n2, temp_color2.rgb());
    }

    st_color color3 = Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].colorcycle.color[_current_color3][2];
    //std::cout << "color3: " << color3.r << "," << color3.g << "," << color3.b << std::endl;
    if (color3.r != -1) {
        QColor temp_color3(color3.r, color3.g, color3.b);
        image.setColor(key_n3, temp_color3.rgb());
    }
}

void colorcycle_map_preview::timer_check()
{
    //std::cout << "colorcycle_map_preview::timer_check" << std::endl;
    unsigned int now_time = QDateTime::currentMSecsSinceEpoch();
    bool update1 = color1_next(now_time);
    bool update2 = color2_next(now_time);
    bool update3 = color3_next(now_time);
    if (update1 == true || update2 == true || update3 == true) {
        repaint();
    }
    _timer->stop();
    _timer->start(TIMER_DELAY);

}
