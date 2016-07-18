#ifndef ANDROID_PORT_h
#define ANDROID_PORT_h

#ifdef ANDROID

#include "defines.h"

#include <jni.h>
#include <android/log.h>
#include <SDL_screenkeyboard.h>

#include "graphicslib.h"
extern graphicsLib graphLib;

extern jobject activity_ref;



void set_android_default_buttons_pos(int size) {
    SDL_Rect pos = {0, 0, 0, 0};

    const SDL_VideoInfo* screen_info = SDL_GetVideoInfo();
    int screen_width = screen_info->current_w;
    int screen_height = screen_info->current_h;

    std::cout << "screen_width: " << screen_width << ", screen_height: " << screen_height << std::endl;

    float BUTTON_SIZE_MULTI = 1;
    if (size == ANDROID_BUTTON_SMALL) {
        BUTTON_SIZE_MULTI = 0.8;
    } else if (size == ANDROID_BUTTON_BIG) {
        BUTTON_SIZE_MULTI = 1.2;
    }

    SDL_Rect pos_old = {0, 0, 0, 0};
    // get button 0 (A) as base
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_old);

    // Move button START(4) to the top-left of screen
    pos.w = pos_old.w * BUTTON_SIZE_MULTI;
    pos.h = pos_old.h * BUTTON_SIZE_MULTI;
    float button_diff_w = pos.w - (pos.w * BUTTON_SIZE_MULTI);
    pos.w = pos.w * BUTTON_SIZE_MULTI;
    pos.h = pos.h * BUTTON_SIZE_MULTI;
    pos.x = screen_width/16;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos);

    // Move button R(5) to the top-right of screen
    pos.x = pos_old.x;
    pos.x += button_diff_w*1.5;
    pos.x += button_diff_w;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &pos);

    // adjust buttons size
    if (size != ANDROID_BUTTON_NORMAL) {
        // Adjust D-Pad size and position
        SDL_Rect dpad_pos_old = {0, 0, 0, 0};
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos_old);
        dpad_pos_old.w = dpad_pos_old.w * BUTTON_SIZE_MULTI;
        dpad_pos_old.h = dpad_pos_old.h * BUTTON_SIZE_MULTI;
        dpad_pos_old.y += button_diff_w*1.5;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos_old);

        // @TODO: must also adjust position because of bigger/smaller size

        // button 0 (A)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        pos_old.x += button_diff_w*1.5;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_old);

        // button 1 (B)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        pos_old.x += button_diff_w*2;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_old);

        // button 2 (X)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        pos_old.x += button_diff_w*1.5;
        pos_old.y += button_diff_w*1.5;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_old);


        // button 3 (Y)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        pos_old.x += button_diff_w*2;
        pos_old.y += button_diff_w*2;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_old);

    }
}

#endif

#endif
