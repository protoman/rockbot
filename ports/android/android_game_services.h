#ifndef PLAY_GAME_SERVICES_H
#define PLAY_GAME_SERVICES_H

#ifdef ANDROID
#include <string>
#include <iostream>
#include <cstdlib>
#include <SDL_android.h>
#include <android/log.h>
#include <SDL_screenkeyboard.h>
#include "defines.h"
#include "file/file_io.h"

//### NDK ###//
/*
#include <gpg/android_initialization.h>
#include <gpg/android_platform_configuration.h>
#include <gpg/android_support.h>
*/
#define TOUCH_KEYBOARD_MARGIN 30
#define BUTTON_DISTANCE 1.3
#define MARGIN_DIVISOR 6
#define SAVE_TITLE "Rockbot Save"
#endif

class android_game_services
{
public:
    android_game_services();
#ifdef ANDROID
    void cloud_load_game(int save_n);
    void cloud_save_game(int save_n);
    void connect();
    void disconnect();
    void unclock_achievement(std::string achievement_id);
    // stores original size before setting a different size
    void init_android_button_size();
    void set_android_default_buttons_size(int size);
    void set_touch_controls_visible(bool show);
    void firebase_log(std::string log_msg);
    void crash_handler(std::string log);

    //### NDK GAME SERVICES ###//
    //void ndk_connect();

private:
    std::string get_save_id(int save_n);
    void check_button_size(SDL_Rect &dest_size, SDL_Rect &default_size);


private:
     SDL_Rect original_dpad_pos;
     SDL_Rect original_pos_a;

#endif
};

#endif // PLAY_GAME_SERVICES_H
