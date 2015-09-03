#ifndef V_1_H
#define V_1_H

//#include "defines.h"

namespace format_v1_0 {

	// data that describes an NPC properties
	// TODO: change names to reflect new ones from classnpc.h
    /**
     * @brief
     *
     */
    struct st_file_npc {
        char name[50]; /**< TODO */
        struct st_hit_points hp; /**< TODO */
        short int direction;              // defines how it behavies (kink of AI*) /**< TODO */
        short int speed;               // defines the distances it can see enemies /**< TODO */
        short int walk_range;                 // defines how long from the start point it can go /**< TODO */
        short int facing;                     // defines the side npc is facing before start moving (also used by LINEWALK behavior) /**< TODO */
        struct st_position start_point; /**< TODO */
        char graphic_filename[50];						// graphic file used on it /**< TODO */
		//int filesize;
        int frameset[NPC_FRAME_NUMBER];				// holds information about the type of each frame /**< TODO */
        int frameset_time[NPC_FRAME_NUMBER];		// the number of milisseconds this frame leasts /**< TODO */
        int framesize_w; /**< TODO */
        int framesize_h; /**< TODO */
        int can_shoot; /**< TODO */
        int shield_type; /**< TODO */
        unsigned short int is_boss; /**< TODO */
        int IA_type;										// IA types: no_move, wait, attack, zig-zag (loop), side to side (origin to direction) /**< TODO */
        int projectile_type_id;							// indicates the projectile ID (relation with file_projectile) /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct st_file_map_npc {
        unsigned short int id; /**< TODO */
        struct st_position start_point; /**< TODO */
        unsigned short int direction; /**< TODO */
	};


	#define WEAPON_COUNT SKULLCASTLE4
	// FOR SAVE
    /**
     * @brief
     *
     */
    struct st_items {
        unsigned short int energy_tanks; /**< TODO */
        unsigned short int weapon_tanks; /**< TODO */
        unsigned short int special_tanks; /**< TODO */
        unsigned short int balancer; /**< TODO */
        unsigned short int lifes; /**< TODO */
        unsigned short int bolts; /**< TODO */
        unsigned short int weapons[WEAPON_COUNT]; // 0 position is not used, 5 is for skullcastle (not used too), 10 is for item coil and 11 for item jet /**< TODO */
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


	#define DIALOG_MAX_STAGE 20
	// TODO - use vectors instead of C/like fixed-size arrays
    /**
     * @brief
     *
     */
    struct st_save {
        short int finished_stages; // number of already finished stages, just to simplify calc /**< TODO */
        short int stages[DIALOG_MAX_STAGE]; // 0 still open, 1 finished /**< TODO */
        st_items items; /**< TODO */
		st_save() {
			finished_stages = 0;
			for (int i=0; i<DIALOG_MAX_STAGE; i++) {
				//stages[i] = 0;
				stages[i] = 0; // DEBUG
			}
		}
	};



    /**
     * @brief
     *
*/
    struct st_game_config {
        bool two_players; /**< TODO */
        bool sound_enabled; /**< TODO */
/**
 * @brief
 *
 */
        bool video_fullscreen; /**< TODO */
        short int video_filter; /**< TODO */
        short int selected_player; /**< TODO */
        short int input_mode; /**< TODO */
		st_game_config() {
			two_players = false;
			sound_enabled = true;
			video_fullscreen = false;
			video_filter = 1;
			selected_player = 2;
			input_mode = INPUT_MODE_DOUBLE;
		}
	};


    /**
     * @brief
     *
     */
    struct st_map_link {
        short int stage; /**< TODO */ /**< TODO */
        struct st_position link_pos;				// position in map of origin /**< TODO */
        struct st_position link_dest;				// position in map of destiny /**< TODO */
        short int map_origin;			// number of origin map /**< TODO */
        short int map_dest;			// number of destiny map /**< TODO */
        unsigned short int link_size;			// size of it, in tiles /**< TODO */
        unsigned short int link_type;			// can be TELEPORTER, LINK_VERTICAL OR LINK_HORIZONTAL /**< TODO */
        unsigned short int link_bidi;			// indicates if it is bidirecional /**< TODO */
        unsigned short int is_door;				// if it is a door, it is not bidiand willmake door sound and show animation of door opening/closing /**< TODO */
	};

	// TODO - must remove sprite and next from the file-struct
    /**
     * @brief
     *
     */
    struct st_object { // same as the file object, but also contain pointers to be list and surface for graphics
        char id[4]; // holds the number for the relative object file. eg '003' for 003.obj /**< TODO */
        char name[20]; /**< TODO */
        char filename[50];						// graphic file used on it /**< TODO */
        short int type; /**< TODO */
        int timer;				// used as time for disapearing block /**< TODO */
        int speed;				// used as speed for moving platform /**< TODO */
        int limit;				// used as range (pixels) for moving platform, visible time for disapearing block /**< TODO */
        int direction;			// used to check if moving away from oiginalpoint or moving to it /**< TODO */
        int distance; /**< TODO */
        int framesize_w; /**< TODO */
        int framesize_h; /**< TODO */
        int frame;				// indicates what is the used frame /**< TODO */
        struct st_position start_point; /**< TODO */
        struct st_position position; /**< TODO */
        SDL_Surface *sprite; /**< TODO */
        struct st_object *next; /**< TODO */
		st_object()
		{
			sprite = NULL;
		}
	};

    /**
     * @brief
     *
     */
    struct st_file_object { // logic object. combination of data from the map object information and the object file
        char id[4]; // holds the number for the relative object file. eg '003' for 003.obj /**< TODO */
        struct st_position start_point; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct st_dialog {
        int face; /**< TODO */
        bool top_side; /**< TODO */
        std::string lines[3]; /**< TODO */
		st_dialog(int set_face, bool set_left, std::string line1, std::string line2, std::string line3)
		{
			face = set_face;
			top_side = set_left;
			lines[0] = line1;
            /**
             * @brief
             *
             */
            lines[1] = line2;
			lines[2] = line3;
		}
	};


	/// @TODO: revisar de acordo com novo sistema de colorcycle

    /**
     * @brief
     *
     */
    struct st_colorcycle_color {
        unsigned short int duration; /**< TODO */
        short int r; /**< TODO */
        short int g; /**< TODO */
        short int b; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct st_color {
        short int r; /**< TODO */
        short int g; /**< TODO */
        short int b; /**< TODO */
	};

	#define COLORCYCLE_MAX_STEPS 5
    /**
     * @brief
     *
     */
    struct st_colorcycle_file {
        struct st_color original; /**< TODO */
        struct st_colorcycle_color current_color[COLORCYCLE_MAX_STEPS]; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct st_file_stage {
        char stage_name[50]; /**< TODO */
        char boss_name[50]; /**< TODO */
        char boss_sprites_filename[255]; /**< TODO */
        char boss_face_filename[255]; /**< TODO */
        char bg_music_filename[255]; /**< TODO */
        struct st_colorcycle_file colorcycle[3]; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct map_tile {
        short int locked; /**< TODO */
        struct st_position tile1;                  // position of tile (x and y) in the graphic file (each tile is 16*16) /**< TODO */
        struct st_position tile2;                  // second level tile. this can be used to create more complex backgrounds /**< TODO */
        struct st_position tile3;                  // tird level, to be shown over player /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct file_map_v1 {
        char filename[50];							// used tilesed /**< TODO */
        struct map_tile tiles[MAP_W][MAP_H-2];		// map tiles /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct struct_zone {
        char bg_filename[100];					// background image filename /**< TODO */
        struct st_file_stage stages[8]; /**< TODO */
	};

	#define LINKS_NUMBER 300
    /**
     * @brief
     *
     */
    struct file_game {
        char name[50]; /**< TODO */
        struct st_map_link map_links[LINKS_NUMBER];// links from the map to another one (two for each map is more than enought) /**< TODO */
        struct struct_zone zones[3];			// stage select, castle 1 and castle 2 /**< TODO */
	};


	// data that describes a projectile properties
    /**
     * @brief
     *
     */
    struct st_file_projectile {
        char name[50]; /**< TODO */
        int trajectory_type; /**< TODO */
        int is_destructible; /**< TODO */
        int frame_w; /**< TODO */
        int frame_h; /**< TODO */
        char img_filename[255]; /**< TODO */
        int HP; /**< TODO */
	};


    /**
     * @brief
     *
     */
    struct links_list {
        struct st_map_link link; /**< TODO */
        struct links_list *next; /**< TODO */
	};


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


#define ANIM_N1 2
#define ANIM_N2 ANIM_STATES_N
#define ANIM_N3 10

    /**
     * @brief
     *
     */
    struct anim_state {
        short int move_type; /**< TODO */
        short int move_status[ANIM_N3]; /**< TODO */
        short int move_frame[ANIM_N3]; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct anim_frame {
        SDL_Surface *frame; /**< TODO */
        unsigned short int duration; /**< TODO */
        unsigned int init; /**< TODO */
	};



    /**
     * @brief
     *
     */
    struct sprite_struct {
        short int x; /**< TODO */
        short int y; /**< TODO */
        short int w; /**< TODO */
        short int h; /**< TODO */
        short int frame_pos[2];							// indicates what is the animation type and frame position /**< TODO */
        short int frame_timer;							// timer is used to control the animation speed /**< TODO */
        unsigned long int frame_animation[2];			// controls where in the animation process the sprite is. /**< TODO */
														// First part indicates the status and second the position
														// (for example in jump status 0 is nothing, 1 is going up and position raises until reaches limit, then status changes to 2)
		// new animation variables
        short int direction; /**< TODO */
        short int anim_type; /**< TODO */
        short int anim_pos; /**< TODO */
        long int move_timer; /**< TODO */
        struct anim_state anim_status; /**< TODO */
        struct anim_frame sprite_frames[ANIM_N1][ANIM_N2][ANIM_N3]; /**< TODO */
        short int visible; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct npc_static_data_list {
        struct st_file_npc npc_data; /**< TODO */
        struct sprite_struct *sprite_list; /**< TODO */
        struct npc_static_data_list *next; /**< TODO */
	};

    /**
     * @brief
     *
     */
    struct list_map_npc {
        short int id; /**< TODO */
        struct st_position start_point; /**< TODO */
        unsigned short int direction; /**< TODO */
        struct list_map_npc *next; /**< TODO */
	};
}

#endif // V_1_H
