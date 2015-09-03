#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <QDir>
#include <QListWidget>
#include <QComboBox>
#include "../file/format.h"

extern std::string FILEPATH;
extern CURRENT_FILE_FORMAT::file_game game_data;


class common
{

public:
    common();
    ~common();
	static void fill_files_combo(std::string directory, QComboBox* combo, bool show_none=false);
    static void fill_graphicfiles_listwidget(std::string directory, QListWidget* listWidget);
    static void fill_graphicfiles_combobox(std::string directory, QComboBox* comboWidget);
    static void fill_npc_combo(QComboBox* combo);
	static void fill_object_combo(QComboBox* combo);
	static void fill_weapons_combo(QComboBox* combo);
    static void fill_weapons_names_combo(QComboBox* combo);
    static void fill_weapons_combo_plus(QComboBox* combo); // same as fill_weapons_combo(), but also add jet and coil items
    static void fill_projectiles_combo(QComboBox* combo);
	static void fill_trajectories_combo(QComboBox *combo);
    static void fill_ai_actions_combo(QComboBox *combo, bool include_null=false);
	static void fill_ai_options_combo(int action, QComboBox *combo);
	static void fill_ai_list(QComboBox *combo);
    static void fill_stages_combo(QComboBox *combo);
    static void fill_players_combo(QComboBox *combo);
    static void fill_map_list_combo(QComboBox *combo);
    static void fill_npc_listwidget(QListWidget* listWidget);
    static void fill_object_listWidget(QListWidget* listWidget);
    static std::vector<std::string> get_npc_names_list();

private:


};


#endif // COMMON_H
