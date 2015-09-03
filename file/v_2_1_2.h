#ifndef V_2_1_2_H
#define V_2_1_2_H


#include <fstream>
#include <iostream>
#include "v_2_0_0.h"
#include "v_2_0_4.h"
#include "v_2_1.h"
#include "v_2_1_1.h"
#include "../defines.h"

extern std::string FILEPATH; /**< TODO */

namespace format_v_2_1_2 {


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

struct reaction {
    int action; /**< TODO */
    int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random) /**< TODO */
    int go_to_delay;                                            // delay before going to the next action /**< TODO */
    int extra_parameter;										// left, right, etc /**< TODO */
    reaction() {
        action = -1;
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
    struct format_v_2_1::file_ai_action states[AI_MAX_STATES]; /**< TODO */
    struct reaction reactions[MAX_AI_REACTIONS]; // reactions near, hit and dead

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
    format_v_2_1_2::file_artificial_inteligence ai_types[MAX_AI_TYPES]; /**< TODO */
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
        version = 1.01;
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

struct st_game_config {
    bool sound_enabled; /**< TODO */
    bool video_fullscreen; /**< TODO */
    short int video_filter; /**< TODO */
    E_INPUT_TYPES input_type; // if keyboard or joystick
    E_INPUT_MODES input_mode; // inf directional is joypad-digital, analog sick or hat
    E_PLATFORM platform;      // if changed, must reset config to default
    int keys_codes[BTN_COUNT]; // number indicator for the keyboard-keys
    int button_codes[BTN_COUNT]; // number indicator for the keyboard-keys

    void set_default_keys() {
#ifdef PLAYSTATION2
        input_type = INPUT_TYPE_JOYSTICK;
        input_mode = INPUT_MODE_DIGITAL;
        for (int i=0; i<BTN_COUNT; i++) {
            keys_codes[i] = -1;
        }
        /*
        button_codes[BTN_SHIELD] = 3;
        button_codes[BTN_DASH] = 2;
        button_codes[BTN_JUMP] = 1;
        button_codes[BTN_ATTACK] = 0;
        */
        button_codes[BTN_SHIELD] = 0;
        button_codes[BTN_DASH] = 3;
        button_codes[BTN_JUMP] = 2;
        button_codes[BTN_ATTACK] = 1;

        button_codes[BTN_L] = 6;
        button_codes[BTN_R] = 7;
        button_codes[BTN_QUIT] = -1;
        button_codes[BTN_START] = 5;
        button_codes[BTN_LEFT] = -1; // uses default axis
        button_codes[BTN_RIGHT] = -1; // uses default axis
        button_codes[BTN_UP] = -1; // uses default axis
        button_codes[BTN_DOWN] = -1; // uses default axis
        std::cout << "v_2_1_2 - input_mode: " << input_mode << ", BTN_JUMP: " << button_codes[BTN_JUMP] << std::endl;
#elif PSP
        input_type = INPUT_TYPE_JOYSTICK;
        input_mode = INPUT_MODE_DIGITAL;
        for (int i=0; i<BTN_COUNT; i++) {
            keys_codes[i] = -1;
        }
        button_codes[BTN_SHIELD] = 3;
        button_codes[BTN_DASH] = 0;
        button_codes[BTN_JUMP] = 1;
        button_codes[BTN_ATTACK] = 2;
        button_codes[BTN_L] = 4;
        button_codes[BTN_R] = 5;
        button_codes[BTN_QUIT] = -1;
        button_codes[BTN_START] = 11;
        button_codes[BTN_LEFT] = 7; // PSP maps axis as buttons. Will use thos unless you pick analog mode
        button_codes[BTN_RIGHT] = 9; // PSP maps axis as buttons. Will use thos unless you pick analog mode
        button_codes[BTN_UP] = 8; // PSP maps axis as buttons. Will use thos unless you pick analog mode
        button_codes[BTN_DOWN] = 6; // PSP maps axis as buttons. Will use thos unless you pick analog mode
#elif WII
        input_type = INPUT_TYPE_JOYSTICK;
        input_mode = INPUT_MODE_DIGITAL;
        for (int i=0; i<BTN_COUNT; i++) {
            keys_codes[i] = -1;
        }
        button_codes[BTN_SHIELD] = 1;
        button_codes[BTN_DASH] = 0;
        button_codes[BTN_JUMP] = 3;
        button_codes[BTN_ATTACK] = 2;
        button_codes[BTN_L] = 4;
        button_codes[BTN_R] = 5;
        button_codes[BTN_QUIT] = -1;
        button_codes[BTN_START] = 6;
#elif DINGUX
        input_type = INPUT_TYPE_DOUBLE;
        input_mode = INPUT_MODE_DIGITAL;
        for (int i=0; i<BTN_COUNT; i++) {
            keys_codes[i] = -1;
        }
        keys_codes[BTN_SHIELD] = SDLK_LSHIFT;
        keys_codes[BTN_DASH] = SDLK_SPACE;
        keys_codes[BTN_JUMP] = SDLK_LCTRL;
        keys_codes[BTN_ATTACK] = SDLK_LALT;
        keys_codes[BTN_L] = SDLK_TAB;
        keys_codes[BTN_R] = SDLK_BACKSPACE;
        keys_codes[BTN_QUIT] = SDLK_ESCAPE;
        keys_codes[BTN_START] = SDLK_RETURN;
        keys_codes[BTN_LEFT] = SDLK_LEFT;
        keys_codes[BTN_RIGHT] = SDLK_RIGHT;
        keys_codes[BTN_UP] = SDLK_UP;
        keys_codes[BTN_DOWN] = SDLK_DOWN;
#elif OPEN_PANDORA
        input_type = INPUT_TYPE_KEYBOARD;
        input_mode = INPUT_MODE_DIGITAL;
        for (int i=0; i<BTN_COUNT; i++) {
            keys_codes[i] = -1;
        }
        keys_codes[BTN_SHIELD] = SDLK_PAGEUP;
        keys_codes[BTN_DASH] = SDLK_HOME;
        button_codes[BTN_JUMP] = SDLK_PAGEDOWN;
        keys_codes[BTN_ATTACK] = SDLK_END;
        keys_codes[BTN_L] = SDLK_RSHIFT;
        keys_codes[BTN_R] = SDLK_RCTRL;
        keys_codes[BTN_QUIT] = SDLK_LCTRL;
        keys_codes[BTN_START] = SDLK_LALT;
        keys_codes[BTN_LEFT] = SDLK_LEFT;
        keys_codes[BTN_RIGHT] = SDLK_RIGHT;
        keys_codes[BTN_UP] = SDLK_UP;
        keys_codes[BTN_DOWN] = SDLK_DOWN;
#else
        for (int i=0; i<BTN_COUNT; i++) {
            button_codes[i] = -1;
        }
        input_type = INPUT_TYPE_DOUBLE;
        button_codes[BTN_ATTACK] = 2;
        button_codes[BTN_JUMP] = 1;
        button_codes[BTN_DASH] = 0;
        button_codes[BTN_SHIELD] = 3;
        button_codes[BTN_L] = 6;
        button_codes[BTN_R] = 7;
        button_codes[BTN_QUIT] = 8;
        button_codes[BTN_START] = 9;
        keys_codes[BTN_ATTACK] = SDLK_a;
        keys_codes[BTN_JUMP] = SDLK_x;
        keys_codes[BTN_DASH] = SDLK_c;
        keys_codes[BTN_SHIELD] = SDLK_z;
        keys_codes[BTN_L] = SDLK_q;
        keys_codes[BTN_R] = SDLK_w;
        keys_codes[BTN_QUIT] = SDLK_ESCAPE;
        keys_codes[BTN_START] = SDLK_RETURN;
        keys_codes[BTN_LEFT] = SDLK_LEFT;
        keys_codes[BTN_RIGHT] = SDLK_RIGHT;
        keys_codes[BTN_UP] = SDLK_UP;
        keys_codes[BTN_DOWN] = SDLK_DOWN;
#endif
    }

    E_PLATFORM get_current_platform() const {
#ifdef DINGUX
        return PLATFORM_DINGUX;
#elif LINUX
        return PLATFORM_LINUX;
#elif OPEN_PANDORA
        return PLATFORM_PANDORA;
#elif PLAYSTATION2
        return PLATFORM_PS2;
#elif PSP
        return PLATFORM_PSP;
#elif WII
        return PLATFORM_WII;
#else
        return PLATFORM_WINDOWS;
#endif
    }

    void reset() {
        sound_enabled = true;
        video_fullscreen = false;
        video_filter = VIDEO_FILTER_NOSCALE;
        // SET KEYS/TYPES - triangle=shield; circle=dash; x=jump; square=attack; start=start; select=quit
        set_default_keys();
        platform = get_current_platform();
    }


    st_game_config() {
        reset();
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
    // items that can be bought on shop
    bool exit;
    unsigned short int shock_guard; // do not move back when hit. consumed after finished/game-over the stage
    unsigned short int half_damage; // takes only half the damage (if > 1). consumed after finished/game-over the stage
    unsigned short int spike_guard;
    bool speed_up;                  // faster movement
    bool hyper_jump;                // bigger jump
    bool energy_saver;              // weapons consume half the wpn-energy
    bool speed_shot;                // normal shots become faster (can shoot more often)
    bool power_shot;                // charged shots causes more damage

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
        // shop items
        exit = false;
        shock_guard = 0;
        half_damage = 0;
        speed_up = false;
        hyper_jump = false;
        energy_saver = false;
        speed_shot = false;
        power_shot = false;
        spike_guard = 0;
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
    short int difficulty;       // 0 - easy, 1 - normal, 2 - hard
    st_save() {
        finished_stages = 0;
        for (int i=0; i<MAX_STAGES; i++) {
            //stages[i] = 0;
            stages[i] = 0; // DEBUG
        }
        selected_player = 1;
        difficulty = 1;
    }
};

struct file_stage {
    // WARNING - do not add any data out of order or that will break reading methods. Add new fields on bottom - WARNING //
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
        //printf("file_stage.constructor");
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
struct file_stages {
    format_v_2_1_2::file_stage stages[MAX_STAGES]; /**< TODO */
};


}





#endif // V_2_1_2_H
