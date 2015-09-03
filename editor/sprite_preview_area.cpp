#include "sprite_preview_area.h"

#include "mediator.h"
extern Mediator *dataExchanger;

sprite_preview_area::sprite_preview_area(QWidget *parent) :
    QWidget(parent)
{
    _sprite_n = 0;
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(updateBG()));
    _timer->start(game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].duration);
}

void sprite_preview_area::updateBG()
{
	//std::cout << "sprite_preview_area::updateBG - current_npc: " << dataExchanger->current_npc_n << ", current_sprite_type: " << dataExchanger->current_sprite_type << std::endl;
    _sprite_n++;
    if (_sprite_n > ANIM_FRAMES_COUNT-1) {
        _sprite_n = 0;
    }
    if (game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].used == false) {
        _sprite_n = 0;
    }
    _timer->stop();
    _timer->start(game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].duration);
    repaint();
}

void sprite_preview_area::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    std::string filename = FILEPATH + "/data/images/sprites/enemies/" + game_data.game_npcs[dataExchanger->current_npc_n].graphic_filename;

    QPixmap fg_image(filename.c_str());
    std::string _bg_graphic_filename = FILEPATH + "/data/images/sprites/backgrounds/" + game_data.game_npcs[dataExchanger->current_npc_n].bg_graphic_filename;
    QPixmap bg_image(_bg_graphic_filename.c_str());

    int total_w = game_data.game_npcs[dataExchanger->current_npc_n].frame_size.width;
    int total_h = game_data.game_npcs[dataExchanger->current_npc_n].frame_size.height;
    // calculate total image size of background exists
    if (!bg_image.isNull()) {
        total_w = bg_image.width();
        total_h = bg_image.height();
    }
    int center_x = this->width()/2 - total_w;
    int center_y = this->height()/2 - total_h;
    int adjust_x = game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.x*2;
    int adjust_y = game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.y*2;

    //std::cout << "sprite_preview_area - this.w: " << this->width() << ", total_w: " << total_w << std::endl;


    //std::cout << "sprite_preview_area::paintEvent - filename: " << filename << ", img.w: " << fg_image.width() << ", _sprite_n: " << _sprite_n << std::endl;
    if (fg_image.isNull() == false && fg_image.width() > 0) {
        fg_image = fg_image.scaled(fg_image.width()*2, fg_image.height()*2);
        QRectF target(QPoint(center_x+adjust_x, center_y+adjust_y), QSize(game_data.game_npcs[dataExchanger->current_npc_n].frame_size.width*2, game_data.game_npcs[dataExchanger->current_npc_n].frame_size.height*2));
        //std::cout << "center_x: " << center_x << ", center_y: " << center_y << std::endl;
        QRectF source(QPoint(game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].sprite_graphic_pos_x*game_data.game_npcs[dataExchanger->current_npc_n].frame_size.width*2, 0), QSize(game_data.game_npcs[dataExchanger->current_npc_n].frame_size.width*2, game_data.game_npcs[dataExchanger->current_npc_n].frame_size.height*2));
        painter.drawPixmap(target, fg_image, source);
		//game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].colision_rect.h
        //painter.drawRect(center_x+game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].colision_rect.x, center_y+game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].colision_rect.y, game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].colision_rect.w*2, game_data.game_npcs[dataExchanger->current_npc_n].sprites[dataExchanger->current_sprite_type][_sprite_n].colision_rect.h*2);
    }

    if (bg_image.isNull() == false && bg_image.width() > 0) {
        bg_image = bg_image.scaled(bg_image.width()*2, bg_image.height()*2);
        QRectF target(QPoint(center_x, center_y), QSize(bg_image.width()*2, bg_image.height()*2));
        QRectF source(QPoint(0, 0), QSize(bg_image.width()*2, bg_image.height()*2));
        painter.drawPixmap(target, bg_image, source);
    }
    // draw hit-area
    painter.setBrush(QColor(255, 0, 0, 50));
    if (game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w != 0 && game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h != 0) {
        painter.drawRect(center_x+game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.x*2, center_y+adjust_y+game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.y*2, game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w*2, game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h*2);
    }
}

