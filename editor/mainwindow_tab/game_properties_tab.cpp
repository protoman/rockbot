#include "game_properties_tab.h"
#include "ui_game_properties_tab.h"
#include "common.h"

#include "mediator.h"

game_properties_tab::game_properties_tab(QWidget *parent) : QWidget(parent), ui(new Ui::game_properties_tab), _data_loading(false), _current_stage(0)
{
    ui->setupUi(this);
    fill_data();
}

game_properties_tab::~game_properties_tab()
{
    delete ui;
}

void game_properties_tab::reload()
{
    fill_data();
}

void game_properties_tab::fill_data()
{
    if (FILEPATH.length() == 0) {
        return;
    }
    _data_loading = true;
    _current_stage = 0;
    common::fill_projectiles_combo(ui->semicharged_shot_combo);
    common::fill_stages_combo(ui->stagefaces_stage_combo);
    ui->stagefaces_stage_combo->setCurrentIndex(_current_stage);
    common::fill_graphicfiles_combobox(std::string("images/faces"), ui->stagefaces_face_combo);
    common::fill_object_combo(ui->special_item1_combo);
    common::fill_object_combo(ui->special_item2_combo);
    common::fill_npc_combo(ui->finalBoss_comboBox);

    // set values/positions
    ui->special_item1_combo->setCurrentIndex(Mediator::get_instance()->game_data.player_items[0]);
    ui->special_item2_combo->setCurrentIndex(Mediator::get_instance()->game_data.player_items[1]);
    ui->semicharged_shot_combo->setCurrentIndex(Mediator::get_instance()->game_data.semi_charged_projectile_id);
    ui->finalBoss_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.final_boss_id);


    int combo_n = ui->stagefaces_face_combo->findText(QString(Mediator::get_instance()->game_data.stage_face_filename[_current_stage]));
    ui->stagefaces_face_combo->setCurrentIndex(combo_n);

    ui->bossName_lineEdit->setText(Mediator::get_instance()->game_data.stages_face_name[0]);
    ui->lineEdit->setText(Mediator::get_instance()->game_data.name);

    // file-format 4.0 fields
    common::fill_files_combo(std::string("/music"), ui->bossMusic_comboBox);
    common::fill_files_combo(std::string("/music"), ui->finalBossMusic_comboBox);
    common::fill_files_combo(std::string("/music"), ui->gotWeaponMusic_comboBox);
    common::fill_files_combo(std::string("/music"), ui->gameOverMusic_comboBox);
    common::fill_files_combo(std::string("/music"), ui->stageSelectMusic_comboBox);
    common::fill_files_combo(std::string("/music"), ui->gameStartMusic_comboBox);


    ui->gameStyle_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.game_style);
    combo_n = ui->bossMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.boss_music_filename));
    ui->bossMusic_comboBox->setCurrentIndex(combo_n);
    combo_n = ui->finalBossMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.final_boss_music_filename));
    ui->finalBossMusic_comboBox->setCurrentIndex(combo_n);
    combo_n = ui->gotWeaponMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.got_weapon_music_filename));
    ui->gotWeaponMusic_comboBox->setCurrentIndex(combo_n);
    combo_n = ui->gameOverMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.game_over_music_filename));
    ui->gameOverMusic_comboBox->setCurrentIndex(combo_n);
    combo_n = ui->stageSelectMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.stage_select_music_filename));
    ui->stageSelectMusic_comboBox->setCurrentIndex(combo_n);
    combo_n = ui->gameStartMusic_comboBox->findText(QString(Mediator::get_instance()->game_data.game_start_screen_music_filename));
    ui->gameStartMusic_comboBox->setCurrentIndex(combo_n);


    if (Mediator::get_instance()->game_data.use_second_castle == true) {
        ui->stageNumber_comboBox->setCurrentIndex(1);
    } else {
        ui->stageNumber_comboBox->setCurrentIndex(0);
    }

    ui->castlePoint_1_x->setValue(Mediator::get_instance()->castle_data.points[0].x);
    ui->castlePoint_1_y->setValue(Mediator::get_instance()->castle_data.points[0].y);

    ui->castlePoint_2_x->setValue(Mediator::get_instance()->castle_data.points[1].x);
    ui->castlePoint_2_y->setValue(Mediator::get_instance()->castle_data.points[1].y);

    ui->castlePoint_3_x->setValue(Mediator::get_instance()->castle_data.points[2].x);
    ui->castlePoint_3_y->setValue(Mediator::get_instance()->castle_data.points[2].y);

    ui->castlePoint_4_x->setValue(Mediator::get_instance()->castle_data.points[3].x);
    ui->castlePoint_4_y->setValue(Mediator::get_instance()->castle_data.points[3].y);



    _data_loading = false;
}

void game_properties_tab::on_special_item1_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->game_data.player_items[0] = index;
}

void game_properties_tab::on_special_item2_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->game_data.player_items[1] = index;
}

void game_properties_tab::on_semicharged_shot_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->game_data.semi_charged_projectile_id = index;
}

void game_properties_tab::on_stagefaces_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    _current_stage = index;
    // set face from data
    int combo_n = ui->stagefaces_face_combo->findText(QString(Mediator::get_instance()->game_data.stage_face_filename[_current_stage]));
    _data_loading = true;


    std::cout << "face-name #2: " << Mediator::get_instance()->game_data.stages_face_name[_current_stage] << std::endl;
    ui->bossName_lineEdit->setText(Mediator::get_instance()->game_data.stages_face_name[_current_stage]);
    _data_loading = false;
    ui->stagefaces_face_combo->setCurrentIndex(combo_n);
}



void game_properties_tab::on_stagefaces_face_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.stage_face_filename[_current_stage], "%s", arg1.toStdString().c_str());
    std::cout << "SET stage_filename[" << _current_stage << "]: " << Mediator::get_instance()->game_data.stage_face_filename[_current_stage] << std::endl;
}

void game_properties_tab::on_lineEdit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.name, "%s", arg1.toStdString().c_str());

}

void game_properties_tab::on_gameStyle_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->game_data.game_style = index;
}

void game_properties_tab::on_bossMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.boss_music_filename, "%s", arg1.toStdString().c_str());
}

void game_properties_tab::on_finalBossMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.final_boss_music_filename, "%s", arg1.toStdString().c_str());
}


void game_properties_tab::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.game_start_screen_music_filename, "%s", arg1.toStdString().c_str());
}

void game_properties_tab::on_gotWeaponMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.got_weapon_music_filename, "%s", arg1.toStdString().c_str());

}

void game_properties_tab::on_gameOverMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.game_over_music_filename, "%s", arg1.toStdString().c_str());
}

void game_properties_tab::on_stageSelectMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.stage_select_music_filename, "%s", arg1.toStdString().c_str());
}

void game_properties_tab::on_stageNumber_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    if (index == 0) {
        Mediator::get_instance()->game_data.use_second_castle = false;
    } else {
        Mediator::get_instance()->game_data.use_second_castle = true;
    }
}

void game_properties_tab::on_bossName_lineEdit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.stages_face_name[_current_stage], "%s", arg1.toStdString().c_str());

    std::cout << "face-name #3: " << Mediator::get_instance()->game_data.stages_face_name[_current_stage] << std::endl;
}



void game_properties_tab::on_gameStartMusic_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->game_data.game_start_screen_music_filename, "%s", arg1.toStdString().c_str());
}

void game_properties_tab::on_castlePoint_1_x_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[0].x = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_1_y_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[0].y = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_2_x_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[1].x = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_2_y_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[1].y = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_3_x_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[2].x = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_3_y_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[2].y = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_4_x_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[3].x = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_4_y_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[3].y = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_5_x_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[4].x = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_castlePoint_5_y_valueChanged(int arg1)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->castle_data.points[4].y = arg1;
    ui->castlePointsPreviewArea->repaint();
}

void game_properties_tab::on_finalBoss_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->game_data.final_boss_id = index;
}

void game_properties_tab::on_editMap_pushButton_clicked()
{
    if (_data_loading == true) return;
    map_dialog.show();
}
