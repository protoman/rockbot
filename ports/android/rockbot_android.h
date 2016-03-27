#ifdef ANDROID
#ifndef ANDROID_PORT
#define ANDROID_PORT

#include "defines.h"

#include <SDL_screenkeyboard.h>

#include "graphicslib.h"
extern graphicsLib graphLib;

void set_android_default_buttons_pos() {
    SDL_Rect pos = {0, 0, 0, 0};

    const SDL_VideoInfo* screen_info = SDL_GetVideoInfo();
    int screen_width = screen_info->current_w;
    int screen_height = screen_info->current_h;

    std::cout << "screen_width: " << screen_width << ", screen_height: " << screen_height << std::endl;


    SDL_Rect pos_old = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_old);

    // Move button START(5) to the top-left of screen
    pos.w = pos_old.w;
    pos.h = pos_old.h;
    pos.x = screen_width/16;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos);

    // Move button R(6) to the top-right of screen
    pos.x = pos_old.x;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &pos);
}
#endif
#endif
