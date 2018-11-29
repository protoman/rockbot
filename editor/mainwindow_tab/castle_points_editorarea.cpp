#include "castle_points_editorarea.h"

castle_points_editorarea::castle_points_editorarea(QWidget *parent)
{
    myParent = parent;
    image = nullptr;
}

castle_points_editorarea::~castle_points_editorarea()
{

}

void castle_points_editorarea::set_current_point(int current)
{
    currentPoint = current;
}

void castle_points_editorarea::load_image()
{
    QString filename;
    filename = QString(FILEPATH.c_str()) + QString("/images/backgrounds/castle.png");

    image = new QPixmap(filename);
    if (image->isNull()) {
       printf("castle_points_editorarea::paintEvent::ERROR - Could not load image file '%s'\n", qPrintable(filename));
    } else {
        std::cout << "+++++++++++ LOADED IMAGE, w[" << image->size().width() << "][" << image->size().height() << "]" << std::endl;
        this->resize(image->size());
        myParent->adjustSize();
    }
}

void castle_points_editorarea::paintEvent(QPaintEvent *event)
{
    if (image == nullptr) {
        load_image();
        return;
    }
    QPainter painter(this);
    QRectF target(QPoint(0, 0), image->size());
    QRectF source(QPoint(0, 0), image->size());
    painter.drawPixmap(target, *image, source);
    painter.setOpacity(1);

    for (int i=0; i<CASTLE_STAGES_MAX; i++) {
        st_position point = Mediator::get_instance()->points_castle1.points[i];
        if (i == currentPoint) {
            painter.setBrush(Qt::red);
        } else {
            painter.setBrush(Qt::yellow);
        }
        painter.drawEllipse(point.x, point.y, 8, 8);
    }

    //painter.drawEllipse(dest_x, dest_y, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);

}


