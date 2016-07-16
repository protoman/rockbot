#ifndef FILE_CONFIG_301_H
#define FILE_CONFIG_301_H


namespace format_v4 {




    struct st_game_config {
        bool sound_enabled;
        bool video_fullscreen;
        Sint8 video_filter;
        E_INPUT_TYPES input_type;               // if keyboard or joystick
        E_INPUT_MODES input_mode;               // inf directional is joypad-digital, analog sick or hat
        E_PLATFORM platform;                    // if changed, must reset config to default
        int keys_codes[BTN_COUNT];              // number indicator for the keyboard-keys
        int button_codes[BTN_COUNT];            // number indicator for the keyboard-keys
        bool game_finished;                     // stores if game was finished, so we can show more options to player
        Uint8 selected_input_device;
        char selected_language[2];
        bool turbo_mode;
        Uint8 volume_sfx;
        Uint8 volume_music;
        bool android_use_play_services;         // for android to use cloud save, trophies, etc
        bool android_touch_controls_hide;       // define if touch controls must be hidden
        Uint8 android_touch_controls_size;      // for android, size of the on-screen controls 0 = small, 1 = normal, 2 = big
        Sint8 wii_joystick_type;                // for wii to define between wiimote, classic, gamecube, etc


        void get_default_keys(int keys_codes_copy[BTN_COUNT]) {
#ifdef PLAYSTATION2
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
#elif PSP
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
#elif WII
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
#elif DINGUX
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
#elif OPEN_PANDORA
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
            keys_codes_copy[BTN_SHIELD] = SDLK_PAGEUP;
            keys_codes_copy[BTN_DASH] = SDLK_HOME;
            keys_codes_copy[BTN_ATTACK] = SDLK_END;
            keys_codes_copy[BTN_L] = SDLK_RSHIFT;
            keys_codes_copy[BTN_R] = SDLK_RCTRL;
            keys_codes_copy[BTN_QUIT] = SDLK_LCTRL;
            keys_codes_copy[BTN_START] = SDLK_LALT;
            keys_codes_copy[BTN_LEFT] = SDLK_LEFT;
            keys_codes_copy[BTN_RIGHT] = SDLK_RIGHT;
            keys_codes_copy[BTN_UP] = SDLK_UP;
            keys_codes_copy[BTN_DOWN] = SDLK_DOWN;
#else
            keys_codes_copy[BTN_ATTACK] = SDLK_a;
            keys_codes_copy[BTN_JUMP] = SDLK_x;
            keys_codes_copy[BTN_DASH] = SDLK_c;
            keys_codes_copy[BTN_SHIELD] = SDLK_z;
            keys_codes_copy[BTN_L] = SDLK_q;
            keys_codes_copy[BTN_R] = SDLK_w;
            keys_codes_copy[BTN_QUIT] = SDLK_ESCAPE;
            keys_codes_copy[BTN_START] = SDLK_RETURN;
            keys_codes_copy[BTN_LEFT] = SDLK_LEFT;
            keys_codes_copy[BTN_RIGHT] = SDLK_RIGHT;
            keys_codes_copy[BTN_UP] = SDLK_UP;
            keys_codes_copy[BTN_DOWN] = SDLK_DOWN;
#endif
        }

        void get_default_buttons(int button_codes_copy[BTN_COUNT]) {
#ifdef PLAYSTATION2
            button_codes_copy[BTN_SHIELD] = 0;
            button_codes_copy[BTN_DASH] = 3;
            button_codes_copy[BTN_JUMP] = 2;
            button_codes_copy[BTN_ATTACK] = 1;
            button_codes_copy[BTN_L] = 6;
            button_codes_copy[BTN_R] = 7;
            button_codes_copy[BTN_QUIT] = -1;
            button_codes_copy[BTN_START] = 5;
            button_codes_copy[BTN_LEFT] = -1; // uses default axis
            button_codes_copy[BTN_RIGHT] = -1; // uses default axis
            button_codes_copy[BTN_UP] = -1; // uses default axis
            button_codes_copy[BTN_DOWN] = -1; // uses default axis
#elif PSP
            button_codes_copy[BTN_SHIELD] = 3;
            button_codes_copy[BTN_DASH] = 0;
            button_codes_copy[BTN_JUMP] = 1;
            button_codes_copy[BTN_ATTACK] = 2;
            button_codes_copy[BTN_L] = 4;
            button_codes_copy[BTN_R] = 5;
            button_codes_copy[BTN_QUIT] = -1;
            button_codes_copy[BTN_START] = 11;
            button_codes_copy[BTN_LEFT] = 7; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_RIGHT] = 9; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_UP] = 8; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_DOWN] = 6; // PSP maps axis as buttons. Will use thos unless you pick analog mode
#elif WII
            button_codes_copy[BTN_SHIELD] = 1;
            button_codes_copy[BTN_DASH] = 0;
            button_codes_copy[BTN_JUMP] = 3;
            button_codes_copy[BTN_ATTACK] = 2;
            button_codes_copy[BTN_L] = 4;
            button_codes_copy[BTN_R] = 5;
            button_codes_copy[BTN_QUIT] = -1;
            button_codes_copy[BTN_START] = 6;
#elif OPEN_PANDORA
            button_codes_copy[BTN_JUMP] = SDLK_PAGEDOWN;
#elif ANDROID
            for (int i=0; i<BTN_COUNT; i++) {
                button_codes_copy[i] = -1;
            }
#else
            for (int i=0; i<BTN_COUNT; i++) {
                button_codes_copy[i] = -1;
            }
            button_codes_copy[BTN_ATTACK] = 2;
            button_codes_copy[BTN_JUMP] = 1;
            button_codes_copy[BTN_DASH] = 0;
            button_codes_copy[BTN_SHIELD] = 3;
            button_codes_copy[BTN_L] = 6;
            button_codes_copy[BTN_R] = 7;
            button_codes_copy[BTN_QUIT] = 8;
            button_codes_copy[BTN_START] = 9;
    #endif
        }

        void set_default_keys() {
#ifdef PLAYSTATION2
            input_type = INPUT_TYPE_JOYSTICK;
            input_mode = INPUT_MODE_DIGITAL;
#elif PSP
            input_type = INPUT_TYPE_JOYSTICK;
            input_mode = INPUT_MODE_DIGITAL;
#elif WII
            input_type = INPUT_TYPE_JOYSTICK;
            input_mode = INPUT_MODE_DIGITAL;
#elif DINGUX
            input_type = INPUT_TYPE_DOUBLE;
            input_mode = INPUT_MODE_DIGITAL;
#elif OPEN_PANDORA
            input_type = INPUT_TYPE_KEYBOARD;
            input_mode = INPUT_MODE_DIGITAL;
#else
            input_type = INPUT_TYPE_DOUBLE;
#endif
            get_default_keys(keys_codes);
            get_default_buttons(button_codes);
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
            // Note: do not change game_finished when resetting
            selected_input_device = 0;
            selected_language[0] = '\0';
            turbo_mode = false;
            volume_sfx = 128;
            volume_music = 128;

            android_use_play_services = false;
            android_touch_controls_hide = false;
            android_touch_controls_size = 1;
            wii_joystick_type = 0;

        }


        st_game_config() {
            game_finished = false;
            reset();
        }
    };

}

#endif // FILE_CONFIG_H
