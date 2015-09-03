#ifndef FILE_CONFIG_H
#define FILE_CONFIG_H


namespace format_v_3_0_0 {


struct st_game_config {
    bool sound_enabled;
    bool video_fullscreen;
    Sint8 video_filter;
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
        button_codes[BTN_SHIELD] = 3;
        button_codes[BTN_DASH] = 2;
        button_codes[BTN_JUMP] = 1;
        button_codes[BTN_ATTACK] = 0;
        button_codes[BTN_L] = 6;
        button_codes[BTN_R] = 7;
        button_codes[BTN_QUIT] = -1;
        button_codes[BTN_START] = 5;
        button_codes[BTN_LEFT] = -1; // uses default axis
        button_codes[BTN_RIGHT] = -1; // uses default axis
        button_codes[BTN_UP] = -1; // uses default axis
        button_codes[BTN_DOWN] = -1; // uses default axis
        //std::cout << "v_2_1_2 - input_mode: " << input_mode << ", BTN_JUMP: " << button_codes[BTN_JUMP] << std::endl;
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

}

#endif // FILE_CONFIG_H
