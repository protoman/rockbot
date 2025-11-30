#include "npc_jump_preview_area.h"

#define DELAY 40

npc_jump_preview_area::npc_jump_preview_area(QWidget *parent) : QWidget(parent) {
    myParent = parent;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateBG()));
    timer->start(DELAY);
    parent->resize(QSize(320, 240));
    this->show();

}

void npc_jump_preview_area::set_graphicfile(std::string filename)
{
    graphic_filename = filename;

    if (graphic_filename.find(".png") != std::string::npos) {
        move_speed = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).speed;
        if (move_speed == 1) { // TODO: more adjusts to make jump faster or slower depending on distance and speed
            xinx_multiplier = 4;
        }
        int _dest_point_x = calc_jump_pos_x(move_speed);
        parabola_arc = new trajectory_parabola(_dest_point_x);
        reset_pos();
    }
}

std::string npc_jump_preview_area::get_graphicfile()
{
    return graphic_filename;
}

void npc_jump_preview_area::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRectF target, source;


    if (graphic_filename.find(".png") == std::string::npos) {
        return;
    }

    QPixmap image(graphic_filename.c_str());
    if (image.isNull() == true || image.width() <= 0) {
        return;
    }

    int npc_w = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width;
    int npc_h = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height;


    QBitmap mask = image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
    image.setMask(mask);

    myParent->adjustSize();

    float speed_multi_x = Mediator::get_instance()->enemy_list_extra_data.at(Mediator::get_instance()->current_npc_n).jump_speed_x;
    float speed_multi_y = Mediator::get_instance()->enemy_list_extra_data.at(Mediator::get_instance()->current_npc_n).jump_speed_y;

    position.x += move_speed;
    position.y = 100 - speed_multi_y*parabola_arc->get_y_point(position.x);

    if (position.y <= 100) {
        target = QRectF(QPoint(position.x*speed_multi_x, 100+position.y), QSize(npc_w, npc_h));
        source = QRectF(QPoint(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[Mediator::get_instance()->current_sprite_type][ANIM_TYPE_JUMP].sprite_graphic_pos_x * npc_w, 0), QSize(npc_w, npc_h));
        painter.drawPixmap(target, image, source);
    } else {
        reset_pos();
    }
    // TODO: Draw a border
    painter.setPen(QColor(0, 120, 0));
    QLineF line;
    line = QLineF(0, 0, 0, 240);
    painter.drawLine(line);

    int line_w = 319;
    int line_h = 240;
    line = QLineF(line_w, 0, line_w, line_h);
    painter.drawLine(line);

    line = QLineF(0, 0, line_w, 0);
    painter.drawLine(line);

    line = QLineF(0, 0, line_w, 0);
    painter.drawLine(line);

    line = QLineF(0, line_h-1, line_w, line_h-1);
    painter.drawLine(line);
}

void npc_jump_preview_area::reset_pos()
{
    position = st_float_position(0, 0);
}

int npc_jump_preview_area::calc_jump_pos_x(int distance)
{
    if (distance < 1) {
        distance = 1;
    }
    int dest_point_x = TILESIZE*distance;
    return dest_point_x;
}

void npc_jump_preview_area::updateBG()
{
    repaint();
}

