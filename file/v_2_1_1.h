#ifndef V_2_1_1_H
#define V_2_1_1_H

#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_4.h"
#include "v_2_1.h"
#include "../defines.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_1_1 {

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
    float touch_damage_reduction; /**< TODO */
    format_v_2_1::st_sprite_data sprites[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]; // changed in 2.0.4 /**< TODO */
    format_v2_0::file_weapon_colors weapon_colors[MAX_WEAPON_N]; /**< TODO */
    st_color color_keys[3]; /**< TODO */
    // habilities part
    bool have_shield; /**< TODO */
    unsigned short max_shots;		// number of maximum simultaneous projectiles /**< TODO */
    unsigned short simultaneous_shots; // number of projectiles shot at one button press /**< TODO */
    bool double_jump; /**< TODO */
    bool can_slide; // if false, dashes instead of sliding /**< TODO */
    bool can_charge_shot; /**< TODO */
    // NEW PROPERTIES
    short int full_charged_projectile_id; /**< TODO */


/**
 * @brief
 *
 */
    file_player() {
        id = -1;
        name[0] = '\0';
        graphic_filename[0] = '\0';
        face_filename[0] = '\0';
        HP = 0;
        move_speed = 2;
        jump_initial_speed = 1;
        jump_gravity = 0.6;
        touch_damage_reduction = 0;
        max_shots = 1;
        simultaneous_shots = 1;
        double_jump = false;
        have_shield = true;
        can_slide = false;
        can_charge_shot = true;
        full_charged_projectile_id = 0;
    }

    /**
     * @brief
     *
     * @param old_player
     * @return file_player &operator
     */
    file_player & operator=(format_v_2_0_4::file_player old_player) // 2.0.3 to 2.1 conversion
    {
        id = old_player.id;
        sprintf(name, "%s", old_player.name);
        sprintf(graphic_filename, "%s", old_player.graphic_filename);
        sprintf(face_filename, "%s", old_player.face_filename);
        HP = old_player.HP;
        sprite_size = old_player.sprite_size;
        sprite_hit_area = old_player.sprite_hit_area;
        move_speed = old_player.move_speed;
        jump_initial_speed = old_player.jump_initial_speed;
        jump_gravity = old_player.jump_gravity;
        touch_damage_reduction = old_player.fall_gravity;
        for (int i=0; i<ANIM_TYPE_COUNT; i++) {
            for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
                sprites[i][j].colision_rect = old_player.sprites[i][j].colision_rect;
                sprites[i][j].duration = old_player.sprites[i][j].duration;
                sprites[i][j].sprite_graphic_pos_x = old_player.sprites[i][j].sprite_graphic_pos_x;
                sprites[i][j].used = old_player.sprites[i][j].used;
            }
        }
        for (int i=0; i<MAX_WEAPON_N; i++) {
            weapon_colors[i] = old_player.weapon_colors[i];
        }
        for (int i=0; i<3; i++) {
            color_keys[i] = old_player.color_keys[i];
        }
        have_shield = old_player.have_shield;
        max_shots = old_player.max_shots;
        simultaneous_shots = old_player.simultaneous_shots;
        double_jump = old_player.double_jump;
        can_slide = old_player.can_slide;
        can_charge_shot = old_player.can_charge_shot;
        full_charged_projectile_id = 0;
        return *this;
    }

};

/**
 * @brief
 *
 */
struct file_artificial_inteligence {
    short id; /**< TODO */
    char name[CHAR_NAME_SIZE]; /**< TODO */
    struct format_v_2_1::file_ai_action states[AI_MAX_STATES]; /**< TODO */
	file_artificial_inteligence() {
		id = -1;
		name[0] = '\0';
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
    format_v_2_1_1::file_player players[MAX_FILE_PLAYERS]; // up to 5 different players the user can select from /**< TODO */
    format_v_2_1_1::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
    // NEW PROPERTIES
    short int semi_charged_projectile_id; // common to all players /**< TODO */
    short int player_items[2]; // common to all players -> to be used in add_coil_object and add_jet_object /**< TODO */
    char stage_face_filename[MAX_STAGES][20]; /**< TODO */



/**
 * @brief
 *
 */
    file_game() {
        /// *** hardcoded parts *** ///
        version = 1.0; // 0.9 means 1.0 beta, 0.92 means beta #2
        sprintf(name, "%s", "Rockbot");
        semi_charged_projectile_id = 0;
        player_items[0] = 0;
        player_items[1] = 0;
        for (int i=0; i<MAX_STAGES; i++) {
            stage_face_filename[i][0] = '\0';
        }
    }



};

/**
 * @brief
 *
 */
struct file_stages {
    format_v_2_0_3::file_stage stages[MAX_STAGES]; /**< TODO */
};


/**
 * @brief
 *
 */
struct st_key_config {
    short key_type; // defines.h -> e_input_types (type of input, joybutton, keyboard, joyhat, joyaxis..) /**< TODO */
    int key_number; /**< TODO */
/**
 * @brief
 *
 */
    st_key_config() {
        key_type = -1;
        key_number = -1;
    }
};


struct st_key_configuration {
    short type; // keyboard, joybutton, joyhat or joyaxis, default -1 indicates is not used
    short key_map[KEY_MAP_N];
    st_key_configuration() {
        type = -1;
        for (int i=0; i<KEY_MAP_N; i++) {
            key_map[i] = 0;
        }
    }
};

/**
 * @brief
 *
 */
struct st_game_config {
    bool sound_enabled; /**< TODO */
    bool video_fullscreen; /**< TODO */
    short int video_filter; /**< TODO */
    E_INPUT_MODES input_mode; /**< TODO */

/**
 * @brief
 *
 */
    st_game_config() {
        sound_enabled = true;
        video_fullscreen = false;
        video_filter = 1;
        input_mode = INPUT_MODE_DOUBLE;
    }
};

}



#endif // V_2_1_1_H
