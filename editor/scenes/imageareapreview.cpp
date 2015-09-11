#include <QPainter>
#include <iostream>

#include "imageareapreview.h"

#define IMAGE_SCALE 2

extern char EDITOR_FILEPATH[512];

ImageAreaPreview::ImageAreaPreview(QWidget *parent) : QWidget(parent)
{
    graphic_filename = "";
}

void ImageAreaPreview::setImageFilename(QString name)
{
    graphic_filename = QString(EDITOR_FILEPATH) + QString("/data/images/scenes/") + name;
    std::cout << ">> ImageAreaPreview::setImageFilename: " << graphic_filename.toStdString() << std::endl;
    x = 0;
    y = 0;
    w = 0;
    h = 0;
}

void ImageAreaPreview::set_x(int set)
{
    x = set;
}

void ImageAreaPreview::set_y(int set)
{
    y = set;
}

void ImageAreaPreview::set_w(int set)
{
    w = set;
}

void ImageAreaPreview::set_h(int set)
{
    h = set;
}

void ImageAreaPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QLineF line;
    QRectF target, source;

    if (graphic_filename.length() == 0) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #1" << std::endl;
        return;
    }
    if (graphic_filename.toStdString().find(".png") == std::string::npos) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #3" << std::endl;
        return;
    }

    QPixmap image(graphic_filename.toStdString().c_str());

    if (image.isNull() == true || image.width() <= 0) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #4" << std::endl;
        return;
    }

    if (w == 0) {
        // @TODO: isto deve gerar um SIGNAL que a UI vai receber para atualizar o valor na spinbox
        w = image.width();
        emit on_image_w_changed(w);
    }
    if (h == 0) {
        // @TODO: isto deve gerar um SIGNAL que a UI vai receber para atualizar o valor na spinbox
        h = image.height();
        emit on_image_h_changed(h);
    }

    target = QRectF(QPoint(0, 0), QSize(image.width()*IMAGE_SCALE, image.height()*IMAGE_SCALE));
    source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
    painter.drawPixmap(target, image, source);

    painter.setPen(QColor(255, 0, 0));
    // linha topo
    line = QLineF(x*IMAGE_SCALE, y*IMAGE_SCALE, (x+w)*IMAGE_SCALE, y*IMAGE_SCALE);
    painter.drawLine(line);
    // linha baixo
    line = QLineF(x*IMAGE_SCALE, (y+h)*IMAGE_SCALE, (x+w)*IMAGE_SCALE, (y+h)*IMAGE_SCALE);
    painter.drawLine(line);
    // linha esquerda
    line = QLineF(x*IMAGE_SCALE, y*IMAGE_SCALE, x*IMAGE_SCALE, (y+h)*IMAGE_SCALE);
    painter.drawLine(line);
    // linha direita
    line = QLineF((x+w)*IMAGE_SCALE, y*IMAGE_SCALE, (x+w)*IMAGE_SCALE, (y+h)*IMAGE_SCALE);
    painter.drawLine(line);



}

