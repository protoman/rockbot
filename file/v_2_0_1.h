#ifndef V_2_0_1_H
#define V_2_0_1_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_0_1 {

    /**
     * @brief
     *
     */
    struct st_map_background {
        short id; /**< TODO */
        char filename[CHAR_FILENAME_SIZE]; /**< TODO */
        short adjust_y; // distance of the background from the screen top /**< TODO */
        short speed; // 0 means will scroll in the same speed as foreground (so the bg is used only to ease map-creation) /**< TODO */
		st_map_background()
		{
			id = -1;
			filename[0] = '\0';
			adjust_y = 0;
			speed = 0;
		}
	};

    /**
     * @brief
     *
     */
    struct file_map {
        short id; /**< TODO */
        st_map_background backgrounds[2]; /**< TODO */ /**< TODO */
        st_color background_color; /**< TODO */
        format_v2_0::map_tile tiles[MAP_W][MAP_H-2];		// map tiles /**< TODO */
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
        file_map maps[STAGE_MAX_MAPS]; /**< TODO */
        format_v2_0::file_link links[STAGE_MAX_LINKS]; /**< TODO */ /**< TODO */
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
	};


    /**
     * @brief
     *
     */
    struct file_game {
        float version; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        format_v2_0::file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */
        format_v2_0::file_object objects[GAME_MAX_OBJS]; /**< TODO */ /**< TODO */
        format_v2_0::file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */
        format_v2_0::file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
        format_v2_0::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
        file_stage stages[MAX_STAGES]; /**< TODO */
        format_v2_0::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
		file_game() {
			/// *** hardcoded parts *** ///
			// game
			version = 0.91; // 0.9 means 1.0 beta, 0.91 means beta1
			sprintf(name, "%s", "Rockbot");
		}
	};



	// FOR SAVE
    /**
     * @brief
     *
     */
    struct st_items {
        unsigned short int energy_tanks; /**< TODO */ /**< TODO */
        unsigned short int weapon_tanks; /**< TODO */
        unsigned short int special_tanks; /**< TODO */
        unsigned short int balancer; /**< TODO */
        unsigned short int lifes; /**< TODO */
        unsigned short int bolts; /**< TODO */
        unsigned short int weapons[WEAPON_COUNT]; /**< TODO */
		st_items() {
            energy_tanks = 0;
            weapon_tanks = 0;
            special_tanks = 0;
			balancer = 0;
			lifes = 3;
			bolts = 0;
			for (int i=0; i<WEAPON_COUNT; i++) {
				weapons[i] = PLAYER_INITIAL_HP;
			}
		}
	};

	// TODO - use vectors instead of C/like fixed-size arrays
    /**
     * @brief
     *
     */
    struct st_save {
        short int finished_stages; // number of already finished stages, just to simplify calc /**< TODO */
        short int stages[MAX_STAGES]; // 0 still open, 1 finished /**< TODO */
        st_items items; /**< TODO */
        short int selected_player;
		st_save() {
			finished_stages = 0;
			for (int i=0; i<MAX_STAGES; i++) {
				//stages[i] = 0;
				stages[i] = 0; // DEBUG
			}
            selected_player = 1;
		}
	};

    /**
     * @brief
     *
     */
    struct st_checkpoint {
        int map; /**< TODO */
        int x; /**< TODO */
        int y; /**< TODO */
        int map_scroll_x; /**< TODO */
		st_checkpoint() {
			map = 0;
			x = 50;
			y = -1;
			map_scroll_x = 0;
		}
	};

}

#endif // V_2_0_1_H
