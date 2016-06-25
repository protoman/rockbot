#include "npc_edit.h"
#include "ui_npc_edit.h"

#include "common.h"

npc_edit::npc_edit(QWidget *parent) : _data_loading(false), QWidget(parent), _npcedit_tab_selectednpc(0), ui(new Ui::npc_edit)
{
	_data_loading = true;
    _ignore_spritelist_row_changed = false;
	ui->setupUi(this);
	QObject::connect(ui->npc_edit_tab_previewarea, SIGNAL(clickedIn()), this, SLOT(set_npc_frame()));
	_npcedit_tab_selected_weakness_weapon = 0;

    fill_data();
	on_npc_edit_tab_selectnpccombo_currentIndexChanged(0);
	_data_loading = false;
	reload_frame_list(ui->frame_list_selector->currentIndex());
}

npc_edit::~npc_edit()
{
    delete ui;
}

void npc_edit::reload()
{
    _data_loading = true;
    fill_data();
    on_npc_edit_tab_selectnpccombo_currentIndexChanged(0);
    _data_loading = false;
    reload_frame_list(ui->frame_list_selector->currentIndex());
}



void npc_edit::fill_data()
{
    if (FILEPATH.length() == 0) {
        return;
    }
	_data_loading = true;
    common::fill_files_combo("images/sprites/enemies", ui->npc_edit_tab_graphiccombo);

    common::fill_npc_combo(ui->npc_edit_tab_selectnpccombo);
    common::fill_weapons_names_combo(ui->npc_edit_tab_weakness_list);

    if (Mediator::get_instance()->enemy_list.size() > 0) {
        ui->npc_edit_tab_graphiccombo->setCurrentIndex(ui->npc_edit_tab_graphiccombo->findText(QString(Mediator::get_instance()->enemy_list.at(0).graphic_filename)));
        ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/images/sprites/enemies/")+std::string(Mediator::get_instance()->enemy_list.at(0).graphic_filename));
        ui->npc_edit_tab_previewarea->set_sprite_pos(Mediator::get_instance()->enemy_list.at(0).sprites_pos_bg);
        ui->npc_edit_tab_previewarea->repaint();

        _data_loading = true;

        if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).fly_flag == 0) {
            ui->checkBox->setChecked(false);
        } else {
            ui->checkBox->setChecked(true);
        }

        ui->hitarea_x_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.x);
        ui->hitarea_y_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.y);

        if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w != 0) {
            ui->hitarea_w_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w);
        } else {
            ui->hitarea_w_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width);
        }
        if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h != 0) {
            ui->hitarea_h_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h);
        } else {
            ui->hitarea_h_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height);
        }

        ui->respawn_time_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).respawn_delay);

        ui->npc_edit_tab_graphicheight->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height);
        ui->npc_edit_tab_graphicwidth->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width);
        Mediator::get_instance()->npcGraphicSize_h = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height;
        Mediator::get_instance()->npcGraphicSize_w = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width;

        ui->projectileOriginX_spinBox->setValue(Mediator::get_instance()->enemy_list.at(0).attack_arm_pos.x);
        ui->projectileOriginY_spinBox->setValue(Mediator::get_instance()->enemy_list.at(0).attack_arm_pos.y);
    }

}

void npc_edit::add_frame_one()
{
    CURRENT_FILE_FORMAT::file_npc* new_npc_ref = &Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n);
    if (new_npc_ref->sprites[ANIM_TYPE_STAND][0].used == false) {
        new_npc_ref->sprites[ANIM_TYPE_STAND][0].used = true;
        new_npc_ref->sprites[ANIM_TYPE_STAND][0].duration = 100;
        new_npc_ref->sprites[ANIM_TYPE_STAND][0].sprite_graphic_pos_x = 0;
        ui->frame_list_selector->setCurrentIndex(0);
        reload_frame_list(ANIM_TYPE_STAND);
        ui->frameList_listWidget->setCurrentRow(0);
    }
}


void npc_edit::on_npc_edit_tab_selectnpccombo_currentIndexChanged(int index)
{
    if (index == -1) {
        return;
    }
	_data_loading = true;
    Mediator::get_instance()->current_npc_n = index;

	_npcedit_tab_selectednpc = index;

    if (Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }

    ui->npc_edit_tab_canshoot->setChecked(Mediator::get_instance()->enemy_list.at(index).is_ghost);
    std::string image_filename(Mediator::get_instance()->enemy_list.at(index).graphic_filename);
    if (image_filename.length() > 0) {
        ui->npc_edit_tab_graphiccombo->setCurrentIndex(ui->npc_edit_tab_graphiccombo->findText(QString(image_filename.c_str())));
    }
    ui->npc_edit_tab_graphicheight->setValue(Mediator::get_instance()->enemy_list.at(index).frame_size.height);
    ui->npc_edit_tab_graphicwidth->setValue(Mediator::get_instance()->enemy_list.at(index).frame_size.width);

    ui->npc_edit_tab_previewarea->set_grid_w(Mediator::get_instance()->enemy_list.at(index).frame_size.width);
    ui->npc_edit_tab_previewarea->set_grid_h(Mediator::get_instance()->enemy_list.at(index).frame_size.height);

    Mediator::get_instance()->npcGraphicSize_h = Mediator::get_instance()->enemy_list.at(index).frame_size.height;
    Mediator::get_instance()->npcGraphicSize_w = Mediator::get_instance()->enemy_list.at(index).frame_size.width;
    ui->npc_edit_tab_movespeed->setValue(Mediator::get_instance()->enemy_list.at(index).speed);
    ui->npc_edit_tab_NpcHP->setValue(Mediator::get_instance()->enemy_list.at(index).hp.total);
    ui->npc_edit_tab_NpcName->setText(Mediator::get_instance()->enemy_list.at(index).name);
    ui->npc_edit_tab_range->setValue(Mediator::get_instance()->enemy_list.at(index).walk_range);
    ui->npc_edit_tab_shieldtype->setCurrentIndex(Mediator::get_instance()->enemy_list.at(index).shield_type);
	ui->npc_edit_tab_weakness_list->setCurrentIndex(0);
    ui->npc_edit_tab_weakness_points->setValue(Mediator::get_instance()->enemy_list.at(index).weakness[0].damage_multiplier);

    ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/images/sprites/enemies/")+std::string(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).graphic_filename));

    //int bg_pos = ui->bg_graphic_combo->findText(QString(Mediator::get_instance()->enemy_list.at(index).bg_graphic_filename));
    //ui->bg_graphic_combo->setCurrentIndex(bg_pos);
    ui->npc_edit_tab_previewarea->set_bg_graphicfile(FILEPATH+std::string("/images/sprites/backgrounds/")+std::string(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).bg_graphic_filename));
    ui->npc_edit_tab_previewarea->set_sprite_pos(Mediator::get_instance()->enemy_list.at(index).sprites_pos_bg);
    ui->npc_edit_tab_previewarea->repaint();
	reload_frame_list(ui->frame_list_selector->currentIndex());
	ui->sprites_preview_widget->repaint();

    ui->isBoss_checkBox->setChecked(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_boss);
    ui->isSubBoss_checkbox->setChecked(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_sub_boss);

    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).fly_flag == 0) {
        ui->checkBox->setChecked(false);
    } else {
        ui->checkBox->setChecked(true);
    }

    ui->hitarea_x_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.x);
    ui->hitarea_y_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.y);

    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w != 0) {
        ui->hitarea_w_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w);
    } else {
        ui->hitarea_w_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.width);
    }
    if (Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h != 0) {
        ui->hitarea_h_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h);
    } else {
        ui->hitarea_h_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).frame_size.height);
    }

    ui->respawn_time_spinBox->setValue(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).respawn_delay);

    ui->projectileOriginX_spinBox->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_arm_pos.x);
    ui->projectileOriginY_spinBox->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_arm_pos.y);

	_data_loading = false;
}

void npc_edit::on_npc_edit_tab_graphiccombo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    sprintf(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).graphic_filename, "%s", arg1.toStdString().c_str());
    ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/images/sprites/enemies/")+arg1.toStdString());
    add_frame_one();
    ui->npc_edit_tab_previewarea->repaint();
}

void npc_edit::on_npc_edit_tab_graphicwidth_valueChanged(int arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    ui->npc_edit_tab_previewarea->set_grid_w(arg1);
    // if hitarea is zero or old value, update to new one
    _data_loading = true;
    if (ui->hitarea_w_spinBox->value() == 0 || ui->hitarea_w_spinBox->value() == Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width) {
        ui->hitarea_w_spinBox->setValue(arg1);
        if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width == Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w) {
            Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w = arg1;
        }
        ui->npc_edit_tab_previewarea->update();
        reload_frame_list(ui->frame_list_selector->currentIndex());
    }
    _data_loading = false;
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width = arg1;
}

void npc_edit::on_npc_edit_tab_graphicheight_valueChanged(int arg1)
{
    if (_data_loading) {
        return;
    }
    if (Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    ui->npc_edit_tab_previewarea->set_grid_h(arg1);
    // if hitarea is zero or old value, update to new one
    _data_loading = true;
    if (ui->hitarea_h_spinBox->value() == 0 || ui->hitarea_h_spinBox->value() == Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height) {
        ui->hitarea_h_spinBox->setValue(arg1);

        if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height == Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h) {
            std::cout << "### SET collistion H" << std::endl;
            Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h = arg1;
        }

        ui->npc_edit_tab_previewarea->update();
        reload_frame_list(ui->frame_list_selector->currentIndex());
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height = arg1;
    _data_loading = false;
}

void npc_edit::on_npc_edit_tab_NpcName_textChanged(const QString &arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    sprintf(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).name, "%s", arg1.toStdString().c_str());
    QString temp_str = QString("[");
    if (_npcedit_tab_selectednpc < 10) {
        temp_str += QString("0");
    }
    temp_str += QString::number(_npcedit_tab_selectednpc) + QString("] - ") + QString(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).name);
    ui->npc_edit_tab_selectnpccombo->setItemText(_npcedit_tab_selectednpc, temp_str);
}

void npc_edit::on_npc_edit_tab_NpcHP_valueChanged(int arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).hp.total = arg1;
}

void npc_edit::on_npc_edit_tab_canshoot_toggled(bool checked)
{
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).is_ghost = checked;
}

void npc_edit::on_npc_edit_tab_shieldtype_currentIndexChanged(int index)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).shield_type = index;
}

void npc_edit::on_npc_edit_tab_movespeed_valueChanged(int arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).speed = arg1;
}

void npc_edit::on_npc_edit_tab_range_valueChanged(int arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).walk_range = arg1;
}

void npc_edit::on_npc_edit_tab_frametype_currentIndexChanged(int index)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    // @204
    //Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frames[Mediator::get_instance()->getPalleteX()].state = index-1;
}


void npc_edit::set_npc_frame() {
	printf(">> DialogNPCEdit::setNPCFrame - CALLED\n");
    // @204
    //ui->npc_edit_tab_frameduration->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frames[Mediator::get_instance()->getPalleteX()].duration);
    //ui->npc_edit_tab_frametype->setCurrentIndex(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frames[Mediator::get_instance()->getPalleteX()].state+1);
}


void npc_edit::on_bg_graphic_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    if (arg1.length() == 0) {
        //std::cout << "*************** on_bg_graphic_combo_currentIndexChanged - ZERO" << std::endl;
        sprintf(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).bg_graphic_filename, "%s", "");
        ui->npc_edit_tab_previewarea->set_bg_graphicfile("");
    } else {
        //std::cout << "*************** on_bg_graphic_combo_currentIndexChanged - SET to '" << arg1.toStdString() << "'" << std::endl;
        sprintf(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).bg_graphic_filename, "%s", arg1.toStdString().c_str());
        ui->npc_edit_tab_previewarea->set_bg_graphicfile(FILEPATH+std::string("/images/sprites/backgrounds/")+std::string(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).bg_graphic_filename));
    }
}

void npc_edit::on_npc_edit_tab_weakness_list_currentIndexChanged(int index)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    _npcedit_tab_selected_weakness_weapon = index;
    ui->npc_edit_tab_weakness_points->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).weakness[_npcedit_tab_selected_weakness_weapon].damage_multiplier);
}

void npc_edit::on_npc_edit_tab_weakness_points_valueChanged(int arg1)
{
    if (_data_loading || Mediator::get_instance()->enemy_list.size() == 0) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).weakness[_npcedit_tab_selected_weakness_weapon].damage_multiplier = arg1;

}


void npc_edit::on_frame_list_selector_currentIndexChanged(int index)
{
	if (_data_loading == true) {
		return;
	}
    Mediator::get_instance()->current_sprite_type = index;


    _data_loading = true;
    if (ui->frame_list_selector->currentText() == "ATTACK") {
        ui->isAttackFrame_checkBox->setEnabled(true);
    } else {
        ui->isAttackFrame_checkBox->setEnabled(false);
    }
    _data_loading = false;
    reload_frame_list(index);
}

void npc_edit::reload_frame_list(int index)
{
    int current_row = -1;
    if (ui->frameList_listWidget->selectedItems().size() > 0) {
        current_row = ui->frameList_listWidget->currentRow();
    }
	// clean all items from combo
	ui->frameList_listWidget->clear();


	// insert all sprites for the given type into the combo
    //std::cout << ">> Adding frames for NPC[" << Mediator::get_instance()->game_data.game_npcs[i].name << "] - type: " << index << std::endl;
    std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).graphic_filename;
    // @204
    for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
        if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[index][j].used == true) {
            int calc_pos_x = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[index][j].sprite_graphic_pos_x * Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width;
            //std::cout << "NPC[" << Mediator::get_instance()->game_data.game_npcs[i].name << "] - frame.pos_x: " << Mediator::get_instance()->game_data.game_npcs[i].sprites[index][j].sprite_graphic_pos_x << ", cal_pos_x: " << calc_pos_x << std::endl;
            QListWidgetItem* item = new QListWidgetItem;
            QString temp_str(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).name + QString(" (") + QString::number(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[index][j].duration) + QString(")"));
            item->setText(temp_str);
            //Mediator::get_instance()->game_data.game_npcs[i].name + " (" + QString::number(Mediator::get_instance()->game_data.game_npcs[i].sprites[index][j].duration).c_ + ")");
            QPixmap image(filename.c_str());
            if (image.isNull() == false && image.width() > 0) {
                image = image.copy(calc_pos_x, 0, Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width, Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height);
                image = image.scaled(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width*2, Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height*2);
                item->setIcon(image);
            }
            int h = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height*2+4;
            if (h < 24) {
                h = 24;
            }
            item->setSizeHint(QSize(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width*2, h));
            ui->frameList_listWidget->addItem(item);
        }
    }


    if (current_row != -1) {
        _ignore_spritelist_row_changed = true;
        if (ui->frameList_listWidget->count() >= current_row) {
            ui->frameList_listWidget->setCurrentRow(current_row);
        } else if (ui->frameList_listWidget->count() > 0) {
            ui->frameList_listWidget->setCurrentRow(0);
        }
    } else {
        ui->frameList_listWidget->setCurrentRow(0);
    }
    ui->frameList_listWidget->update();
}

void npc_edit::on_pushButton_clicked()
{
    if (ui->frameList_listWidget->selectedItems().size() == 0) {
        return;
    }
    //copy next sprite to current position
    for (int j=ui->frameList_listWidget->currentRow(); j<ANIM_FRAMES_COUNT-1; j++) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][j] = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][j+1];
    }
    // clear last sprite
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ANIM_FRAMES_COUNT-1].used = false;
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_frameUp_clicked()
{
    if (ui->frameList_listWidget->currentRow() == 0) {
        return;
    }
    if (ui->frameList_listWidget->selectedItems().size() == 0) {
        return;
    }
    CURRENT_FILE_FORMAT::st_sprite_data temp = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()-1];
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()-1] = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()];
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()] = temp;
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_frameDown_clicked()
{
    if (ui->frameList_listWidget->selectedItems().size() == 0) {
        return;
    }
    if (ui->frameList_listWidget->currentRow() >= ANIM_FRAMES_COUNT-1) {
        return;
    }
    if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1].used == false) {
        return;
    }
    CURRENT_FILE_FORMAT::st_sprite_data temp = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1];
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1] = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()];
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()] = temp;
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_frameList_listWidget_currentRowChanged(int currentRow)
{
    ui->sprite_duration_spinBox->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][currentRow].duration);
    ui->sprite_collision_x->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][currentRow].collision_rect.x);
    ui->sprite_collision_y->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][currentRow].collision_rect.y);
    ui->sprite_collision_w->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][currentRow].collision_rect.w);
    ui->sprite_collision_h->setValue(Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[ui->frame_list_selector->currentIndex()][currentRow].collision_rect.h);

    if (ui->frame_list_selector->currentText() == "ATTACK") {
        std::cout << "attack_frame: " << (int)Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_frame << ", currentRow: " << currentRow << std::endl;
        _data_loading = true;
        if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_frame == currentRow) {
            ui->isAttackFrame_checkBox->setChecked(true);
        } else {
            ui->isAttackFrame_checkBox->setChecked(false);
        }
        _data_loading = false;
    }
    Mediator::get_instance()->current_sprite_selection = currentRow;
}

void npc_edit::on_sprite_duration_spinBox_valueChanged(int arg1)
{
    if (ui->frameList_listWidget->selectedItems().size() > 0) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].duration = arg1;
        if (_ignore_spritelist_row_changed == false) {
            reload_frame_list(ui->frame_list_selector->currentIndex());
        } else {
            _ignore_spritelist_row_changed = false;
        }
    }
}

void npc_edit::on_sprite_collision_x_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].collision_rect.x = arg1;
	}
	ui->sprites_preview_widget->repaint();
}

void npc_edit::on_sprite_collision_y_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].collision_rect.y = arg1;
	}
	ui->sprites_preview_widget->repaint();
}

void npc_edit::on_sprite_collision_w_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].collision_rect.w = arg1;
	}
	ui->sprites_preview_widget->repaint();

}

void npc_edit::on_sprite_collision_h_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
        Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].collision_rect.h = arg1;
	}
	ui->sprites_preview_widget->repaint();

}


void npc_edit::on_isBoss_checkBox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_boss = checked;
    // a sub-boss can't be a boss and vice-versa
    if (checked == true && Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_sub_boss == true) {
        Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_sub_boss = false;
        _data_loading = true;
        ui->isSubBoss_checkbox->setChecked(false);
        _data_loading = false;
    }
}

void npc_edit::on_isSubBoss_checkbox_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_sub_boss = checked;
    // a sub-boss can't be a boss and vice-versa
    if (checked == true && Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_boss == true) {
        Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).is_boss = false;
        _data_loading = true;
        ui->isBoss_checkBox->setChecked(false);
        _data_loading = false;
    }
}

void npc_edit::on_sprite_pos_x_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.x = arg1;
    ui->npc_edit_tab_previewarea->set_sprite_pos(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg);
    ui->npc_edit_tab_previewarea->update();
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_sprite_pos_y_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.y = arg1;
    ui->npc_edit_tab_previewarea->set_sprite_pos(Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg);
    ui->npc_edit_tab_previewarea->update();
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_checkBox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    if (checked == true) {
        Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).fly_flag = 1;
    } else {
        Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).fly_flag = 0;
    }
}

void npc_edit::on_hitarea_x_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(x): " << arg1 << std::endl;

    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.x = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_y_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(y): " << arg1 << std::endl;

    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.y = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_w_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(w): " << arg1 << std::endl;

    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.w = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_h_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(h): " << arg1 << std::endl;

    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites[ANIM_TYPE_TELEPORT][0].collision_rect.h = arg1;
    ui->npc_edit_tab_previewarea->update();
}



void npc_edit::on_respawn_time_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).respawn_delay = arg1;
}

void npc_edit::on_AddFrame_Button_clicked()
{
    //frame_list_selector
    // go to the first free slot
    // @204
    std::cout << "## FOUND NPC!!!!" << std::endl;
    for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
        int frame_type = ui->frame_list_selector->currentIndex();
        if (Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].used == false) {
            std::cout << "ADD-SPRITE-FRAME: FOUND EMPTY SLOT AT [" << j << "]" << std::endl;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].used = true;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].duration = 100;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].sprite_graphic_pos_x = Mediator::get_instance()->getPalleteX();
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].collision_rect.w = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.width;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].collision_rect.h = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).frame_size.height;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].collision_rect.x = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.x;
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[frame_type][j].collision_rect.y = Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->current_npc_n).sprites_pos_bg.y;
            reload_frame_list(ui->frame_list_selector->currentIndex());
            return;
        }
    }
}

void npc_edit::on_addEnemy_pushButton_clicked()
{
    _data_loading = true;
    Mediator::get_instance()->enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc());
    // add equivalent AI for enemy
    Mediator::get_instance()->ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
    ui->npc_edit_tab_selectnpccombo->addItem(QString("[") + QString::number(Mediator::get_instance()->enemy_list.size()-1) + QString("] Enemy Name"));
    ui->npc_edit_tab_graphiccombo->setCurrentIndex(-1);
    _data_loading = false;
    ui->npc_edit_tab_selectnpccombo->setCurrentIndex(Mediator::get_instance()->enemy_list.size()-1);
}

void npc_edit::on_isAttackFrame_checkBox_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    if (ui->frame_list_selector->currentText() == "ATTACK") {
        if (checked == true) {
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_frame = ui->frameList_listWidget->currentRow();
        } else {
            Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_frame = 0;
        }
    }
}

void npc_edit::on_projectileOriginX_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_arm_pos.x = arg1;
}

void npc_edit::on_projectileOriginY_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).attack_arm_pos.y = arg1;
}

void npc_edit::on_pushButton_2_clicked()
{
    if (_data_loading == true) { return; }
    if (ui->frameList_listWidget->selectedItems().size() > 0) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
            if (i != ui->frameList_listWidget->currentRow()) {
                Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][i].collision_rect = Mediator::get_instance()->enemy_list.at(_npcedit_tab_selectednpc).sprites[Mediator::get_instance()->current_sprite_type][ui->frameList_listWidget->currentRow()].collision_rect;
            }
        }
        ui->sprites_preview_widget->repaint();
    }
}
