#include "mediator.h"
#include <stdio.h>
#include <QFile>
#include <QDir>
#include "../file/format.h"
#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "../file/file_io.h"

#define EDIT_MODE_NEW 0
#define EDIT_MODE_EDIT 1

// Global static pointer used to ensure a single instance of the class.
Mediator* Mediator::_instance = NULL;

Mediator::Mediator() : stage_data() {
	palleteX=0;
	palleteY=0;
    selectedTileset = "/images/tilesets/default.png";
	editMode = EDITMODE_NORMAL;
	editTool = EDITMODE_NORMAL;
	npcGraphicSize_w = 16;
	npcGraphicSize_h = 16;
	// projectile
	projectileGraphicSize_w = 16;
	projectileGraphicSize_h = 16;

	NpcAddNumber=0;
	layerLevel=1;
	selectedNPC = -1;
    selectedAnimTileset = 0;
	terrainType = 1;
	editModeNPC = EDIT_MODE_NEW;
	currentMap = 0;
	link_type = LINK_HORIZONTAL;
	object_type = 3;
	npc_direction = 0;
    object_direction = 0;

	zoom = 1;
	currentStage = 1;
	sprintf(addNpcFilename, "%c", '\0');
    sprintf(addProjectileFilename, "%c", '\0');

	link_bidi = true;
	link_is_door = false;
	picked_color_n = 0;


	current_weapon = 0;
    current_projectile = 0;
	current_player = 0;
	show_background_color = true;
	show_bg1 = true;
	show_bg2 = true;
    playing_sprites = false;
    current_sprite_type = 0;
    current_sprite_selection = 0;
    current_npc_n = 0;
	current_ai = 0;

    show_objects_flag = true;
    show_npcs_flag = true;
    show_teleporters_flag = true;

    combobox_select_string = "-Select-";

    selectedTileset = FILEPATH + "/images/tilesets/default.png";

    if (enemy_list.size() == 0) { // add one first item to avoid errors
        enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc());
    }
    if (object_list.size() == 0) { // add one first item to avoid errors
        object_list.push_back(CURRENT_FILE_FORMAT::file_object());
    }
    if (ai_list.size() == 0) { // add one first item to avoid errors
        ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
    }
    if (projectile_list.size() == 0) {
        projectile_list.push_back(CURRENT_FILE_FORMAT::file_projectile());
    }
    if (scene_list.size() == 0) {
        scene_list.push_back(CURRENT_FILE_FORMAT::file_scene_list());
    }

    if (player_list.size() == 0) {
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            player_list.push_back(CURRENT_FILE_FORMAT::file_player(i));
        }
    }

    /*
    CURRENT_FILE_FORMAT::file_stage temp_stage;
    for (int i=0; i<FS_MAX_STAGES; i++) {
        stage_data.stages[i].dialog_face_graphics_filename[0] = '\03';
    }
    */


}

void Mediator::save_dialogs()
{
    std::map<int, std::vector<std::string> >::iterator it;
    for (it = stage_dialog_list.begin(); it != stage_dialog_list.end(); it++) {
        std::vector<std::string> list_copy = it->second;
        for (int i=0; i<list_copy.size(); i++) {
            list_copy.at(i) = list_copy.at(i) + "\n";
        }
        fio_str.save_stage_dialogs(it->first, list_copy);
    }
}


Mediator *Mediator::get_instance()
{
    if (!_instance) {
        _instance = new Mediator();
    }
    return _instance;
}


int Mediator::getPalleteX() {
	return palleteX;
}

int Mediator::getPalleteY() {
	return palleteY;
}

void Mediator::setPalleteX(int value) {
	palleteX = value;
}

void Mediator::setPalleteY(int value) {
	palleteY = value;
}

std::string Mediator::getPallete() {
    return selectedTileset;
}

void Mediator::setPallete(std::string filename)
{
    selectedTileset =  filename;
}

void Mediator::setPallete(char *value) {
	selectedTileset = value;
}


void Mediator::load_game() {
    Mediator::get_instance()->fio.read_game(game_data);
    Mediator::get_instance()->fio.read_all_stages(stage_data);
    Mediator::get_instance()->fio.read_all_maps(maps_data);

    enemy_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat");
    if (enemy_list.size() == 0) { // add one first item to avoid errors
        enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc());
    }
    object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat");
    if (object_list.size() == 0) { // add one first item to avoid errors
        object_list.push_back(CURRENT_FILE_FORMAT::file_object());
    }
    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat");

    std::cout << "MEDIATOR::load_game::ai_list.size(): " << ai_list.size() << std::endl;
    if (ai_list.size() == 0) { // add one first item to avoid errors
        for (int i=0; i<enemy_list.size(); i++) {
            ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
        }
    }
    projectile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_projectile>("game_projectile_list.dat");
    if (projectile_list.size() == 0) {
        projectile_list.push_back(CURRENT_FILE_FORMAT::file_projectile());
    }

    scene_list = fio_scenes.load_scenes();
    if (scene_list.size() == 0) {
        scene_list.push_back(CURRENT_FILE_FORMAT::file_scene_list());
    }

    anim_block_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat");

    player_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_player>("player_list.dat");
    if (player_list.size() == 0) {
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            player_list.push_back(CURRENT_FILE_FORMAT::file_player(i));
        }
    }

}

void Mediator::save_game()
{
    Mediator::get_instance()->fio.write_game(game_data);
    Mediator::get_instance()->fio.write_all_stages(stage_data);
    Mediator::get_instance()->fio.write_all_maps(maps_data);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat", enemy_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat", object_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat", ai_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_projectile>("game_projectile_list.dat", projectile_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat", anim_block_list);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_player>("player_list.dat", player_list);

    save_dialogs();

}


void Mediator::centNumberFormat(int n) {
	if (n >= 100) {
		sprintf(centNumber, "%d\0", n);
	} else if (n >= 10) {
		sprintf(centNumber, "0%d\0", n);
	} else {
		sprintf(centNumber, "00%d\0", n);
	}
	//printf("DEBUG.Mediator::centNumberFormat - centNumber: %s\n", centNumber);
}


void Mediator::resetMap(int map_n) {
	int j, k;
/*
    sprintf(map.filename, "%s/images/tilesets/default.png", FILEPATH.c_str());
	for (j=0; j<MAP_W; j++) {
		for (k=0; k<MAP_H; k++) {
			map.tiles[j][k].locked=0;
			map.tiles[j][k].tile1.x=-1;
			map.tiles[j][k].tile1.y=-1;
			map.tiles[j][k].tile2.x=-1;
			map.tiles[j][k].tile2.y=-1;
			map.tiles[j][k].tile3.x=-1;
			map.tiles[j][k].tile3.y=-1;
		}
	}
*/
}

/*
void Mediator::getGameName(int n) {
	long lSize;
	char filename[512];
	centNumberFormat(n);
	struct format_v1_0::file_game temp_game;
    sprintf(filename, "%s/game/%s.gme", FILEPATH.c_str(), centNumber);
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		fseek (fp , 0 , SEEK_END);
		lSize = ftell (fp);
		rewind (fp);
		fread(&temp_game, sizeof(temp_game), 1, fp);
		fclose(fp);
		printf("DEBUG;Mediator::getGameName - temp_game.name: %s\n", temp_game.name);
		sprintf(gameName, "%s", temp_game.name);
	} else {
		printf("DEBUG.Mediator::getGameName - Error opening file '%s'.\n",filename);
		sprintf(gameName, "");
	}
}
*/


int Mediator::get_stage_n(const int map_n) {
	if (map_n < 10) {
		return map_n;
	}
	if (map_n < 19) {
		return map_n-9;
	}
	return map_n-18;
}





