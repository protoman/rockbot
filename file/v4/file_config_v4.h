#ifndef FILE_CONFIG_301_H
#define FILE_CONFIG_301_H

#include "../format/st_common.h"
#include "file/v4/file_config_old.h"

namespace format_v4 {


    struct st_game_config {
        bool sound_enabled;
        bool video_fullscreen;
        Sint8 video_filter;
        E_INPUT_TYPES input_type;                           // if keyboard or joystick
        E_INPUT_MODES input_mode;                           // inf directional is joypad-digital, analog sick or hat
        E_PLATFORM platform;                                // if changed, must reset config to default
        int keys_codes[BTN_COUNT];                          // number indicator for the keyboard-keys
        st_input_button_config button_codes[BTN_COUNT];     // number indicator for the joystick-button-keys

        bool game_finished;                                 // stores if game was finished, so we can show more options to player
        Uint8 selected_input_device;
        Uint8 selected_language;
        bool turbo_mode;
        bool auto_charge_mode;
        Uint8 volume_sfx;
        Uint8 volume_music;
        bool android_touch_controls_hide;                   // define if touch controls must be hidden
        Uint8 android_touch_controls_size;                  // for android, size of the on-screen controls 0 = small, 1 = normal, 2 = big
        Sint8 wii_joystick_type;                            // for wii to define between wiimote, classic, gamecube, etc
        Uint8 playstation2_video_mode;                      // for playstation 2, define screen resolution setting
        Uint8 graphics_performance_mode;                    // 0 => lowend, 1=> normal, 2 => highend
        bool first_run;
        Sint8 android_button_spacing = 1;
        Sint8 android_audio_buffer_size = 0;
        // FOR FUTURE USAGE
        Sint8 old_old_style_music = 0;
        Sint8 android_use_mod_music = 0;
        Sint8 scale_int = 0;
        Sint8 CONFIG_PLACEHOLDER_004 = 0;
        Sint8 CONFIG_PLACEHOLDER_005 = 0;
        Sint8 CONFIG_PLACEHOLDER_006 = 0;
        Sint8 CONFIG_PLACEHOLDER_007 = 0;
        Sint8 CONFIG_PLACEHOLDER_008 = 0;
        Sint8 CONFIG_PLACEHOLDER_009 = 0;
        Sint8 CONFIG_PLACEHOLDER_010 = 0;

        void get_default_keys(int (&keys_codes_copy)[BTN_COUNT]) {
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
            keys_codes_copy[BTN_SHIELD] = SDLK_LSHIFT;
            keys_codes_copy[BTN_DASH] = SDLK_SPACE;
            keys_codes_copy[BTN_ATTACK] = SDLK_LALT;
            keys_codes_copy[BTN_JUMP] = SDLK_LCTRL;
            keys_codes_copy[BTN_L] = SDLK_TAB;
            keys_codes_copy[BTN_R] = SDLK_BACKSPACE;
            keys_codes_copy[BTN_QUIT] = SDLK_UNKNOWN;
            keys_codes_copy[BTN_START] = SDLK_RETURN;
            keys_codes_copy[BTN_LEFT] = SDLK_LEFT;
            keys_codes_copy[BTN_RIGHT] = SDLK_RIGHT;
            keys_codes_copy[BTN_UP] = SDLK_UP;
            keys_codes_copy[BTN_DOWN] = SDLK_DOWN;
#elif POCKETGO
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
            keys_codes_copy[BTN_SHIELD] = SDLK_SPACE;
            keys_codes_copy[BTN_DASH] = SDLK_LSHIFT;
            keys_codes_copy[BTN_ATTACK] = SDLK_LCTRL;
            keys_codes_copy[BTN_JUMP] = SDLK_LALT;
            keys_codes_copy[BTN_L] = SDLK_TAB;
            keys_codes_copy[BTN_R] = SDLK_BACKSPACE;
            keys_codes_copy[BTN_QUIT] = SDLK_RCTRL;
            keys_codes_copy[BTN_START] = SDLK_RETURN;
            keys_codes_copy[BTN_LEFT] = SDLK_LEFT;
            keys_codes_copy[BTN_RIGHT] = SDLK_RIGHT;
            keys_codes_copy[BTN_UP] = SDLK_UP;
            keys_codes_copy[BTN_DOWN] = SDLK_DOWN;
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
#elif RASPBERRY
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes_copy[i] = -1;
            }
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

        void get_default_buttons(st_input_button_config (&button_codes_copy)[BTN_COUNT]) {
            // reset values
            for (int i=0; i<BTN_COUNT; i++) {
                button_codes_copy[i].type = JOYSTICK_INPUT_TYPE_BUTTON;
                button_codes_copy[i].value = -1;
                button_codes_copy[i].axis_type = 0;
            }

            // COMMON INPUT FOR JOYSTICK AXIS //

            button_codes_copy[BTN_DOWN].axis_type = 1;
            button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_DOWN].value = 1;

            button_codes_copy[BTN_UP].axis_type = -1;
            button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_UP].value = 1;

            button_codes_copy[BTN_RIGHT].axis_type = 1;
            button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_RIGHT].value = 0;

            button_codes_copy[BTN_LEFT].axis_type = -1;
            button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_LEFT].value = 0;

#ifdef PLAYSTATION2
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 0;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 3;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 2;
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 1;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 6;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 7;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = -1;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 5;

            button_codes_copy[BTN_DOWN].axis_type = 1;
            button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_BUTTON;	            button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_RIGHT].value = -1; // uses default axis	            button_codes_copy[BTN_DOWN].value = 1;
            button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_BUTTON;	            button_codes_copy[BTN_UP].axis_type = -1;
            button_codes_copy[BTN_UP].value = -1; // uses default axis	            button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_BUTTON;	            button_codes_copy[BTN_UP].value = 1;
            button_codes_copy[BTN_DOWN].value = -1; // uses default axis	            button_codes_copy[BTN_RIGHT].axis_type = 1;
            button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_RIGHT].value = 0;
            button_codes_copy[BTN_LEFT].axis_type = -1;
            button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_AXIS;
            button_codes_copy[BTN_LEFT].value = 0;
#elif PSP
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 3;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 0;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 1;
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 2;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 4;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 5;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = -1;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 11;
            button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_LEFT].value = 7; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_RIGHT].value = 9; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_UP].value = 8; // PSP maps axis as buttons. Will use thos unless you pick analog mode
            button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DOWN].value = 6; // PSP maps axis as buttons. Will use thos unless you pick analog mode
#elif WII
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 1;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 0;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 3;
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 2;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 4;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 5;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = -1;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 6;
#elif OPEN_PANDORA
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = SDLK_PAGEDOWN;
#elif ANDROID
            for (int i=0; i<BTN_COUNT; i++) {
                button_codes_copy[i].type = JOYSTICK_INPUT_TYPE_BUTTON;
                button_codes_copy[i].value = -1;
                button_codes_copy[i].axis_type = 0;
            }
#elif DREAMCAST
            // down //
            button_codes_copy[BTN_DOWN].axis_type = 0;
            button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_HAT;
            button_codes_copy[BTN_DOWN].value = 11;
            // up //
            button_codes_copy[BTN_UP].axis_type = 0;
            button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_HAT;
            button_codes_copy[BTN_UP].value = 14;
            // left //
            button_codes_copy[BTN_LEFT].axis_type = 0;
            button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_HAT;
            button_codes_copy[BTN_LEFT].value = 7;
            // right //
            button_codes_copy[BTN_RIGHT].axis_type = 0;
            button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_HAT;
            button_codes_copy[BTN_RIGHT].value = 13;
            // attack //
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 2;
            // jump //
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 1;
            // dash/slide //
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 0;
            // shield //
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 3;
            // shoulder-left //
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 6;
            // shoulder-right //
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 7;
            // quit //
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = 8;
            // start //
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 9;
#elif RASPBERRY
            // shield, tiro, pulo, slide
            // QUIT e START: OK
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 5;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 4;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 0;
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 1;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 3;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 2;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = 8;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 9;
#elif POCKETGO
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 1;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 2;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 3;
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 0;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 6;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 7;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = 8;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 9;
#else
            button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_ATTACK].value = 2;
            button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_JUMP].value = 1;
            button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_DASH].value = 0;
            button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_SHIELD].value = 3;
            button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_L].value = 6;
            button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_R].value = 7;
            button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_QUIT].value = 8;
            button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
            button_codes_copy[BTN_START].value = 9;
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
            input_type = INPUT_TYPE_KEYBOARD;
            input_mode = INPUT_MODE_DIGITAL;
#elif POCKETGO
            input_type = INPUT_TYPE_KEYBOARD;
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
    #elif POCKETGO
            return PLATFORM_POCKETGO;
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
    #elif DREAMCAST
            return PLATFORM_DREAMCAST;
    #elif RASPBERRY
            return PLATFORM_RASPBERRY;
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
            selected_language = LANGUAGE_AUTODETECT;
            turbo_mode = false;
            auto_charge_mode = false;
            volume_sfx = 90;
            volume_music = 128;

            android_touch_controls_hide = false;
            android_touch_controls_size = 1;
            wii_joystick_type = 0;
            playstation2_video_mode = 0;
            graphics_performance_mode = PERFORMANCE_MODE_HIGH;
            android_button_spacing = 1;
            android_audio_buffer_size = 0;
            old_old_style_music = 0;
            android_use_mod_music = 0;
            scale_int = 0;
            CONFIG_PLACEHOLDER_004 = 0;
            CONFIG_PLACEHOLDER_005 = 0;
            CONFIG_PLACEHOLDER_006 = 0;
            CONFIG_PLACEHOLDER_007 = 0;
            CONFIG_PLACEHOLDER_008 = 0;
            CONFIG_PLACEHOLDER_008 = 0;
            CONFIG_PLACEHOLDER_010 = 0;
        }


        st_game_config() {
            game_finished = false;
            first_run = true;
            reset();
        }

        format_v4::st_game_config& operator=(const format_v4_old::st_game_config& original) {
            sound_enabled = original.sound_enabled;
            video_fullscreen = original.video_fullscreen;
            video_filter = original.video_filter;
            input_type = original.input_type;                           // if keyboard or joystick
            input_mode = original.input_mode;                           // inf directional is joypad-digital, analog sick or hat
            platform = original.platform;                                // if changed, must reset config to default
            for (int i=0; i<BTN_COUNT; i++) {
                keys_codes[i] = original.keys_codes[i];                          // number indicator for the keyboard-keys
                button_codes[i] = original.button_codes[i];     // number indicator for the joystick-button-keys
            }

            game_finished = original.game_finished;                                 // stores if game was finished, so we can show more options to player
            selected_input_device = original.selected_input_device;
            selected_language = original.selected_language;
            turbo_mode = original.turbo_mode;
            auto_charge_mode = original.auto_charge_mode;
            volume_sfx = original.volume_sfx;
            volume_music = original.volume_music;
            android_touch_controls_hide = original.android_touch_controls_hide;                   // define if touch controls must be hidden
            android_touch_controls_size = original.android_touch_controls_size;                  // for android, size of the on-screen controls 0 = small, 1 = normal, 2 = big
            wii_joystick_type = original.wii_joystick_type;                            // for wii to define between wiimote, classic, gamecube, etc
            playstation2_video_mode = original.playstation2_video_mode;                      // for playstation 2, define screen resolution setting
            graphics_performance_mode = original.graphics_performance_mode;                    // 0 => lowend, 1=> normal, 2 => highend
            first_run = original.first_run;
            android_button_spacing = 1;
            android_audio_buffer_size = 0;
            old_old_style_music = 0;
            android_use_mod_music = 0;
            scale_int = 0;
            CONFIG_PLACEHOLDER_004 = 0;
            CONFIG_PLACEHOLDER_005 = 0;
            CONFIG_PLACEHOLDER_006 = 0;
            CONFIG_PLACEHOLDER_007 = 0;
            CONFIG_PLACEHOLDER_008 = 0;
            CONFIG_PLACEHOLDER_008 = 0;
            CONFIG_PLACEHOLDER_010 = 0;
            return *this;
        }
    };

}

#endif // FILE_CONFIG_H
