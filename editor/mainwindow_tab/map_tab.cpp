#include "map_tab.h"
#include "ui_map_tab.h"
#include "common.h"

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stages stage_data;

map_tab::map_tab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::map_tab),
    _data_loading(true)
{
    std::cout << ">>>>>>>>>>>>> DEBUG#1 <<<<<<<<<<<<<<<" << std::endl;
    ui->setupUi(this);
    fill_data();
    ui->editArea->repaint();
    QObject::connect(ui->pallete, SIGNAL(signalPalleteChanged()), ui->editArea, SLOT(repaint()));
}

map_tab::~map_tab()
{
    delete ui;
}

void map_tab::reload()
{
    fill_data();
}

void map_tab::save()
{
    ui->editArea->saveGame(dataExchanger->currentGame);
}

void map_tab::set_current_box(short n)
{
    ui->toolBox->setCurrentIndex(n);
    ui->editArea->repaint();
}

void map_tab::update_edit_area()
{
    ui->editArea->repaint();
}

void map_tab::pick_bg_color()
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.b = dataExchanger->colormap[dataExchanger->picked_color_n].b;
    fill_background_list();
    update_edit_area();
}


void map_tab::fill_data()
{
    _data_loading = true;
    common::fill_stages_combo(ui->stageListCombo);
    common::fill_map_list_combo(ui->mapListCombo);
    common::fill_npc_listwidget(ui->npc_listWidget);
    common::fill_object_listWidget(ui->objectListWidget);
    fill_background_list();
    ui->npc_direction_combo->setCurrentIndex(dataExchanger->npc_direction);
    ui->object_direction_combo->setCurrentIndex(dataExchanger->object_direction);
    _data_loading = false;
}

void map_tab::fill_background_list()
{
    QString bg1_filename(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename);
    QString bg2_filename(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename);
    common::fill_files_combo("data/images/map_backgrounds", ui->bg1_filename);
    common::fill_files_combo("data/images/map_backgrounds", ui->bg2_filename);
    ui->bg1_filename->setCurrentIndex(ui->bg1_filename->findText(bg1_filename));
    ui->bg2_filename->setCurrentIndex(ui->bg2_filename->findText(bg2_filename));
    ui->bg1_y_pos->setValue(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].adjust_y);
    ui->bg2_y_pos->setValue(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].adjust_y);

    float bg1_speed = (float)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed/10;
    ui->bg1_speed->setValue(bg1_speed);
    float bg2_speed = (float)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].speed/10;
    ui->bg2_speed->setValue(bg2_speed);
    std::stringstream ss;
    ss.str(std::string());
    ss << "background-color: rgb(" << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.r << ", " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.g << ", " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.b << ")";
    ui->bg_color_pick->setStyleSheet(ss.str().c_str());
    for (int i=0; i<10; i++) {
        ui->current_anim_tile_combobox->addItem(QString::number(i));
    }
    common::fill_files_combo("data/images/tilesets/anim", ui->anim_tile_graphic_combobox);
}

void map_tab::on_stageListCombo_currentIndexChanged(int index)
{
    if (_data_loading == true) {
        return;
    }
    _data_loading = true;
    dataExchanger->currentMap = 0;
    dataExchanger->currentStage = index;
    ui->mapListCombo->setCurrentIndex(0);
    fill_background_list();
    ui->editArea->repaint();
    _data_loading = false;
}

void map_tab::on_mapListCombo_currentIndexChanged(int index)
{
    if (index < 0) {
        return;
    }
    _data_loading = true;
    dataExchanger->currentMap = index;
    fill_background_list();
    _data_loading = false;
    ui->editArea->repaint();
}

void map_tab::on_spinBox_valueChanged(int arg1)
{
    dataExchanger->zoom = arg1;
    ui->editArea->repaint();
}

void map_tab::on_comboBox_currentIndexChanged(int index)
{
    int value = 1;
    if (index == 1) {
        value = 3;
    }
    dataExchanger->layerLevel = value;
    ui->editArea->repaint();
}

void map_tab::on_listWidget_2_currentRowChanged(int currentRow)
{
    dataExchanger->terrainType = currentRow+1;
}

void map_tab::on_npc_listWidget_currentRowChanged(int currentRow)
{
    dataExchanger->selectedNPC = currentRow;
}

void map_tab::on_npc_direction_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    dataExchanger->npc_direction = index;
}

void map_tab::on_link_orientation_combobox_currentIndexChanged(int index)
{
    dataExchanger->link_type = index;
}

void map_tab::on_link_bidi_checkbox_toggled(bool checked)
{
    dataExchanger->link_bidi = checked;
}

void map_tab::on_link_isdoor_checkbox_toggled(bool checked)
{
    dataExchanger->link_is_door = checked;
}

void map_tab::on_objectListWidget_currentRowChanged(int currentRow)
{
    dataExchanger->selectedNPC = currentRow;
}

void map_tab::on_bg_color_pick_clicked()
{
    QDialog *color_pick = new dialog_pick_color;
    color_pick->show();
    QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_bg_color()));
}

void map_tab::on_bg1_filename_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) { return; }
    if (arg1.toStdString() == std::string("None")) {
        stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename[0] = '\0';
    } else {
        sprintf(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename, "%s", arg1.toStdString().c_str());
    }
    update_edit_area();
}

void map_tab::on_bg1_speed_valueChanged(double arg1)
{
    if (_data_loading == true) { return; }
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed = arg1*10;
    std::cout << "#3 *** on_bg1_speed_valueChanged - setvalue: " << arg1 << ", bg1.speed: " << (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed << std::endl;
    update_edit_area();
}

void map_tab::on_bg1_y_pos_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].adjust_y = arg1;
    update_edit_area();
}

void map_tab::on_bg2_filename_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) { return; }
    if (arg1.toStdString() == std::string("None")) {
        stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename[0] = '\0';
    } else {
        sprintf(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename, "%s", arg1.toStdString().c_str());
    }

}

void map_tab::on_bg2_speed_valueChanged(double arg1)
{
    if (_data_loading == true) { return; }
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].speed = arg1*10;
    std::cout << ">> on_bg2_speed_valueChanged, value: " << (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].speed << std::endl;
    update_edit_area();
}

void map_tab::on_bg2_y_pos_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].adjust_y = arg1;
    update_edit_area();
}

void map_tab::on_checkBox_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    dataExchanger->show_background_color = checked;
    update_edit_area();
}

void map_tab::on_checkBox_2_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    dataExchanger->show_bg1 = checked;
    update_edit_area();
}

void map_tab::on_checkBox_3_toggled(bool checked)
{
    if (_data_loading == true) { return; }
    dataExchanger->show_bg2 = checked;
    update_edit_area();
}


void map_tab::on_current_anim_tile_combobox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    dataExchanger->selectedAnimTileset = index;
    ui->anim_tile_graphic_combobox->setCurrentIndex(ui->anim_tile_graphic_combobox->findText(game_data.anim_tiles[index].filename));
    ui->anim_tile_delay_spinbox->setValue(game_data.anim_tiles[index].delay);
}

void map_tab::on_anim_tile_graphic_combobox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) { return; }
    sprintf(game_data.anim_tiles[dataExchanger->selectedAnimTileset].filename, "%s", arg1.toStdString().c_str());
}

void map_tab::on_anim_tile_delay_spinbox_valueChanged(int arg1)
{
    if (_data_loading == true) { return; }
    game_data.anim_tiles[dataExchanger->selectedAnimTileset].delay = arg1;
}

void map_tab::on_object_direction_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    dataExchanger->object_direction = index;
}
