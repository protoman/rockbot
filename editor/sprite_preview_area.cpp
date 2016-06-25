#include "sprite_preview_area.h"

#include "mediator.h"

sprite_preview_area::sprite_preview_area(QWidget *parent) :
    QWidget(parent)
{
    _sprite_n = 0;
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(updateBG()));

    if (Mediator::get_instance()->enemy_list.size() > 0) {
        _timer->start(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[Mediator::get_instance()->current_sprite_type][_sprite_n].duration);
    }
}

void sprite_preview_area::updateBG()
{
    //std::cout << "sprite_preview_area::updateBG - current_npc: " << Mediator::get_instance()->current_npc_n << ", current_sprite_type: " << Mediator::get_instance()->current_sprite_type << std::endl;
    _sprite_n++;
    if (_sprite_n > ANIM_FRAMES_COUNT-1) {
        _sprite_n = 0;
    }
    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[Mediator::get_instance()->current_sprite_type][_sprite_n].used == false) {
        _sprite_n = 0;
    }
    _timer->stop();
    if (Mediator::get_instance()->enemy_list.size() > 0) {
        _timer->start(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[Mediator::get_instance()->current_sprite_type][_sprite_n].duration);
    }
    repaint();
}

void sprite_preview_area::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    CURRENT_FILE_FORMAT::file_npc temp_npc = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n);

    std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).graphic_filename;

    QPixmap fg_image(filename.c_str());
    std::string _bg_graphic_filename = FILEPATH + "/images/sprites/backgrounds/" + Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).bg_graphic_filename;
    QPixmap bg_image(_bg_graphic_filename.c_str());

    int total_w = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width;
    int total_h = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height;
    // calculate total image size of background exists
    if (!bg_image.isNull()) {
        total_w = bg_image.width();
        total_h = bg_image.height();
    }
    int center_x = this->width()/2 - total_w;
    int center_y = this->height()/2 - total_h;
    int adjust_x = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.x*2;
    int adjust_y = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.y*2;

    int npc_w = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width*2;
    int npc_h = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height*2;

    if (fg_image.isNull() == false && fg_image.width() > 0) {
        fg_image = fg_image.scaled(fg_image.width()*2, fg_image.height()*2);
        QRectF target(QPoint(center_x+adjust_x, center_y+adjust_y), QSize(npc_w, npc_h));
        QRectF source(QPoint(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[Mediator::get_instance()->current_sprite_type][_sprite_n].sprite_graphic_pos_x * npc_w, 0), QSize(npc_w, npc_h));
        painter.drawPixmap(target, fg_image, source);
    }

    if (bg_image.isNull() == false && bg_image.width() > 0) {
        bg_image = bg_image.scaled(bg_image.width()*2, bg_image.height()*2);
        QRectF target(QPoint(center_x, center_y), QSize(bg_image.width()*2, bg_image.height()*2));
        QRectF source(QPoint(0, 0), QSize(bg_image.width()*2, bg_image.height()*2));
        painter.drawPixmap(target, bg_image, source);
    }
    // draw hit-area
    painter.setBrush(QColor(255, 0, 0, 50));
    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w != 0 && Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h != 0) {
        int hitx = center_x+Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.x*2;
        int hity = center_y+adjust_y+Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.y*2;
        int hitw = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w*2;
        int hith = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h*2;

        //std::cout << "hitx: " << hitx << ", hity: " << hity << ", hitw: " << hitw << ", hith: " << hith << std::endl;
        painter.drawRect(hitx, hity, hitw, hith);
    }

    // draw attack_position, if set
    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).attack_arm_pos.x > 0 || Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).attack_arm_pos.y > 0) {
        painter.setPen(QPen(QColor(0, 150, 0), 2, Qt::DashLine));
        int attack_x = center_x + npc_w - (adjust_x + Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).attack_arm_pos.x*2);
        //std::cout << ">>>>>>>>> attack_x: " << attack_x << ", npc_w: " << npc_w << std::endl;
        int attack_y = center_y + adjust_y + Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).attack_arm_pos.y*2;
        // vertical line
        painter.drawLine(attack_x, (center_y + adjust_y), attack_x, (center_y + adjust_y + npc_h));
        // horizontal line
        painter.drawLine((center_x + adjust_x), attack_y, (center_x + adjust_x + npc_w), attack_y);
    }

    // FRAME HITBOX
    painter.setPen(QPen(QColor(0, 0, 255), 1, Qt::DashLine));
    int ini_x = center_x + temp_npc.sprites[Mediator::get_instance()->current_sprite_type][Mediator::get_instance()->current_sprite_selection].collision_rect.x*2;
    int end_x = ini_x + temp_npc.sprites[Mediator::get_instance()->current_sprite_type][Mediator::get_instance()->current_sprite_selection].collision_rect.w*2;

    int ini_y = center_y + temp_npc.sprites[Mediator::get_instance()->current_sprite_type][Mediator::get_instance()->current_sprite_selection].collision_rect.y*2;
    int end_y = ini_y + temp_npc.sprites[Mediator::get_instance()->current_sprite_type][Mediator::get_instance()->current_sprite_selection].collision_rect.h*2;

    // horizontal lines
    painter.drawLine(ini_x, ini_y, end_x, ini_y);
    painter.drawLine(ini_x, end_y, end_x, end_y);
    // vertical lines
    painter.drawLine(ini_x, ini_y, ini_x, end_y);
    painter.drawLine(end_x, ini_y, end_x, end_y);

}

