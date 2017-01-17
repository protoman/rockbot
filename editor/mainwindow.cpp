#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defines.h"
#include "addwizard.h"
#include "dialognpcedit.h"
#include "dialogobjectedit.h"
#include "loadgamepicker.h"
#include "stage_swap_dialog.h"
#include "newgamedialog.h"

#include <QListView>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>

#include "../defines.h"
#include "../file/version.h"
#include "file/fio_scenes.h"


#include "common.h"

extern std::string GAMEPATH;
extern std::string FILEPATH;
extern std::string GAMENAME;

bool background_filled = false;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _npcedit_tab_selectednpc(0), _data_loading(false)
{
	ui->setupUi(this);
    QString window_title = QString("Rockbot Editor ") + QString(VERSION_NUMBER);
    setWindowTitle(window_title);

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


    // insert GAME_PROPERTIES tab form
    game_prop_tab = new game_properties_tab();
    ui->gamePropertiesScrollarea->setWidget(game_prop_tab);

    map_edit_tab = new map_tab();
    ui->MapScrollArea->setWidget(map_edit_tab);


    player_edit_tab = new player_edit();
    ui->PlayerScrollArea->setWidget(player_edit_tab);

    armor_edit_tab = new armor_edit();
    ui->armorScrollArea->setWidget(armor_edit_tab);

    game_scenes_tab = new GameScenes();
    ui->gameScenes_scrollArea->setWidget(game_scenes_tab);

    anim_tiles_edit_tab = new anim_tiles_edit();
    ui->anim_tab_scrollArea->setWidget(anim_tiles_edit_tab);

    scenes_window = new SceneEditorWindow;
    QObject::connect(scenes_window, SIGNAL(scenes_editor_window_closed()), this, SLOT(on_scenes_editor_window_closed()));
    scenes_window->hide();

}

MainWindow::~MainWindow()
{
    delete scenes_window;
    delete ui;
}

void MainWindow::loadData()
{
    Mediator::get_instance()->load_game();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Rockbot Editor :: Game Editor", tr("Save data before leaving?\n"), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        Mediator::get_instance()->save_game();
    }
    event->accept();

    /*
    QMessageBox msgBox;
    msgBox.setWindowTitle("Rockbot Editor");
    msgBox.setText("Save data before leaving?\n");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::No);

    QMessageBox::StandardButton resBtn = msgBox.exec();
    if (resBtn == QMessageBox::Yes) {
        on_actionSave_triggered();
    } else if (resBtn == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    */
    event->accept();
}

void MainWindow::show_critial_error(QString error_msg)
{
    QMessageBox msgBox;
    msgBox.setText(error_msg);
    msgBox.exec();
    close();
}

void MainWindow::copy_path(QString src, QString dst)
{
    src = src.replace(QString("//"), QString("/"));
    dst = dst.replace(QString("//"), QString("/"));
    //std::cout << "MainWindow::copy_path::src: " << src.toStdString() << ", dst: " << dst.toStdString() << std::endl;
    //exit(-1);
    QDir dir(src);
    if (! dir.exists())
        return;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        QDir dst_dir(dst_path);
        dst_dir.mkpath(dst_path);
        copy_path(src+ QDir::separator() + d, dst_path);
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
}

void MainWindow::reload()
{
    Mediator::get_instance()->selectedNPC = 0;
    Mediator::get_instance()->currentMap = 0;
    Mediator::get_instance()->currentGame = 0;
    Mediator::get_instance()->currentStage = 0;
    Mediator::get_instance()->current_player = 0;
    Mediator::get_instance()->current_weapon = 0;
    game_prop_tab->reload();
    projectile_edit_tab->reload();
    stage_edit_tab->reload();
    weapon_edit_tab->reload();
    object_edit_tab->reload();
    ai_edit_tab->reload();
    npc_edit_tab->reload();
    map_edit_tab->reload();
    player_edit_tab->reload();
    armor_edit_tab->reload();
    anim_tiles_edit_tab->reload();
    game_scenes_tab->reload();
    this->show();
}





void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionSave_triggered()
{
    Mediator::get_instance()->save_game();
}

void MainWindow::on_actionOpen_triggered()
{
    this->hide();
	QDialog *open = new loadGamePicker;
    QObject::connect(open, SIGNAL(game_picked()), this, SLOT(reload()));
	open->show();
}

void MainWindow::on_pallete_signalPalleteChanged()
{
	printf("DEBUG on_pallete_signalPalleteChanged\n");
}



void MainWindow::on_actionNew_triggered()
{
    NewGameDialog *new_game_dialog = new NewGameDialog();
    QObject::connect(new_game_dialog, SIGNAL(on_accepted(QString)), this, SLOT(on_new_game_accepted(QString)));
    new_game_dialog->show();
}


// ------------------- EDIT BUTTONS --------------------- //






void MainWindow::on_MainWindow_iconSizeChanged(QSize iconSize)
{
	Q_UNUSED (iconSize);
	saveGeometry();
}







void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    Mediator::get_instance()->layerLevel = index+1;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
	printf(">>>> MainWindow::on_listWidget_currentRowChanged, row: %d\n", currentRow);
    Mediator::get_instance()->selectedNPC = currentRow;
}

void MainWindow::on_editNPCButton_clicked()
{
    Mediator::get_instance()->editModeNPC = 1;
	QDialog *npc_editor = new DialogNPCEdit;
	npc_editor->show();
	QObject::connect(npc_editor, SIGNAL(finishedNPCEditor()), this, SLOT(reloadComboItems()));
}


void MainWindow::on_actionOne_triggered()
{
    Mediator::get_instance()->layerLevel = 1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionTwo_triggered()
{
    Mediator::get_instance()->layerLevel = 2;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionThree_triggered()
{
    Mediator::get_instance()->layerLevel = 3;
    map_edit_tab->update_edit_area();
}




void MainWindow::on_toolBox_currentChanged(int index)
{
	Q_UNUSED (index);
    Mediator::get_instance()->selectedNPC = -1;
}


void MainWindow::on_listWidget_2_currentRowChanged(int currentRow)
{
    Mediator::get_instance()->terrainType = currentRow+1;
}

void MainWindow::on_spinBox_valueChanged(int value)
{
        Mediator::get_instance()->zoom = value;
}

void MainWindow::on_link_orientation_combobox_currentIndexChanged(int index)
{
    Mediator::get_instance()->link_type = index;
}

void MainWindow::on_npc_direction_combo_currentIndexChanged(int index)
{
    Mediator::get_instance()->npc_direction = index;
}








void MainWindow::on_comboBox_6_currentIndexChanged(int index)
{
    Mediator::get_instance()->current_player = index;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    reload();
}



void MainWindow::on_bg1_filename_currentIndexChanged(const QString &arg1)
{
	if (arg1.toStdString() == std::string("None")) {
        Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename[0] = '\0';
	} else {
        sprintf(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename, "%s", arg1.toStdString().c_str());
	}
    map_edit_tab->update_edit_area();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    Mediator::get_instance()->show_background_color = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg1_y_pos_valueChanged(int arg1)
{
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].adjust_y = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg1_speed_valueChanged(int arg1)
{
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].speed = arg1*10;
    std::cout << "#1 *** on_bg1_speed_valueChanged - setvalue: " << arg1 << ", bg1.speed: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].speed << std::endl;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_bg2_filename_currentIndexChanged(const QString &arg1)
{
	if (arg1.toStdString() == std::string("None")) {
        Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename[0] = '\0';
	} else {
        sprintf(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename, "%s", arg1.toStdString().c_str());
	}
}

void MainWindow::on_bg2_y_pos_valueChanged(int arg1)
{
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].adjust_y = arg1;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_bg2_speed_valueChanged(int arg1)
{
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].speed = arg1*10;
    map_edit_tab->update_edit_area();
}


void MainWindow::on_checkBox_2_clicked(bool checked)
{
    Mediator::get_instance()->show_bg1 = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_checkBox_3_clicked(bool checked)
{
    Mediator::get_instance()->show_fg_layer = checked;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_stage_boss_weapon_combo_currentIndexChanged(int index)
{
    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon = index;
}

void MainWindow::on_bg1_speed_valueChanged(double arg1)
{
    if (background_filled == false) {
		return;
	}
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].speed = arg1*10;
    std::cout << "#2 *** on_bg1_speed_valueChanged - setvalue: " << arg1 << ", bg1.speed: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].speed << std::endl;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_bg2_speed_valueChanged(double arg1)
{
    if (background_filled == false) {
		return;
	}
    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].speed = arg1*10;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionReset_Map_triggered()
{
	for (int i=0; i<MAP_W; i++) {
		for (int j=0; j<MAP_H; j++) {
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked = 0;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x = -1;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y = -1;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x = -1;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y = -1;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename[0] = '\0';
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename[0] = '\0';
		}
	}
}

void MainWindow::on_players_tab_maxshots_valueChanged(int arg1)
{
    if (_data_loading == true) {
		return;
	}
    std::cout << "Mediator::get_instance()->current_player: " << Mediator::get_instance()->current_player << ", max_shots: " << Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].max_shots << std::endl;
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].max_shots = arg1;
    std::cout << "max_shots: " << Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].max_shots << std::endl;
}

void MainWindow::on_can_slide_checkbox_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].can_slide = checked;
}

void MainWindow::on_players_tab_movespeed_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].move_speed = arg1;
}

void MainWindow::on_players_tab_hasshield_toggled(bool checked)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].have_shield = checked;
}

void MainWindow::on_players_tab_hp_valueChanged(int arg1)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].HP = arg1;
}

void MainWindow::on_players_tab_name_textChanged(const QString &arg1)
{
    if (_data_loading == true) {
        return;
    }
    sprintf(Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].name, "%s", arg1.toStdString().c_str());
}

void MainWindow::on_chargedshot_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) {
        return;
    }
    Mediator::get_instance()->player_list[Mediator::get_instance()->current_player].full_charged_projectile_id = index;
}

void MainWindow::on_players_tab_list_combo_2_currentIndexChanged(int index)
{
    Mediator::get_instance()->current_player = index;
    std::cout << "MainWindow::on_players_tab_list_combo_2_currentIndexChanged - index: " << index << ", max_shots: " << Mediator::get_instance()->player_list[index].max_shots << std::endl;
}

void MainWindow::on_actionSwap_Maps_triggered()
{
    // open swap maps dialog
    QDialog *stage_swap = new stage_swap_dialog;
    stage_swap->show();
}





void MainWindow::on_actionScenes_Editor_triggered()
{
    scenes_window->reload();
    scenes_window->show();
}

void MainWindow::on_actionObjects_toggled(bool arg1)
{
    Mediator::get_instance()->show_objects_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionNPCs_toggled(bool arg1)
{
    Mediator::get_instance()->show_npcs_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionTeleporters_toggled(bool arg1)
{
    Mediator::get_instance()->show_teleporters_flag = arg1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionAbout_triggered()
{
    about_window = new AboutWindow();
    about_window->show();

}

void MainWindow::on_actionImage_Browser_triggered()
{
    files_editor_window = new FilesEditor();
    files_editor_window->show();
}


void MainWindow::on_new_game_accepted(QString name)
{
    /// @TODO: create game files
    QString games_folder_path = QString(GAMEPATH.c_str()) + QString("/games/");
    if (QDir(games_folder_path).exists() == false) {
        if (QDir().mkdir(games_folder_path) == false) {
            QString error_msg = QString("Can't create games container folder '") + games_folder_path + QString("'.");
            show_critial_error(error_msg);
            return;
        }
    }
    QString filepath = QString(GAMEPATH.c_str()) + QString("/games/") + name;
    if (QDir().mkdir(filepath) == false) {
        QString error_msg = QString("Can't create new game folder '") + filepath + QString("'.");
        show_critial_error(error_msg);
        return;
    }

    // copy data directories
    QString template_path = QString(GAMEPATH.c_str()) + QString("/template/");
    copy_path(template_path, filepath);

    FILEPATH = GAMEPATH + std::string("/games/") + name.toStdString() + std::string("/");

    // generate empty/default game files
    CURRENT_FILE_FORMAT::file_io fio;
    fio.generate_files();

    CURRENT_FILE_FORMAT::fio_strings fio_str;
    fio_str.create_files();

    std::string scenes_filename = std::string(FILEPATH) + "/scenes/";

    if (QDir(scenes_filename.c_str()).exists() == false) {
        QDir().mkdir(scenes_filename.c_str());
    }

    CURRENT_FILE_FORMAT::fio_scenes fio_scenes;
    fio_scenes.generate_files();


    /// @TODO: copy image files
    Mediator::get_instance()->load_game();

    GAMENAME = name.toStdString();

    reload();

    this->show();
}

void MainWindow::on_load_game_accepted()
{
    Mediator::get_instance()->load_game();
    reload();
}

void MainWindow::on_actionMovie_Editor_triggered()
{
    scenes_window->reload();
    scenes_window->show();
}

void MainWindow::on_actionStrings_Editor_triggered()
{
    strings_editor_window = new StringsEditor(this, 0);
    strings_editor_window->show();
}

void MainWindow::on_actionReset_Stage_Links_triggered()
{
    for (int i=0; i<FS_STAGE_MAX_LINKS; i++) {
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].bidirecional = true;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny = -1;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin = -1;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].is_door = false;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x = 0;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y = 0;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x = 0;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y = 0;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size = 1;
        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type = LINK_VERTICAL;
    }
}

void MainWindow::on_actionZoomOne_triggered()
{
    Mediator::get_instance()->zoom = 1;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionZoomTwo_triggered()
{
    Mediator::get_instance()->zoom = 2;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_actionZoomThree_triggered()
{
    Mediator::get_instance()->zoom = 3;
    map_edit_tab->update_edit_area();
}

void MainWindow::on_scenes_editor_window_closed()
{
    game_scenes_tab->reload();
}
