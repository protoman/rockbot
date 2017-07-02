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
    void set_android_default_buttons_size(int size);
    void set_touch_controls_visible(bool show);

private:
    std::string get_save_id(int save_n);
    void check_button_size(SDL_Rect &dest_size, SDL_Rect &default_size);
#endif
};

#endif // PLAY_GAME_SERVICES_H
