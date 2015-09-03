#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defines.h"
#include "addwizard.h"
#include "dialognpcedit.h"
#include "dialogobjectedit.h"
#include "loadgamepicker.h"
#include "dialog_pick_color.h"
#include "stage_swap_dialog.h"
//#include <QStandardItemModel>
#include <QListView>
#include <QList>
#include <QListWidgetItem>
#include "../defines.h"
#include "../file/version.h"


#include "common.h"

extern Mediator *dataExchanger;
extern char EDITOR_FILEPATH[512];
extern std::string FILEPATH;

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stages stage_data;

bool background_filled = false;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _npcedit_tab_selectednpc(0), _data_loading(false)
{
	ui->setupUi(this);
    QString window_title = QString("Rockbot Editor ") + QString(VERSION_NUMBER);
    setWindowTitle(window_title);
	dataExchanger->loadGame(1);


	// insert NPC tab form
	npc_edit_tab = new npc_edit();
    ui->NPCScrollArea->setWidget(npc_edit_tab);


	// insert AI tab form
	ai_edit_tab = new artificial_inteligence_tab();
    ui->AIScrollArea->setWidget(ai_edit_tab);

	// insert OBJECT tab form
	object_edit_tab = new object_tab();
    ui->ObjectScrollArea->setWidget(object_edit_tab);


	// insert OBJECT tab form
	weapon_edit_tab = new weapon_edit();
    ui->WeaponScrollArea->setWidget(weapon_edit_tab);

	// insert STAGE tab form
	stage_edit_tab = new stage_edit();
    ui->StagesScrollarea->setWidget(stage_edit_tab);

    // insert PROJECTILE tab form
    projectile_edit_tab = new projectile_edit();
    ui->ProjectileScrollArea->setWidget(projectile_edit_tab);


	// insert COLORCYCLE tab form
	colorcycle_edit_tab = new colorcycle_edit();
    ui->ColorcycleScrollArea->setWidget(colorcycle_edit_tab);

    // insert GAME_PROPERTIES tab form
    game_prop_tab = new game_properties_tab();
    ui->gamePropertiesScrollarea->setWidget(game_prop_tab);

    map_edit_tab = new map_tab();
    ui->MapScrollArea->setWidget(map_edit_tab);


    player_edit_tab = new player_edit();
    ui->PlayerScrollArea->setWidget(player_edit_tab);

    armor_edit_tab = new armor_edit();
    ui->armorScrollArea->setWidget(armor_edit_tab);

    scenes_window = NULL;
}

MainWindow::~MainWindow()
{
	delete ui;
    if (scenes_window != NULL) {
        delete scenes_window;
    }
}









void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionSave_triggered()
{
    map_edit_tab->save();
}

void MainWindow::on_actionOpen_triggered()
{
	QDialog *open = new loadGamePicker;
	open->show();
	//dataExchanger->loadGame();
    //map_edit_tab->update_edit_area();
}

void MainWindow::on_pallete_signalPalleteChanged()
{
	printf("DEBUG on_pallete_signalPalleteChanged\n");
}



void MainWindow::on_actionNew_triggered()
{
	dataExchanger->createGame();
}


// ------------------- EDIT BUTTONS --------------------- //






void MainWindow::on_MainWindow_iconSizeChanged(QSize iconSize)
{
	Q_UNUSED (iconSize);
	saveGeometry();
}







void MainWindow::on_comboBox_currentIndexChanged(int index)
{
	dataExchanger->layerLevel = index+1;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
	printf(">>>> MainWindow::on_listWidget_currentRowChanged, row: %d\n", currentRow);
	dataExchanger->selectedNPC = currentRow;
}

void MainWindow::on_editNPCButton_clicked()
{
	dataExchanger->editModeNPC = 1;
	QDialog *npc_editor = new DialogNPCEdit;
	npc_editor->show();
	QObject::connect(npc_editor, SIGNAL(finishedNPCEditor()), this, SLOT(reloadComboItems()));
}


void MainWindow::on_actionOne_triggered()
{
    dataExchanger->layerLevel = 1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionTwo_triggered()
{
    dataExchanger->layerLevel = 2;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionThree_triggered()
{
    dataExchanger->layerLevel = 3;
    map_edit_tab->update_edit_area();
}




void MainWindow::on_toolBox_currentChanged(int index)
{
	Q_UNUSED (index);
	dataExchanger->selectedNPC = -1;
}


void MainWindow::on_listWidget_2_currentRowChanged(int currentRow)
{
	dataExchanger->terrainType = currentRow+1;
}

void MainWindow::on_spinBox_valueChanged(int value)
{
        dataExchanger->zoom = value;
}

void MainWindow::on_link_orientation_combobox_currentIndexChanged(int index)
{
	dataExchanger->link_type = index;
}

void MainWindow::on_npc_direction_combo_currentIndexChanged(int index)
{
	dataExchanger->npc_direction = index;
}





void MainWindow::on_pushButton_2_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color1()));
}



void MainWindow::on_pushButton_7_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor1()));
}

void MainWindow::on_pushButton_8_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor2()));
}

void MainWindow::on_pushButton_9_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_keycolor3()));
}


void MainWindow::on_comboBox_6_currentIndexChanged(int index)
{
	dataExchanger->current_player = index;
}

void MainWindow::on_pushButton_3_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color2()));
}

void MainWindow::on_pushButton_4_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_player_color3()));
}





void MainWindow::on_tabWidget_currentChanged(int index)
{
    dataExchanger->selectedNPC = 0;
	dataExchanger->currentMap = 0;
	dataExchanger->currentGame = 0;
	dataExchanger->currentStage = 0;
	dataExchanger->current_player = 0;
	dataExchanger->current_weapon = 0;
    game_prop_tab->reload();
    colorcycle_edit_tab->reload();
    projectile_edit_tab->reload();
    stage_edit_tab->reload();
    weapon_edit_tab->reload();
    object_edit_tab->reload();
    ai_edit_tab->reload();
    npc_edit_tab->reload();
    map_edit_tab->reload();
}


void MainWindow::on_actionLock_Edit_triggered()
{
    if (ui->actionLock_Edit->isChecked()) {
        ui->actionNormal_Edit->setChecked(false);
        ui->actionEraser->setChecked(false);
        ui->actionFill->setChecked(false);
        ui->actionLink->setChecked(false);
        ui->actionStairs->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        map_edit_tab->set_current_box(2);
        dataExchanger->editTool = EDITMODE_LOCK;
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionLock_Edit->setChecked(true);
    }
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionNormal_Edit_triggered()
{
    if (ui->actionNormal_Edit->isChecked()) {
        ui->actionLock_Edit->setChecked(false);
        ui->actionEraser->setChecked(false);
        ui->actionFill->setChecked(false);
        ui->actionLink->setChecked(false);
        ui->actionStairs->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editTool = EDITMODE_NORMAL;
        if (ui->actionEdit_Tileset->isChecked()) {
            map_edit_tab->set_current_box(1);
        } else if (ui->actionEdit_NPC->isChecked()) {
            map_edit_tab->set_current_box(3);
        } else if (ui->actionAdd_Object->isChecked()) {
            map_edit_tab->set_current_box(5);
        }
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionNormal_Edit->setChecked(true);
    }
}

void MainWindow::on_actionEraser_triggered()
{
    if (ui->actionEraser->isChecked()) {
        ui->actionLock_Edit->setChecked(false);
        ui->actionNormal_Edit->setChecked(false);
        ui->actionFill->setChecked(false);
        ui->actionLink->setChecked(false);
        ui->actionStairs->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editTool = EDITMODE_ERASER;
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionEraser->setChecked(true);
    }
}




void MainWindow::on_actionFill_triggered()
{
    if (ui->actionFill->isChecked()) {
        ui->actionLock_Edit->setChecked(false);
        ui->actionNormal_Edit->setChecked(false);
        ui->actionEraser->setChecked(false);
        ui->actionLink->setChecked(false);
        ui->actionStairs->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editTool = EDITMODE_FILL;
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionFill->setChecked(true);
    }
}

void MainWindow::on_actionLink_triggered()
{
    map_edit_tab->set_current_box(4);
    if (ui->actionLink->isChecked()) {
        ui->actionLock_Edit->setChecked(false);
        ui->actionNormal_Edit->setChecked(false);
        ui->actionEraser->setChecked(false);
        ui->actionFill->setChecked(false);
        ui->actionStairs->setChecked(false);
        ui->actionAdd_Object->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editTool = EDITMODE_LINK;
        map_edit_tab->update_edit_area();
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionFill->setChecked(true);
        map_edit_tab->update_edit_area();
    }
}

void MainWindow::on_actionStairs_triggered()
{
    if (ui->actionStairs->isChecked()) {
        ui->actionNormal_Edit->setChecked(false);
        ui->actionEraser->setChecked(false);
        ui->actionFill->setChecked(false);
        ui->actionLock_Edit->setChecked(false);
        ui->actionLink->setChecked(false);
        ui->actionAdd_Object->setChecked(false);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editTool = EDITMODE_STAIRS;
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    } else {
        ui->actionStairs->setChecked(true);
    }
    map_edit_tab->update_edit_area();
}


void MainWindow::on_actionAdd_Object_triggered()
{
    map_edit_tab->set_current_box(5);
    ui->actionEdit_Tileset->setChecked(false);
    ui->actionEdit_NPC->setChecked(false);
    ui->actionNormal_Edit->setChecked(true);
    ui->actionEraser->setChecked(false);
    ui->actionFill->setChecked(false);
    ui->actionLock_Edit->setChecked(false);
    ui->actionLock_Edit->setEnabled(false);
    ui->actionLink->setChecked(false);
    ui->actionAdd_Object->setChecked(true);
    ui->actionAction_subboss->setChecked(false);
    dataExchanger->editMode = EDITMODE_OBJECT;
    dataExchanger->editTool = EDITMODE_NORMAL;
    // to make things simpler, we do not allow "uncheck" of the tool, you must pick another one to uncheck
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionEdit_NPC_triggered()
{
    map_edit_tab->set_current_box(3);
    ui->actionEdit_Tileset->setChecked(false);
    ui->actionAdd_Object->setChecked(false);
    ui->actionEdit_NPC->setChecked(true);
    ui->actionFill->setDisabled(true);
    ui->actionLink->setDisabled(true);
    ui->actionLock_Edit->setDisabled(true);
    ui->actionStairs->setDisabled(true);
    ui->actionAction_subboss->setChecked(false);
    dataExchanger->editMode = EDITMODE_NPC;
}

void MainWindow::on_actionEdit_Tileset_triggered()
{
    ui->actionEdit_NPC->setChecked(false);
    ui->actionAdd_Object->setChecked(false);
    ui->actionEdit_Tileset->setChecked(true);
    ui->actionFill->setDisabled(false);
    ui->actionLink->setDisabled(false);
    ui->actionLock_Edit->setDisabled(false);
    ui->actionStairs->setDisabled(false);
    map_edit_tab->set_current_box(1);
    ui->actionAction_subboss->setChecked(false);
    dataExchanger->editMode = EDITMODE_NORMAL;
}



void MainWindow::on_actionSet_Boss_triggered(bool checked)
{
    if (checked == true) {
        map_edit_tab->set_current_box(3);
        ui->actionEdit_Tileset->setChecked(false);
        ui->actionAdd_Object->setChecked(false);
        ui->actionEdit_NPC->setChecked(false);
        ui->actionFill->setDisabled(true);
        ui->actionLink->setDisabled(true);
        ui->actionLock_Edit->setDisabled(true);
        ui->actionStairs->setDisabled(true);
        ui->actionAction_subboss->setChecked(false);
        dataExchanger->editMode = EDITMODE_SET_BOSS;
    }
}


void MainWindow::on_actionAction_subboss_triggered(bool checked)
{
    if (checked == true) {
        map_edit_tab->set_current_box(3);
        ui->actionEdit_Tileset->setChecked(false);
        ui->actionAdd_Object->setChecked(false);
        ui->actionEdit_NPC->setChecked(false);
        ui->actionFill->setDisabled(true);
        ui->actionLink->setDisabled(true);
        ui->actionLock_Edit->setDisabled(true);
        ui->actionStairs->setDisabled(true);
        dataExchanger->editMode = EDITMODE_SET_SUBBOSS;
    }
}

void MainWindow::on_bg1_filename_currentIndexChanged(const QString &arg1)
{
	if (arg1.toStdString() == std::string("None")) {
        stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename[0] = '\0';
	} else {
        sprintf(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename, "%s", arg1.toStdString().c_str());
	}
    map_edit_tab->update_edit_area();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
	dataExchanger->show_background_color = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg1_y_pos_valueChanged(int arg1)
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].adjust_y = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg1_speed_valueChanged(int arg1)
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed = arg1*10;
    std::cout << "#1 *** on_bg1_speed_valueChanged - setvalue: " << arg1 << ", bg1.speed: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed << std::endl;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_bg2_filename_currentIndexChanged(const QString &arg1)
{
	if (arg1.toStdString() == std::string("None")) {
        stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename[0] = '\0';
	} else {
        sprintf(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename, "%s", arg1.toStdString().c_str());
	}
}

void MainWindow::on_bg2_y_pos_valueChanged(int arg1)
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].adjust_y = arg1;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_bg2_speed_valueChanged(int arg1)
{
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].speed = arg1*10;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg_color_pick_clicked()
{
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_bg_color()));
}

void MainWindow::on_checkBox_2_clicked(bool checked)
{
	dataExchanger->show_bg1 = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_checkBox_3_clicked(bool checked)
{
	dataExchanger->show_bg2 = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_stage_boss_weapon_combo_currentIndexChanged(int index)
{
    stage_data.stages[dataExchanger->currentStage].boss.id_weapon = index;
}

void MainWindow::on_bg1_speed_valueChanged(double arg1)
{
    if (background_filled == false) {
		return;
	}
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed = arg1*10;
    std::cout << "#2 *** on_bg1_speed_valueChanged - setvalue: " << arg1 << ", bg1.speed: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].speed << std::endl;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg2_speed_valueChanged(double arg1)
{
    if (background_filled == false) {
		return;
	}
    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].speed = arg1*10;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionReset_Map_triggered()
{
	for (int i=0; i<MAP_W; i++) {
		for (int j=0; j<MAP_H; j++) {
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked = 0;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x = -1;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y = -1;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x = -1;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y = -1;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename[0] = '\0';
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename[0] = '\0';
		}
	}
}

void MainWindow::on_players_tab_maxshots_valueChanged(int arg1)
{
    if (_data_loading == true) {
		return;
	}
	std::cout << "dataExchanger->current_player: " << dataExchanger->current_player << ", max_shots: " << game_data.players[dataExchanger->current_player].max_shots << std::endl;
	game_data.players[dataExchanger->current_player].max_shots = arg1;
	std::cout << "max_shots: " << game_data.players[dataExchanger->current_player].max_shots << std::endl;
}

void MainWindow::on_can_slide_checkbox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    game_data.players[dataExchanger->current_player].can_slide = checked;
}

void MainWindow::on_players_tab_movespeed_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    game_data.players[dataExchanger->current_player].move_speed = arg1;
}

void MainWindow::on_players_tab_hasshield_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    game_data.players[dataExchanger->current_player].have_shield = checked;
}

void MainWindow::on_players_tab_hp_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    game_data.players[dataExchanger->current_player].HP = arg1;
}

void MainWindow::on_players_tab_name_textChanged(const QString &arg1)
{
    if (_data_loading == true) {
        return;
    }
    sprintf(game_data.players[dataExchanger->current_player].name, "%s", arg1.toStdString().c_str());
}

void MainWindow::on_chargedshot_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) {
        return;
    }
    game_data.players[dataExchanger->current_player].full_charged_projectile_id = index;
}

void MainWindow::on_players_tab_list_combo_2_currentIndexChanged(int index)
{
    dataExchanger->current_player = index;
    std::cout << "MainWindow::on_players_tab_list_combo_2_currentIndexChanged - index: "	 << index << ", max_shots: " << game_data.players[index].max_shots << std::endl;
}

void MainWindow::on_actionSwap_Maps_triggered()
{
    // open swap maps dialog
    QDialog *stage_swap = new stage_swap_dialog;
    stage_swap->show();

}





void MainWindow::on_actionScenes_Editor_triggered()
{
    scenes_window = new SceneEditorWindow();
    scenes_window->show();
}

void MainWindow::on_actionObjects_toggled(bool arg1)
{
    dataExchanger->show_objects_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionNPCs_toggled(bool arg1)
{
    dataExchanger->show_npcs_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionTeleporters_toggled(bool arg1)
{
    dataExchanger->show_teleporters_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionAbout_triggered()
{
    about_window = new AboutWindow();
    about_window->show();

}
