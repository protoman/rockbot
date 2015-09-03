#ifndef V_2_0_0_H
#define V_2_0_0_H

#include "../file/format/st_common.h"

// new file format, a single file structured into limited-size arrays;
/// @NOTE: an id negative (-1) means the struct is empty/deleted
namespace format_v2_0 {



    /**
     * @brief
     *
     */
    enum ANIM_STATES { ANIM_STAND, ANIM_WALK, ANIM_JUMP, ANIM_ATTACK, ANIM_TELEPORT, ANIM_JUMP_ATTACK, ANIM_WALK_ATTACK,
					   ANIM_HIT, ANIM_STAIRS, ANIM_STAIRS_SEMI, ANIM_STAIRS_ATTACK, ANIM_STAIRS_MOVE, ANIM_TROW, ANIM_TURN,
					   ANIM_MOVE_SEMI, // like turn, the start of the movement
					   ANIM_WALK_AIR, // for those that can walk on air
					   ANIM_INTRO, // for bosses
					   ANIM_SPECIAL_ATTACK, // for bosses, charging weapon, etc
					   ANIM_STATES_N }; // this must be always the last one to be used as counter

    /**
     * @brief
     *
     */
    struct st_sprite_info { // stores information about each animation sprite
        int id; /**< TODO */
        unsigned short duration; /**< TODO */
        short state; /**< TODO */
        short sprite_pos_x; /**< TODO */
		st_sprite_info() {
			id = -1;
			duration = 0;
			state = 0;
			sprite_pos_x = 0;
		}
	};

	// ----------------------- START GAME SECTION ----------------------- //

    /**
     * @brief
     *
     */
    struct file_ai_group {
/**
 * @brief
 *
 */
        short state; //enum IA_STATES state; /**< TODO */
        enum IA_CONDITIONALS conditional; // controls if this state will be executed or not, kinda like an "IF" /**< TODO */
        int parameter1; /**< TODO */
        int parameter2; /**< TODO */
		file_ai_group() {
			state = IA_STATE_SLEEP;
			conditional = IA_CONDITION_NO_CONDITION;
			parameter1 = -1;
			parameter2 = -1;
		}
	};

    /**
     * @brief
     *
     */
    struct file_artificial_inteligence {
        short id; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        struct file_ai_group states[AI_MAX_STATES]; /**< TODO */
		file_artificial_inteligence() {
			id = -1;
			name[0] = '\0';
		}
	};


    /**
     * @brief
     *
     */
    struct file_npc { // DONE - Game /**< TODO */
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
        st_sprite_info frames[NPC_FRAME_NUMBER];		// holds information about the type of each frame /**< TODO */
        st_size frame_size; /**< TODO */
        bool can_shoot; /**< TODO */
        short shield_type; /**< TODO */
        short IA_type;										// IA types: no_move, wait, attack, zig-zag (loop), side to side (origin to direction) /**< TODO */
        short death_spawn;									// if different than -1, the NPC will spawn the given npc ID after killed /**< TODO */
		file_npc() {
			id = -1;
			name[0] = '\0';
			graphic_filename[0] = '\0';
			projectile_id[0] = -1;
			projectile_id[1] = -1;
			death_spawn = -1;
            direction = 0;
            speed = 0;
            walk_range = 0;
            facing = 0;
            can_shoot = false;
            shield_type = 0;
            IA_type = 0;
		}
	};

    /**
     * @brief
     *
     */
    struct file_object { // DONE - Game /**< TODO */
        short id; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        short type;													///@ TODO: use enum /**< TODO */
        int timer;													// used as time for disapearing block /**< TODO */
        short speed;												// used as speed for moving platform /**< TODO */
        short limit;												// used as range (pixels) for moving platform, visible time for disapearing block /**< TODO */
        short direction;											// used to check if moving away from oiginalpoint or moving to it /**< TODO */
        short distance; /**< TODO */
        st_size size; /**< TODO */
        short frame;												// indicates what is the used frame /**< TODO */
		file_object() {
			id = -1;
			name[0] = '\0';
			graphic_filename[0] = '\0';
            type = 0;
            timer = 0;
            speed = 0;
            limit = 0;
            direction = 0;
            distance = 0;
            frame = 0;
		}
	};



    /**
     * @brief
     *
     */
    struct file_projectile { // DONE -> Game
        short id; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        st_size size; /**< TODO */
/**
 * @brief
 *
 */
        bool is_destructible; // indicates if the projectile can be shot down, if true, uses HP /**< TODO */
        unsigned short hp; /**< TODO */
        enum PROJECTILE_TRAJECTORIES projectile_type; /**< TODO */
        unsigned short max_shots; // number of maximum simultaneous projectiles, if 0, is infinite /**< TODO */
		file_projectile() {
			id = -1;
			name[0] = '\0';
			graphic_filename[0] = '\0';
			is_destructible = false;
			hp = 1;
			size.width = 3;
            size.height = 2;
			projectile_type = TRAJECTORY_LINEAR;
            max_shots = 3;
		}
	};


    /**
     * @brief
     *
     */
    struct file_weapon_colors { // DONE -> Player
/**
 * @brief
 *
 */
        short id_weapon; /**< TODO */
        st_color color1; /**< TODO */
        st_color color2; /**< TODO */
        st_color color3; /**< TODO */
		file_weapon_colors() {
			id_weapon = -1;
		}
	};

    /**
     * @brief
     *
     */
    struct file_weapon { // DONE - Game
/**
 * @brief
 *
 */
        short id; /**< TODO */
        unsigned short id_projectile; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        unsigned int damage; /**< TODO */
		file_weapon() {
			id = -1;
			name[0] = '\0';
			damage = 1;
            id_projectile = 0;
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
        file_weapon_colors weapon_colors[MAX_WEAPON_N]; /**< TODO */
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
            HP = 0;
            move_speed = 3;
            jump_initial_speed = 3;
            jump_gravity = 1;
            fall_gravity = 1;
            max_shots = 3;
            simultaneous_shots = 3;
		}
	};


	// ----------------------- START MAP SECTION ----------------------- //



    /**
     * @brief
     *
     */
    struct file_map_npc {
        short id_npc; /**< TODO */
        struct st_position start_point; /**< TODO */
        unsigned short direction; /**< TODO */
		file_map_npc() {
			id_npc = -1;
            direction = 0;
		}
	};

    /**
     * @brief
     *
     */
    struct file_map_object {
        /**
         * @brief
         *
         */
        short id_object; /**< TODO */
        struct st_position start_point; /**< TODO */
        unsigned short direction; /**< TODO */
		file_map_object() {
			id_object = -1;
            direction = 0;
		}
	};

    /**
     * @brief
    */
/**
* * @brief
* *
* */
    struct map_tile {
        short id; /**< TODO */
/**
 * @brief
 *
 */
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
        char bg1_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        char bg2_filename[CHAR_FILENAME_SIZE]; /**< TODO */
/**
 * @brief
 *
 */
        st_color bg_color; /**< TODO */
        map_tile tiles[MAP_W][MAP_H-2];		// map tiles /**< TODO */
        file_map_npc map_npcs[MAX_MAP_NPC_N]; /**< TODO */
        file_map_object map_objects[MAX_MAP_NPC_N]; /**< TODO */
		file_map() {
			id = -1;
			bg1_filename[0] = '\0';
			bg2_filename[0] = '\0';
		}
	};


	// ----------------------- END MAP SECTION ----------------------- //



	// ----------------------- START STAGE SECTION ----------------------- //


	#define COLORCYCLE_MAX_ITEMS 10
    /**
     * @brief
     *
     */
    enum COLOR_KEYS {
		COLOR_KEY_GREEN,
		COLOR_KEY_PURPLE,
		COLOR_KEY_CYAN,
		COLOR_KEY_LENGTH
	};
    /**
     * @brief
     *
     */
    struct file_colorcycle { // DONE - Stage
        short id; /**< TODO */
        st_color color[COLORCYCLE_MAX_ITEMS][COLOR_KEY_LENGTH]; /**< TODO */
        short duration[COLORCYCLE_MAX_ITEMS][COLOR_KEY_LENGTH]; /**< TODO */
		file_colorcycle() {
            id = -1;
		}
	};


    /**
     * @brief
     *
     */
    struct file_boss { // DONE - Stage
/**
 * @brief
 *
 */
        unsigned short id_npc; /**< TODO */
        unsigned short id_weapon; /**< TODO */
        char name[20]; /**< TODO */
        char face_graphics_filename[20]; /**< TODO */
		file_boss() {
			id_npc = -1;
			id_weapon = -1;
			name[0] = '\0';
			face_graphics_filename[0] = '\0';
		}
	};


    /**
     * @brief
     *
     */
    struct file_dialog { // DONE - Stage x2
        unsigned short id; /**< TODO */
        char face_graphics_filename[20]; /**< TODO */ /**< TODO */
        bool top_side; /**< TODO */
        char line1[3][DIALOG_LINE_LIMIT]; /**< TODO */
        char answer1[MAX_FILE_PLAYERS][3][DIALOG_LINE_LIMIT]; /**< TODO */
        char line2[3][DIALOG_LINE_LIMIT]; /**< TODO */
        char answer2[MAX_FILE_PLAYERS][3][DIALOG_LINE_LIMIT]; /**< TODO */

		file_dialog() {
			id = -1;
			top_side = true;
			face_graphics_filename[0] = '\0';
			for (int i=0; i<MAX_FILE_PLAYERS; i++) {
				for (int j=0; j<3; j++) {
					answer1[i][j][0] = '\0';
					answer2[i][j][0] = '\0';
				}
			}
			for (int j=0; j<3; j++) {
				line1[j][0] = '\0';
				line2[j][0] = '\0';
			}
		}
	};

    /**
     * @brief
     *
     */
    enum LINK_TYPE {
		HORIZONTAL,
		VERTICAL,
		TELEPORTER,
		LINK_TYPE_COUNT
	};
    /**
     * @brief
     *
     */
    struct file_link { // DONE - Stage
        short id_map_origin; /**< TODO */
        short id_map_destiny; /**< TODO */
        st_position pos_origin; /**< TODO */
        st_position pos_destiny; /**< TODO */
        unsigned int size; /**< TODO */
        bool bidirecional; /**< TODO */
        bool is_door; /**< TODO */
        short type; /// @TODO: use enum LINK_TYPE when conversion is completed /**< TODO */
		file_link() {
			id_map_origin = -1;
			id_map_destiny = -1;
            size = 1;
            bidirecional = false;
            is_door = false;
            type = 0;
		}
	};


    /**
     * @brief
     *
     */
    struct file_stage {
        short id; /**< TODO */ /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        char bgmusic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
        file_map maps[STAGE_MAX_MAPS]; /**< TODO */
        file_link links[STAGE_MAX_LINKS]; /**< TODO */
        file_boss boss; /**< TODO */
        file_dialog intro_dialog; /**< TODO */
        file_dialog boss_dialog; /**< TODO */
        file_colorcycle colorcycle; /**< TODO */
		file_stage() {
			id = -1;
			boss = file_boss();
			name[0] = '\0';
			bgmusic_filename[0] = '\0';
		}
	};


	// ----------------------- END STAGE SECTION ----------------------- //

    /**
     * @brief
     *
     */
    struct file_game {
        int version; /**< TODO */
        char name[CHAR_NAME_SIZE]; /**< TODO */
        file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */
        file_object objects[GAME_MAX_OBJS]; /**< TODO */
        file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */ /**< TODO */
        file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
        file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
        file_stage stages[MAX_STAGES]; /**< TODO */
        file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
		file_game() {
			/// *** hardcoded parts *** ///
			// game
			version = 1;
			sprintf(name, "%s", "Rockbot");
		}
	};
	// ----------------------- END GAME SECTION ----------------------- //




}


#endif // V_2_0_0_H
