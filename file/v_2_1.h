#ifndef V_2_1_H
#define V_2_1_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_4.h"
#include "../defines.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_1 {

/**
 * @brief
 *
 */
struct st_sprite_data {
    bool used;                                                  // if true, the sprite exists /**< TODO */
    unsigned short duration; /**< TODO */
    unsigned short sprite_graphic_pos_x; /**< TODO */
    st_rectangle colision_rect; /**< TODO */
    st_sprite_data() {
        used = false;
        duration = 0;
        sprite_graphic_pos_x = 0;
    }

    st_sprite_data & operator=(st_sprite_data new_value) {
        used = new_value.used;
        duration = new_value.duration;
        sprite_graphic_pos_x = new_value.sprite_graphic_pos_x;
        colision_rect = new_value.colision_rect;
        return *this;
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
    bool is_ghost; /**< TODO */
    short shield_type; /**< TODO */
    short IA_type;                                              // IA types. For custom (edited) ones, use IA_TYPES_COUNT + position (in the array) /**< TODO */
    short fly_flag;                                             // 0 - can't fly, 1 - flyer (...) /**< TODO */
    format_v_2_0_2::st_weakness weakness[9];                    // each npc have weaknesses for each boss weapon, plus the normal weapon /**< TODO */
    char bg_graphic_filename[CHAR_FILENAME_SIZE];               // holds a static background /**< TODO */
    st_position sprites_pos_bg;                                 // holds position of sprites in relation with background /**< TODO */
    bool is_boss;                                               // indicates if this NPC is a boss /**< TODO */
    int gravity_level; /**< TODO */


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
        bg_graphic_filename[0] = '\0';
        gravity_level = 0;
        walk_range = 160;
        fly_flag = 0;
        IA_type = 0;
        is_boss = false;
        speed = 3;
    }

    /**
     * @brief
     *
     * @param old_npc
     * @return file_npc &operator
     */
    file_npc & operator=(format_v_2_0_4::file_npc old_npc) // 2.0.3 to 2.1 conversion
    {
        id = old_npc.id;
        projectile_id[0] = projectile_id[0];
        projectile_id[1] = projectile_id[1];
        sprintf(name, "%s", old_npc.name);
        sprintf(graphic_filename, "%s", old_npc.graphic_filename);
        hp = old_npc.hp;
        direction = old_npc.direction;
        speed = old_npc.speed;
        walk_range = old_npc.walk_range;
        facing = old_npc.facing;
        start_point = old_npc.start_point;
        // @TODO: do not copy, we can read directly from game_data instead of copying
        for (int i=0; i<ANIM_TYPE_COUNT; i++) {
            for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
                sprites[i][j].colision_rect = old_npc.sprites[i][j].colision_rect;
                sprites[i][j].duration = old_npc.sprites[i][j].duration;
                sprites[i][j].sprite_graphic_pos_x = old_npc.sprites[i][j].sprite_graphic_pos_x;
                sprites[i][j].used = old_npc.sprites[i][j].used;
            }
        }
        frame_size = old_npc.frame_size;
        is_ghost = old_npc.can_shoot;
        shield_type = old_npc.shield_type;
        IA_type = old_npc.IA_type;
        fly_flag = old_npc.death_spawn;
        for (int i=0; i<9; i++) {
            weakness[i] = old_npc.weakness[i];
        }
        sprintf(bg_graphic_filename, "%s", old_npc.bg_graphic_filename);
        sprites_pos_bg = old_npc.sprites_pos_bg;
        gravity_level = 0;
        return *this;
    }
};

/**
 * @brief
 *
 */
struct file_ai_action {
    int chance; /**< TODO */
    int action; /**< TODO */
    int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random) /**< TODO */
    int go_to_delay;                                            // delay before going to the next action /**< TODO */
    int extra_parameter;										// left, right, etc /**< TODO */

/**
 * @brief
 *
 */
    file_ai_action()
    {
        chance = 0;
        action = 0;
		go_to = 0;
		go_to_delay = 500;
		extra_parameter = 0;
    }
};

/**
 * @brief
 *
 */
struct file_artificial_inteligence {
    short id; /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    struct file_ai_action states[4]; /**< TODO */
/**
 * @brief
 *
 */
    file_artificial_inteligence() {
        id = -1;
        name[0] = '\0';
    }
};

/**
 * @brief
 *
 */
struct file_object { // DONE - Game
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
    short frame_duration;												// indicates what is the used frame /**< TODO */
    bool animation_auto_start;                                  // false means it will not be animated until started /**< TODO */
    bool animation_reverse;                                     // start animating /**< TODO */
    bool animation_loop;                                        // true = animation will loop, false = animation will run once /**< TODO */
    file_object() {
        id = -1;
        name[0] = '\0';
        graphic_filename[0] = '\0';
        animation_auto_start = true;
		animation_reverse = false;
        animation_loop = true;
		frame_duration = 100;
    }
};

/**
 * @brief
 *
 */
struct file_projectile {
    short id; /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    char graphic_filename[CHAR_FILENAME_SIZE]; /**< TODO */
    st_size size; /**< TODO */
    bool is_destructible;                                   // indicates if the projectile can be shot down, if true, uses HP /**< TODO */
    unsigned short hp;                                      // how much danage needs to be taken to destruct the projectile /**< TODO */
    enum PROJECTILE_TRAJECTORIES trajectory;                // RENAMED from projectile_type in 2.1 /**< TODO */
    unsigned short max_shots;                               // number of maximum simultaneous projectiles, if 0, is infinite /**< TODO */
    unsigned short speed;                                   // NEW IN 2.1 /**< TODO */
    unsigned short damage;                                  // NEW IN 2.1 /**< TODO */
    file_projectile() {
        id = -1;
        name[0] = '\0';
        graphic_filename[0] = '\0';
        is_destructible = false;
        hp = 1;
        size.width = 3;
        size.height = 2;
        trajectory = TRAJECTORY_LINEAR;
        max_shots = 3;
        speed = PROJECTILE_DEFAULT_SPEED;
        damage = PROJECTILE_DEFAULT_DAMAGE;
    }
};


/**
 * @brief
 *
 */
struct file_game {
    float version; /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    format_v_2_1::file_npc game_npcs[GAME_MAX_OBJS]; // 60 enemy types is more than good enought /**< TODO */
    format_v_2_1::file_object objects[GAME_MAX_OBJS]; /**< TODO */
    format_v_2_1::file_projectile projectiles[MAX_FILE_PROJECTILES]; /**< TODO */
    format_v2_0::file_weapon weapons[MAX_WEAPON_N]; // 8 stage-bosses and 3 item-like /**< TODO */
    format_v_2_0_4::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
    format_v_2_0_3::file_stage stages[MAX_STAGES]; /**< TODO */
    format_v_2_1::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
/**
 * @brief
 *
 */
    file_game() {
        /// *** hardcoded parts *** ///
        version = 1.0; // 0.9 means 1.0 beta, 0.92 means beta #2
        sprintf(name, "%s", "Rockbot");
    }
};

}


#endif // V_2_1_H
