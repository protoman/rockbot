#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iostream>
#include <map>

#include "defines.h"
#include "file/format/st_hitPoints.h"
#include "../../file/format/st_common.h"

#include "fio_v1.h"
#include "file/file_io.h"
#include "file/fio_strings.h"

//#undef main

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
std::map<int, int> npc_id_list;
std::map<int, int> object_id_list;
std::vector<std::string> common_strings;

CURRENT_FILE_FORMAT::file_io fio;
CURRENT_FILE_FORMAT::fio_strings fio_str;
fio_common fio_cmm;

void convert_dialog_strings(v1_file_stage stage_v1, CURRENT_FILE_FORMAT::file_stage& stage_v2, short stage_id) {
    sprintf(stage_v2.dialog_face_graphics_filename, "%s", stage_v1.intro_dialog.face_graphics_filename);
    stage_v2.dialog_top_side = stage_v1.intro_dialog.top_side;

    std::vector<std::string> dialog_strings;

    // *** INTRO DIALOGS *** //
    // line-1
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        std::string line = std::string(stage_v1.intro_dialog.line1[i]) + "\n";
        std::cout << "LINE#1[" << i << "]: " << line << ", common_strings.size(): " << dialog_strings.size() << std::endl;
        dialog_strings.push_back(line);
    }

    // line-2
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        std::string line = std::string(stage_v1.intro_dialog.line2[i]) + "\n";
        dialog_strings.push_back(line);
    }


    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        // answer-1
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            std::string line = std::string(stage_v1.intro_dialog.answer1[i][j]) + "\n";
            dialog_strings.push_back(line);
        }
        // answer-2
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            std::string line = std::string(stage_v1.intro_dialog.answer2[i][j]) + "\n";
            dialog_strings.push_back(line);
        }
    }


    // *** BOSS DIALOGS *** //
    // line-1
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        std::string line = std::string(stage_v1.boss_dialog.line1[i]) + "\n";
        dialog_strings.push_back(line);
    }

    // line-2
    for (int i=0; i<V1_FS_DIALOG_LINES; i++) {
        std::string line = std::string(stage_v1.boss_dialog.line2[i]) + "\n";
        dialog_strings.push_back(line);
    }


    for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
        // answer-1
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            std::string line = std::string(stage_v1.boss_dialog.answer1[i][j]) + "\n";
            dialog_strings.push_back(line);
        }
        // answer-2
        for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
            std::string line = std::string(stage_v1.boss_dialog.answer2[i][j]) + "\n";
            dialog_strings.push_back(line);
        }
    }
    fio_str.save_stage_dialogs(stage_id, dialog_strings);

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


void convert_stage_maps(int stage_id, v1_file_stage& stage_v1) {
    for (int i=0; i<V1_FS_STAGE_MAX_MAPS; i++) {
        for (int j=0; j<2; j++) {
            maps_data[stage_id][i].backgrounds[j].adjust_y = stage_v1.maps[i].backgrounds[j].adjust_y;
            maps_data[stage_id][i].backgrounds[j].auto_scroll = false;
            sprintf(maps_data[stage_id][i].backgrounds[j].filename, "%s", stage_v1.maps[i].backgrounds[j].filename);
            maps_data[stage_id][i].backgrounds[j].speed = stage_v1.maps[i].backgrounds[j].speed;
        }
        maps_data[stage_id][i].background_color = convert_color(stage_v1.maps[i].background_color);
        for (int j=0; j<V1_FS_MAX_MAP_NPCS; j++) {
            // check for invalid NPCs
            std::map<int, int>::iterator it;
            it = npc_id_list.find(stage_v1.maps[i].map_npcs[j].id_npc);
            if (it == npc_id_list.end()) {
                maps_data[stage_id][i].map_npcs[j].id_npc = -1;
            } else {
                int new_id = it->second;
                maps_data[stage_id][i].map_npcs[j].direction = stage_v1.maps[i].map_npcs[j].direction;
                maps_data[stage_id][i].map_npcs[j].id_npc = new_id;
                maps_data[stage_id][i].map_npcs[j].start_point.x = stage_v1.maps[i].map_npcs[j].start_point.x;
                maps_data[stage_id][i].map_npcs[j].start_point.y = stage_v1.maps[i].map_npcs[j].start_point.y;
            }
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
                maps_data[stage_id][i].tiles[j][k].locked = stage_v1.maps[i].tiles[j][k].locked;
                maps_data[stage_id][i].tiles[j][k].tile1.x = stage_v1.maps[i].tiles[j][k].tile1.x;
                maps_data[stage_id][i].tiles[j][k].tile1.y = stage_v1.maps[i].tiles[j][k].tile1.y;
                maps_data[stage_id][i].tiles[j][k].tile3.x = stage_v1.maps[i].tiles[j][k].tile3.x;
                maps_data[stage_id][i].tiles[j][k].tile3.y = stage_v1.maps[i].tiles[j][k].tile3.y;
            }
        }
    }
}

void convert_stages_and_maps(v1_file_stages& stages) {
    for (int i=0; i<V1_FS_MAX_STAGES; i++) {
        v1_file_stage temp_v1 = stages.stages[i];
        CURRENT_FILE_FORMAT::file_stage temp_v2;
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            temp_v2.autoscroll[j] = false;
        }
        sprintf(temp_v2.bgmusic_filename, "%s", temp_v1.bgmusic_filename);
        temp_v2.boss.id_npc = temp_v1.boss.id_npc;
        temp_v2.boss.id_weapon = temp_v1.boss.id_weapon;
        sprintf(temp_v2.boss.name, "%s", temp_v1.boss.name);

        convert_dialog_strings(temp_v1, temp_v2, i);

        temp_v2.cutscene_pos = -1;
        temp_v2.cutscene_pre = -1;
        for (int i=0; i<FS_STAGE_MAX_LINKS; i++) {
            temp_v2.links[i].bidirecional = temp_v1.links[i].bidirecional;
            temp_v2.links[i].id_map_destiny = temp_v1.links[i].id_map_destiny;
            temp_v2.links[i].id_map_origin = temp_v1.links[i].id_map_origin;
            temp_v2.links[i].is_door = temp_v1.links[i].is_door;
            temp_v2.links[i].pos_destiny.x = temp_v1.links[i].pos_destiny.x;
            temp_v2.links[i].pos_destiny.y = temp_v1.links[i].pos_destiny.y;
            temp_v2.links[i].pos_origin.x = temp_v1.links[i].pos_origin.x;
            temp_v2.links[i].pos_origin.y = temp_v1.links[i].pos_origin.y;
            temp_v2.links[i].size = temp_v1.links[i].size;
            temp_v2.links[i].type = temp_v1.links[i].type;
        }
        sprintf(temp_v2.name, "%s", temp_v1.name);
        sprintf(temp_v2.tileset_filename, "%s", "default.png");

        stage_data.stages[i] = temp_v2;

        convert_stage_maps(i, temp_v1);

    }
}


void convert_ai_types(v1_file_game& game_v1) {
    for (int n=0; n<V1_FS_GAME_MAX_NPCS; n++) {
        int i = game_v1.game_npcs[n].IA_type;
        std::string name(game_v1.ai_types[i].name);
        if (name.length() < 1) {
            continue;
        }
        CURRENT_FILE_FORMAT::file_artificial_inteligence new_ai;
        sprintf(new_ai.name, "%s",  game_v1.ai_types[i].name);
        for (int j=0; j<MAX_AI_REACTIONS; j++) {
            new_ai.reactions[j].action = game_v1.ai_types[i].reactions[j].action;
            new_ai.reactions[j].extra_parameter = game_v1.ai_types[i].reactions[j].extra_parameter;
            new_ai.reactions[j].go_to = game_v1.ai_types[i].reactions[j].go_to;
            new_ai.reactions[j].go_to_delay = game_v1.ai_types[i].reactions[j].go_to_delay;
        }

        for (int j=0; j<AI_MAX_STATES; j++) {
            new_ai.states[j].action = game_v1.ai_types[i].states[j].action;
            new_ai.states[j].chance = game_v1.ai_types[i].states[j].chance;
            new_ai.states[j].extra_parameter = game_v1.ai_types[i].states[j].extra_parameter;
            new_ai.states[j].go_to = game_v1.ai_types[i].states[j].go_to;
            new_ai.states[j].go_to_delay = game_v1.ai_types[i].states[j].go_to_delay;
        }
        ai_list.push_back(new_ai);
    }


}

void convert_game_npcs(v1_file_game& game_v1) {
    for (int i=0; i<V1_FS_GAME_MAX_NPCS; i++) {
        std::string name(game_v1.game_npcs[i].name);
        // ignore ones using default name
        if (name == "NPC") {
            std::cout << "npc[" << i << "] invalid. id: " << (int)game_v1.game_npcs[i].id << std::endl;
            continue;
        }
        CURRENT_FILE_FORMAT::file_npc new_enemy;
        new_enemy.attack_frame = 0;
        sprintf(new_enemy.bg_graphic_filename, "%s", game_v1.game_npcs[i].bg_graphic_filename);
        new_enemy.direction = game_v1.game_npcs[i].direction;
        new_enemy.facing = game_v1.game_npcs[i].facing;
        new_enemy.fly_flag = game_v1.game_npcs[i].fly_flag;
        new_enemy.frame_size.width = game_v1.game_npcs[i].frame_size.width;
        new_enemy.frame_size.height = game_v1.game_npcs[i].frame_size.height;
        sprintf(new_enemy.graphic_filename, "%s", game_v1.game_npcs[i].graphic_filename);
        new_enemy.hp.current = game_v1.game_npcs[i].hp.current;
        new_enemy.hp.total = game_v1.game_npcs[i].hp.total;
        new_enemy.IA_type = game_v1.game_npcs[i].IA_type;
        new_enemy.id = game_v1.game_npcs[i].id;
        new_enemy.is_boss = game_v1.game_npcs[i].is_boss;
        new_enemy.is_ghost = game_v1.game_npcs[i].is_ghost;
        new_enemy.is_sub_boss = game_v1.game_npcs[i].is_sub_boss;
        sprintf(new_enemy.name, "%s", game_v1.game_npcs[i].name);
        new_enemy.projectile_id[0] = game_v1.game_npcs[i].projectile_id[0];
        new_enemy.projectile_id[1] = game_v1.game_npcs[i].projectile_id[1];
        new_enemy.respawn_delay = game_v1.game_npcs[i].respawn_delay;
        new_enemy.shield_type = game_v1.game_npcs[i].shield_type;
        new_enemy.speed = game_v1.game_npcs[i].speed;
        for (int j=0; j<V1_ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<V1_ANIM_FRAMES_COUNT; k++) {
                new_enemy.sprites[j][k].colision_rect = convert_rectangle(game_v1.game_npcs[i].sprites[j][k].colision_rect);
                new_enemy.sprites[j][k].duration = game_v1.game_npcs[i].sprites[j][k].duration;
                new_enemy.sprites[j][k].sprite_graphic_pos_x = game_v1.game_npcs[i].sprites[j][k].sprite_graphic_pos_x;
                new_enemy.sprites[j][k].used = game_v1.game_npcs[i].sprites[j][k].used;
            }
        }
        new_enemy.sprites_pos_bg.x = game_v1.game_npcs[i].sprites_pos_bg.x;
        new_enemy.sprites_pos_bg.y = game_v1.game_npcs[i].sprites_pos_bg.y;
        new_enemy.start_point.x = game_v1.game_npcs[i].start_point.x;
        new_enemy.start_point.y = game_v1.game_npcs[i].start_point.y;
        new_enemy.walk_range = game_v1.game_npcs[i].walk_range;
        for (int j=0; j<V1_FS_NPC_WEAKNESSES; j++) {
            new_enemy.weakness[j].damage_multiplier = game_v1.game_npcs[i].weakness[j].damage_multiplier;
            new_enemy.weakness[j].weapon_id = game_v1.game_npcs[i].weakness[j].weapon_id;
        }
        enemy_list.push_back(new_enemy);
        // store the old npc position and the new one into a list so we can use in map-npcs
        //std::cout << "old-id: " << i << ", new-id: " << (enemy_list.size()-1) << std::endl;
        npc_id_list.insert(std::pair<int, int>(i, (enemy_list.size()-1)));
    }
}

void convert_game_objects(v1_file_game& game_v1) {
    for (int i=0; i<V1_FS_GAME_MAX_NPCS; i++) {
        std::string name(game_v1.objects[i].name);
        if (name.length() < 1 || name == "Object") {
            continue;
        }
        CURRENT_FILE_FORMAT::file_object new_object;
        new_object.animation_auto_start = game_v1.objects[i].animation_auto_start;
        new_object.animation_loop = game_v1.objects[i].animation_loop;
        new_object.animation_reverse = game_v1.objects[i].animation_reverse;
        new_object.direction = game_v1.objects[i].direction;
        new_object.distance = game_v1.objects[i].distance;
        new_object.frame_duration = game_v1.objects[i].frame_duration;
        sprintf(new_object.graphic_filename, "%s", game_v1.objects[i].graphic_filename);
        new_object.limit = game_v1.objects[i].limit;
        sprintf(new_object.name, "%s", game_v1.objects[i].name);
        new_object.size.width = game_v1.objects[i].size.width;
        new_object.size.height = game_v1.objects[i].size.height;
        new_object.speed = game_v1.objects[i].speed;
        new_object.timer = game_v1.objects[i].timer;
        new_object.type = game_v1.objects[i].type;
        object_list.push_back(new_object);
    }
}

void convert_projectiles(v1_file_game& game_v1) {
    for (int i=0; i<V1_FS_MAX_PROJECTILES; i++) {
        CURRENT_FILE_FORMAT::file_projectile new_projectile;
        new_projectile.can_be_reflected = game_v1.projectiles[i].can_be_reflected;
        new_projectile.damage = game_v1.projectiles[i].damage;
        sprintf(new_projectile.graphic_filename, "%s", game_v1.projectiles[i].graphic_filename);
        new_projectile.hp = game_v1.projectiles[i].hp;
        new_projectile.is_destructible = game_v1.projectiles[i].is_destructible;
        new_projectile.max_shots = game_v1.projectiles[i].max_shots;
        sprintf(new_projectile.name, "%s", game_v1.projectiles[i].name);
        sprintf(new_projectile.sfx_filename, "%s", game_v1.projectiles[i].sfx_filename);
        new_projectile.size.width = game_v1.projectiles[i].size.width;
        new_projectile.size.height = game_v1.projectiles[i].size.height;
        new_projectile.spawn_npc_id = game_v1.projectiles[i].spawn_npc_id;
        new_projectile.spawn_npc_n = game_v1.projectiles[i].spawn_npc_n;
        new_projectile.speed = game_v1.projectiles[i].speed;
        new_projectile.trajectory = (int)game_v1.projectiles[i].trajectory;
        projectile_list.push_back(new_projectile);
    }

}

void convert_game_players(v1_file_game& game_v1) {
    std::vector<CURRENT_FILE_FORMAT::file_player> res;
    for (int j=0; j<FS_MAX_PLAYERS; j++) {
        CURRENT_FILE_FORMAT::file_player temp;
        temp.can_air_dash = game_v1.players[j].can_air_dash;
        temp.can_charge_shot = game_v1.players[j].can_charge_shot;
        temp.can_double_jump = game_v1.players[j].can_double_jump;
        temp.can_slide = game_v1.players[j].can_slide;
        temp.damage_modifier = game_v1.players[j].damage_modifier;
        sprintf(temp.face_filename, "%s", game_v1.players[j].face_filename);
        temp.full_charged_projectile_id = game_v1.players[j].full_charged_projectile_id;
        sprintf(temp.graphic_filename, "%s", game_v1.players[j].graphic_filename);
        temp.have_shield = game_v1.players[j].have_shield;
        temp.HP = game_v1.players[j].HP;
        temp.max_shots = game_v1.players[j].max_shots;
        sprintf(temp.name, "%s", game_v1.players[j].name);
        temp.simultaneous_shots = game_v1.players[j].simultaneous_shots;
        for (int k=0; k<V1_ANIM_TYPE_COUNT; k++) {
            for (int l=0; l<V1_ANIM_FRAMES_COUNT; l++) {
                temp.sprites[k][l].colision_rect = convert_rectangle(game_v1.players[j].sprites[k][l].colision_rect);
                temp.sprites[k][l].duration = game_v1.players[j].sprites[k][l].duration;
                temp.sprites[k][l].sprite_graphic_pos_x = game_v1.players[j].sprites[k][l].sprite_graphic_pos_x;
                temp.sprites[k][l].used = game_v1.players[j].sprites[k][l].used;
            }
        }
        temp.sprite_hit_area = convert_rectangle(game_v1.players[j].sprite_hit_area);
        temp.sprite_size.width = game_v1.players[j].sprite_size.width;
        temp.sprite_size.height = game_v1.players[j].sprite_size.height;
        for (int k=0; k<V1_MAX_WEAPON_N; k++) {
            temp.weapon_colors[k].color1 = convert_color(game_v1.players[j].weapon_colors[k].color1);
            temp.weapon_colors[k].color2 = convert_color(game_v1.players[j].weapon_colors[k].color2);
            temp.weapon_colors[k].color3 = convert_color(game_v1.players[j].weapon_colors[k].color3);
        }
        res.push_back(temp);
    }
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_player>("player_list.dat", res);
}


void convert_game(v1_file_game& game_v1) {
    for (int i=0; i<V1_FS_PLAYER_ARMOR_PIECES_MAX; i++) {
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                std::string line = std::string(game_v1.armor_pieces[i].got_message[j][k]);
                if (line.length() > 0) {
                    common_strings.push_back(line);
                    game_data.armor_pieces[i].got_message[j][k] = common_strings.size()-1;
                } else {
                    game_data.armor_pieces[i].got_message[j][k] = -1;
                }
            }
        }
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            game_data.armor_pieces[i].special_ability[j] = game_v1.armor_pieces[i].special_ability[j];
        }
    }
    /// @TODO - add old hardcoded attacks like hadouken
    sprintf(game_data.name, "%s", game_v1.name);

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

    convert_ai_types(game_v1);

    convert_game_npcs(game_v1);

    convert_game_objects(game_v1);

    convert_projectiles(game_v1);

    convert_game_players(game_v1);
}


void set_rockbot1_hardcoded() {
    /// @TODO: set the hardcoded parts:
    // ROCKBOT 1 HARDCODED PARTS //
    game_data.game_style = 0;
    sprintf(game_data.stages_face_name[0], "%s", "INTRO");
    sprintf(game_data.stages_face_name[1], "%s", "APE");
    sprintf(game_data.stages_face_name[2], "%s", "DAISIE");
    sprintf(game_data.stages_face_name[3], "%s", "SEAHORSE");
    sprintf(game_data.stages_face_name[4], "%s", "MUMMY");
    sprintf(game_data.stages_face_name[5], "%s", "MAGE");
    sprintf(game_data.stages_face_name[6], "%s", "DYNAMITE");
    sprintf(game_data.stages_face_name[7], "%s", "SPIKE");
    sprintf(game_data.stages_face_name[8], "%s", "TECHNO");
    sprintf(game_data.stages_face_name[9], "%s", "CASTLE");

    /// music (boss battle, final boss, game_over, got_weapon)
    sprintf(game_data.boss_music_filename, "%s", "boss_battle.mod");
    sprintf(game_data.final_boss_music_filename, "%s", "final_boss.mod");
    sprintf(game_data.got_weapon_music_filename, "%s", "got_weapon.mod");
    sprintf(game_data.game_over_music_filename, "%s", "OTHER.xm");
    sprintf(game_data.stage_select_music_filename, "%s", "menu.mod");
    sprintf(game_data.game_start_screen_music_filename, "%s", "opening.mod");


    /// projectiles (see doc code-changes)

    /// armor-pieces weapon powers

    /// anim tiles
}

int main(int argc, char *argv[])
{
    std::string EXEC_NAME("conversor");

#ifndef WIN32
    EXEC_NAME = "conversor";
#else
    EXEC_NAME = "conversor.exe";
#endif

    std::string argvString = std::string(argv[0]);
    GAMEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    FILEPATH = GAMEPATH;
    GAMENAME = "Rockbot1";

    fio_v1 fv1;

    v1_file_game game_v1;
    fv1.read_game(game_v1);

    v1_file_stages stages_v1;
    fv1.read_all_stages(stages_v1);

    convert_game(game_v1);

    convert_stages_and_maps(stages_v1);

    set_rockbot1_hardcoded();

    FILEPATH += "/out/";
    fio.write_all_stages(stage_data);
    fio.write_all_maps(maps_data);


    fio.write_game(game_data);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat", enemy_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat", object_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat", ai_list);
    fio_cmm.save_data_to_disk<CURRENT_FILE_FORMAT::file_projectile>("game_projectile_list.dat", projectile_list);


    CURRENT_FILE_FORMAT::fio_strings fio_str;
    fio_str.save_common_strings(common_strings);




    return 1;

}
