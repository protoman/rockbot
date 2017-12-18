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
void android_game_services::cloud_load_game(int save_n)
{
    //SDLCALL SDL_ANDROID_CloudLoad(const char *filename, const char *saveId, const char *dialogTitle);
    SDL_ANDROID_CloudLoad(fio.get_save_filename(save_n).c_str(), get_save_id(save_n).c_str(), SAVE_TITLE);
}

void android_game_services::cloud_save_game(int save_n)
{
    //extern DECLSPEC int SDLCALL SDL_ANDROID_CloudSave(const char *filename, const char *saveId,
    // const char *dialogTitle, const char *description, const char *screenshotFile, uint64_t playedTimeMs);
    SDL_ANDROID_CloudSave(fio.get_save_filename(save_n).c_str(), get_save_id(save_n).c_str(), SAVE_TITLE, NULL, NULL, SDL_GetTicks());
}

void android_game_services::connect()
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", ">>>>>>>>>>>>>>>>> android_game_services::connect");
#endif
    SDL_ANDROID_ConnectPlayServices();
}

void android_game_services::disconnect()
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", ">>>>>>>>>>>>>>>>> android_game_services::disconnect");
#endif
    SDL_ANDROID_DisconnectPlayServices();
}

void android_game_services::unclock_achievement(std::string achievement_id)
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", ">>>>>>>>>>>>>>>>> android_game_services::unclock_achievement");
#endif
    SDL_ANDROID_UnlockAchievement(achievement_id.c_str());
}

void android_game_services::firebase_log(std::string log_msg)
{
    SDL_ANDROID_FirebaseLog(log_msg.c_str());
}

void android_game_services::crash_handler(std::string log_msg)
{
    SDL_ANDROID_CrashHandler(log_msg.c_str());
}

void android_game_services::init_android_button_size()
{
    // get button 0 (A) and D-PAD as base to calc distance between buttons
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &original_pos_a);
    SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &original_dpad_pos);
}

void android_game_services::set_android_default_buttons_size(int size)
{
    if (size >= ANDROID_TOUCH_CONTROL_SIZE_COUNT) {
        return;
    }
    {
       int screen_width = 800; // Pelya's default
       int screen_height = 480;
       if (const char* env_w = std::getenv("DISPLAY_RESOLUTION_WIDTH")) {
            screen_width = atoi(env_w);
       }
       if (const char* env_h = std::getenv("DISPLAY_RESOLUTION_HEIGHT")) {
            screen_height = atoi(env_h);
       }

       // @TODO: if screen_width or screen_height are too small, we'll use buttons position as clue for the resolution

       std::cout << "screen_width: " << screen_width << ", screen_height: " << screen_height << std::endl;

       __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", ">>>>> set_android_default_buttons_size::screensize: w[%d], h[%d]", screen_width, screen_height);

       float BUTTON_SIZE_MULTI = 1.2; // medium
       if (size == ANDROID_BUTTON_SMALL) {
           BUTTON_SIZE_MULTI = 1.0;
       } else if (size == ANDROID_BUTTON_BIG) {
           BUTTON_SIZE_MULTI = 1.4;
       }

       int button_w = original_pos_a.w * BUTTON_SIZE_MULTI;
       int button_h = original_pos_a.h * BUTTON_SIZE_MULTI;

       // default size is 1/20 of screen
       if (original_pos_a.w == 0) {
           original_pos_a.w = (screen_width/20)*BUTTON_SIZE_MULTI;
       }
       if (original_pos_a.h == 0) {
           original_pos_a.h = (screen_height/20)*BUTTON_SIZE_MULTI;
       }
       float button_diff_w = original_pos_a.w - (original_pos_a.w * BUTTON_SIZE_MULTI);
       float button_diff_h = original_pos_a.h - (original_pos_a.h * BUTTON_SIZE_MULTI);

       // Move button START(4) to the top-left of screen
       SDL_Rect pos_button_start = {0, 0, 0, 0};
       SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_button_start);
       pos_button_start.w = button_w;
       pos_button_start.h = button_h;
       check_button_size(pos_button_start, original_pos_a);
       pos_button_start.x = 0;
       pos_button_start.y = 0;
       SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos_button_start);


       // D-PAD //
       SDL_Rect dpad_pos = {0, 0, 0, 0};
       dpad_pos.w = original_dpad_pos.w * BUTTON_SIZE_MULTI;
       dpad_pos.h = original_dpad_pos.h * BUTTON_SIZE_MULTI;
       dpad_pos.x = -(dpad_pos.w/MARGIN_DIVISOR);
       dpad_pos.y = screen_height-dpad_pos.h+(dpad_pos.h/MARGIN_DIVISOR);
       SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &dpad_pos);

       // BUTTON R(5) - TOP RIGHT
       SDL_Rect r5_pos = {0, 0, 0, 0};
       SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &r5_pos);
       r5_pos.w = button_w;
       r5_pos.h = button_h;
       check_button_size(r5_pos, original_pos_a);
       r5_pos.x = screen_width-r5_pos.w+(r5_pos.w/MARGIN_DIVISOR);
       r5_pos.y = 0;

       __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", ">>>>> set_android_default_buttons_size::R(5): x[%d], w[%d], h[%d]", r5_pos.x, r5_pos.w, r5_pos.h);

       SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_5, &r5_pos);

       // @TODO: must also adjust position because of bigger/smaller size

       // button 0 (A)
       SDL_Rect pos_button_a = {0, 0, 0, 0};
       SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_button_a);
       pos_button_a.w = button_w;
       pos_button_a.h = button_h;
       check_button_size(pos_button_a, original_pos_a);
       pos_button_a.x = screen_width-pos_button_a.w+(pos_button_a.w/MARGIN_DIVISOR);
       pos_button_a.y = screen_height-pos_button_a.h;
       SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &pos_button_a);

       // button 1 (B)
       SDL_Rect pos_button_b = {0, 0, 0, 0};
       SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_button_b);
       pos_button_b.w = button_w;
       pos_button_b.h = button_h;
       check_button_size(pos_button_b, original_pos_a);
       pos_button_b.x = pos_button_a.x - button_diff_w - pos_button_b.w;
       pos_button_b.y = pos_button_a.y;
       SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos_button_b);

       // button 2 (X)
       SDL_Rect pos_button_x = {0, 0, 0, 0};
       SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos_button_x);
       pos_button_x.w = button_w;
       pos_button_x.h = button_h;
       check_button_size(pos_button_x, original_pos_a);
       pos_button_x.x = pos_button_a.x;
       pos_button_x.y = pos_button_a.y - button_diff_h - pos_button_x.h;
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

/*
void android_game_services::ndk_connect()
{

}
*/

std::string android_game_services::get_save_id(int save_n)
{
    char save_id[20];
    sprintf(save_id, "Rockbot2_0%d", save_n);
    return std::string(save_id);
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


