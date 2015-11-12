#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <iostream>

#include <QApplication>

#include "defines.h"
#include "file/format/st_hitPoints.h"
#include "../../file/format/st_common.h"

#include "fio_v1.h"
#include "file/file_io.h"

#undef main

std::string FILEPATH;
std::string GAMEPATH;
std::string SAVEPATH;
std::string GAMENAME;

CURRENT_FILE_FORMAT::file_game game_data;
CURRENT_FILE_FORMAT::file_stages stage_data;
CURRENT_FILE_FORMAT::file_map maps_data[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]; // stage, map_n

std::vector<CURRENT_FILE_FORMAT::file_npc> enemy_list;
std::vector<CURRENT_FILE_FORMAT::file_object> object_list;
std::vector<CURRENT_FILE_FORMAT::file_artificial_inteligence> ai_list;
std::vector<CURRENT_FILE_FORMAT::file_projectile> projectile_list;
std::vector<CURRENT_FILE_FORMAT::file_scene_list> scene_list;

std::vector<std::string> common_strings;

CURRENT_FILE_FORMAT::file_io fio;

void convert_dialog_strings(v1_file_stage stage_v1, CURRENT_FILE_FORMAT::file_stage& stage_v2) {
    sprintf(stage_v2.intro_dialog.face_graphics_filename, "%s", stage_v1.intro_dialog.face_graphics_filename);
    stage_v2.intro_dialog.top_side = stage_v1.intro_dialog.top_side;
    // *** INTRO DIALOGS *** //
    // line-1
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        common_strings.push_back(std::string(stage_v1.intro_dialog.line1[i]));
        stage_v2.intro_dialog.text1_string_ids[i] = common_strings.size()-1;
    }

    // line-2
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        common_strings.push_back(std::string(stage_v1.intro_dialog.line2[i]));
        stage_v2.intro_dialog.text2_string_ids[i] = common_strings.size()-1;
    }

    // answer-1
    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            common_strings.push_back(std::string(stage_v1.intro_dialog.answer1[i][j]));
            stage_v2.intro_dialog.answer1_string_ids[i][j] = common_strings.size()-1;
        }
    }

    // answer-2
    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            common_strings.push_back(std::string(stage_v1.intro_dialog.answer2[i][j]));
            stage_v2.intro_dialog.answer2_string_ids[i][j] = common_strings.size()-1;
        }
    }

    // *** BOSS DIALOGS *** //
    // line-1
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        common_strings.push_back(std::string(stage_v1.boss_dialog.line1[i]));
        stage_v2.boss_dialog.text1_string_ids[i] = common_strings.size()-1;
    }

    // line-2
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        common_strings.push_back(std::string(stage_v1.boss_dialog.line2[i]));
        stage_v2.boss_dialog.text2_string_ids[i] = common_strings.size()-1;
    }

    // answer-1
    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            common_strings.push_back(std::string(stage_v1.boss_dialog.answer1[i][j]));
            stage_v2.boss_dialog.answer1_string_ids[i][j] = common_strings.size()-1;
        }
    }

    // answer-2
    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            common_strings.push_back(std::string(stage_v1.boss_dialog.answer2[i][j]));
            stage_v2.boss_dialog.answer2_string_ids[i][j] = common_strings.size()-1;
        }
    }
}

st_position_uint8 convert_uint8_pos(v1_st_position_uint8 pos) {
    st_position_uint8 res;
    res.x = pos.x;
    res.y = pos.y;
    return res;
}


st_color convert_color(v1_st_color color) {
    st_color res;
    res.r = color.r;
    res.g = color.g;
    res.b = color.b;
    return res;
}

st_rectangle convert_rectangle(v1_st_rectangle v1_rect) {
    st_rectangle res;
    res.h = v1_rect.h;
    res.w = v1_rect.w;
    res.x = v1_rect.x;
    res.y = v1_rect.y;
    return res;
}


void convert_stage_maps(int stage_id, v1_file_stage stage_v1) {
    for (int i=0; i<V1_FS_STAGE_MAX_MAPS; i++) {
        for (int j=0; j<2; j++) {
            maps_data[stage_id][i].backgrounds[j].adjust_y = stage_v1.maps[i].backgrounds[j].adjust_y;
            maps_data[stage_id][i].backgrounds[j].auto_scroll = false;
            sprintf(maps_data[stage_id][i].backgrounds[j].filename, "%s", stage_v1.maps[i].backgrounds[j].filename);
            maps_data[stage_id][i].backgrounds[j].speed = stage_v1.maps[i].backgrounds[j].speed;
        }
        maps_data[stage_id][i].background_color = convert_color(stage_v1.maps[i].background_color);
        for (int j=0; j<V1_FS_MAX_MAP_NPCS; j++) {
            maps_data[stage_id][i].map_npcs[j].direction = stage_v1.maps[i].map_npcs[i].direction;
            maps_data[stage_id][i].map_npcs[j].id_npc = stage_v1.maps[i].map_npcs[i].id_npc;
            maps_data[stage_id][i].map_npcs[j].start_point.x = stage_v1.maps[i].map_npcs[i].start_point.x;
            maps_data[stage_id][i].map_npcs[j].start_point.y = stage_v1.maps[i].map_npcs[i].start_point.y;
        }
        for (int j=0; j<V1_FS_MAX_MAP_OBJECTS; j++) {
            maps_data[stage_id][i].map_objects[j].direction = stage_v1.maps[i].map_objects[j].direction;
            maps_data[stage_id][i].map_objects[j].id_object = stage_v1.maps[i].map_objects[j].id_object;
            maps_data[stage_id][i].map_objects[j].link_dest.x = stage_v1.maps[i].map_objects[j].link_dest.x;
            maps_data[stage_id][i].map_objects[j].link_dest.y = stage_v1.maps[i].map_objects[j].link_dest.y;
            maps_data[stage_id][i].map_objects[j].map_dest = stage_v1.maps[i].map_objects[j].map_dest;
            maps_data[stage_id][i].map_objects[j].start_point.x = stage_v1.maps[i].map_objects[j].start_point.x;
            maps_data[stage_id][i].map_objects[j].start_point.y = stage_v1.maps[i].map_objects[j].start_point.y;
        }
        for (int j=0; j<V1_MAP_W; j++) {
            for (int k=0; k<V1_MAP_H; k++) {
                maps_data[stage_id][i].tiles[i][j].locked = stage_v1.maps[i].tiles[i][j].locked;
                maps_data[stage_id][i].tiles[i][j].tile1.x = stage_v1.maps[i].tiles[i][j].tile1.x;
                maps_data[stage_id][i].tiles[i][j].tile1.y = stage_v1.maps[i].tiles[i][j].tile1.y;
                maps_data[stage_id][i].tiles[i][j].tile3.x = stage_v1.maps[i].tiles[i][j].tile3.x;
                maps_data[stage_id][i].tiles[i][j].tile3.y = stage_v1.maps[i].tiles[i][j].tile3.y;
            }
        }
    }
}

void convert_stages_and_maps(v1_file_stages stages) {
    for (int i=0; i<V1_FS_MAX_STAGES; i++) {
        v1_file_stage temp_v1 = stages.stages[i];
        CURRENT_FILE_FORMAT::file_stage temp_v2;
        temp_v2.autoscroll = false;
        sprintf(temp_v2.bgmusic_filename, "%s", temp_v1.bgmusic_filename);
        temp_v2.boss.id_npc = temp_v1.boss.id_npc;
        temp_v2.boss.id_weapon = temp_v1.boss.id_weapon;
        sprintf(temp_v2.boss.name, "%s", temp_v1.boss.name);

        convert_dialog_strings(temp_v1, temp_v2);

        temp_v2.cutscene_pos = -1;
        temp_v2.cutscene_pre = -1;
        for (int i=0; i<FS_STAGE_MAX_LINKS; i++) {
            temp_v2.links[i].bidirecional = temp_v1.links[i].bidirecional;
            temp_v2.links[i].id_map_destiny = temp_v1.links[i].id_map_destiny;
            temp_v2.links[i].id_map_origin = temp_v1.links[i].id_map_origin;
            temp_v2.links[i].is_door = temp_v1.links[i].is_door;
            temp_v2.links[i].pos_destiny = convert_uint8_pos(temp_v1.links[i].pos_destiny);
            temp_v2.links[i].pos_origin = convert_uint8_pos(temp_v1.links[i].pos_origin);
            temp_v2.links[i].size = temp_v1.links[i].size;
            temp_v2.links[i].type = temp_v1.links[i].type;
        }
        sprintf(temp_v2.name, "%s", temp_v1.name);
        sprintf(temp_v2.tileset_filename, "%s", "default.png");

        convert_stage_maps(i, temp_v1);
    }
}

void convert_game(v1_file_game game_v1) {
    for (int i=0; i<V1_FS_PLAYER_ARMOR_PIECES_MAX; i++) {
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                common_strings.push_back(std::string(game_v1.armor_pieces[i].got_message[j][k]));
                game_data.armor_pieces[i].got_message[j][k] = common_strings.size()-1;
            }
        }
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            game_data.armor_pieces[i].special_ability[j] = game_v1.armor_pieces[i].special_ability[j];
        }
    }
    /// @TODO - add old hardcoded attacks like hadouken
    sprintf(game_data.name, "%s", game_v1.name);
    for (int j=0; j<FS_MAX_PLAYERS; j++) {
        game_data.players[j].can_air_dash = game_v1.players[j].can_air_dash;
        game_data.players[j].can_charge_shot = game_v1.players[j].can_charge_shot;
        game_data.players[j].can_double_jump = game_v1.players[j].can_double_jump;
        game_data.players[j].can_slide = game_v1.players[j].can_slide;
        game_data.players[j].damage_modifier = game_v1.players[j].damage_modifier;
        sprintf(game_data.players[j].face_filename, "%s", game_v1.players[j].face_filename);
        game_data.players[j].full_charged_projectile_id = game_v1.players[j].full_charged_projectile_id;
        sprintf(game_data.players[j].graphic_filename, "%s", game_v1.players[j].graphic_filename);
        game_data.players[j].have_shield = game_v1.players[j].have_shield;
        game_data.players[j].HP = game_v1.players[j].HP;
        game_data.players[j].max_shots = game_v1.players[j].max_shots;
        sprintf(game_data.players[j].name, "%s", game_v1.players[j].name);
        game_data.players[j].simultaneous_shots = game_v1.players[j].simultaneous_shots;
        for (int k=0; k<V1_ANIM_TYPE_COUNT; k++) {
            for (int l=0; l<V1_ANIM_FRAMES_COUNT; l++) {
                game_data.players[j].sprites[k][l].colision_rect = convert_rectangle(game_v1.players[j].sprites[k][l].colision_rect);
                game_data.players[j].sprites[k][l].duration = game_v1.players[j].sprites[k][l].duration;
                game_data.players[j].sprites[k][l].sprite_graphic_pos_x = game_v1.players[j].sprites[k][l].sprite_graphic_pos_x;
                game_data.players[j].sprites[k][l].used = game_v1.players[j].sprites[k][l].used;
            }
        }
        game_data.players[j].sprite_hit_area = convert_rectangle(game_v1.players[j].sprite_hit_area);
        game_data.players[j].sprite_size.width = game_v1.players[j].sprite_size.width;
        game_data.players[j].sprite_size.height = game_v1.players[j].sprite_size.height;
        for (int k=0; k<V1_MAX_WEAPON_N; k++) {
            game_data.players[j].weapon_colors[k].color1 = convert_color(game_v1.players[j].weapon_colors[k].color1);
            game_data.players[j].weapon_colors[k].color2 = convert_color(game_v1.players[j].weapon_colors[k].color2);
            game_data.players[j].weapon_colors[k].color3 = convert_color(game_v1.players[j].weapon_colors[k].color3);
        }

    }
    for (int k=0; k<V1_FS_PLATER_ITEMS_N; k++) {
        game_data.player_items[k] = game_v1.player_items[k];
    }
    game_data.semi_charged_projectile_id = game_v1.semi_charged_projectile_id;
    for (int k=0; k<V1_MAX_STAGES; k++) {
        sprintf(game_data.stage_face_filename[k], "%s", game_v1.stage_face_filename[k]);
    }
    for (int k=0; k<V1_TROPHIES_MAX; k++) {
        game_data.trophies[k].condition = game_v1.trophies[k].condition;
        sprintf(game_data.trophies[k].filename, "%s", game_v1.trophies[k].filename);
        sprintf(game_data.trophies[k].name, "%s", game_v1.trophies[k].name);
    }
    game_data.version = game_v1.version;
    for (int k=0; k<V1_FS_MAX_WEAPONS; k++) {
        game_data.weapons[k].damage = game_v1.weapons[k].damage;
        game_data.weapons[k].id_projectile = game_v1.weapons[k].id_projectile;
        sprintf(game_data.weapons[k].name, "%s", game_v1.weapons[k].name);
    }

    /// @TODO: convert tyles that are on their own files now:
    /// ai_types
    /// game_npcs
    /// objects
    /// projectiles
}


int main(int argc, char *argv[])
{
    std::string EXEC_NAME("conversor");

#ifndef WIN32
    EXEC_NAME = "conversor";
#else
    EXEC_NAME = "conversor.exe";
#endif

    std::cout << "AAA" << std::endl;

    QApplication a(argc, argv);

    std::string argvString = std::string(argv[0]);
    GAMEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    FILEPATH = GAMEPATH;
    GAMENAME = "Rockbot1";

    fio_v1 fv1;

    v1_file_game game_v1;
    fv1.read_game(game_v1);

    v1_file_stages stages_v1;
    fv1.read_all_stages(stages_v1);

    convert_stages_and_maps(stages_v1);

    convert_game(game_v1);

    fio.write_all_stages(stage_data);
    fio.write_all_maps(maps_data);


    std::cout << "AAA" << std::endl;

}
