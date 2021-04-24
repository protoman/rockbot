#include "mediator.h"
#include <stdio.h>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include "../file/format.h"
#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "../file/file_io.h"

#define EDIT_MODE_NEW 0
#define EDIT_MODE_EDIT 1

#define PROJECTILE_FILE_V3 "data/game_projectile_list_v3.dat"

// Global static pointer used to ensure a single instance of the class.
Mediator* Mediator::_instance = NULL;

Mediator::Mediator() : stage_data(), stage_extra_data() {
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

    zoom = 2;
	currentStage = 1;
    currentDifficulty = DIFFICULTY_EASY;
    currentDifficultyMode = DIFFICULTY_MODE_GREATER;
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
	show_fg_layer = true;
    playing_sprites = false;
    current_sprite_type = 0;
    current_sprite_selection = 0;
    current_npc_n = 0;
	current_ai = 0;

    show_objects_flag = true;
    show_npcs_flag = true;
    show_teleporters_flag = true;
    show_grid = true;

    combobox_select_string = "-Select-";

    selectedTileset = FILEPATH + "/images/tilesets/default.png";

    if (enemy_list.size() == 0) { // add one first item to avoid errors
        enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc_v3_1_2());
    }
    if (object_list.size() == 0) { // add one first item to avoid errors
        object_list.push_back(CURRENT_FILE_FORMAT::file_object());
    }
    if (ai_list.size() == 0) { // add one first item to avoid errors
        ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
    }

    if (scene_list.size() == 0) {
        scene_list.push_back(CURRENT_FILE_FORMAT::file_scene_list());
    }

    if (player_list_v3_1.size() == 0) {
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            player_list_v3_1.push_back(CURRENT_FILE_FORMAT::file_player_v3_1_1(i));
        }
    }

    stage_dialog_list.clear();

    /*
    CURRENT_FILE_FORMAT::file_stage temp_stage;
    for (int i=0; i<FS_MAX_STAGES; i++) {
        stage_data.stages[i].dialog_face_graphics_filename[0] = '\03';
    }
    */


    stage_select_edit_mode = STAGE_SELECT_EDIT_MODE_PATH;
    stage_select_stage_number = STAGE1;



    colormap[0].r = 97;
    colormap[0].g = 97;
    colormap[0].b = 97;
    colormap[1].r = 39;
    colormap[1].g = 27;
    colormap[1].b = 143;
    colormap[2].r = 0;
    colormap[2].g = 0;
    colormap[2].b = 171;
    colormap[3].r = 71;
    colormap[3].g = 0;
    colormap[3].b = 159;
    colormap[4].r = 143;
    colormap[4].g = 0;
    colormap[4].b = 119;
    colormap[5].r = 171;
    colormap[5].g = 0;
    colormap[5].b = 19;
    colormap[6].r = 167;
    colormap[6].g = 0;
    colormap[6].b = 0;
    colormap[7].r = 127;
    colormap[7].g = 11;
    colormap[7].b = 0;
    colormap[8].r = 67;
    colormap[8].g = 47;
    colormap[8].b = 0;
    colormap[9].r = 0;
    colormap[9].g = 71;
    colormap[9].b = 0;
    colormap[10].r = 0;
    colormap[10].g = 81;
    colormap[10].b = 0;
    colormap[11].r = 0;
    colormap[11].g = 63;
    colormap[11].b = 23;
    colormap[12].r = 27;
    colormap[12].g = 63;
    colormap[12].b = 95;
    colormap[13].r = 0;
    colormap[13].g = 0;
    colormap[13].b = 0;
    colormap[14].r = 196;
    colormap[14].g = 93;
    colormap[14].b = 0;
    colormap[15].r = 255;
    colormap[15].g = 153;
    colormap[15].b = 102;
    colormap[16].r = 188;
    colormap[16].g = 188;
    colormap[16].b = 188;
    colormap[17].r = 0;
    colormap[17].g = 115;
    colormap[17].b = 239;
    colormap[18].r = 35;
    colormap[18].g = 59;
    colormap[18].b = 239;
    colormap[19].r = 131;
    colormap[19].g = 0;
    colormap[19].b = 243;
    colormap[20].r = 191;
    colormap[20].g = 0;
    colormap[20].b = 191;
    colormap[21].r = 231;
    colormap[21].g = 0;
    colormap[21].b = 91;
    colormap[22].r = 219;
    colormap[22].g = 43;
    colormap[22].b = 0;
    colormap[23].r = 203;
    colormap[23].g = 79;
    colormap[23].b = 15;
    colormap[24].r = 139;
    colormap[24].g = 115;
    colormap[24].b = 0;
    colormap[25].r = 0;
    colormap[25].g = 151;
    colormap[25].b = 0;
    colormap[26].r = 0;
    colormap[26].g = 171;
    colormap[26].b = 0;
    colormap[27].r = 0;
    colormap[27].g = 147;
    colormap[27].b = 59;
    colormap[28].r = 0;
    colormap[28].g = 131;
    colormap[28].b = 139;
    colormap[29].r = 255;
    colormap[29].g = 102;
    colormap[29].b = 0;
    colormap[30].r = 255;
    colormap[30].g = 51;
    colormap[30].b = 0;
    colormap[31].r = 204;
    colormap[31].g = 204;
    colormap[31].b = 0;
    colormap[32].r = 235;
    colormap[32].g = 235;
    colormap[32].b = 235;
    colormap[33].r = 63;
    colormap[33].g = 191;
    colormap[33].b = 255;
    colormap[34].r = 95;
    colormap[34].g = 151;
    colormap[34].b = 255;
    colormap[35].r = 167;
    colormap[35].g = 139;
    colormap[35].b = 253;
    colormap[36].r = 247;
    colormap[36].g = 123;
    colormap[36].b = 255;
    colormap[37].r = 255;
    colormap[37].g = 119;
    colormap[37].b = 183;
    colormap[38].r = 255;
    colormap[38].g = 119;
    colormap[38].b = 99;
    colormap[39].r = 255;
    colormap[39].g = 155;
    colormap[39].b = 59;
    colormap[40].r = 243;
    colormap[40].g = 191;
    colormap[40].b = 63;
    colormap[41].r = 131;
    colormap[41].g = 211;
    colormap[41].b = 19;
    colormap[42].r = 79;
    colormap[42].g = 223;
    colormap[42].b = 75;
    colormap[43].r = 88;
    colormap[43].g = 248;
    colormap[43].b = 152;
    colormap[44].r = 0;
    colormap[44].g = 235;
    colormap[44].b = 219;
    colormap[45].r = 102;
    colormap[45].g = 51;
    colormap[45].b = 204;
    colormap[46].r = 51;
    colormap[46].g = 102;
    colormap[46].b = 255;
    colormap[47].r = 81;
    colormap[47].g = 81;
    colormap[47].b = 81;
    colormap[48].r = 230;
    colormap[48].g = 255;
    colormap[48].b = 0;
    colormap[49].r = 171;
    colormap[49].g = 231;
    colormap[49].b = 255;
    colormap[50].r = 199;
    colormap[50].g = 215;
    colormap[50].b = 255;
    colormap[51].r = 215;
    colormap[51].g = 203;
    colormap[51].b = 255;
    colormap[52].r = 255;
    colormap[52].g = 199;
    colormap[52].b = 255;
    colormap[52].r = 255;
    colormap[53].r = 255;
    colormap[53].g = 199;
    colormap[53].b = 219;
    colormap[54].r = 255;
    colormap[54].g = 191;
    colormap[54].b = 179;
    colormap[55].r = 255;
    colormap[55].g = 219;
    colormap[55].b = 171;
    colormap[56].r = 255;
    colormap[56].g = 231;
    colormap[56].b = 163;
    colormap[57].r = 227;
    colormap[57].g = 255;
    colormap[57].b = 163;
    colormap[58].r = 171;
    colormap[58].g = 243;
    colormap[58].b = 191;
    colormap[59].r = 179;
    colormap[59].g = 255;
    colormap[59].b = 207;
    colormap[60].r = 159;
    colormap[60].g = 255;
    colormap[60].b = 243;
    colormap[61].r = 112;
    colormap[61].g = 110;
    colormap[61].b = 110;
    colormap[62].r = 255;
    colormap[62].g = 192;
    colormap[62].b = 0;
    colormap[63].r = 255;
    colormap[63].g = 234;
    colormap[63].b = 0;
    colormap[64].r = 55;
    colormap[64].g = 255;
    colormap[64].b = 0;
    colormap[65].r = 255;
    colormap[65].g = 0;
    colormap[65].b = 255;
    colormap[66].r = 0;
    colormap[66].g = 255;
    colormap[66].b = 255;

}

void Mediator::save_dialogs()
{

    std::map<int, std::vector<std::string> >::iterator it;
    for (int i=0; i<LANGUAGE_COUNT; i++) {
        for (it = stage_dialog_list[i].begin(); it != stage_dialog_list[i].end(); it++) {
            std::vector<std::string> list_copy = it->second;

            if (list_copy.size() > STAGE_DIALOG_NUMBER) {
                return;
            }

            for (int i=0; i<list_copy.size(); i++) {
                list_copy.at(i) = list_copy.at(i) + "\n";
            }
            fio_str.save_stage_dialogs(it->first, i, list_copy);
        }
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
    fio.read_game(game_data);
    fio.read_all_stages(stage_data);
    std::string stages_extra_data_filename = "data/stages_extra_data" + fio.get_sufix() + ".dat";


    if (fio.file_exists(FILEPATH + "/" + stages_extra_data_filename)) {
        stage_extra_data = fio_cmm.load_struct_data<CURRENT_FILE_FORMAT::file_stages_extra_data>(stages_extra_data_filename);
    }
    load_game_data();
    Mediator::get_instance()->fio.read_castle_data(castle_data);

    // convert enemy-ist to 3.1.2
    enemy_list_3_1_1 = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc_v3_1_1>("game_enemy_list_3_1_1.dat");
    enemy_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc_v3_1_2>("game_enemy_list_3_1_2.dat");
    if (enemy_list.size() == 0) {
        if (enemy_list_3_1_1.size() == 0) {
            enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc_v3_1_2());
        } else {
            for (int i=0; i<enemy_list_3_1_1.size(); i++) {
                enemy_list.push_back(CURRENT_FILE_FORMAT::file_npc_v3_1_2(enemy_list_3_1_1.at(i)));
            }
        }
    }

    // fix invalid ghost values
    for (int i=0; i<enemy_list.size(); i++) {
        if (enemy_list.at(i).is_ghost > 1) {
            enemy_list.at(i).is_ghost = 0;
        }
    }

    object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat");
    if (object_list.size() == 0) { // add one first item to avoid errors
        object_list.push_back(CURRENT_FILE_FORMAT::file_object());
    }

    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat");
    if (ai_list.size() == 0) { // add one first item to avoid errors
        for (int i=0; i<enemy_list.size(); i++) {
            ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
        }
    }

    // *** AI V3 *** //
    /*
    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence_v3>("game_ai_list_v3.dat");
    if (ai_list.size() == 0) { // add one first item to avoid errors
        for (int i=0; i<enemy_list.size(); i++) {
            ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence_v3());
        }
    }
    */
    // *** AI V3 *** //


    projectile_list_v2 = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_projectilev2>("data/game_projectile_list_v2.dat");
    if (projectile_list_v2.size() == 0) {
        projectile_list_v2.push_back(CURRENT_FILE_FORMAT::file_projectilev2());
    }

    // converts projectile_v2 into projectile_v3 //
    /*
    projectile_list_v3.clear();
    for (int i=0; i<projectile_list_v2.size(); i++) {
        projectile_list_v3.push_back(CURRENT_FILE_FORMAT::file_projectilev3(projectile_list_v2.at(i)));
    }
    */
    projectile_list_v3 = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_projectilev3>(PROJECTILE_FILE_V3);
    if (projectile_list_v3.size() == 0) {
        projectile_list_v3.push_back(CURRENT_FILE_FORMAT::file_projectilev3());
    }


    scene_list = fio_scenes.load_scenes();
    if (scene_list.size() == 0) {
        scene_list.push_back(CURRENT_FILE_FORMAT::file_scene_list());
    }

    anim_block_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat");

    player_list_v3_1 = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_player_v3_1_1>("player_list_v3_1_1.dat");
    if (player_list_v3_1.size() == 0) {
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            player_list_v3_1.push_back(CURRENT_FILE_FORMAT::file_player_v3_1_1(i));
        }
    }

    ScenesMediator::get_instance()->load_game_scenes();

}

void Mediator::load_game_data()
{
    Mediator::get_instance()->fio.read_all_maps_v2(maps_data_v2);
    maps_data_npc_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_map_npc_v2>(std::string("/map_npc_data.dat"));
    maps_data_object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_map_object_v2>(std::string("/map_object_data.dat"));
}

void Mediator::save_game()
{
    clean_data();

    Mediator::get_instance()->fio.write_game(game_data);
    Mediator::get_instance()->fio.write_all_stages(stage_data);

    std::string stages_extra_data_filename = "data/stages_extra_data" + fio.get_sufix() + ".dat";
    fio_cmm.save_struct_data<CURRENT_FILE_FORMAT::file_stages_extra_data>(stages_extra_data_filename, stage_extra_data);


    save_map_data();
    Mediator::get_instance()->fio.write_castle_data(castle_data);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_npc_v3_1_2>("game_enemy_list_3_1_2.dat", enemy_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat", object_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat", ai_list);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_projectilev3>("data/game_projectile_list_v3.dat", projectile_list_v3);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat", anim_block_list);

    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_player_v3_1_1>("player_list_v3_1_1.dat", player_list_v3_1);


    ScenesMediator::get_instance()->save_game_scenes();

    save_dialogs();
}

void Mediator::clean_map_data()
{
    //maps_data_v2[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]
    for (int i=0; i<FS_MAX_STAGES; i++) {
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            //tiles[MAP_W][MAP_H]
            for (int k=0; k<MAP_W; k++) {
                for (int l=0; l<MAP_H; l++) {
                    if (maps_data_v2[i][j].tiles[k][l].locked == TERRAIN_CHECKPOINT) {
                        maps_data_v2[i][j].tiles[k][l].locked = TERRAIN_UNBLOCKED;
                    }
                }
            }
        }
    }

}

void Mediator::save_map_data()
{
    clean_map_data();
    Mediator::get_instance()->fio.write_all_maps_v2(maps_data_v2);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_map_npc_v2>("/map_npc_data.dat", maps_data_npc_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_map_object_v2>("/map_object_data.dat", maps_data_object_list);
}

void Mediator::clean_data()
{

    QString filename;
    if (Mediator::get_instance()->getPallete().length() < 1) {
         filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/") + QString("default.png");
    } else {
         filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/") + QString(Mediator::get_instance()->getPallete().c_str());
    }

    QPixmap *image = new QPixmap(filename);
    if (image->isNull()) {
        return;
    }

    int tileset_w = image->width();
    int tileset_h = image->height();

    // remove all invalid level-3 tiles from maps
    for (int i=0; i<FS_MAX_STAGES; i++) {
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            for (int x=0; x<MAP_W; x++) {
                for (int y=0; y<MAP_H; y++) {
                    if (maps_data_v2[i][j].tiles[x][y].tile3.x*TILESIZE >= tileset_w || maps_data_v2[i][j].tiles[x][y].tile3.y*TILESIZE >= tileset_h) {
                        maps_data_v2[i][j].tiles[x][y].tile3.x = -1;
                        maps_data_v2[i][j].tiles[x][y].tile3.y = -1;
                    }
                }
            }

        }

    }
}

void Mediator::temp_fix_player_colors_order()
{
    for (int i=0; i<player_list_v3_1.size(); i++) {
        CURRENT_FILE_FORMAT::file_player_v3_1_1 temp_player = player_list_v3_1.at(i);
        CURRENT_FILE_FORMAT::file_weapon_colors copy_weapon_colors[MAX_WEAPON_N];
        for (int j=0; j<MAX_WEAPON_N; j++) {
            copy_weapon_colors[j] = player_list_v3_1.at(i).weapon_colors[j];
        }
        // gear <-> beast
        player_list_v3_1.at(i).weapon_colors[1] = copy_weapon_colors[5];
        player_list_v3_1.at(i).weapon_colors[5] = copy_weapon_colors[1];
        // dragon (laranja) <-> snow
        player_list_v3_1.at(i).weapon_colors[2] = copy_weapon_colors[8];
        player_list_v3_1.at(i).weapon_colors[8] = copy_weapon_colors[2];
        // zodiac (yellow) <-> phantom (dark-blue+white)
        player_list_v3_1.at(i).weapon_colors[6] = copy_weapon_colors[7];
        player_list_v3_1.at(i).weapon_colors[7] = copy_weapon_colors[6];
    }
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

void Mediator::reload_game_scenes()
{
    scene_list = fio_scenes.load_scenes();
    if (scene_list.size() == 0) {
        scene_list.push_back(CURRENT_FILE_FORMAT::file_scene_list());
    }
}





