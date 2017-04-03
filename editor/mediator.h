#ifndef MEDIATOR_H
#define MEDIATOR_H
#include <QString>
#include "file/format.h"
#include "file/file_io.h"
#include "file/fio_scenes.h"
#include "file/fio_strings.h"

#include "scenes/scenesmediator.h"

struct st_player_graphics_data {
	st_size frame_size;
	st_rectangle hit_area;
	std::string graphics_filename;

	st_player_graphics_data()
	{

		frame_size.width = 29;
        frame_size.height = 29;
		hit_area.x = 4;
		hit_area.y = 8;
		hit_area.w = 21;
		hit_area.h = 24;
		graphics_filename = std::string("p1.png");
	}
};

class Mediator {
public:
    static Mediator* get_instance();


public:
	char gameName[50];
	char centNumber[4];
	int currentGame;
	int currentMap;
	int currentStage;
	//QString *NpcPreviewFile;
	char addNpcFilename[512];
	int NpcAddNumber;
	int layerLevel;
	int selectedNPC;
	int editMode;
	int editTool;
	int editModeNPC;
	int object_type;
	int terrainType;
	int zoom;
	int link_type;
	int npc_direction;
    int object_direction;
	bool show_background_color;
	bool show_bg1;
    bool show_fg_layer;
    // sprites
    bool playing_sprites;
    int current_sprite_type;
    int current_sprite_selection;
    int current_npc_n;
    int selectedAnimTileset;
    bool show_objects_flag;
    bool show_npcs_flag;
    bool show_teleporters_flag;
    short stage_select_edit_mode;
    short stage_select_stage_number;
    CURRENT_FILE_FORMAT::file_stage_select stage_select_data;


	int getPalleteX();
	int getPalleteY();
	void setPalleteX(int value);
	void setPalleteY(int value);
    std::string getPallete();
    void setPallete(std::string filename);
	void setPallete(char *value);
    void load_game();
    void save_game();
    void clean_data();                                      // any sanity data cleaning goest here
    void temp_fix_player_colors_order();
    void load_stage_select_data();
    void save_stage_select_data();

    //void convertProjectileListToV2();

	int get_stage_n(const int map_n);

    void reload_game_scenes();

	// auxiliary functions for map_links


	int npcGraphicSize_w;
	int npcGraphicSize_h;

	// projectile
	int projectileGraphicSize_w;
	int projectileGraphicSize_h;
	char addProjectileFilename[512];

	bool link_bidi;
	bool link_is_door;


	void centNumberFormat(int n);
    //void getGameName(int n);
	void resetMap(int);

	struct st_player_graphics_data player_graphics_data;

	int picked_color_n;
	st_color colormap[67]; /// @TODO: colormap should be a combination of three enums
	int current_weapon;
    int current_projectile;
	int current_player;
    int current_ai;

    // old globals
    CURRENT_FILE_FORMAT::file_game game_data;
    CURRENT_FILE_FORMAT::file_stages stage_data;
    CURRENT_FILE_FORMAT::file_map maps_data[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]; // stage, map_n
    CURRENT_FILE_FORMAT::file_castle castle_data;

    std::vector<CURRENT_FILE_FORMAT::file_npc> enemy_list;
    std::vector<CURRENT_FILE_FORMAT::file_object> object_list;
    std::vector<CURRENT_FILE_FORMAT::file_artificial_inteligence> ai_list;


    //std::vector<CURRENT_FILE_FORMAT::file_projectile> projectile_list;
    std::vector<CURRENT_FILE_FORMAT::file_projectilev2> projectile_list_v2;

    std::vector<CURRENT_FILE_FORMAT::file_scene_list> scene_list;
    std::vector<CURRENT_FILE_FORMAT::file_anim_block> anim_block_list;
    std::vector<CURRENT_FILE_FORMAT::file_player> player_list;



    CURRENT_FILE_FORMAT::file_io fio;
    CURRENT_FILE_FORMAT::fio_scenes fio_scenes;
    bool GAME_FLAGS[FLAG_COUNT]; // compability for fio
    std::string combobox_select_string;

    // stage dialogs
    std::map<int, std::vector<std::string> > stage_dialog_list;



private:
    Mediator();
    Mediator(Mediator const&){};             // copy constructor is private
    Mediator& operator=(Mediator const&){};  // assignment operator is private

    void save_dialogs();
    void load_dialogs();


private:
    static Mediator* _instance;
    int palleteX;
    int palleteY;
    std::string selectedTileset;
    fio_common fio_cmm;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
};


#endif // MEDIATOR_H
