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
    std::string graphic_filename(Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).graphic_filename);
    if (graphic_filename.length() == 0) {
        projectile_filename = FILEPATH + "/images/projectiles/projectile_normal.png";
	} else {
        projectile_filename = FILEPATH + "/images/projectiles/" + Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).graphic_filename;
	}

	QPixmap image(projectile_filename.c_str());
    if (image.isNull() == true || image.width() <= 0) {
		std::cout << "projectilePreviewArea::paintEvent - Could not load image file '" << Mediator::get_instance()->addProjectileFilename << "'" << std::endl;
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

	//printf("projectilePreviewArea::paintEvent - gSize: %d\n", Mediator::get_instance()->projectileGraphicSize_w);

   painter.setPen(QColor(0, 120, 0));
   int step = Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).size.width*2;
   int max = this->width()+1;
   if (step < 1) {
       step = 1;
   }
   for (i=0; i<=max; i=i+step) {
	  // linhas verticais
      line = QLineF(i, 0, i, this->height()+100);
      painter.drawLine(line);
   }
   max = this->height()+1;
   step = Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).size.height*2;
   if (step < 1) {
       step = 1;
   }
   for (i=0; i<max; i=i+step) {
	  // linhas horizontais
      line = QLineF(0, i, this->width()+100, i);
      painter.drawLine(line);
   }
}


