#include "game_properties_tab.h"
#include "ui_game_properties_tab.h"
#include "common.h"

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
    _data_loading = true;
    _current_stage = 0;
    common::fill_projectiles_combo(ui->semicharged_shot_combo);
    common::fill_stages_combo(ui->stagefaces_stage_combo);
    ui->stagefaces_stage_combo->setCurrentIndex(_current_stage);
    common::fill_graphicfiles_combobox(std::string("data/images/faces"), ui->stagefaces_face_combo);
    common::fill_object_combo(ui->special_item1_combo);
    common::fill_object_combo(ui->special_item2_combo);

    // set values/positions
    ui->special_item1_combo->setCurrentIndex(game_data.player_items[0]);
    ui->special_item2_combo->setCurrentIndex(game_data.player_items[1]);
    ui->semicharged_shot_combo->setCurrentIndex(game_data.semi_charged_projectile_id+1);
    int combo_n = ui->stagefaces_face_combo->findText(QString(game_data.stage_face_filename[_current_stage]));
    std::cout << "combo_n: " << combo_n << ", stage_filename[" << _current_stage << "]: " << game_data.stage_face_filename[_current_stage] << std::endl;
    ui->stagefaces_face_combo->setCurrentIndex(combo_n);
    ui->lineEdit->setText(game_data.name);

    _data_loading = false;
}

void game_properties_tab::on_special_item1_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    game_data.player_items[0] = index;
}

void game_properties_tab::on_special_item2_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    game_data.player_items[1] = index;
}

void game_properties_tab::on_semicharged_shot_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    game_data.semi_charged_projectile_id = index-1;
}

void game_properties_tab::on_stagefaces_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    _current_stage = index;
    // set face from data
    int combo_n = ui->stagefaces_face_combo->findText(QString(game_data.stage_face_filename[_current_stage]));
    ui->stagefaces_face_combo->setCurrentIndex(combo_n);
}



void game_properties_tab::on_stagefaces_face_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(game_data.stage_face_filename[_current_stage], "%s", arg1.toStdString().c_str());
    std::cout << "SET stage_filename[" << _current_stage << "]: " << game_data.stage_face_filename[_current_stage] << std::endl;
}

void game_properties_tab::on_lineEdit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(game_data.name, "%s", arg1.toStdString().c_str());

}
