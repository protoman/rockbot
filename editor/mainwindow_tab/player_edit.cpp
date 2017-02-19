#include "player_edit.h"
#include "ui_player_edit.h"
#include "common.h"

#include <QColorDialog>

player_edit::player_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::player_edit)
{
    ui->setupUi(this);
    Mediator::get_instance()->current_player = 0;
    common::fill_players_combo(ui->players_tab_list_combo_2);
    fill_players_data();
}

player_edit::~player_edit()
{
    delete ui;
}

void player_edit::reload()
{
    fill_players_data();
}


void player_edit::fill_players_data()
{
    if (FILEPATH.length() == 0) {
        return;
    }

    _loading = true;
    int index = Mediator::get_instance()->current_player;


    common::fill_graphicfiles_combobox("/images/sprites/", ui->player_graphics_combo);
    ui->players_tab_name->setText(QString(Mediator::get_instance()->player_list[index].name));
    ui->players_tab_hp->setValue(Mediator::get_instance()->player_list[index].HP);
    ui->players_tab_hasshield->setChecked(Mediator::get_instance()->player_list[index].have_shield);
    ui->players_tab_maxshots->setValue(Mediator::get_instance()->player_list[index].max_shots);
    ui->damageModSpinBox->setValue(Mediator::get_instance()->player_list[index].damage_modifier);
    double move_speed = Mediator::get_instance()->player_list[index].move_speed;
    move_speed = move_speed / 10;
    //std::cout << "move_speed: " << move_speed << std::endl;
    QString graphic_qstring = QString(Mediator::get_instance()->player_list[index].graphic_filename);
    ui->player_graphics_combo->setCurrentIndex(ui->player_graphics_combo->findText(graphic_qstring));

    common::fill_graphicfiles_combobox("/images/faces/", ui->playerFace_comboBox);
    ui->playerFace_comboBox->setCurrentIndex(ui->playerFace_comboBox->findText(QString(Mediator::get_instance()->player_list[index].face_filename)));

    ui->player_sprite_w->setValue(Mediator::get_instance()->player_list[index].sprite_size.width);
    ui->player_sprite_h->setValue(Mediator::get_instance()->player_list[index].sprite_size.height);

    ui->player_hitarea_x->setValue(Mediator::get_instance()->player_list[index].sprite_hit_area.x);
    ui->player_hitarea_y->setValue(Mediator::get_instance()->player_list[index].sprite_hit_area.y);
    ui->player_hitarea_w->setValue(Mediator::get_instance()->player_list[index].sprite_hit_area.w);
    ui->player_hitarea_h->setValue(Mediator::get_instance()->player_list[index].sprite_hit_area.h);
    ui->can_slide_checkbox->setChecked(Mediator::get_instance()->player_list[index].can_slide);

    common::fill_projectiles_combo(ui->chargedshot_combo);
    ui->chargedshot_combo->setCurrentIndex(Mediator::get_instance()->player_list[index].full_charged_projectile_id);

    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->canDoubleJumpCheckBox->setChecked(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_double_jump);
    ui->CanAirDashCheckBox->setChecked(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_air_dash);
    ui->canShotDiagonal->setChecked(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_shot_diagonal);

    ui->simultaneousShots_spinBox->setValue(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].simultaneous_shots);

    common::fill_weapons_combo_plus(ui->weaponlist_combo);

    ui->player_preview_widget->update_sprites();

    _loading = false;
}



void player_edit::pick_bg_color()
{
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.r = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].r;
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.g = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].g;
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.b = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].b;
    /// @TDO - send a signal for map edit and others to update themselves
    //map_edit_tab->update_edit_area();
}

void player_edit::on_color_selected1(const QColor &color)
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r = color.red();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g = color.green();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b = color.blue();

    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(color.red()) + QString(", ") +  QString::number(color.green()) + QString(", ") +  QString::number(color.blue()) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->update_sprites();
}

void player_edit::on_color_selected2(const QColor &color)
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r = color.red();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g = color.green();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b = color.blue();

    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(color.red()) + QString(", ") +  QString::number(color.green()) + QString(", ") +  QString::number(color.blue()) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->update_sprites();
    ui->player_preview_widget->update_sprites();
}

void player_edit::on_color_selected3(const QColor &color)
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r = color.red();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g = color.green();
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b = color.blue();

    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(color.red()) + QString(", ") +  QString::number(color.green()) + QString(", ") +  QString::number(color.blue()) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->update_sprites();
    ui->player_preview_widget->update_sprites();
}

void player_edit::pick_player_color1()
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].r;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].g;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].b;

    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->update_sprites();
    ui->player_preview_widget->update_sprites();
}

void player_edit::pick_player_color2()
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].r;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].g;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].b;

    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->update_sprites();
    ui->player_preview_widget->update_sprites();
}

void player_edit::pick_player_color3()
{
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].r;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].g;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].b;

    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));

    ui->player_preview_widget->update_sprites();
    ui->player_preview_widget->update_sprites();
}

void player_edit::on_players_tab_list_combo_currentIndexChanged(int index)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->current_player = index;
    fill_players_data();
}

void player_edit::on_players_tab_name_textChanged(const QString &arg1)
{
    if (_loading == true) { return; }
    sprintf(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].name, "%s", arg1.toStdString().c_str());
}

void player_edit::on_players_tab_hp_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].HP = arg1;
}

void player_edit::on_players_tab_hasshield_toggled(bool checked)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].have_shield = checked;
}

void player_edit::on_players_tab_maxshots_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].max_shots = arg1;
}

void player_edit::on_can_slide_checkbox_toggled(bool checked)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_slide = checked;
}


void player_edit::on_players_tab_list_combo_2_currentIndexChanged(int index)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->current_player = index;
    fill_players_data();
}

void player_edit::on_player_graphics_combo_currentIndexChanged(const QString &arg1)
{
    Mediator::get_instance()->player_graphics_data.graphics_filename = arg1.toStdString();

    if (_loading == false) {
        sprintf(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].graphic_filename, "%s", arg1.toStdString().c_str());
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_sprite_w_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.frame_size.width = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_size.width = arg1;
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_sprite_h_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.frame_size.height = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_size.height = arg1;
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_hitarea_x_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.hit_area.x = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_hit_area.x;
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_hitarea_y_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.hit_area.y = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_hit_area.y;
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_hitarea_w_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.hit_area.w = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_hit_area.w;
        ui->player_preview_widget->update_sprites();
    }
}

void player_edit::on_player_hitarea_h_valueChanged(int arg1)
{
    Mediator::get_instance()->player_graphics_data.hit_area.h = arg1;
    if (_loading == false) {
        Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].sprite_hit_area.h;
        ui->player_preview_widget->update_sprites();
    }
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
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].full_charged_projectile_id = index;
}

void player_edit::on_weaponlist_combo_currentIndexChanged(int index)
{
    Mediator::get_instance()->current_weapon = index;
    ui->ColorValueIndicator1->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color1.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator2->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color2.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->ColorValueIndicator3->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.r) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.g) + QString(", ") +  QString::number(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].weapon_colors[Mediator::get_instance()->current_weapon].color3.b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
    ui->player_preview_widget->update_sprites();
}

void player_edit::on_players_tab_movespeed_valueChanged(double arg1)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].move_speed = arg1 * 10;
}

void player_edit::on_damageModSpinBox_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].damage_modifier = arg1;
}

void player_edit::on_canDoubleJumpCheckBox_toggled(bool checked)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_double_jump = checked;
}

void player_edit::on_CanAirDashCheckBox_toggled(bool checked)
{
    if (_loading == true) { return; }
     Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_air_dash = checked;
}

void player_edit::on_canShotDiagonal_toggled(bool checked)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_shot_diagonal = checked;
}


void player_edit::on_playerFace_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_loading == true) { return; }
    sprintf(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].face_filename, "%s", arg1.toStdString().c_str());
}


void player_edit::on_weaponColor_picker_clicked()
{
    QColorDialog *colorDialog = new QColorDialog(this);
    QObject::connect(colorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(on_weapon_color_selected(QColor)));
    colorDialog->show();
}

void player_edit::on_simultaneousShots_spinBox_valueChanged(int arg1)
{
    if (_loading == true) { return; }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].simultaneous_shots = arg1;
}


