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
	_data_loading = true;
    common::fill_files_combo("data/images/sprites/enemies", ui->npc_edit_tab_graphiccombo);
    ui->npc_edit_tab_graphiccombo->setCurrentIndex(ui->npc_edit_tab_graphiccombo->findText(QString(game_data.game_npcs[0].graphic_filename)));
    ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/data/images/sprites/enemies/")+std::string(game_data.game_npcs[0].graphic_filename));
    ui->npc_edit_tab_previewarea->set_sprite_pos(game_data.game_npcs[0].sprites_pos_bg);
    ui->npc_edit_tab_previewarea->repaint();

    common::fill_ai_list(ui->npc_edit_tab_iatype);
    common::fill_npc_combo(ui->npc_edit_tab_selectnpccombo);
    common::fill_weapons_names_combo(ui->npc_edit_tab_weakness_list);

    //common::fill_files_combo("data/images/sprites/backgrounds", ui->bg_graphic_combo);

    common::fill_projectiles_combo(ui->projectile1_comboBox);
    ui->projectile1_comboBox->setCurrentIndex(game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[0]+1);
    common::fill_projectiles_combo(ui->projectile2_comboBox);
	ui->projectile2_comboBox->setCurrentIndex(game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[1]+1);

	_data_loading = true;

    ui->npc_edit_tab_iatype->setCurrentIndex(game_data.game_npcs[dataExchanger->current_npc_n].IA_type);
    if (game_data.game_npcs[dataExchanger->current_npc_n].fly_flag == 0) {
        ui->checkBox->setChecked(false);
    } else {
        ui->checkBox->setChecked(true);
    }

    ui->hitarea_x_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.x);
    ui->hitarea_y_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.y);
    ui->hitarea_w_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w);
    ui->hitarea_h_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h);

    ui->respawn_time_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].respawn_delay);

}


void npc_edit::on_npc_edit_tab_selectnpccombo_currentIndexChanged(int index)
{
	_data_loading = true;
    dataExchanger->current_npc_n = index;

	_npcedit_tab_selectednpc = index;
    ui->npc_edit_tab_canshoot->setChecked(game_data.game_npcs[index].is_ghost);
	ui->npc_edit_tab_graphiccombo->setCurrentIndex(ui->npc_edit_tab_graphiccombo->findText(QString(game_data.game_npcs[index].graphic_filename)));
	ui->npc_edit_tab_graphicheight->setValue(game_data.game_npcs[index].frame_size.height);
	ui->npc_edit_tab_graphicwidth->setValue(game_data.game_npcs[index].frame_size.width);
	dataExchanger->npcGraphicSize_h = game_data.game_npcs[index].frame_size.height;
	dataExchanger->npcGraphicSize_w = game_data.game_npcs[index].frame_size.width;
	ui->npc_edit_tab_iatype->setCurrentIndex(game_data.game_npcs[index].IA_type);
	ui->npc_edit_tab_movespeed->setValue(game_data.game_npcs[index].speed);
	ui->npc_edit_tab_NpcHP->setValue(game_data.game_npcs[index].hp.total);
	ui->npc_edit_tab_NpcName->setText(game_data.game_npcs[index].name);
	ui->npc_edit_tab_range->setValue(game_data.game_npcs[index].walk_range);
	ui->npc_edit_tab_shieldtype->setCurrentIndex(game_data.game_npcs[index].shield_type);
	ui->npc_edit_tab_weakness_list->setCurrentIndex(0);
	ui->npc_edit_tab_weakness_points->setValue(game_data.game_npcs[index].weakness[0].damage_multiplier);

    ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/data/images/sprites/enemies/")+std::string(game_data.game_npcs[_npcedit_tab_selectednpc].graphic_filename));

    //int bg_pos = ui->bg_graphic_combo->findText(QString(game_data.game_npcs[index].bg_graphic_filename));
    //ui->bg_graphic_combo->setCurrentIndex(bg_pos);
    ui->npc_edit_tab_previewarea->set_bg_graphicfile(FILEPATH+std::string("/data/images/sprites/backgrounds/")+std::string(game_data.game_npcs[_npcedit_tab_selectednpc].bg_graphic_filename));
    ui->npc_edit_tab_previewarea->set_sprite_pos(game_data.game_npcs[index].sprites_pos_bg);
    ui->npc_edit_tab_previewarea->repaint();
	reload_frame_list(ui->frame_list_selector->currentIndex());
	ui->sprites_preview_widget->repaint();

    ui->projectile1_comboBox->setCurrentIndex(game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[0]+1);
	ui->projectile2_comboBox->setCurrentIndex(game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[1]+1);

    //std::cout << "2. >> _data_loading: " << _data_loading << ", game_data.game_npcs[dataExchanger->current_npc_n].IA_type: " << game_data.game_npcs[dataExchanger->current_npc_n].IA_type << std::endl;
	ui->isBoss_checkBox->setChecked(game_data.game_npcs[dataExchanger->current_npc_n].is_boss);
    ui->isSubBoss_checkbox->setChecked(game_data.game_npcs[dataExchanger->current_npc_n].is_sub_boss);

    if (game_data.game_npcs[dataExchanger->current_npc_n].fly_flag == 0) {
        ui->checkBox->setChecked(false);
    } else {
        ui->checkBox->setChecked(true);
    }

    std::cout << "NPC: " << dataExchanger->current_npc_n << ", hit_area - x: " << game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.x << ", y: " << game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.y << ", w: " << game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w << ", h: " << game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h << std::endl;
    ui->hitarea_x_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.x);
    ui->hitarea_y_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.y);
    ui->hitarea_w_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w);
    ui->hitarea_h_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h);

    ui->respawn_time_spinBox->setValue(game_data.game_npcs[dataExchanger->current_npc_n].respawn_delay);

	_data_loading = false;
}

void npc_edit::on_npc_edit_tab_graphiccombo_currentIndexChanged(const QString &arg1)
{
	if (_data_loading) {
        //std::cout << "\\\\\\\\\\\ on_npc_edit_tab_graphiccombo_currentIndexChanged /////////////" << std::endl;
		return;
	}
	sprintf(game_data.game_npcs[_npcedit_tab_selectednpc].graphic_filename, "%s", arg1.toStdString().c_str());
	std::cout << "npc[" << _npcedit_tab_selectednpc << "].graphic_filename: " << game_data.game_npcs[_npcedit_tab_selectednpc].graphic_filename << std::endl;
    ui->npc_edit_tab_previewarea->set_graphicfile(FILEPATH+std::string("/data/images/sprites/enemies/")+arg1.toStdString());
    ui->npc_edit_tab_previewarea->repaint();
}

void npc_edit::on_npc_edit_tab_graphicwidth_valueChanged(int arg1)
{
	ui->npc_edit_tab_previewarea->set_grid_w(arg1);
    // if hitarea is zero or old value, update to new one
    _data_loading = true;
    if (ui->hitarea_w_spinBox->value() == 0 || ui->hitarea_w_spinBox->value() == game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width) {
        ui->hitarea_w_spinBox->setValue(arg1);

        if (game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width == game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w) {
            std::cout << "### SET collistion W" << std::endl;
            game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w = arg1;
        }
        ui->npc_edit_tab_previewarea->update();
    }
    _data_loading = false;
	game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width = arg1;
}

void npc_edit::on_npc_edit_tab_graphicheight_valueChanged(int arg1)
{
	ui->npc_edit_tab_previewarea->set_grid_h(arg1);
    // if hitarea is zero or old value, update to new one
    _data_loading = true;
    if (ui->hitarea_h_spinBox->value() == 0 || ui->hitarea_h_spinBox->value() == game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height) {
        ui->hitarea_h_spinBox->setValue(arg1);

        if (game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height == game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h) {
            std::cout << "### SET collistion H" << std::endl;
            game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h = arg1;
        }

        ui->npc_edit_tab_previewarea->update();
    }
    game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height = arg1;
}

void npc_edit::on_npc_edit_tab_NpcName_textChanged(const QString &arg1)
{
	sprintf(game_data.game_npcs[_npcedit_tab_selectednpc].name, "%s", arg1.toStdString().c_str());
    QString temp_str = QString("[");
    if (_npcedit_tab_selectednpc < 10) {
        temp_str += QString("0");
    }
    temp_str += QString::number(_npcedit_tab_selectednpc) + QString("] - ") + QString(game_data.game_npcs[_npcedit_tab_selectednpc].name);
    ui->npc_edit_tab_selectnpccombo->setItemText(_npcedit_tab_selectednpc, temp_str);
}

void npc_edit::on_npc_edit_tab_NpcHP_valueChanged(int arg1)
{
	game_data.game_npcs[_npcedit_tab_selectednpc].hp.total = arg1;
}

void npc_edit::on_npc_edit_tab_canshoot_toggled(bool checked)
{
    game_data.game_npcs[_npcedit_tab_selectednpc].is_ghost = checked;
}

void npc_edit::on_npc_edit_tab_shieldtype_currentIndexChanged(int index)
{
	if (_data_loading) {
		return;
	}
	game_data.game_npcs[_npcedit_tab_selectednpc].shield_type = index;
}

void npc_edit::on_npc_edit_tab_iatype_currentIndexChanged(int index)
{
	if (_data_loading) {
		return;
	}
	game_data.game_npcs[_npcedit_tab_selectednpc].IA_type = index;
}

void npc_edit::on_npc_edit_tab_movespeed_valueChanged(int arg1)
{
	game_data.game_npcs[_npcedit_tab_selectednpc].speed = arg1;
}

void npc_edit::on_npc_edit_tab_range_valueChanged(int arg1)
{
	game_data.game_npcs[_npcedit_tab_selectednpc].walk_range = arg1;
}

void npc_edit::on_npc_edit_tab_frametype_currentIndexChanged(int index)
{
	if (_data_loading) {
		return;
	}
    // @204
    //game_data.game_npcs[_npcedit_tab_selectednpc].frames[dataExchanger->getPalleteX()].state = index-1;
}


void npc_edit::set_npc_frame() {
	printf(">> DialogNPCEdit::setNPCFrame - CALLED\n");
    // @204
    //ui->npc_edit_tab_frameduration->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].frames[dataExchanger->getPalleteX()].duration);
    //ui->npc_edit_tab_frametype->setCurrentIndex(game_data.game_npcs[_npcedit_tab_selectednpc].frames[dataExchanger->getPalleteX()].state+1);
}


void npc_edit::on_bg_graphic_combo_currentIndexChanged(const QString &arg1)
{
	if (_data_loading) {
		return;
	}
	if (arg1.length() == 0) {
        //std::cout << "*************** on_bg_graphic_combo_currentIndexChanged - ZERO" << std::endl;
		sprintf(game_data.game_npcs[_npcedit_tab_selectednpc].bg_graphic_filename, "%s", "");
        ui->npc_edit_tab_previewarea->set_bg_graphicfile("");
    } else {
        //std::cout << "*************** on_bg_graphic_combo_currentIndexChanged - SET to '" << arg1.toStdString() << "'" << std::endl;
		sprintf(game_data.game_npcs[_npcedit_tab_selectednpc].bg_graphic_filename, "%s", arg1.toStdString().c_str());
        ui->npc_edit_tab_previewarea->set_bg_graphicfile(FILEPATH+std::string("/data/images/sprites/backgrounds/")+std::string(game_data.game_npcs[_npcedit_tab_selectednpc].bg_graphic_filename));
    }
}

void npc_edit::on_npc_edit_tab_weakness_list_currentIndexChanged(int index)
{
	if (_data_loading) {
        //std::cout << "*** on_npc_edit_tab_weakness_list_currentIndexChanged - NOT ***" << std::endl;
		return;
	} else {
        //std::cout << "*** on_npc_edit_tab_weakness_list_currentIndexChanged - YES ***" << std::endl;
	}
	_npcedit_tab_selected_weakness_weapon = index;
	ui->npc_edit_tab_weakness_points->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].weakness[_npcedit_tab_selected_weakness_weapon].damage_multiplier);
}

void npc_edit::on_npc_edit_tab_weakness_points_valueChanged(int arg1)
{
	game_data.game_npcs[_npcedit_tab_selectednpc].weakness[_npcedit_tab_selected_weakness_weapon].damage_multiplier = arg1;

}


void npc_edit::on_frame_list_selector_currentIndexChanged(int index)
{
	if (_data_loading == true) {
		return;
	}
    dataExchanger->current_sprite_type = index;
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


    // teleport must not be used
    if (index == ANIM_TYPE_TELEPORT) {
        ui->AddFrame_Button->setEnabled(false);
        return;
    } else {
        ui->AddFrame_Button->setEnabled(true);
    }


	// insert all sprites for the given type into the combo
    //std::cout << ">> Adding frames for NPC[" << game_data.game_npcs[i].name << "] - type: " << index << std::endl;
    std::string filename = FILEPATH + "/data/images/sprites/enemies/" + game_data.game_npcs[_npcedit_tab_selectednpc].graphic_filename;
    // @204
    for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
        if (game_data.game_npcs[_npcedit_tab_selectednpc].sprites[index][j].used == true) {
            int calc_pos_x = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[index][j].sprite_graphic_pos_x * game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width;
            //std::cout << "NPC[" << game_data.game_npcs[i].name << "] - frame.pos_x: " << game_data.game_npcs[i].sprites[index][j].sprite_graphic_pos_x << ", cal_pos_x: " << calc_pos_x << std::endl;
            QListWidgetItem* item = new QListWidgetItem;
            QString temp_str(game_data.game_npcs[_npcedit_tab_selectednpc].name + QString(" (") + QString::number(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[index][j].duration) + QString(")"));
            item->setText(temp_str);
            //game_data.game_npcs[i].name + " (" + QString::number(game_data.game_npcs[i].sprites[index][j].duration).c_ + ")");
            QPixmap image(filename.c_str());
            if (image.isNull() == false && image.width() > 0) {
                image = image.copy(calc_pos_x, 0, game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width, game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height);
                image = image.scaled(game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width*2, game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height*2);
                item->setIcon(image);
            }
            item->setSizeHint(QSize(game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width*2, game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height*2+4));
            ui->frameList_listWidget->addItem(item);
        }
    }


    if (current_row != -1) {
        _ignore_spritelist_row_changed = true;
        ui->frameList_listWidget->setCurrentRow(current_row);
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
        game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][j] = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][j+1];
    }
    // clear last sprite
    game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ANIM_FRAMES_COUNT-1].used = false;
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
    CURRENT_FILE_FORMAT::st_sprite_data temp = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()-1];
    game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()-1] = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()];
    game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()] = temp;
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
    if (game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1].used == false) {
        return;
    }
    CURRENT_FILE_FORMAT::st_sprite_data temp = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1];
    game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()+1] = game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()];
    game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][ui->frameList_listWidget->currentRow()] = temp;
    reload_frame_list(ui->frame_list_selector->currentIndex());
}

void npc_edit::on_frameList_listWidget_currentRowChanged(int currentRow)
{
    ui->sprite_duration_spinBox->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][currentRow].duration);
    ui->sprite_colision_x->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][currentRow].colision_rect.x);
    ui->sprite_colision_y->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][currentRow].colision_rect.y);
    ui->sprite_colision_w->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][currentRow].colision_rect.w);
    ui->sprite_colision_h->setValue(game_data.game_npcs[_npcedit_tab_selectednpc].sprites[ui->frame_list_selector->currentIndex()][currentRow].colision_rect.h);
}

void npc_edit::on_sprite_duration_spinBox_valueChanged(int arg1)
{
    if (ui->frameList_listWidget->selectedItems().size() > 0) {
        game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].duration = arg1;
        if (_ignore_spritelist_row_changed == false) {
            reload_frame_list(ui->frame_list_selector->currentIndex());
        } else {
            _ignore_spritelist_row_changed = false;
        }
    }
}

void npc_edit::on_sprite_colision_x_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
		game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].colision_rect.x = arg1;
	}
	ui->sprites_preview_widget->repaint();
}

void npc_edit::on_sprite_colision_y_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
		game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].colision_rect.y = arg1;
	}
	ui->sprites_preview_widget->repaint();
}

void npc_edit::on_sprite_colision_w_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
		game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].colision_rect.w = arg1;
	}
	ui->sprites_preview_widget->repaint();

}

void npc_edit::on_sprite_colision_h_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
	if (ui->frameList_listWidget->selectedItems().size() > 0) {
		game_data.game_npcs[_npcedit_tab_selectednpc].sprites[dataExchanger->current_sprite_type][ui->frameList_listWidget->currentRow()].colision_rect.h = arg1;
	}
	ui->sprites_preview_widget->repaint();

}

void npc_edit::on_projectile1_comboBox_currentIndexChanged(int index)
{
	if (_data_loading == false) {
		game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[0] = index-1;
	}
}

void npc_edit::on_projectile2_comboBox_currentIndexChanged(int index)
{
	if (_data_loading == false) {
		game_data.game_npcs[dataExchanger->current_npc_n].projectile_id[1] = index-1;
	}
}

void npc_edit::on_ai_chain_currentIndexChanged(int index)
{
	if (_data_loading == true) {
		return;
	}
    std::cout << "3. >> _data_loading: " << _data_loading << ", game_data.game_npcs[dataExchanger->current_npc_n].IA_type: " << game_data.game_npcs[dataExchanger->current_npc_n].IA_type << ", index: " << index << std::endl;
    game_data.game_npcs[dataExchanger->current_npc_n].IA_type = index; // @TODO use ai_type when system is working
}


void npc_edit::on_isBoss_checkBox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
	game_data.game_npcs[dataExchanger->current_npc_n].is_boss = checked;
    // a sub-boss can't be a boss and vice-versa
    if (checked == true && game_data.game_npcs[dataExchanger->current_npc_n].is_sub_boss == true) {
        game_data.game_npcs[dataExchanger->current_npc_n].is_sub_boss = false;
        _data_loading = true;
        ui->isSubBoss_checkbox->setChecked(false);
        _data_loading = false;
    }
}

void npc_edit::on_isSubBoss_checkbox_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    game_data.game_npcs[dataExchanger->current_npc_n].is_sub_boss = checked;
    // a sub-boss can't be a boss and vice-versa
    if (checked == true && game_data.game_npcs[dataExchanger->current_npc_n].is_boss == true) {
        game_data.game_npcs[dataExchanger->current_npc_n].is_boss = false;
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
    game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.x = arg1;
    ui->npc_edit_tab_previewarea->set_sprite_pos(game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg);
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_sprite_pos_y_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.y = arg1;
    ui->npc_edit_tab_previewarea->set_sprite_pos(game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg);
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_checkBox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    if (checked == true) {
        game_data.game_npcs[dataExchanger->current_npc_n].fly_flag = 1;
    } else {
        game_data.game_npcs[dataExchanger->current_npc_n].fly_flag = 0;
    }
}

void npc_edit::on_hitarea_x_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(x): " << arg1 << std::endl;

    game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.x = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_y_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(y): " << arg1 << std::endl;

    game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.y = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_w_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(w): " << arg1 << std::endl;

    game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.w = arg1;
    ui->npc_edit_tab_previewarea->update();
}

void npc_edit::on_hitarea_h_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }

    std::cout << "hi_area::change(h): " << arg1 << std::endl;

    game_data.game_npcs[dataExchanger->current_npc_n].sprites[ANIM_TYPE_TELEPORT][0].colision_rect.h = arg1;
    ui->npc_edit_tab_previewarea->update();
}



void npc_edit::on_respawn_time_spinBox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    game_data.game_npcs[dataExchanger->current_npc_n].respawn_delay = arg1;
}

void npc_edit::on_AddFrame_Button_clicked()
{
    //frame_list_selector
    // go to the first free slot
    // @204
    std::cout << "## FOUND NPC!!!!" << std::endl;
    for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
        int frame_type = ui->frame_list_selector->currentIndex();
        if (game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].used == false) {
            std::cout << "ADD-SPRITE-FRAME: FOUND EMPTY SLOT AT [" << j << "]" << std::endl;
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].used = true;
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].duration = ui->npc_edit_tab_frameduration->value();
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].sprite_graphic_pos_x = dataExchanger->getPalleteX();
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].colision_rect.w = game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.width;
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].colision_rect.h = game_data.game_npcs[_npcedit_tab_selectednpc].frame_size.height;
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].colision_rect.x = game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.x;
            game_data.game_npcs[_npcedit_tab_selectednpc].sprites[frame_type][j].colision_rect.y = game_data.game_npcs[dataExchanger->current_npc_n].sprites_pos_bg.y;
            reload_frame_list(ui->frame_list_selector->currentIndex());
            return;
        }
    }
}
