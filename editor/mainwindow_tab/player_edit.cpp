#include "player_edit.h"
#include "ui_player_edit.h"
#include "common.h"
#include "dialog_pick_color.h"

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stages stage_data;

player_edit::player_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::player_edit)
{
    ui->setupUi(this);
    dataExchanger->current_player = 0;
    common::fill_players_combo(ui->players_tab_list_combo_2);
    fill_players_data();
}

player_edit::~player_edit()
{
    delete ui;
}


void player_edit::fill_players_data()
{
    _loading = true;
    int index = dataExchanger->current_player;
    common::fill_graphicfiles_combobox("/data/images/sprites/", ui->player_graphics_combo);
    ui->players_tab_name->setText(QString(game_data.players[index].name));
    ui->players_tab_hp->setValue(game_data.players[index].HP);
    ui->players_tab_hasshield->setChecked(game_data.players[index].have_shield);
    ui->players_tab_maxshots->setValue(game_data.players[index].max_shots);
    ui->damageModSpinBox->setValue(game_data.players[index].damage_modifier);
    double move_speed = game_data.players[index].move_speed;
    move_speed = move_speed / 10;
    //std::cout << "move_speed: " << move_speed << std::endl;
    ui->player_graphics_combo->setCurrentIndex(ui->player_graphics_combo->findText(QString(game_data.players[index].graphic_filename)));

    common::fill_graphicfiles_combobox("/data/images/faces/", ui->playerFace_comboBox);
    ui->playerFace_comboBox->setCurrentIndex(ui->playerFace_comboBox->findText(QString(game_data.players[index].face_filename)));

    ui->player_sprite_w->setValue(game_data.players[index].sprite_size.width);
    ui->player_sprite_h->setValue(game_data.players[index].sprite_size.height);

    ui->player_hitarea_x->setValue(game_data.players[index].sprite_hit_area.x);
    ui->player_hitarea_y->setValue(game_data.players[index].sprite_hit_area.y);
    ui->player_hitarea_w->setValue(game_data.players[index].sprite_hit_area.w);
    ui->player_hitarea_h->setValue(game_data.players[index].sprite_hit_area.h);
    ui->can_slide_checkbox->setChecked(game_data.players[index].can_slide);

    common::fill_projectiles_combo(ui->chargedshot_combo);
    ui->chargedshot_combo->setCurrentIndex(game_data.players[index].full_charged_projectile_id+1);
    ui->ColorKeyIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[0].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[0].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[0].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorKeyIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[1].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[1].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[1].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorKeyIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[2].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[2].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[2].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->canDoubleJumpCheckBox->setChecked(game_data.players[dataExchanger->current_player].can_double_jump);
    ui->CanAirDashCheckBox->setChecked(game_data.players[dataExchanger->current_player].can_air_dash);
    ui->canShotDiagonal->setChecked(game_data.players[dataExchanger->current_player].can_shot_diagonal);

    common::fill_weapons_combo_plus(ui->weaponlist_combo);
    _loading = false;
}


void player_edit::pick_player_keycolor1()
{
    std::cout << "MainWindow::pick_player_keycolor1 - setting colorkey #0 as " << dataExchanger->picked_color_n << std::endl;
    game_data.players[dataExchanger->current_player].color_keys[0].r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].color_keys[0].g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].color_keys[0].b = dataExchanger->colormap[dataExchanger->picked_color_n].b;

    ui->ColorKeyIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[0].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[0].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[0].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->repaint();
}

void player_edit::pick_player_keycolor2()
{
    game_data.players[dataExchanger->current_player].color_keys[1].r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].color_keys[1].g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].color_keys[1].b = dataExchanger->colormap[dataExchanger->picked_color_n].b;
    ui->ColorKeyIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[1].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[1].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[1].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->repaint();
}

void player_edit::pick_player_keycolor3()
{
    game_data.players[dataExchanger->current_player].color_keys[2].r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].color_keys[2].g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].color_keys[2].b = dataExchanger->colormap[dataExchanger->picked_color_n].b;
    ui->ColorKeyIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].color_keys[2].r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[2].g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].color_keys[2].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->repaint();
}

void player_edit::pick_player_color1()
{
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.b = dataExchanger->colormap[dataExchanger->picked_color_n].b;

    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->repaint();
}

void player_edit::pick_player_color2()
{
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.b = dataExchanger->colormap[dataExchanger->picked_color_n].b;

    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->repaint();
}

void player_edit::pick_player_color3()
{
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.b = dataExchanger->colormap[dataExchanger->picked_color_n].b;

    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->repaint();
}

void player_edit::pick_bg_color()
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.b = dataExchanger->colormap[dataExchanger->picked_color_n].b;
    /// @TDO - send a signal for map edit and others to update themselves
    //map_edit_tab->update_edit_area();
}


void player_edit::on_players_tab_list_combo_currentIndexChanged(int index)
{
    if (_loading == true) { return; }
    dataExchanger->current_player = index;
    fill_players_data();
}

void player_edit::on_players_tab_name_textChanged(const QString &arg1)
{
    if (_loading == true) { return; }
    sprintf(game_data.players[dataExchanger->current_player].name, "%s", arg1.toStdString().c_str());
}

void player_edit::on_players_tab_hp_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].HP = arg1;
}

void player_edit::on_players_tab_hasshield_toggled(bool checked)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].have_shield = checked;
}

void player_edit::on_players_tab_maxshots_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].max_shots = arg1;
}

void player_edit::on_can_slide_checkbox_toggled(bool checked)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].can_slide = checked;
}


void player_edit::on_players_tab_list_combo_2_currentIndexChanged(int index)
{
    if (_loading == true) { return; }
    dataExchanger->current_player = index;
    fill_players_data();
}

void player_edit::on_player_graphics_combo_currentIndexChanged(const QString &arg1)
{
    dataExchanger->player_graphics_data.graphics_filename = arg1.toStdString();

    ui->player_preview_widget->repaint();
    if (_loading == false) {
        sprintf(game_data.players[dataExchanger->current_player].graphic_filename, "%s", arg1.toStdString().c_str());
    }
}

void player_edit::on_player_sprite_w_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.frame_size.width = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_size.width = arg1;
    }
}

void player_edit::on_player_sprite_h_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.frame_size.height = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_size.height = arg1;
    }
}

void player_edit::on_player_hitarea_x_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.hit_area.x = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_hit_area.x;
    }
}

void player_edit::on_player_hitarea_y_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.hit_area.y = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_hit_area.y;
    }
}

void player_edit::on_player_hitarea_w_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.hit_area.w = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_hit_area.w;
    }
}

void player_edit::on_player_hitarea_h_valueChanged(int arg1)
{
    dataExchanger->player_graphics_data.hit_area.h = arg1;
    ui->player_preview_widget->repaint();
    if (_loading == false) {
        game_data.players[dataExchanger->current_player].sprite_hit_area.h;
    }
}

void player_edit::on_key1_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor1()));
}

void player_edit::on_key2_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor2()));
}

void player_edit::on_key3_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor3()));
}

void player_edit::on_color1_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color1()));
}

void player_edit::on_color2_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color2()));
}

void player_edit::on_color3_picker_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color3()));
}

void player_edit::on_chargedshot_combo_currentIndexChanged(int index)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].full_charged_projectile_id = index-1;
}

void player_edit::on_weaponlist_combo_currentIndexChanged(int index)
{
    dataExchanger->current_weapon = index;
    ui->player_preview_widget->repaint();
    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.r) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.g) + QString(", ") +  QString::number(game_data.players[dataExchanger->current_player].weapon_colors[dataExchanger->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
}

void player_edit::on_players_tab_movespeed_valueChanged(double arg1)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].move_speed = arg1 * 10;
}

void player_edit::on_damageModSpinBox_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].damage_modifier = arg1;
}

void player_edit::on_canDoubleJumpCheckBox_toggled(bool checked)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].can_double_jump = checked;
}

void player_edit::on_CanAirDashCheckBox_toggled(bool checked)
{
    if (_loading == true) { return; }
     game_data.players[dataExchanger->current_player].can_air_dash = checked;
}

void player_edit::on_canShotDiagonal_toggled(bool checked)
{
    if (_loading == true) { return; }
    game_data.players[dataExchanger->current_player].can_shot_diagonal = checked;
}


void player_edit::on_playerFace_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_loading == true) { return; }
    sprintf(game_data.players[dataExchanger->current_player].face_filename, "%s", arg1.toStdString().c_str());
}
