#ifndef V_2_0_2_H
#define V_2_0_2_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_1.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_0_2 {

    /**
     * @brief
     *
     */
    struct st_weakness {
        short weapon_id;									// weapon number of weakness, if -1, no weakness /**< TODO */
        short damage_multiplier;							// how many time more damage is taken, if zero, no damage from this weapon is taken /**< TODO */
		st_weakness() {
			weapon_id = -1;
			damage_multiplier = 1;
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
        format_v2_0::st_sprite_info frames[NPC_FRAME_NUMBER];		// holds information about the type of each frame /**< TODO */
        st_size frame_size; /**< TODO */
        bool can_shoot; /**< TODO */
        short shield_type; /**< TODO */
        short IA_type;										// IA types: no_move, wait, attack, zig-zag (loop), side to side (origin to direction) /**< TODO */
        short death_spawn;									// if different than -1, the NPC will spawn the given npc ID after killed /**< TODO */
        st_weakness weakness[9];							// each npc have weaknesses for each boss weapon, plus the normal weapon /**< TODO */
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
            direction = 0;
		}
	};


    /**
     * @brief
     *
     */
    struct file_game {
        float version; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        format_v_2_0_2::file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */ /**< TODO */
        format_v2_0::file_object objects[GAME_MAX_OBJS]; /**< TODO */
        format_v2_0::file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */
        format_v2_0::file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
        format_v2_0::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
        format_v_2_0_1::file_stage stages[MAX_STAGES]; /**< TODO */
        format_v2_0::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
		file_game() {
			/// *** hardcoded parts *** ///
			version = 0.92; // 0.9 means 1.0 beta, 0.92 means beta #2
			sprintf(name, "%s", "Rockbot");
		}
	};

}

#endif // V_2_0_2_H
