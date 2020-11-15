#include "parallaxpreviewarea.h"

#include <QPainter>
#include <QPixmap>
#include <iostream>
#include <QBitmap>

parallaxpreviewarea::parallaxpreviewarea(QWidget *parent) : QWidget(parent)
{
    for (int i=0; i<PARALLAX_LAYERS_MAX; i++) {
        ypos[i] = 0;
        hpos[i] = 0;
    }
}

void parallaxpreviewarea::set_filename(int layer_n, std::string new_filename)
{
    filename[layer_n] = new_filename;
}

void parallaxpreviewarea::set_y(int layer_n, int y)
{
    ypos[layer_n] = y;
}

void parallaxpreviewarea::set_h(int layer_n, int h)
{
    hpos[layer_n] = h;
}

void parallaxpreviewarea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRectF target, source;

    painter.fillRect(0, 0, RES_W, RES_H, QColor(0, 0, 0, 255));

    for (int i=0; i<PARALLAX_LAYERS_MAX; i++) {
        QString graphic_filename = QString(filename[i].c_str());
        if (graphic_filename.length() == 0) {
            // TODO: show error
            return;
        }
        if (graphic_filename.toStdString().find(".png") == std::string::npos) {
            // TODO: show error
            return;
        }

        QPixmap image(graphic_filename.toStdString().c_str());
        QBitmap mask = image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
        image.setMask(mask);


        if (image.isNull() == true || image.width() <= 0) {
            // TODO: show error
            return;
        }

        int w = image.width();
        int h = image.height();
        if (w > RES_W) {
            w = RES_W;
        }
        if (h > RES_H) {
            h = RES_H;
        }

        target = QRectF(QPoint(0, ypos[i]*2), QSize(w*2, h*2));
        source = QRectF(QPoint(0, 0), QSize(w, h));
        painter.drawPixmap(target, image, source);
    }
}
