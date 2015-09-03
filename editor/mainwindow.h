#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QComboBox>
#include <QListWidget>

#include "mainwindow_tab/npc_edit.h"
#include "mainwindow_tab/object_tab.h"
#include "mainwindow_tab/weapon_edit.h"
#include "mainwindow_tab/stage_edit.h"
#include "mainwindow_tab/projectile_edit.h"
#include "mainwindow_tab/artificial_inteligence_tab.h"
#include "mainwindow_tab/colorcycle.h"
#include "mainwindow_tab/game_properties_tab.h"
#include "mainwindow_tab/map_tab.h"
#include "mainwindow_tab/player_edit.h"
#include "mainwindow_tab/armor_edit.h"
#include "scenes/sceneeditorwindow.h"
#include "aboutwindow.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //QStandardItemModel model;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void fillGraphicsList();
	void fillMapList();
	void fillNpcList();
	void fillObjList();
	void fillStageList();


private:
    Ui::MainWindow *ui;
	npc_edit *npc_edit_tab;
	artificial_inteligence_tab *ai_edit_tab;
	object_tab *object_edit_tab;
	weapon_edit *weapon_edit_tab;
	stage_edit *stage_edit_tab;
    projectile_edit *projectile_edit_tab;
	colorcycle_edit *colorcycle_edit_tab;
    game_properties_tab *game_prop_tab;
    map_tab *map_edit_tab;
    player_edit *player_edit_tab;
    armor_edit *armor_edit_tab;

	void fill_NPC_edit_tab();
	void fill_background_list();

private slots:

 void on_npc_direction_combo_currentIndexChanged(int index);
	void on_link_orientation_combobox_currentIndexChanged(int index);
	void on_spinBox_valueChanged(int );
	void on_listWidget_2_currentRowChanged(int currentRow);
	void on_toolBox_currentChanged(int index);
    void on_actionAdd_Object_triggered();
    void on_actionThree_triggered();
    void on_actionTwo_triggered();
    void on_actionOne_triggered();
    void on_editNPCButton_clicked();
    void on_listWidget_currentRowChanged(int currentRow);
    void on_actionEdit_Tileset_triggered();
    void on_actionEdit_NPC_triggered();
    void on_comboBox_currentIndexChanged(int index);
    void on_actionStairs_triggered();
    void on_actionLink_triggered();
    void on_MainWindow_iconSizeChanged(QSize iconSize);
    void on_actionNew_triggered();
    void on_actionFill_triggered();
    void on_actionEraser_triggered();
    void on_actionNormal_Edit_triggered();
    void on_actionLock_Edit_triggered();
    void on_pallete_signalPalleteChanged();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionQuit_triggered();

	void on_pushButton_2_clicked();

	void on_pushButton_7_clicked();

	void on_pushButton_8_clicked();

	void on_pushButton_9_clicked();

	void on_comboBox_6_currentIndexChanged(int index);

	void on_pushButton_3_clicked();

	void on_pushButton_4_clicked();

	void on_tabWidget_currentChanged(int index);

    void on_actionSet_Boss_triggered(bool checked);

	void on_bg1_filename_currentIndexChanged(const QString &arg1);

	void on_checkBox_clicked(bool checked);

	void on_bg1_y_pos_valueChanged(int arg1);

	void on_bg2_filename_currentIndexChanged(const QString &arg1);

	void on_bg2_y_pos_valueChanged(int arg1);

	void on_bg1_speed_valueChanged(int arg1);

	void on_bg2_speed_valueChanged(int arg1);

	void on_bg_color_pick_clicked();

	void on_checkBox_2_clicked(bool checked);

	void on_checkBox_3_clicked(bool checked);

	void on_stage_boss_weapon_combo_currentIndexChanged(int index);

	void on_bg1_speed_valueChanged(double arg1);

	void on_bg2_speed_valueChanged(double arg1);

	void on_actionReset_Map_triggered();

	void on_players_tab_maxshots_valueChanged(int arg1);

	void on_can_slide_checkbox_toggled(bool checked);

    void on_players_tab_movespeed_valueChanged(int arg1);

    void on_players_tab_hasshield_toggled(bool checked);

    void on_players_tab_hp_valueChanged(int arg1);

    void on_players_tab_name_textChanged(const QString &arg1);

    void on_chargedshot_combo_currentIndexChanged(int index);

    void on_players_tab_list_combo_2_currentIndexChanged(int index);

    void on_actionSwap_Maps_triggered();

    void on_actionAction_subboss_triggered(bool checked);

    void on_actionScenes_Editor_triggered();

    void on_actionObjects_toggled(bool arg1);

    void on_actionNPCs_toggled(bool arg1);

    void on_actionTeleporters_toggled(bool arg1);

    void on_actionAbout_triggered();

private:
	// USED IN NPC TAB
	std::string _npcedit_tab_graphic;
	int _npcedit_tab_selectednpc;
	bool _data_loading;
    SceneEditorWindow* scenes_window;
    AboutWindow* about_window;

};


#endif // MAINWINDOW_H
