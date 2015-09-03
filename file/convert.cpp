#include "convert.h"

#include "file_io.h"


format_v_2_1_1::file_game game_data_v211;
format_v_2_1_1::file_stages v211_stages;

format_v_2_1_2::file_game game_data_v212;
format_v_2_1_2::file_stages v212_stages;


convert::convert()
{
}

void convert::v211_to_v212() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_2_1_1::file_io fio_v211;
    std::cout << "Converting data from 2.1.1 to 2.1.2 ..." << std::endl;
    fio_v211.read_game(game_data_v211);
    fio_v211.read_all_stages(v211_stages);

    /// @TODO - convert gravity back to AI
    for (int i=0; i<GAME_MAX_OBJS; i++) {
        game_data_v212.game_npcs[i] = game_data_v211.game_npcs[i];
        game_data_v212.objects[i] = game_data_v211.objects[i];
    }
    for (int i=0; i<MAX_FILE_PROJECTILES; i++) {
        game_data_v212.projectiles[i] = game_data_v211.projectiles[i];
    }
    for (int i=0; i<MAX_WEAPON_N; i++) {
        game_data_v212.weapons[i] = game_data_v211.weapons[i];
    }
    for (int i=0; i<MAX_FILE_PLAYERS; i++) {
        game_data_v212.players[i] = game_data_v211.players[i];
    }
    for (int i=0; i<MAX_AI_TYPES; i++) {
        game_data_v212.ai_types[i].id = game_data_v211.ai_types[i].id;
        sprintf(game_data_v212.ai_types[i].name, "%s", game_data_v211.ai_types[i].name);
        for (int j=0; j<4; j++) {
            game_data_v212.ai_types[i].states[j] = game_data_v211.ai_types[i].states[j];
        }
        for (int j=0; j<MAX_AI_REACTIONS; j++) {
            game_data_v212.ai_types[i].reactions[j].action = game_data_v212.game_npcs[i].sprites[ANIM_TYPE_TELEPORT][j+1].colision_rect.x;
            game_data_v212.ai_types[i].reactions[j].extra_parameter = game_data_v212.game_npcs[i].sprites[ANIM_TYPE_TELEPORT][j+1].colision_rect.y;
        }
    }
    game_data_v212.version = 1.0;
    sprintf(game_data_v212.name, "%s", "Rockbot");
    game_data_v212.semi_charged_projectile_id = 0;
    game_data_v212.player_items[0] = 0;
    game_data_v212.player_items[1] = 0;

    for (int i=0; i<MAX_STAGES; i++) {
        sprintf(game_data_v212.stage_face_filename[i], "%s", game_data_v211.stage_face_filename[i]);
        /// @REDO
        //v212_stages.stages[i] = v211_stages.stages[i];
    }


    fio_v212.write_game(game_data_v212);
    fio_v212.write_all_stages(v212_stages);
    std::cout << "Data written to file." << std::endl;
}

void convert::v212_to_v3() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_3_0_0::file_io fio_v300;

    format_v_3_0_0::file_game game_data_v300;
    format_v_3_0_0::file_stages v300_stages;


    std::cout << "Converting data from 2.1.2 to 3.0.0 ..." << std::endl;
    fio_v212.read_game(game_data_v212);
    fio_v212.read_all_stages(v212_stages);

    /// @TODO - convert gravity back to AI
    for (int i=0; i<GAME_MAX_OBJS; i++) { // using old value for loop, as we increased the number of npcs
        // CONVERT GAME_NPCS

        //std::cout << ">>v212_to_v3, i[" << i << "]" << std::endl;
        //std::cout << ">>v212_to_v3, i[" <<  game_data_v212.game_npcs[i].id << "]" << std::endl;

        if (game_data_v212.game_npcs[i].id == -1) {
            continue;
        }

        //game_data_v300.game_npcs[i] = game_data_v212.game_npcs[i];
        sprintf(game_data_v300.game_npcs[i].bg_graphic_filename, "%s", game_data_v212.game_npcs[i].bg_graphic_filename);
        game_data_v300.game_npcs[i].direction = game_data_v212.game_npcs[i].direction;
        game_data_v300.game_npcs[i].facing = game_data_v212.game_npcs[i].facing;
        game_data_v300.game_npcs[i].fly_flag = game_data_v212.game_npcs[i].fly_flag;
        game_data_v300.game_npcs[i].frame_size.width = game_data_v212.game_npcs[i].frame_size.width;
        game_data_v300.game_npcs[i].frame_size.height = game_data_v212.game_npcs[i].frame_size.height;
        sprintf(game_data_v300.game_npcs[i].graphic_filename, "%s", game_data_v212.game_npcs[i].graphic_filename);
        game_data_v300.game_npcs[i].hp = game_data_v212.game_npcs[i].hp;
        game_data_v300.game_npcs[i].IA_type = game_data_v212.game_npcs[i].gravity_level;
        game_data_v300.game_npcs[i].id = game_data_v212.game_npcs[i].id;
        game_data_v300.game_npcs[i].is_boss = game_data_v212.game_npcs[i].is_boss;
        game_data_v300.game_npcs[i].is_ghost = game_data_v212.game_npcs[i].is_ghost;
        sprintf(game_data_v300.game_npcs[i].name, "%s", game_data_v212.game_npcs[i].name);
        game_data_v300.game_npcs[i].projectile_id[0] = game_data_v212.game_npcs[i].projectile_id[0];
        game_data_v300.game_npcs[i].projectile_id[1] = game_data_v212.game_npcs[i].projectile_id[1];
        game_data_v300.game_npcs[i].shield_type = game_data_v212.game_npcs[i].shield_type;
        game_data_v300.game_npcs[i].speed = game_data_v212.game_npcs[i].speed;
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                game_data_v300.game_npcs[i].sprites[j][k].colision_rect.h = game_data_v212.game_npcs[i].sprites[j][k].colision_rect.h;
                game_data_v300.game_npcs[i].sprites[j][k].colision_rect.w = game_data_v212.game_npcs[i].sprites[j][k].colision_rect.w;
                game_data_v300.game_npcs[i].sprites[j][k].colision_rect.x = game_data_v212.game_npcs[i].sprites[j][k].colision_rect.x;
                game_data_v300.game_npcs[i].sprites[j][k].colision_rect.y = game_data_v212.game_npcs[i].sprites[j][k].colision_rect.y;
                game_data_v300.game_npcs[i].sprites[j][k].duration = game_data_v212.game_npcs[i].sprites[j][k].duration;
                game_data_v300.game_npcs[i].sprites[j][k].sprite_graphic_pos_x = game_data_v212.game_npcs[i].sprites[j][k].sprite_graphic_pos_x;
                game_data_v300.game_npcs[i].sprites[j][k].used = game_data_v212.game_npcs[i].sprites[j][k].used;
            }
        }
        game_data_v300.game_npcs[i].sprites_pos_bg = game_data_v212.game_npcs[i].sprites_pos_bg;
        game_data_v300.game_npcs[i].start_point.x = game_data_v212.game_npcs[i].start_point.x;
        game_data_v300.game_npcs[i].start_point.y = game_data_v212.game_npcs[i].start_point.y;
        game_data_v300.game_npcs[i].walk_range = game_data_v212.game_npcs[i].walk_range;
        for (int j=0; j<FS_NPC_WEAKNESSES; j++) {
            game_data_v300.game_npcs[i].weakness[j].damage_multiplier = game_data_v212.game_npcs[i].weakness[j].damage_multiplier;
            game_data_v300.game_npcs[i].weakness[j].weapon_id = game_data_v212.game_npcs[i].weakness[j].weapon_id;
        }
    }

    for (int i=0; i<FS_GAME_MAX_OBJS; i++) { // using new value for loop, as we decreased the number of objects
        // convert objects
        game_data_v300.objects[i].animation_auto_start = game_data_v212.objects[i].animation_auto_start;
        game_data_v300.objects[i].animation_loop = game_data_v212.objects[i].animation_loop;
        game_data_v300.objects[i].animation_reverse = game_data_v212.objects[i].animation_reverse;
        game_data_v300.objects[i].direction = game_data_v212.objects[i].direction;
        game_data_v300.objects[i].distance = game_data_v212.objects[i].distance;
        game_data_v300.objects[i].frame_duration = game_data_v212.objects[i].frame_duration;
        sprintf(game_data_v300.objects[i].graphic_filename, "%s", game_data_v212.objects[i].graphic_filename);
        game_data_v300.objects[i].limit = game_data_v212.objects[i].limit;
        sprintf(game_data_v300.objects[i].name, "%s", game_data_v212.objects[i].name);
        game_data_v300.objects[i].size.height = game_data_v212.objects[i].size.height;
        game_data_v300.objects[i].size.width = game_data_v212.objects[i].size.width;
        game_data_v300.objects[i].speed = game_data_v212.objects[i].speed;
        game_data_v300.objects[i].timer = game_data_v212.objects[i].timer;
        game_data_v300.objects[i].type = game_data_v212.objects[i].type;
    }


    for (int i=0; i<MAX_FILE_PROJECTILES; i++) {
        game_data_v300.projectiles[i].damage = game_data_v212.projectiles[i].damage;
        sprintf(game_data_v300.projectiles[i].graphic_filename, "%s", game_data_v212.projectiles[i].graphic_filename);
        game_data_v300.projectiles[i].hp = game_data_v212.projectiles[i].hp;
        game_data_v300.projectiles[i].is_destructible = game_data_v212.projectiles[i].is_destructible;
        game_data_v300.projectiles[i].max_shots = game_data_v212.projectiles[i].max_shots;
        sprintf(game_data_v300.projectiles[i].name, "%s", game_data_v212.projectiles[i].name);
        game_data_v300.projectiles[i].size.height = game_data_v212.projectiles[i].size.height;
        game_data_v300.projectiles[i].size.width = game_data_v212.projectiles[i].size.width;
        game_data_v300.projectiles[i].speed = game_data_v212.projectiles[i].speed;
        game_data_v300.projectiles[i].trajectory = game_data_v212.projectiles[i].trajectory;
    }
    for (int i=0; i<MAX_WEAPON_N; i++) {
        game_data_v300.weapons[i].damage = game_data_v212.weapons[i].damage;
        game_data_v300.weapons[i].id_projectile = game_data_v212.weapons[i].id_projectile;
        sprintf(game_data_v300.weapons[i].name, "%s", game_data_v212.weapons[i].name);
    }
    for (int i=0; i<MAX_FILE_PLAYERS; i++) {
        game_data_v300.players[i].can_charge_shot = game_data_v212.players[i].can_charge_shot;
        game_data_v300.players[i].can_slide = game_data_v212.players[i].can_slide;
        for (int j=0; j<3; j++) {
            game_data_v300.players[i].color_keys[j] = game_data_v212.players[i].color_keys[j];
        }
        game_data_v300.players[i].can_double_jump = game_data_v212.players[i].double_jump;
        sprintf(game_data_v300.players[i].face_filename, "%s", game_data_v212.players[i].face_filename);
        game_data_v300.players[i].full_charged_projectile_id = game_data_v212.players[i].full_charged_projectile_id;
        sprintf(game_data_v300.players[i].graphic_filename, "%s", game_data_v212.players[i].graphic_filename);
        game_data_v300.players[i].have_shield = game_data_v212.players[i].have_shield;
        game_data_v300.players[i].HP = game_data_v212.players[i].HP;
        game_data_v300.players[i].jump_gravity = game_data_v212.players[i].jump_gravity;
        game_data_v300.players[i].max_shots = game_data_v212.players[i].max_shots;
        game_data_v300.players[i].move_speed = game_data_v212.players[i].move_speed;
        sprintf(game_data_v300.players[i].name, "%s", game_data_v212.players[i].name);
        game_data_v300.players[i].simultaneous_shots = game_data_v212.players[i].simultaneous_shots;
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                game_data_v300.players[i].sprites [j][k].colision_rect= game_data_v212.players[i].sprites[j][k].colision_rect;
                game_data_v300.players[i].sprites [j][k].duration= game_data_v212.players[i].sprites[j][k].duration;
                game_data_v300.players[i].sprites [j][k].sprite_graphic_pos_x= game_data_v212.players[i].sprites[j][k].sprite_graphic_pos_x;
                game_data_v300.players[i].sprites [j][k].used= game_data_v212.players[i].sprites[j][k].used;
            }
        }
        game_data_v300.players[i].sprite_hit_area = game_data_v212.players[i].sprite_hit_area;
        game_data_v300.players[i].sprite_size.height = game_data_v212.players[i].sprite_size.height;
        game_data_v300.players[i].sprite_size.width = game_data_v212.players[i].sprite_size.width;
        for (int j=0; j<MAX_WEAPON_N; j++) {
            game_data_v300.players[i].weapon_colors[j].color1 = game_data_v212.players[i].weapon_colors[j].color1;
            game_data_v300.players[i].weapon_colors[j].color2 = game_data_v212.players[i].weapon_colors[j].color2;
            game_data_v300.players[i].weapon_colors[j].color3 = game_data_v212.players[i].weapon_colors[j].color3;
        }
    }

    for (int i=0; i<MAX_AI_TYPES; i++) {
        sprintf(game_data_v300.ai_types[i].name, "%s", game_data_v212.ai_types[i].name);
        for (int j=0; j<4; j++) {
            game_data_v300.ai_types[i].states[j].action = game_data_v212.ai_types[i].states[j].action;
            game_data_v300.ai_types[i].states[j].chance = game_data_v212.ai_types[i].states[j].chance;
            game_data_v300.ai_types[i].states[j].extra_parameter = game_data_v212.ai_types[i].states[j].extra_parameter;
            game_data_v300.ai_types[i].states[j].go_to = game_data_v212.ai_types[i].states[j].go_to;
            game_data_v300.ai_types[i].states[j].go_to_delay = game_data_v212.ai_types[i].states[j].go_to_delay;
        }
        for (int j=0; j<MAX_AI_REACTIONS; j++) {
            game_data_v300.ai_types[i].reactions[j].action = game_data_v300.game_npcs[i].sprites[ANIM_TYPE_TELEPORT][j+1].colision_rect.x;
            game_data_v300.ai_types[i].reactions[j].extra_parameter = game_data_v300.game_npcs[i].sprites[ANIM_TYPE_TELEPORT][j+1].colision_rect.y;
        }
    }
    game_data_v300.version = 1.1;
    sprintf(game_data_v300.name, "%s", "Rockbot");
    game_data_v300.semi_charged_projectile_id = 0;
    game_data_v300.player_items[0] = 0;
    game_data_v300.player_items[1] = 0;

    for (int i=0; i<16; i++) { // 16 because old file-format use this (new is 25)
        sprintf(v300_stages.stages[i].bgmusic_filename, "%s", v212_stages.stages[i].bgmusic_filename);
        //std::cout << "stages[" << i << "].boss.face_graphics_filename: " << std::endl;
        sprintf(v300_stages.stages[i].boss.face_graphics_filename, "%s", v212_stages.stages[i].boss.face_graphics_filename);
        v300_stages.stages[i].boss.id_npc = v212_stages.stages[i].boss.id_npc;
        v300_stages.stages[i].boss.id_weapon = v212_stages.stages[i].boss.id_weapon;
        sprintf(v300_stages.stages[i].boss.name, "%s", v212_stages.stages[i].boss.name);

        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                //std::cout << "stages[" << i << "].boss_dialog.answer1[" << j << "][" << k << "]: ";
                //std::cout << v212_stages.stages[i].boss_dialog.answer1[j][k] << std::endl;
                sprintf(v300_stages.stages[i].boss_dialog.answer1[j][k], "%s", v212_stages.stages[i].boss_dialog.answer1[j][k]);
                sprintf(v300_stages.stages[i].boss_dialog.answer2[j][k], "%s", v212_stages.stages[i].boss_dialog.answer2[j][k]);
            }
        }

        for (int j=0; j<FS_DIALOG_LINES; j++) {
            //std::cout << "i: " << i << ", j: " << j << std::endl;
            sprintf(v300_stages.stages[i].boss_dialog.line1[j], "%s", v212_stages.stages[i].boss_dialog.line1[j]);
            sprintf(v300_stages.stages[i].boss_dialog.line2[j], "%s", v212_stages.stages[i].boss_dialog.line2[j]);
        }

        for (int j=0; j<FS_COLORCYCLE_MAX_ITEMS; j++) {
            for (int k=0; k<COLOR_KEY_LENGTH; k++) {
                v300_stages.stages[i].colorcycle.color[j][k] = v212_stages.stages[i].colorcycle.color[j][k];
                v300_stages.stages[i].colorcycle.duration[j][k] = v212_stages.stages[i].colorcycle.duration[j][k];
            }
        }

        sprintf(v300_stages.stages[i].intro_dialog.face_graphics_filename, "%s", v212_stages.stages[i].intro_dialog.face_graphics_filename);
        v300_stages.stages[i].intro_dialog.top_side = v212_stages.stages[i].intro_dialog.top_side;
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                sprintf(v300_stages.stages[i].intro_dialog.answer1[j][k], "%s", v212_stages.stages[i].intro_dialog.answer1[j][k]);
                sprintf(v300_stages.stages[i].intro_dialog.answer2[j][k], "%s", v212_stages.stages[i].intro_dialog.answer2[j][k]);
            }
        }

        for (int k=0; k<FS_DIALOG_LINES; k++) {
            sprintf(v300_stages.stages[i].intro_dialog.line1[k], "%s", v212_stages.stages[i].intro_dialog.line1[k]);
            sprintf(v300_stages.stages[i].intro_dialog.line2[k], "%s", v212_stages.stages[i].intro_dialog.line2[k]);
        }

        for (int k=0; k<STAGE_MAX_LINKS; k++) {
            v300_stages.stages[i].links[k].bidirecional = v212_stages.stages[i].links[k].bidirecional;
            v300_stages.stages[i].links[k].id_map_destiny = v212_stages.stages[i].links[k].id_map_destiny;
            v300_stages.stages[i].links[k].id_map_origin = v212_stages.stages[i].links[k].id_map_origin;
            v300_stages.stages[i].links[k].is_door = v212_stages.stages[i].links[k].is_door;
            v300_stages.stages[i].links[k].pos_destiny.x = v212_stages.stages[i].links[k].pos_destiny.x;
            v300_stages.stages[i].links[k].pos_destiny.y = v212_stages.stages[i].links[k].pos_destiny.y;
            v300_stages.stages[i].links[k].pos_origin.x = v212_stages.stages[i].links[k].pos_origin.x;
            v300_stages.stages[i].links[k].pos_origin.y = v212_stages.stages[i].links[k].pos_origin.y;
            v300_stages.stages[i].links[k].size = v212_stages.stages[i].links[k].size;
            v300_stages.stages[i].links[k].type = v212_stages.stages[i].links[k].type;
        }

        // convert maps
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            for (int k=0; k<2; k++) {
                v300_stages.stages[i].maps[j].backgrounds[k].adjust_y = v212_stages.stages[i].maps[j].backgrounds[k].adjust_y;
                sprintf(v300_stages.stages[i].maps[j].backgrounds[k].filename, "%s", v212_stages.stages[i].maps[j].backgrounds[k].filename);
                v300_stages.stages[i].maps[j].backgrounds[k].speed = v212_stages.stages[i].maps[j].backgrounds[k].speed;
            }
            v300_stages.stages[i].maps[j].background_color = v212_stages.stages[i].maps[j].background_color;

            for (int k=0; k<FS_MAX_MAP_NPCS; k++) {
                v300_stages.stages[i].maps[j].map_npcs[k].direction = v212_stages.stages[i].maps[j].map_npcs[k].direction;
                v300_stages.stages[i].maps[j].map_npcs[k].id_npc = v212_stages.stages[i].maps[j].map_npcs[k].id_npc;
                v300_stages.stages[i].maps[j].map_npcs[k].start_point = v212_stages.stages[i].maps[j].map_npcs[k].start_point;
            }

            for (int k=0; k<FS_MAX_MAP_OBJECTS; k++) {
                v300_stages.stages[i].maps[j].map_objects[k].direction = v212_stages.stages[i].maps[j].map_objects[k].direction;
                v300_stages.stages[i].maps[j].map_objects[k].id_object = v212_stages.stages[i].maps[j].map_objects[k].id_object;
                v300_stages.stages[i].maps[j].map_objects[k].start_point = v212_stages.stages[i].maps[j].map_objects[k].start_point;
            }

            for (int k=0; k<MAP_W; k++) {
                for (int l=0; l<MAP_H; l++) {
                    v300_stages.stages[i].maps[j].tiles[k][l].locked = v212_stages.stages[i].maps[j].tiles[k][l].locked;
                    v300_stages.stages[i].maps[j].tiles[k][l].tile1.x = v212_stages.stages[i].maps[j].tiles[k][l].tile1.x;
                    v300_stages.stages[i].maps[j].tiles[k][l].tile1.y = v212_stages.stages[i].maps[j].tiles[k][l].tile1.y;
                    if (v212_stages.stages[i].maps[j].tiles[k][l].tile2.x != -1 && v212_stages.stages[i].maps[j].tiles[k][l].tile2.y != -1) {
                        v300_stages.stages[i].maps[j].tiles[k][l].tile1.x = v212_stages.stages[i].maps[j].tiles[k][l].tile2.x;
                        v300_stages.stages[i].maps[j].tiles[k][l].tile1.y = v212_stages.stages[i].maps[j].tiles[k][l].tile2.y;
                    }
                    v300_stages.stages[i].maps[j].tiles[k][l].tile3.x = v212_stages.stages[i].maps[j].tiles[k][l].tile3.x;
                    v300_stages.stages[i].maps[j].tiles[k][l].tile3.y = v212_stages.stages[i].maps[j].tiles[k][l].tile3.y;
                }
            }



        }

        sprintf(v300_stages.stages[i].name, "%s", v212_stages.stages[i].name);

        /// hardcoded values
        // PLAYERS
        game_data_v300.players[1].damage_modifier = -1;
        sprintf(game_data_v300.players[2].name, "%s", "CandyBot");
        game_data_v300.players[2].can_double_jump = true;
        game_data_v300.players[2].damage_modifier = 1;
        game_data_v300.players[2].max_shots = 2;
        sprintf(game_data_v300.players[3].name, "%s", "KittyBot");
        game_data_v300.players[3].can_air_dash = true;
        game_data_v300.players[2].damage_modifier = 1;
        game_data_v300.players[3].max_shots = 3;


    }


    fio_v300.write_game(game_data_v300);
    fio_v300.write_all_stages(v300_stages);
    std::cout << "Data written to file." << std::endl;

}

void convert::save_v212_to_v3() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_3_0_0::file_io fio_v300;

    format_v_2_1_2::st_save save_v212;
    format_v_3_0_0::st_save save_v300;

    std::cout << "Converting SAVE data from 2.1.2 to 3.0.0 ..." << std::endl;
    fio_v212.read_save(save_v212);


    save_v300.difficulty = save_v212.difficulty;
    save_v300.finished_stages = save_v212.finished_stages;
    save_v300.items.balancer = save_v212.items.balancer;
    save_v300.items.bolts = save_v212.items.bolts;
    save_v300.items.energy_saver = save_v212.items.energy_saver;
    save_v300.items.energy_tanks = save_v212.items.energy_tanks;
    save_v300.items.exit = save_v212.items.exit;
    save_v300.items.half_damage = save_v212.items.half_damage;
    save_v300.items.hyper_jump = save_v212.items.hyper_jump;
    save_v300.items.lifes = save_v212.items.lifes;
    save_v300.items.power_shot = save_v212.items.power_shot;
    save_v300.items.shock_guard = save_v212.items.shock_guard;
    save_v300.items.special_tanks = save_v212.items.special_tanks;
    save_v300.items.speed_shot = save_v212.items.speed_shot;
    save_v300.items.speed_up = save_v212.items.speed_up;
    save_v300.items.spike_guard = save_v212.items.spike_guard;
    for (int i=0; i<WEAPON_COUNT; i++) {
        save_v300.items.weapons[i] =  save_v212.items.weapons[i];
    }
    save_v300.items.weapon_tanks = save_v212.items.weapon_tanks;

    save_v300.selected_player = save_v212.selected_player-1; // we now start counting from zero
    for (int i=0; i<MAX_STAGES; i++) {
        save_v300.stages[i] = save_v212.stages[i];
    }

    fio_v300.write_save(save_v300);

}

void convert::config_v212_to_v3() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_3_0_0::file_io fio_v300;

    format_v_2_1_2::st_game_config config_v212;
    format_v_3_0_0::st_game_config config_v300;

    std::cout << "Converting SAVE data from 2.1.2 to 3.0.0 ..." << std::endl;
    fio_v212.load_config(config_v212);

    for (int i=0; i<BTN_COUNT ; i++) {
        config_v300.button_codes[i] = config_v212.button_codes[i];
    }
    config_v300.input_mode = config_v212.input_mode;
    config_v300.input_type = config_v212.input_type;
    for (int i=0; i<BTN_COUNT ; i++) {
        config_v300.keys_codes[i] = config_v212.keys_codes[i];
    }
    config_v300.platform = config_v212.platform;
    config_v300.sound_enabled = config_v212.sound_enabled;
    config_v300.video_filter = config_v212.video_filter;
    config_v300.video_fullscreen = config_v212.video_fullscreen;

    fio_v300.save_config(config_v300);

}

void convert::v300_to_v301() const
{
    format_v_3_0_0::file_io fio_v300;
    format_v_3_0_0::file_game game_data_v300;
    format_v_3_0_0::file_stages stages_v300;


    format_v_3_0_1::file_io fio_v301;
    format_v_3_0_1::file_game game_data_v301;
    format_v_3_0_1::file_stages stages_v301;

    std::cout << "Converting data from 3.0.0 to 3.0.1..." << std::endl;

    fio_v300.read_game(game_data_v300);
    fio_v300.read_all_stages(stages_v300);

    for (int i =0; i<FS_MAX_STAGES; i++) {
        //char name[FS_CHAR_NAME_SIZE];
        //char bgmusic_filename[FS_CHAR_FILENAME_SIZE];
        //file_map maps[FS_STAGE_MAX_MAPS];
        //file_link links[FS_STAGE_MAX_LINKS];
        //file_boss boss;
        //file_dialog intro_dialog;
        //file_dialog boss_dialog;
        //file_colorcycle colorcycle;
        sprintf(stages_v301.stages[i].name, "%s", stages_v300.stages[i].name);
        sprintf(stages_v301.stages[i].bgmusic_filename, "%s", stages_v300.stages[i].bgmusic_filename);
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            //st_map_background backgrounds[2];
            //st_color background_color;
            //map_tile tiles[MAP_W][MAP_H];		// map tiles
            //file_map_npc map_npcs[FS_MAX_MAP_NPCS];
            //file_map_object map_objects[FS_MAX_MAP_OBJECTS];
            for (int k=0; k<2; k++) {
                stages_v301.stages[i].maps[j].backgrounds[k].adjust_y = stages_v300.stages[i].maps[j].backgrounds[k].adjust_y;
                stages_v301.stages[i].maps[j].backgrounds[k].auto_scroll = stages_v300.stages[i].maps[j].backgrounds[k].auto_scroll;
                sprintf(stages_v301.stages[i].maps[j].backgrounds[k].filename, "%s", stages_v300.stages[i].maps[j].backgrounds[k].filename);
                stages_v301.stages[i].maps[j].backgrounds[k].speed = stages_v300.stages[i].maps[j].backgrounds[k].speed;
            }
            stages_v301.stages[i].maps[j].background_color = stages_v300.stages[i].maps[j].background_color;
            for (int k=0; k<MAP_W; k++) {
                for (int l=0; l<MAP_H; l++) {
                    stages_v301.stages[i].maps[j].tiles[k][l].locked = stages_v300.stages[i].maps[j].tiles[k][l].locked;
                    stages_v301.stages[i].maps[j].tiles[k][l].tile1 = stages_v300.stages[i].maps[j].tiles[k][l].tile1;
                    stages_v301.stages[i].maps[j].tiles[k][l].tile3 = stages_v300.stages[i].maps[j].tiles[k][l].tile3;
                }
            }
            for (int k=0; k<FS_MAX_MAP_NPCS; k++) {
                stages_v301.stages[i].maps[j].map_npcs[k].direction = stages_v300.stages[i].maps[j].map_npcs[k].direction;
                stages_v301.stages[i].maps[j].map_npcs[k].id_npc = stages_v300.stages[i].maps[j].map_npcs[k].id_npc;
                stages_v301.stages[i].maps[j].map_npcs[k].start_point = stages_v300.stages[i].maps[j].map_npcs[k].start_point;
            }
            for (int k=0; k<FS_MAX_MAP_OBJECTS; k++) {
                stages_v301.stages[i].maps[j].map_objects[k].direction = stages_v300.stages[i].maps[j].map_objects[k].direction;
                stages_v301.stages[i].maps[j].map_objects[k].id_object = stages_v300.stages[i].maps[j].map_objects[k].id_object;
                stages_v301.stages[i].maps[j].map_objects[k].start_point = stages_v300.stages[i].maps[j].map_objects[k].start_point;
            }
        }

        for (int j=0; j<FS_STAGE_MAX_LINKS; j++) {
            stages_v301.stages[i].links[j].bidirecional = stages_v300.stages[i].links[j].bidirecional;
            stages_v301.stages[i].links[j].id_map_destiny = stages_v300.stages[i].links[j].id_map_destiny;
            stages_v301.stages[i].links[j].id_map_origin = stages_v300.stages[i].links[j].id_map_origin;
            stages_v301.stages[i].links[j].is_door = stages_v300.stages[i].links[j].is_door;
            stages_v301.stages[i].links[j].pos_destiny = stages_v300.stages[i].links[j].pos_destiny;
            stages_v301.stages[i].links[j].pos_origin = stages_v300.stages[i].links[j].pos_origin;
            stages_v301.stages[i].links[j].size = stages_v300.stages[i].links[j].size;
            stages_v301.stages[i].links[j].type = stages_v300.stages[i].links[j].type;
        }
        sprintf(stages_v301.stages[i].boss.face_graphics_filename, "%s", stages_v300.stages[i].boss.face_graphics_filename);
        stages_v301.stages[i].boss.id_npc = stages_v300.stages[i].boss.id_npc;
        stages_v301.stages[i].boss.id_weapon = stages_v300.stages[i].boss.id_weapon;
        sprintf(stages_v301.stages[i].boss.name, "%s", stages_v300.stages[i].boss.name);

        for (int j=0; j<FS_DIALOG_LINES; j++) {
            sprintf(stages_v301.stages[i].intro_dialog.line1[j], "%s", stages_v300.stages[i].intro_dialog.line1[j]);
            sprintf(stages_v301.stages[i].intro_dialog.line2[j], "%s", stages_v300.stages[i].intro_dialog.line2[j]);
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                sprintf(stages_v301.stages[i].intro_dialog.answer1[j][k], "%s", stages_v300.stages[i].intro_dialog.answer1[j][k]);
                sprintf(stages_v301.stages[i].intro_dialog.answer2[j][k], "%s", stages_v300.stages[i].intro_dialog.answer2[j][k]);
            }
        }
        sprintf(stages_v301.stages[i].intro_dialog.face_graphics_filename, "%s", stages_v300.stages[i].intro_dialog.face_graphics_filename);
        stages_v301.stages[i].intro_dialog.top_side = stages_v300.stages[i].intro_dialog.top_side;



        for (int j=0; j<FS_DIALOG_LINES; j++) {
            sprintf(stages_v301.stages[i].boss_dialog.line1[j], "%s", stages_v300.stages[i].boss_dialog.line1[j]);
            sprintf(stages_v301.stages[i].boss_dialog.line2[j], "%s", stages_v300.stages[i].boss_dialog.line2[j]);
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                sprintf(stages_v301.stages[i].boss_dialog.answer1[j][k], "%s", stages_v300.stages[i].boss_dialog.answer1[j][k]);
                sprintf(stages_v301.stages[i].boss_dialog.answer2[j][k], "%s", stages_v300.stages[i].boss_dialog.answer2[j][k]);
            }
        }
        sprintf(stages_v301.stages[i].boss_dialog.face_graphics_filename, "%s", stages_v300.stages[i].boss_dialog.face_graphics_filename);
        stages_v301.stages[i].boss_dialog.top_side = stages_v300.stages[i].boss_dialog.top_side;

        for (int j=0; j<FS_COLORCYCLE_MAX_ITEMS; j++) {
            for (int k=0; k<COLOR_KEY_LENGTH; k++) {
                stages_v301.stages[i].colorcycle.color[j][k] = stages_v300.stages[i].colorcycle.color[j][k];
                stages_v301.stages[i].colorcycle.duration[j][k] = stages_v300.stages[i].colorcycle.duration[j][k];
            }
        }
    }
    fio_v301.write_all_stages(stages_v301);


    //game_data_v301 = game_data_v300;
    //float version;
    //char name[FS_CHAR_NAME_SIZE];
    //file_npc game_npcs[FS_GAME_MAX_NPCS];                           // 60 enemy types is more than good enought
    //file_object objects[FS_GAME_MAX_NPCS];
    //file_projectile projectiles[FS_MAX_PROJECTILES];
    //file_weapon weapons[FS_MAX_WEAPONS];                            // 8 stage-bosses and 3 item-like
    //file_player players[FS_MAX_PLAYERS];                            // up to 4 different players the user can select from
    //file_artificial_inteligence ai_types[FS_MAX_AI_TYPES];
    //Sint8 semi_charged_projectile_id;                              // common to all players
    //Sint8 player_items[FS_PLATER_ITEMS_N];                                         // common to all players -> to be used in add_coil_object and add_jet_object
    //char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX];
    //st_file_trophy trophies[TROPHIES_MAX];
    //st_shop_dialog shop_dialog_welcome;
    //st_shop_dialog shop_dialog_goodbye;
    //st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX];
    //st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX];

    game_data_v301.version = game_data_v300.version;
    sprintf(game_data_v301.name, "%s", game_data_v300.name);
    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            game_data_v301.game_npcs[i].attack_frame_n[j] = game_data_v300.game_npcs[i].attack_frame_n[j];
        }
        sprintf(game_data_v301.game_npcs[i].bg_graphic_filename, "%s", game_data_v300.game_npcs[i].bg_graphic_filename);
        game_data_v301.game_npcs[i].direction = game_data_v300.game_npcs[i].direction;
        game_data_v301.game_npcs[i].facing = game_data_v300.game_npcs[i].facing;
        game_data_v301.game_npcs[i].fly_flag = game_data_v300.game_npcs[i].fly_flag;
        game_data_v301.game_npcs[i].frame_size = game_data_v300.game_npcs[i].frame_size;
        sprintf(game_data_v301.game_npcs[i].graphic_filename, "%s", game_data_v300.game_npcs[i].graphic_filename);
        game_data_v301.game_npcs[i].hp = game_data_v300.game_npcs[i].hp;
        game_data_v301.game_npcs[i].IA_type = game_data_v300.game_npcs[i].IA_type;
        game_data_v301.game_npcs[i].id = game_data_v300.game_npcs[i].id;
        game_data_v301.game_npcs[i].is_boss = game_data_v300.game_npcs[i].is_boss;
        game_data_v301.game_npcs[i].is_ghost = game_data_v300.game_npcs[i].is_ghost;
        game_data_v301.game_npcs[i].is_sub_boss = game_data_v300.game_npcs[i].is_sub_boss;
        sprintf(game_data_v301.game_npcs[i].name, "%s", game_data_v300.game_npcs[i].name);
        for (int j=0; j<2; j++) {
            game_data_v301.game_npcs[i].projectile_id[j] = game_data_v300.game_npcs[i].projectile_id[j];
        }
        game_data_v301.game_npcs[i].respawn_delay = 0;
        game_data_v301.game_npcs[i].shield_type = game_data_v300.game_npcs[i].shield_type;
        game_data_v301.game_npcs[i].speed = game_data_v300.game_npcs[i].speed;
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                game_data_v301.game_npcs[i].sprites[j][k].colision_rect = game_data_v300.game_npcs[i].sprites[j][k].colision_rect;
                game_data_v301.game_npcs[i].sprites[j][k].duration = game_data_v300.game_npcs[i].sprites[j][k].duration;
                game_data_v301.game_npcs[i].sprites[j][k].sprite_graphic_pos_x = game_data_v300.game_npcs[i].sprites[j][k].sprite_graphic_pos_x;
                game_data_v301.game_npcs[i].sprites[j][k].used = game_data_v300.game_npcs[i].sprites[j][k].used;
            }
        }
        game_data_v301.game_npcs[i].sprites_pos_bg = game_data_v300.game_npcs[i].sprites_pos_bg;
        game_data_v301.game_npcs[i].start_point = game_data_v300.game_npcs[i].start_point;
        game_data_v301.game_npcs[i].walk_range = game_data_v300.game_npcs[i].walk_range;
        for (int j=0; j<FS_NPC_WEAKNESSES; j++) {
            game_data_v301.game_npcs[i].weakness[j].damage_multiplier = game_data_v300.game_npcs[i].weakness[j].damage_multiplier;
            game_data_v301.game_npcs[i].weakness[j].weapon_id = game_data_v300.game_npcs[i].weakness[j].weapon_id;
        }
    }
    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        game_data_v301.objects[i].animation_auto_start = game_data_v300.objects[i].animation_auto_start;
        game_data_v301.objects[i].animation_loop = game_data_v300.objects[i].animation_loop;
        game_data_v301.objects[i].animation_reverse = game_data_v300.objects[i].animation_reverse;
        game_data_v301.objects[i].direction = game_data_v300.objects[i].direction;
        game_data_v301.objects[i].distance = game_data_v300.objects[i].distance;
        game_data_v301.objects[i].frame_duration = game_data_v300.objects[i].frame_duration;
        sprintf(game_data_v301.objects[i].graphic_filename, "%s", game_data_v300.objects[i].graphic_filename);
        game_data_v301.objects[i].limit = game_data_v300.objects[i].limit;
        sprintf(game_data_v301.objects[i].name, "%s", game_data_v300.objects[i].name);
        game_data_v301.objects[i].size = game_data_v300.objects[i].size;
        game_data_v301.objects[i].speed = game_data_v300.objects[i].speed;
        game_data_v301.objects[i].timer = game_data_v300.objects[i].timer;
        game_data_v301.objects[i].type = game_data_v300.objects[i].type;
    }
    for (int i=0; i<FS_MAX_PROJECTILES; i++) {
        game_data_v301.projectiles[i].can_be_reflected = true;
        game_data_v301.projectiles[i].damage = game_data_v300.projectiles[i].damage;
        sprintf(game_data_v301.projectiles[i].graphic_filename, "%s", game_data_v300.projectiles[i].graphic_filename);
        game_data_v301.projectiles[i].hp = game_data_v300.projectiles[i].hp;
        game_data_v301.projectiles[i].is_destructible = game_data_v300.projectiles[i].is_destructible;
        game_data_v301.projectiles[i].max_shots = game_data_v300.projectiles[i].max_shots;
        sprintf(game_data_v301.projectiles[i].name, "%s", game_data_v300.projectiles[i].name);
        game_data_v301.projectiles[i].sfx_filename[0] = '\0';
        game_data_v301.projectiles[i].size = game_data_v300.projectiles[i].size;
        game_data_v301.projectiles[i].spawn_npc_id = -1;
        game_data_v301.projectiles[i].spawn_npc_n = 0;
        game_data_v301.projectiles[i].speed = game_data_v300.projectiles[i].speed;
        game_data_v301.projectiles[i].trajectory = game_data_v300.projectiles[i].trajectory;
    }
    for (int i=0; i<FS_MAX_WEAPONS; i++) {
        game_data_v301.weapons[i].damage = game_data_v300.weapons[i].damage;
        game_data_v301.weapons[i].id_projectile = game_data_v300.weapons[i].id_projectile;
        sprintf(game_data_v301.weapons[i].name, "%s", game_data_v300.weapons[i].name);
    }
    for (int i=0; i<FS_MAX_PLAYERS; i++) {
        game_data_v301.players[i].can_air_dash = game_data_v300.players[i].can_air_dash;
        game_data_v301.players[i].can_charge_shot = game_data_v300.players[i].can_charge_shot;
        game_data_v301.players[i].can_double_jump = game_data_v300.players[i].can_double_jump;
        game_data_v301.players[i].can_shot_diagonal = game_data_v300.players[i].can_shot_diagonal;
        game_data_v301.players[i].can_slide = game_data_v300.players[i].can_slide;
        for (int j=0; j<3; j++) {
            game_data_v301.players[i].color_keys[j] = game_data_v300.players[i].color_keys[j];
        }
        game_data_v301.players[i].damage_modifier = game_data_v300.players[i].damage_modifier;
        sprintf(game_data_v301.players[i].face_filename, "%s", game_data_v300.players[i].face_filename);
        game_data_v301.players[i].full_charged_projectile_id = game_data_v300.players[i].full_charged_projectile_id;
        sprintf(game_data_v301.players[i].graphic_filename, "%s", game_data_v300.players[i].graphic_filename);
        game_data_v301.players[i].have_shield = game_data_v300.players[i].have_shield;
        game_data_v301.players[i].HP = game_data_v300.players[i].HP;
        game_data_v301.players[i].max_shots = game_data_v300.players[i].max_shots;
        game_data_v301.players[i].move_speed = game_data_v300.players[i].move_speed;
        sprintf(game_data_v301.players[i].name, "%s", game_data_v300.players[i].name);
        game_data_v301.players[i].simultaneous_shots = game_data_v300.players[i].simultaneous_shots;
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                game_data_v301.players[i].sprites[j][k].colision_rect = game_data_v300.players[i].sprites[j][k].colision_rect;
                game_data_v301.players[i].sprites[j][k].duration = game_data_v300.players[i].sprites[j][k].duration;
                game_data_v301.players[i].sprites[j][k].sprite_graphic_pos_x = game_data_v300.players[i].sprites[j][k].sprite_graphic_pos_x;
                game_data_v301.players[i].sprites[j][k].used = game_data_v300.players[i].sprites[j][k].used;
            }
        }
        game_data_v301.players[i].sprite_hit_area = game_data_v300.players[i].sprite_hit_area;
        game_data_v301.players[i].sprite_size = game_data_v300.players[i].sprite_size;
        for (int j=0; j<MAX_WEAPON_N; j++) {
            game_data_v301.players[i].weapon_colors[j].color1 = game_data_v300.players[i].weapon_colors[j].color1;
            game_data_v301.players[i].weapon_colors[j].color2 = game_data_v300.players[i].weapon_colors[j].color2;
            game_data_v301.players[i].weapon_colors[j].color3 = game_data_v300.players[i].weapon_colors[j].color3;
        }
    }
    for (int i=0; i<FS_MAX_AI_TYPES; i++) {
        sprintf(game_data_v301.ai_types[i].name, "%s", game_data_v300.ai_types[i].name);
        for (int j=0; j<MAX_AI_REACTIONS; j++) {
            game_data_v301.ai_types[i].reactions[j].action = game_data_v300.ai_types[i].reactions[j].action;
            game_data_v301.ai_types[i].reactions[j].extra_parameter = game_data_v300.ai_types[i].reactions[j].extra_parameter;
            game_data_v301.ai_types[i].reactions[j].go_to = game_data_v300.ai_types[i].reactions[j].go_to;
            game_data_v301.ai_types[i].reactions[j].go_to_delay = game_data_v300.ai_types[i].reactions[j].go_to_delay;
        }
        for (int j=0; j<AI_MAX_STATES; j++) {
            game_data_v301.ai_types[i].states[j].action = game_data_v300.ai_types[i].states[j].action;
            game_data_v301.ai_types[i].states[j].chance = game_data_v300.ai_types[i].states[j].chance;
            game_data_v301.ai_types[i].states[j].extra_parameter = game_data_v300.ai_types[i].states[j].extra_parameter;
            game_data_v301.ai_types[i].states[j].go_to = game_data_v300.ai_types[i].states[j].go_to;
            game_data_v301.ai_types[i].states[j].go_to_delay = game_data_v300.ai_types[i].states[j].go_to_delay;
        }
    }
    game_data_v301.semi_charged_projectile_id = game_data_v300.semi_charged_projectile_id;
    for (int i=0; i<FS_PLATER_ITEMS_N; i++) {
        game_data_v301.player_items[i] = game_data_v300.player_items[i];
    }
    for (int i=0; i<MAX_STAGES; i++) {
        sprintf(game_data_v301.stage_face_filename[i], "%s", game_data_v300.stage_face_filename[i]);
    }
    for (int i=0; i<TROPHIES_MAX; i++) {
        game_data_v301.trophies[i].condition = game_data_v300.trophies[i].condition;
        sprintf(game_data_v301.trophies[i].filename, "%s", game_data_v300.trophies[i].filename);
        sprintf(game_data_v301.trophies[i].name, "%s", game_data_v300.trophies[i].name);
    }

    sprintf(game_data_v301.shop_dialog_welcome.face_graphics_filename, "%s", game_data_v300.shop_dialog_welcome.face_graphics_filename);
    for (int i=0; i<FS_DIALOG_LINES; i++) {
        sprintf(game_data_v301.shop_dialog_welcome.line1[i], "%s", game_data_v300.shop_dialog_welcome.line1[i]);
    }
    sprintf(game_data_v301.shop_dialog_goodbye.face_graphics_filename, "%s", game_data_v300.shop_dialog_goodbye.face_graphics_filename);
    for (int i=0; i<FS_DIALOG_LINES; i++) {
        sprintf(game_data_v301.shop_dialog_goodbye.line1[i], "%s", game_data_v300.shop_dialog_goodbye.line1[i]);
    }

    for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX; i++) {
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            game_data_v301.armor_pieces[i].special_ability[j] = game_data_v300.armor_pieces[i].special_ability[j];
        }
        game_data_v301.armor_pieces[i].projectile_id = -1;
        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                game_data_v301.armor_pieces[i].got_message[j][k][0] = '\0';
            }
        }
    }
    for (int i=0; i<FS_ANIM_TILES_MAX; i++) {
        game_data_v301.anim_tiles[i].delay = game_data_v300.anim_tiles[i].delay;
        sprintf(game_data_v301.anim_tiles[i].filename, "%s", game_data_v300.anim_tiles[i].filename);
    }
    fio_v301.write_game(game_data_v301);
}


void convert::save_v212_to_v301() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_3_0_1::file_io fio_v301;

    format_v_2_1_2::st_save save_v212;
    format_v_3_0_1::st_save save_v301;

    std::cout << "Converting SAVE data from 2.1.2 to 3.0.1 ..." << std::endl;
    fio_v212.read_save(save_v212);


    save_v301.difficulty = save_v212.difficulty;
    save_v301.finished_stages = save_v212.finished_stages;
    save_v301.items.balancer = save_v212.items.balancer;
    save_v301.items.bolts = save_v212.items.bolts;
    save_v301.items.energy_saver = save_v212.items.energy_saver;
    save_v301.items.energy_tanks = save_v212.items.energy_tanks;
    save_v301.items.exit = save_v212.items.exit;
    save_v301.items.half_damage = save_v212.items.half_damage;
    save_v301.items.hyper_jump = save_v212.items.hyper_jump;
    save_v301.items.lifes = save_v212.items.lifes;
    save_v301.items.power_shot = save_v212.items.power_shot;
    save_v301.items.shock_guard = save_v212.items.shock_guard;
    save_v301.items.special_tanks = save_v212.items.special_tanks;
    save_v301.items.speed_shot = save_v212.items.speed_shot;
    save_v301.items.speed_up = save_v212.items.speed_up;
    save_v301.items.spike_guard = save_v212.items.spike_guard;
    for (int i=0; i<WEAPON_COUNT; i++) {
        save_v301.items.weapons[i] =  save_v212.items.weapons[i];
    }
    save_v301.items.weapon_tanks = save_v212.items.weapon_tanks;

    save_v301.selected_player = save_v212.selected_player-1; // we now start counting from zero
    for (int i=0; i<MAX_STAGES; i++) {
        save_v301.stages[i] = save_v212.stages[i];
    }

    fio_v301.write_save(save_v301);

}

void convert::config_v212_to_v301() const
{
    format_v_2_1_2::file_io fio_v212;
    format_v_3_0_0::file_io fio_v301;

    format_v_2_1_2::st_game_config config_v212;
    format_v_3_0_0::st_game_config config_v301;

    std::cout << "Converting SAVE data from 2.1.2 to 3.0.1 ..." << std::endl;
    fio_v212.load_config(config_v212);

    for (int i=0; i<BTN_COUNT ; i++) {
        config_v301.button_codes[i] = config_v212.button_codes[i];
    }
    config_v301.input_mode = config_v212.input_mode;
    config_v301.input_type = config_v212.input_type;
    for (int i=0; i<BTN_COUNT ; i++) {
        config_v301.keys_codes[i] = config_v212.keys_codes[i];
    }
    config_v301.platform = config_v212.platform;
    config_v301.sound_enabled = config_v212.sound_enabled;
    config_v301.video_filter = config_v212.video_filter;
    config_v301.video_fullscreen = config_v212.video_fullscreen;

    fio_v301.save_config(config_v301);

}
