#include "framespreviewarea.h"
#include <iostream>
#include <algorithm>

framesPreviewArea::framesPreviewArea(QWidget *parent) : QWidget(parent), graphic_filename(""), img_grid_w(16), img_grid_h(16) {
    myParent = parent;
	editor_selectedTileX = 0;
	editor_selectedTileY = 0;
	parent->resize(QSize(999, 200));
    this->show();
}

void framesPreviewArea::set_graphicfile(std::string filename)
{
    graphic_filename = filename;
}

std::string framesPreviewArea::get_graphicfile()
{
    return graphic_filename;
}

void framesPreviewArea::set_bg_graphicfile(std::string filename)
{
    _bg_graphic_filename = filename;
}

void framesPreviewArea::set_grid_w(int new_w)
{
	img_grid_w = new_w;
	repaint();
}

void framesPreviewArea::set_grid_h(int new_h)
{
	img_grid_h = new_h;
	repaint();
}

void framesPreviewArea::set_sprite_pos(st_position s_pos)
{
	_sprites_pos.x = s_pos.x;
    _sprites_pos.y = s_pos.y;
}


void framesPreviewArea::paintEvent(QPaintEvent *) {
	QPainter painter(this);
    QLineF line;
    QRectF target, source;
	int i;

	if (graphic_filename.find(".png") == std::string::npos) {
		return;
	}

    QPixmap image(graphic_filename.c_str());
    QPixmap bg_image(_bg_graphic_filename.c_str());

    int sprite_x = _sprites_pos.x;
    int sprite_y = _sprites_pos.y;
    int bg_x = 0;
    int bg_y = 0;
    if (_sprites_pos.x < 0) {
        sprite_x = 0;
        bg_x = abs(_sprites_pos.x);
    }
    if (_sprites_pos.y < 0) {
        sprite_y = 0;
        bg_y = abs(_sprites_pos.y);
    }



    if (bg_image.isNull() == false && bg_image.width() > 0) {
        bg_image_w = bg_image.width();
        bg_image_h = bg_image.height();
        bg_image = bg_image.scaled(bg_image.width()*2, bg_image.height()*2);
    }


    if (image.isNull() == true || image.width() <= 0) {
        return;
    }
    image_w = image.width();
    image_h = image.height();
    image = image.scaled(image.width()*2, image.height()*2);
    if (_bg_graphic_filename.length() > 0) {
		this->resize(QSize(std::max(image.width(), bg_image.width())+1, std::max(image.height(), bg_image.height())+1));
	} else {
		this->resize(QSize(image.width()+1, image.height()+1));
    }
    for (i=0; i<=this->width()/(img_grid_w*2); i++) {
        target = QRectF(QPoint(i*bg_image.width()+bg_x, bg_y), QSize(bg_image.width(), bg_image.height()));
        source = QRectF(QPoint(0, 0), QSize(bg_image.width(), bg_image.height()));
        painter.drawPixmap(target, bg_image, source);
    }

	myParent->adjustSize();

    target = QRectF(QPoint(sprite_x, sprite_y), QSize(image.width(), image.height()));
    source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    painter.drawPixmap(target, image, source);

	painter.setPen(QColor(0, 120, 0));
    for (i=0; i<=this->width(); i=i+img_grid_w*2) {
		// linhas verticais
        line = QLineF(i, 0, i, this->height());
        painter.drawLine(line);
	}
    for (i=0; i<this->height(); i=i+img_grid_h*2) {
		// linhas horizontais
        line = QLineF(0, i, this->width(), i);
        painter.drawLine(line);
	}

	int posX = editor_selectedTileX*img_grid_w*2;
    int posY = editor_selectedTileY*img_grid_h*2;
	// paint the selected frame
	painter.setPen(QColor(255, 0, 0));
    line = QLineF(posX, posY, posX+img_grid_w*2, posY);
    painter.drawLine(line);
    line = QLineF(posX, posY, posX, posY+img_grid_h*2);
    painter.drawLine(line);
    line = QLineF(posX, posY+img_grid_h*2, posX+img_grid_w*2, posY+img_grid_h*2);
    painter.drawLine(line);
    line = QLineF(posX+img_grid_w*2, posY+img_grid_h*2, posX+img_grid_w*2, posY);
    painter.drawLine(line);
}


void framesPreviewArea::mousePressEvent(QMouseEvent *event) {
	printf(">> INSIDE npcPreviewArea::mousePressEvent, editMode: %d, editTool: %d\n", Mediator::get_instance()->editMode, Mediator::get_instance()->editTool);

	QPoint pnt = event->pos();
	// checks if click was inside the image
	if (pnt.x() <= image_w*2 && pnt.y() <= image_h*2) {
		editor_selectedTileX = pnt.x()/img_grid_w/2;
		editor_selectedTileY = pnt.y()/img_grid_h/2;
		printf(">> npcPreviewArea::mousePressEvent - x: %d, y: %d\n", editor_selectedTileX, editor_selectedTileY);
		Mediator::get_instance()->setPalleteX(editor_selectedTileX);
		Mediator::get_instance()->setPalleteY(editor_selectedTileY);
		emit clickedIn();
		repaint();
	}
}

int framesPreviewArea::get_cursor_pos()
{
	return editor_selectedTileX;
}
