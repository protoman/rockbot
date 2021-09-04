#include "android_game_services.h"


#ifdef ANDROID

extern CURRENT_FILE_FORMAT::file_io fio;
extern jobject activity_ref;

android_game_services::android_game_services()
{
    original_dpad_pos = {0, 0, 0, 0};
    original_pos_a = {0, 0, 0, 0};

    //gpg::AndroidPlatformConfiguration platform_configuration;
    //platform_configuration.SetActivity(activity_ref);
}

void android_game_services::crash_handler(std::string log_msg)
{
    //SDL_ANDROID_CrashHandler(log_msg.c_str());
}

void android_game_services::init_android_button_size()
{
    // get button 0 (A) and D-PAD as base to calc distance between buttons
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &original_pos_a);
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &original_dpad_pos);
}

void android_game_services::set_android_default_buttons_size(int size, int spacing)
{
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", ">>>>> set_android_default_buttons_size - size[%d], spacing[%d]", size, spacing);
    if (size >= ANDROID_TOUCH_CONTROL_SIZE_COUNT) {
        return;
    }
    if (spacing < 0 || spacing > 3) {
        return;
    }
    int screen_width = 800; // Pelya's default
    int screen_height = 480;
    if (const char* env_w = std::getenv("DISPLAY_RESOLUTION_WIDTH")) {
        screen_width = atoi(env_w);
    }
    if (const char* env_h = std::getenv("DISPLAY_RESOLUTION_HEIGHT")) {
        screen_height = atoi(env_h);
    }

    double spacing_multi = 0.8;
    if (spacing == 1) {
        spacing_multi = 1.0;
    } else if (spacing == 2) {
        spacing_multi = 1.2;
    }

    // @TODO: if screen_width or screen_height are too small, we'll use buttons position as clue for the resolution

    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", ">>>>> set_android_default_buttons_size::screensize: w[%d], h[%d]", screen_width, screen_height);

    float BUTTON_SIZE_MULTI = 1.2; // medium
    if (size == ANDROID_BUTTON_SMALL) {
        BUTTON_SIZE_MULTI = 1.0;
    } else if (size == ANDROID_BUTTON_BIG) {
        BUTTON_SIZE_MULTI = 1.4;
    }

    int button_w = original_pos_a.w * BUTTON_SIZE_MULTI;
    int button_h = original_pos_a.h * BUTTON_SIZE_MULTI;
    //dpad_pos.y = screen_height-dpad_pos.h+(dpad_pos.h/MARGIN_DIVISOR);
    int spacer_w = button_w/5;
    int spacer_h = button_h/5;

    // default size is 1/20 of screen
    if (original_pos_a.w == 0) {
        original_pos_a.w = (screen_width/20)*BUTTON_SIZE_MULTI;
    }
    if (original_pos_a.h == 0) {
        original_pos_a.h = (screen_height/20)*BUTTON_SIZE_MULTI;
    }
    float button_diff_h = original_pos_a.h - (original_pos_a.h * BUTTON_SIZE_MULTI);

    // Move button START(4) to the top-left of screen
    SDL_Rect pos_button_start = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_button_start);
    pos_button_start.w = button_w;
    pos_button_start.h = button_h;
    check_button_size(pos_button_start, original_pos_a);
    pos_button_start.x = -spacer_w;
    pos_button_start.y = -spacer_h;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_button_start);


    // D-PAD //
    SDL_Rect dpad_pos = {0, 0, 0, 0};
    dpad_pos.w = original_dpad_pos.w * BUTTON_SIZE_MULTI;
    dpad_pos.h = original_dpad_pos.h * BUTTON_SIZE_MULTI;
    dpad_pos.x = -button_w/2;

    dpad_pos.y = screen_height - dpad_pos.h + spacer_h*2;

    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos);

    // BUTTON R(5) - TOP RIGHT
    SDL_Rect r5_pos = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &r5_pos);
    r5_pos.w = button_w;
    r5_pos.h = button_h;
    check_button_size(r5_pos, original_pos_a);
    r5_pos.x = screen_width - r5_pos.w;
    r5_pos.y = -spacer_h;

    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", ">>>>> set_android_default_buttons_size::R(5): x[%d], w[%d], h[%d]", r5_pos.x, r5_pos.w, r5_pos.h);

    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &r5_pos);

    // @TODO: must also adjust position because of bigger/smaller size

    // button 0 (A)
    SDL_Rect pos_button_a = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_button_a);
    pos_button_a.w = button_w;
    pos_button_a.h = button_h;
    check_button_size(pos_button_a, original_pos_a);
    pos_button_a.x = r5_pos.x;
    pos_button_a.y = screen_height - pos_button_a.h + spacer_h;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_button_a);

    // button 1 (B)
    SDL_Rect pos_button_b = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_button_b);
    pos_button_b.w = button_w;
    pos_button_b.h = button_h;
    check_button_size(pos_button_b, original_pos_a);
    pos_button_b.x = pos_button_a.x - (button_w*spacing_multi);
    pos_button_b.y = pos_button_a.y;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_button_b);

    // button 2 (X)
    SDL_Rect pos_button_x = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_button_x);
    pos_button_x.w = button_w;
    pos_button_x.h = button_h;
    check_button_size(pos_button_x, original_pos_a);
    pos_button_x.x = pos_button_a.x;
    pos_button_x.y = pos_button_a.y - (button_h*spacing_multi);
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_button_x);


    // button 3 (Y)
    SDL_Rect pos_button_y = {0, 0, 0, 0};
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_button_y);
    pos_button_y.w = button_w;
    pos_button_y.h = button_h;
    check_button_size(pos_button_y, original_pos_a);
    pos_button_y.x = pos_button_b.x;
    pos_button_y.y = pos_button_x.y;
    SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &pos_button_y);
}

void android_game_services::set_touch_controls_visible(bool show)
{
    //SDL_ANDROID_SetScreenKeyboardShown(show);
    /* Set screen keyboard transparency, 255 or SDL_ALPHA_OPAQUE is non-transparent, 0 or SDL_ALPHA_TRANSPARENT is transparent */
    if (show == true) {
        SDL_ANDROID_SetScreenKeyboardTransparency(SDL_ALPHA_OPAQUE*0.4);
    } else {
        SDL_ANDROID_SetScreenKeyboardTransparency(SDL_ALPHA_TRANSPARENT);
    }
}


void android_game_services::check_button_size(SDL_Rect& dest_size, SDL_Rect& default_size)
{
    if (dest_size.w == 0) {
        dest_size.w = default_size.w;
    }
    if (dest_size.h == 0) {
        dest_size.h = default_size.h;
    }

}
#endif


