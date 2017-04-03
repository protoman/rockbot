#include "castlepointspreviewarea.h"

#include "mediator.h"

CastlePointsPreviewArea::CastlePointsPreviewArea(QWidget *parent) : QWidget(parent)
{

}

CastlePointsPreviewArea::~CastlePointsPreviewArea()
{

}

void CastlePointsPreviewArea::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    std::string filename = FILEPATH + "/images/backgrounds/skull_castle.png";
    QPixmap bg_image(filename.c_str());

    if (bg_image.isNull() == false && bg_image.width() > 0) {
        painter.drawPixmap(0, 0, RES_W, RES_H, bg_image);
    }

    std::string point_filename = FILEPATH + "/images/backgrounds/castle_point.png";
    QPixmap point_img(point_filename.c_str());

    painter.setPen(QColor(255, 255, 255, 255));

    for (int i=0; i<CASTLE_STAGES_MAX-1; i++) {
        painter.drawPixmap(Mediator::get_instance()->castle_data.points[i].x, Mediator::get_instance()->castle_data.points[i].y, point_img);
        char p_name[1];
        sprintf(p_name, "%d", (i+1));
        painter.drawText(Mediator::get_instance()->castle_data.points[i].x+12, Mediator::get_instance()->castle_data.points[i].y+6, p_name);
    }

    std::string skull_filename = FILEPATH + "/images/backgrounds/castle_skull_point.png";
    QPixmap skull_img(skull_filename.c_str());
    painter.drawPixmap(Mediator::get_instance()->castle_data.points[CASTLE_STAGES_MAX-1].x, Mediator::get_instance()->castle_data.points[CASTLE_STAGES_MAX-1].y, skull_img);
    painter.drawText(Mediator::get_instance()->castle_data.points[CASTLE_STAGES_MAX-1].x+16, Mediator::get_instance()->castle_data.points[CASTLE_STAGES_MAX-1].y+10, "5");
}
