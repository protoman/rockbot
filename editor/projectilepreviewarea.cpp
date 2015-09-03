#include "projectilepreviewarea.h"

projectilePreviewArea::projectilePreviewArea(QWidget *parent) : QWidget(parent) {
    myParent = parent;
	editor_selectedTileX = 0;
	editor_selectedTileY = 0;
	parent->resize(QSize(999, 200));
	this->show();
}

void projectilePreviewArea::paintEvent(QPaintEvent *) {
	QPainter painter(this);
    QLineF line;
    QRectF target, source;
	int i;

	std::string projectile_filename;
	if (dataExchanger->current_projectile == -1) {
		projectile_filename = FILEPATH + "data/images/projectiles/projectile_normal.png";
	} else {
		projectile_filename = FILEPATH + "data/images/projectiles/" + game_data.projectiles[dataExchanger->current_projectile].graphic_filename;
	}

	QPixmap image(projectile_filename.c_str());
    if (image.isNull() == true || image.width() <= 0) {
		std::cout << "projectilePreviewArea::paintEvent - Could not load image file '" << dataExchanger->addProjectileFilename << "'" << std::endl;
		return;
	}
	image_w = image.width();
	image_h = image.height();
	image = image.scaled(image.width()*2, image.height()*2);
	this->resize(QSize(image.size().width()+1, image.size().height()+1));
	myParent->adjustSize();

    target = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    painter.drawPixmap(target, image, source);

	//printf("projectilePreviewArea::paintEvent - gSize: %d\n", dataExchanger->projectileGraphicSize_w);

   painter.setPen(QColor(0, 120, 0));
   for (i=0; i<=this->width()+1; i=i+game_data.projectiles[dataExchanger->current_projectile].size.width*2) {
	  // linhas verticais
      line = QLineF(i, 0, i, this->height()+100);
      painter.drawLine(line);
   }
   for (i=0; i<this->height()+1; i=i+game_data.projectiles[dataExchanger->current_projectile].size.height*2) {
	  // linhas horizontais
      line = QLineF(0, i, this->width()+100, i);
      painter.drawLine(line);
   }
}


