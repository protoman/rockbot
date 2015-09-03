#ifndef MEDIATOR_H
#define MEDIATOR_H
#include <QString>
#include "../file/format.h"
#include "../file/v_1.h"


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
private:
	int palleteX;
	int palleteY;
    std::string selectedTileset;

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
	bool show_bg2;
    // sprites
    bool playing_sprites;
    int current_sprite_type;
    int current_npc_n;
	bool show_colorcycle1;
	bool show_colorcycle2;
	bool show_colorcycle3;
    int selectedAnimTileset;
    bool show_objects_flag;
    bool show_npcs_flag;
    bool show_teleporters_flag;


	struct format_v1_0::list_map_npc *npc_map_list;
	struct format_v1_0::npc_static_data_list *static_npc_list;

	Mediator();
	int getPalleteX();
	int getPalleteY();
	void setPalleteX(int value);
	void setPalleteY(int value);
    std::string getPallete();
	void setPallete(char *value);
	void initGameVar();
	void loadGame(int n);

	int get_stage_n(const int map_n);

	// auxiliary functions for map_links


	int npcGraphicSize_w;
	int npcGraphicSize_h;

	// projectile
	int projectileGraphicSize_w;
	int projectileGraphicSize_h;
	char addProjectileFilename[512];

	bool link_bidi;
	bool link_is_door;


	void createGame();
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

};


#endif // MEDIATOR_H
