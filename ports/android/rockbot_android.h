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
        BUTTON_SIZE_MULTI = 0.5;
    } else if (size == ANDROID_BUTTON_BIG) {
        BUTTON_SIZE_MULTI = 1.5;
    }

    SDL_Rect pos_old = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_old);

    // Move button START(5) to the top-left of screen
    pos.w = pos_old.w * BUTTON_SIZE_MULTI;
    pos.h = pos_old.h * BUTTON_SIZE_MULTI;
    pos.x = screen_width/16;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos);

    // Move button R(6) to the top-right of screen
    pos.x = pos_old.x;
    pos.y = screen_width/16;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &pos);

    // adjust buttons size
    if (size != ANDROID_BUTTON_NORMAL) {
        // Adjust D-Pad size and position
        SDL_Rect dpad_pos_old = {0, 0, 0, 0};
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos_old);
        dpad_pos_old.w = dpad_pos_old.w * BUTTON_SIZE_MULTI;
        dpad_pos_old.h = dpad_pos_old.h * BUTTON_SIZE_MULTI;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos_old);

        // @TODO: must also adjust position because of bigger/smaller size

        // button 1 (A)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &dpad_pos_old);

        // button 2 (B)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        if (size == ANDROID_BUTTON_SMALL) {
            pos_old.x += pos_old.w * 0.5;
        } else if (size == ANDROID_BUTTON_BIG) {
            pos_old.x -= pos_old.w * 0.5;
        }
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &dpad_pos_old);

        // button 3 (X)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &dpad_pos_old);


        // button 4 (Y)
        SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_old);
        pos_old.w = pos_old.w * BUTTON_SIZE_MULTI;
        pos_old.h = pos_old.h * BUTTON_SIZE_MULTI;
        if (size == ANDROID_BUTTON_SMALL) {
            pos_old.x += pos_old.w * 0.5;
        } else if (size == ANDROID_BUTTON_BIG) {
            pos_old.x -= pos_old.w * 0.5;
        }
        SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &dpad_pos_old);

    }
}

#endif

#endif
