#ifndef V_2_0_4_H
#define V_2_0_4_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_3.h"
#include "../defines.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_0_4 {

/**
 * @brief
 *
 */
struct st_sprite_data {
    bool used;                                                  // if true, the sprite exists /**< TODO */
    unsigned short duration; /**< TODO */
    unsigned short sprite_graphic_pos_x; /**< TODO */
    st_rectangle colision_rect; /**< TODO */

/**
 * @brief
 *
 */
    st_sprite_data() {
        used = false;
        duration = 0;
        sprite_graphic_pos_x = 0;
    }
};

/**
 * @brief
 *
 */
struct file_npc { // DONE - Game
    short id;                                                   // serial number /**< TODO */
    //unsigned int projectile_id;                               // indicates the projectile ID (relation with file_projectile)
    short projectile_id[2];                                     // indicate the id of an attack the NCP can user /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
    struct st_hit_points hp; /**< TODO */
    short direction;                                            // defines how it behavies (kink of AI*) /**< TODO */
    short speed;                                                // defines the distances it can see enemies /**< TODO */
    short walk_range;                                           // defines how long from the start point it can go /**< TODO */
    short facing;                                               // defines the side npc is facing before start moving (also used by LINEWALK behavior) /**< TODO */
    struct st_position start_point; /**< TODO */
    st_sprite_data sprites[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]; // changed in 2.0.4 /**< TODO */
    st_size frame_size; /**< TODO */
    bool can_shoot; /**< TODO */
    short shield_type; /**< TODO */
    short IA_type;                                              // IA types: no_move, wait, attack, zig-zag (loop), side to side (origin to direction) /**< TODO */
    short death_spawn;                                          // if different than -1, the NPC will spawn the given npc ID after killed /**< TODO */
    format_v_2_0_2::st_weakness weakness[9];                    // each npc have weaknesses for each boss weapon, plus the normal weapon /**< TODO */
    char bg_graphic_filename[CHAR_FILENAME_SIZE];               // holds a static background /**< TODO */
    st_position sprites_pos_bg;                                 // holds position of sprites in relation with background /**< TODO */


/**
 * @brief
 *
 */
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
    short id; /**< TODO */
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
    st_sprite_data sprites[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]; // changed in 2.0.4 /**< TODO */
    format_v2_0::file_weapon_colors weapon_colors[MAX_WEAPON_N]; /**< TODO */
    st_color color_keys[3]; /**< TODO */
    // habilities part
    bool have_shield; /**< TODO */
    unsigned short max_shots;		// number of maximum simultaneous projectiles /**< TODO */
    unsigned short simultaneous_shots; // number of projectiles shot at one button press /**< TODO */
    bool double_jump; /**< TODO */
    bool can_slide; // is false, dashes instead of sliding /**< TODO */
    bool can_charge_shot; /**< TODO */

/**
 * @brief
 *
 */
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
struct file_game {
    float version; /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    format_v_2_0_4::file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */
    format_v2_0::file_object objects[GAME_MAX_OBJS]; /**< TODO */
    format_v2_0::file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */
    format_v2_0::file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
    format_v_2_0_4::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
    format_v_2_0_3::file_stage stages[MAX_STAGES]; /**< TODO */
    format_v2_0::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
/**
 * @brief
 *
 */
    file_game() {
        /// *** hardcoded parts *** ///
        version = 0.93; // 0.9 means 1.0 beta, 0.92 means beta #2
        sprintf(name, "%s", "Rockbot");
    }
};

}


#endif // V_2_0_4_H
