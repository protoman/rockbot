#ifndef V_2_0_3_H
#define V_2_0_3_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_2.h"
//#include "st_common.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_0_3 {

    /**
     * @brief
     *
     */
    struct st_sprite_info { // stores information about each animation sprite
        unsigned int id; /**< TODO */
        unsigned short duration; /**< TODO */
        short state; /**< TODO */
        short sprite_pos_x; /**< TODO */
        st_rectangle colision_rect; /**< TODO */
		st_sprite_info() {
			id = -1;
			duration = 0;
			state = 0;
			sprite_pos_x = 0;
			colision_rect.x = 0;
			colision_rect.y = 0;
			colision_rect.w = 0;
			colision_rect.h = 0;
		}
	};


    /**
     * @brief
     *
     */
    struct file_npc { // DONE - Game
        short id; /**< TODO */
		//unsigned int projectile_id;							// indicates the projectile ID (relation with file_projectile)
        short projectile_id[2]; // indicate the id of an attack the NCP can user /**< TODO */ /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        struct st_hit_points hp; /**< TODO */
        short direction;              // defines how it behavies (kink of AI*) /**< TODO */
        short speed;               // defines the distances it can see enemies /**< TODO */
        short walk_range;                 // defines how long from the start point it can go /**< TODO */
        short facing;                     // defines the side npc is facing before start moving (also used by LINEWALK behavior) /**< TODO */
        struct st_position start_point; /**< TODO */
        format_v_2_0_3::st_sprite_info frames[NPC_FRAME_NUMBER];		// holds information about the type of each frame /**< TODO */
        st_size frame_size; /**< TODO */
        bool can_shoot; /**< TODO */
        short shield_type; /**< TODO */
        short IA_type;										// IA types: no_move, wait, attack, zig-zag (loop), side to side (origin to direction) /**< TODO */
        short death_spawn;									// if different than -1, the NPC will spawn the given npc ID after killed /**< TODO */
        format_v_2_0_2::st_weakness weakness[9];			// each npc have weaknesses for each boss weapon, plus the normal weapon /**< TODO */
        char bg_graphic_filename[CHAR_FILENAME_SIZE];		// holds a static background /**< TODO */
        st_position sprites_pos_bg;							// holds position of sprites in relation with background /**< TODO */


		file_npc() {
			id = -1;
			name[0] = '\0';
			graphic_filename[0] = '\0';
			projectile_id[0] = -1;
			projectile_id[1] = -1;
			death_spawn = -1;
			bg_graphic_filename[0] = '\0';
		}
	};


    /**
     * @brief
     *
     */
    struct file_player { // DONE -> Game
        short id; /**< TODO */ /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        char face_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        unsigned short HP; /**< TODO */
        st_size sprite_size;				// size of sprite graphic /**< TODO */
        st_rectangle sprite_hit_area;		// the area of the graphic where is used for hit/colision /**< TODO */
        unsigned short move_speed;		// how many sprites move each step /**< TODO */
        float jump_initial_speed;			// used to adjust jump speed/height /**< TODO */
        float jump_gravity;						// used to adjust jump speed/height /**< TODO */
        float fall_gravity; /**< TODO */
        st_sprite_info sprites[MAX_PLAYER_SPRITES]; /**< TODO */
        format_v2_0::file_weapon_colors weapon_colors[MAX_WEAPON_N]; /**< TODO */
        st_color color_keys[3]; /**< TODO */
		// habilities part
        bool have_shield; /**< TODO */
        unsigned short max_shots;		// number of maximum simultaneous projectiles /**< TODO */
        unsigned short simultaneous_shots; // number of projectiles shot at one button press /**< TODO */
        bool double_jump; /**< TODO */
        bool can_slide; // is false, dashes instead of sliding /**< TODO */
        bool can_charge_shot; /**< TODO */

		file_player() {
			id = -1;
			name[0] = '\0';
			graphic_filename[0] = '\0';
			face_filename[0] = '\0';
			double_jump = false;
			have_shield = true;
			can_slide = false;
			can_charge_shot = true;
		}
	};


    /**
     * @brief
     *
     */
    struct map_tile {
        short id; /**< TODO */
        short locked; /**< TODO */
        struct st_position tile1;                  // position of tile (x and y) in the graphic file (each tile is 16*16) /**< TODO */
        struct st_position tile2;                  // second level tile. this can be used to create more complex backgrounds /**< TODO */
        struct st_position tile3;                  // tird level, to be shown over player /**< TODO */
		map_tile() {
			id = -1;
			locked = 0;
		}
	};


    /**
     * @brief
     *
     */
    struct file_map {
        short id; /**< TODO */
        format_v_2_0_1::st_map_background backgrounds[2]; /**< TODO */
        st_color background_color; /**< TODO */
        format_v_2_0_3::map_tile tiles[MAP_W][MAP_H];		// map tiles /**< TODO */
        format_v2_0::file_map_npc map_npcs[MAX_MAP_NPC_N]; /**< TODO */
        format_v2_0::file_map_object map_objects[MAX_MAP_NPC_N]; /**< TODO */
		file_map() {
			id = -1;
		}
	};

    /**
     * @brief
     *
     */
    struct file_stage {
        short id; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char bgmusic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        format_v_2_0_3::file_map maps[STAGE_MAX_MAPS]; /**< TODO */ /**< TODO */
        format_v2_0::file_link links[STAGE_MAX_LINKS]; /**< TODO */
        format_v2_0::file_boss boss; /**< TODO */
        format_v2_0::file_dialog intro_dialog; /**< TODO */
        format_v2_0::file_dialog boss_dialog; /**< TODO */
        format_v2_0::file_colorcycle colorcycle; /**< TODO */
		file_stage() {
			id = -1;
			boss = format_v2_0::file_boss();
			name[0] = '\0';
			bgmusic_filename[0] = '\0';
		}
        /**
         * @brief
         *
         * @param old_stage
         * @return file_stage &operator
         */
        file_stage & operator=(format_v_2_0_1::file_stage old_stage)
		{
            std::cout << "stage 2.0.1 to 2.0.3" << std::endl;
			id = old_stage.id;
			sprintf(name, "%s", old_stage.name);
			sprintf(bgmusic_filename, "%s", old_stage.bgmusic_filename);
			for (int i=0; i<STAGE_MAX_MAPS; i++) {
				for (int j=0; j<2; j++) {
					maps[i].backgrounds[j].adjust_y = old_stage.maps[i].backgrounds[j].adjust_y;
					sprintf(maps[i].backgrounds[j].filename, "%s", old_stage.maps[i].backgrounds[j].filename);
					maps[i].backgrounds[j].id = old_stage.maps[i].backgrounds[j].id;
					maps[i].backgrounds[j].speed = old_stage.maps[i].backgrounds[j].speed;
				}
				maps[i].background_color = old_stage.maps[i].background_color;
				maps[i].id = old_stage.maps[i].id;

				for (int j=0; j<MAX_MAP_NPC_N; j++) {
					maps[i].map_npcs[j] = old_stage.maps[i].map_npcs[j];
					maps[i].map_npcs[j].start_point.y = old_stage.maps[i].map_npcs[j].start_point.y+2;
				}


				for (int j=0; j<MAX_MAP_NPC_N; j++) {
					maps[i].map_objects[j] = old_stage.maps[i].map_objects[j];
					maps[i].map_objects[j].start_point.y = old_stage.maps[i].map_objects[j].start_point.y+2;
				}

				for (int k=0; k<MAP_H-FORMAT202_MAP_H; k++) {
					for (int j=0; j<MAP_W; j++) {
						maps[i].tiles[j][k].id = old_stage.maps[i].tiles[j][0].id;
						maps[i].tiles[j][k].locked = old_stage.maps[i].tiles[j][0].locked;
						maps[i].tiles[j][k].tile1 = old_stage.maps[i].tiles[j][0].tile1;
						maps[i].tiles[j][k].tile2 = old_stage.maps[i].tiles[j][0].tile2;
						maps[i].tiles[j][k].tile3 = old_stage.maps[i].tiles[j][0].tile3;
					}
				}
				for (int j=0; j<MAP_W; j++) {
					for (int k=0; k<MAP_H-2; k++) {
						maps[i].tiles[j][k+2].id = old_stage.maps[i].tiles[j][k].id;
						maps[i].tiles[j][k+2].locked = old_stage.maps[i].tiles[j][k].locked;
						maps[i].tiles[j][k+2].tile1 = old_stage.maps[i].tiles[j][k].tile1;
						maps[i].tiles[j][k+2].tile2 = old_stage.maps[i].tiles[j][k].tile2;
						maps[i].tiles[j][k+2].tile3 = old_stage.maps[i].tiles[j][k].tile3;
					}
				}

			}
			for (int i=0; i<STAGE_MAX_LINKS; i++) {
				links[i] = old_stage.links[i];
				if (old_stage.links[i].pos_origin.y > MAP_H/2) {
					links[i].pos_origin.y =  old_stage.links[i].pos_origin.y+2;
				}
				if (old_stage.links[i].pos_destiny.y > MAP_H/2) {
					links[i].pos_destiny.y =  old_stage.links[i].pos_destiny.y+2;
				}
			}
			boss = old_stage.boss;
			boss_dialog = old_stage.boss_dialog;
			colorcycle = old_stage.colorcycle;
			return *this;
		}

	};





    /**
     * @brief
     *
     */
    struct file_game {
        float version; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        format_v_2_0_3::file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */
        format_v2_0::file_object objects[GAME_MAX_OBJS]; /**< TODO */
        format_v2_0::file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */
        format_v2_0::file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
        format_v_2_0_3::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
        format_v_2_0_3::file_stage stages[MAX_STAGES]; /**< TODO */
        format_v2_0::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
		file_game() {
			/// *** hardcoded parts *** ///
			version = 0.92; // 0.9 means 1.0 beta, 0.92 means beta #2
			sprintf(name, "%s", "Rockbot");
		}
	};

}

#endif // V_2_0_3_H
