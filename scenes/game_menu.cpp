#include "game_menu.h"

extern std::string FILEPATH;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;

#include "game.h"
extern game gameControl;

#include "timerlib.h"
extern timerLib timer;

#include "ports/android/android_game_services.h"
extern android_game_services game_services;


extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_io fio;


game_menu::game_menu()
{

}

void game_menu::show_extra_menu()
{
    short res = 0;
    st_position config_text_pos;
    std::vector<st_menu_option> options;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 20;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;

    graphLib.show_config_bg();
    draw_lib.update_screen();
    input.clean();
    timer.delay(300);


    short selected_option = 0;
    int current_player = game_save.selected_player;


    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_about, game_config.selected_language)));
    option_picker cheat_config_picker(false, config_text_pos, options, true);

    while (selected_option != -1) {
        option_picker extra_menu_picker(false, config_text_pos, options, true);
        graphLib.show_config_bg();
        extra_menu_picker.draw();
        selected_option = extra_menu_picker.pick(selected_option+1);
        // ABOUT/CREDITS //
        if (selected_option == 0) {
            // only wait for keypress if user did not interrupted credits
            if (draw_lib.show_credits(true) == 0) {
                input.wait_keypress();
            }
        }
    }
}

short game_menu::show_main_config(short stage_finished, bool called_from_game)
{
    short res = 0;
    st_position config_text_pos;
    std::vector<st_menu_option> options;

    graphLib.show_config_bg();
    draw_lib.update_screen();
    input.clean();
    timer.delay(300);

    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_audio, game_config.selected_language)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_input, game_config.selected_language)));
#if defined(PC)
    options.push_back(st_menu_option("PC"));
#elif PSP
    options.push_back(st_menu_option("PSP"));
#elif WII
    options.push_back(st_menu_option("WII"));
#elif ANDROID
    options.push_back(st_menu_option("ANDROID"));
#else
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_wii_platformspecific, game_config.selected_language), true));
#endif
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_language, game_config.selected_language)));

    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config_graphics_performance, game_config.selected_language)));

    options.push_back( strings_map::get_instance()->get_ingame_string(strings_ingame_extras, game_config.selected_language));

    if (called_from_game) {
        if (stage_finished) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage, game_config.selected_language)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage, game_config.selected_language), true));
        }
        options.push_back( strings_map::get_instance()->get_ingame_string(strings_ingame_config_quitgame, game_config.selected_language));
    }



    config_text_pos.x = graphLib.get_config_menu_pos().x + 20;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;

    short selected_option = 0;
    bool picked_once = false;
    while (selected_option != -1) {
        option_picker main_config_picker(false, config_text_pos, options, true);
        if (picked_once == false) {
            selected_option = main_config_picker.pick();
            picked_once = true;
        } else {
            selected_option = main_config_picker.pick(selected_option+1);
        }
        if (selected_option == -1) {
            break;
        }
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        draw_lib.update_screen();
        if (selected_option == 0) { // CONFIG AUDIO //
            show_config_audio();
        } else if (selected_option == 1) { // CONFIG INPUT //
            key_map key_mapper;
            key_mapper.config_input();
        } else if (selected_option == 2) { // CONFIG PLATFORM //
#ifdef PC
            show_config_video();
            //show_config_android(); // line used for test/debug
#elif ANDROID
            show_config_android();
#elif PSP
            show_config_video_PSP();

#elif WII
            show_config_wii();
#elif PLAYSTATION2
            show_config_PS2();
#endif
        } else if (selected_option == 3) { // LANGUAGE //
            show_config_language();
        } else if (selected_option == 4) { // PERFORMANCE //
            show_config_performance();
        } else if (selected_option == 5) { // EXTRAS //
            show_config_extras();
        } else if (selected_option == 6) { // LEAVE STAGE //
            res = 1;
            break;
        } else if (selected_option == 7) { // QUIT GAME //
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        }
        fio.save_config(game_config);

        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        draw_lib.update_screen();
    }
    return res;
}

void game_menu::show_config_android()
{
#ifdef ANDROID
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);
    short selected_option = 0;

    std::vector<st_menu_option> options;

    while (selected_option != -1) {
        options.clear();

        // OPTION #0: SHOW/HIDE controls
        if (game_config.android_touch_controls_hide == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_hidescreencontrols, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_hidescreencontrols, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled, game_config.selected_language)));
        }
        // OPTION #1: controls size
        if (game_config.android_touch_controls_size == 0) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_SMALL, game_config.selected_language)));
        } else if (game_config.android_touch_controls_size == 2) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_BIG, game_config.selected_language)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_MEDIUM, game_config.selected_language)));
        }
        // OPTION #2: use play services
        if (game_config.android_use_play_services == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language), true));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled,game_config.selected_language), true));
        }
        // OPTION #3: use cloud save (only available if use play services is true)
        if (game_config.android_use_cloud_save == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_usecloudsave, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language), true));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_usecloudsave, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled, game_config.selected_language), true));
        }

        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            game_config.android_touch_controls_hide = !game_config.android_touch_controls_hide;
            game_services.set_touch_controls_visible(!game_config.android_touch_controls_hide);
            // @TODO: show warning about emergency reset
        } else if (selected_option == 1) {
            game_config.android_touch_controls_size++;
            if (game_config.android_touch_controls_size > 2) {
                game_config.android_touch_controls_size = 0;
            }
            game_services.set_android_default_buttons_size(game_config.android_touch_controls_size);
        } else if (selected_option == 2) {
            game_config.android_use_play_services = !game_config.android_use_play_services;
            if (game_config.android_use_play_services == true) {
                game_services.connect();
            } else {
                game_services.disconnect();
            }
            // disable cloud service always when changing play-services value
            game_config.android_use_cloud_save = false;
        } else if (selected_option == 3) {
            if (game_config.android_use_play_services == false) {
                game_config.android_use_cloud_save = false;
                if (game_config.android_use_cloud_save == true) {
                    show_config_warning_android_play_services();
                }
            } else {
                game_config.android_use_cloud_save = !game_config.android_use_cloud_save;
                if (game_config.android_use_cloud_save == true) {
                    show_config_warning_android_cloud_save();
                    // load saves from cloud, leave conflits to the library do handle //
                    gameControl.load_save_data_from_cloud();
                }
            }
            // @TODO: show warning that cloud load/save requires network connection
        }
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        graphLib.updateScreen();
    }
    fio.save_config(game_config);
#endif
}


void game_menu::show_config_video()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);


    short selected_option = 0;

    while (selected_option != -1) {
        std::vector<std::string> options;
        if (game_config.video_fullscreen == false) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed, game_config.selected_language));
        } else {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen, game_config.selected_language));
        }

        if (game_config.video_filter == VIDEO_FILTER_NOSCALE) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_noscale, game_config.selected_language));
        } else if (game_config.video_filter == VIDEO_FILTER_BITSCALE) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_size2x, game_config.selected_language));
        } else if (game_config.video_filter == VIDEO_FILTER_SCALE2x) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale2x, game_config.selected_language));
        }


        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            game_config.video_fullscreen = !game_config.video_fullscreen;
        } else if (selected_option == 1) {
            game_config.video_filter++;
            if (game_config.video_filter >= VIDEO_FILTER_COUNT) {
                game_config.video_filter = 0;
            }
        }
        if (selected_option != -1) {
            fio.save_config(game_config);
            show_config_ask_restart();
            st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
            graphLib.clear_area(menu_pos.x-14, menu_pos.y, RES_W, 100, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
            main_config_picker.draw();
        }
    }
}

void game_menu::show_config_video_PSP()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    short selected_option = 0;

    while (selected_option != -1) {
        std::vector<std::string> options;
        if (game_config.video_fullscreen == true) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed, game_config.selected_language));
        } else {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen, game_config.selected_language));
        }
        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            game_config.video_fullscreen = !game_config.video_fullscreen;
        }
        if (selected_option != -1) {
            fio.save_config(game_config);
            show_config_ask_restart();
            main_config_picker.draw();
        }
    }
}

void game_menu::show_config_wii()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    short selected_option = 0;
    while (selected_option != -1) {
        std::vector<std::string> options;
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_WIIMOTE, game_config.selected_language));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_CLASSIC, game_config.selected_language));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_GAMECUBE, game_config.selected_language));
        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option != -1) {
            game_config.wii_joystick_type = selected_option;
            main_config_picker.draw();
        }
    }
}

void game_menu::show_config_PS2()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    options.push_back("320x200");
    options.push_back("320x240");
    options.push_back("320x256");
    options.push_back("400x256");
    options.push_back("512x448");
    options.push_back("720p");

    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    while (selected_option != -1) {
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option != -1) {
            game_config.playstation2_video_mode = selected_option;
            fio.save_config(game_config);
            show_config_ask_restart();
            main_config_picker.draw();
        }
    }
}

void game_menu::show_config_ask_restart()
{
    input.clean();
    timer.delay(300);
    st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart, game_config.selected_language));
    draw_lib.update_screen();
    input.wait_keypress();
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    draw_lib.update_screen();
}

void game_menu::show_config_audio()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    if (game_config.sound_enabled == true) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled, game_config.selected_language));
    } else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language));
    }
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_music, game_config.selected_language));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_sfx, game_config.selected_language));


    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = 0;
    while (selected_option != -1) {
        main_config_picker.draw();
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            if (game_config.sound_enabled == false) {
                soundManager.enable_sound();
            } else {
                soundManager.disable_sound();
            }
        } else if (selected_option == 1) {
            config_int_value(game_config.volume_music, 1, 128);
            soundManager.update_volumes();
            fio.save_config(game_config);
        } else if (selected_option == 2) {
            config_int_value(game_config.volume_sfx, 1, 128);
            soundManager.update_volumes();
            fio.save_config(game_config);
        }
    }
}

void game_menu::show_config_language()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);
    std::vector<st_menu_option> options;
    if (game_config.selected_language == LANGUAGE_FRENCH) {           // FRENCH
        options.push_back(st_menu_option("ANGLAIS"));
        options.push_back(st_menu_option("FRANCAIS", true));
        options.push_back(st_menu_option("ESPANOL", true));
        options.push_back(st_menu_option("ITALIEN", true));
        options.push_back(st_menu_option("PORTUGAIS"));
    } else if (game_config.selected_language == LANGUAGE_SPANISH) {    // SPANISH
        options.push_back(st_menu_option("INGLES"));
        options.push_back(st_menu_option("FRANCES", true));
        options.push_back(st_menu_option("ESPANOL", true));
        options.push_back(st_menu_option("ITALIANO", true));
        options.push_back(st_menu_option("PORTUGUES"));
    } else if (game_config.selected_language == LANGUAGE_ITALIAN) {    // ITALIAN
        options.push_back(st_menu_option("INGLESE"));
        options.push_back(st_menu_option("FRANCESE", true));
        options.push_back(st_menu_option("SPAGNOLO", true));
        options.push_back(st_menu_option("ITALIANO", true));
        options.push_back(st_menu_option("PORTOGHESE"));
    } else if (game_config.selected_language == LANGUAGE_PORTUGUESE) {    // PORTUGUESE (BRAZILIAN)
        options.push_back(st_menu_option("INGLES"));
        options.push_back(st_menu_option("FRANCES", true));
        options.push_back(st_menu_option("ESPANHOL", true));
        options.push_back(st_menu_option("ITALIANO", true));
        options.push_back(st_menu_option("PORTUGUES"));
    } else {                                            // ENGLISH
        options.push_back(st_menu_option("ENGLISH"));
        options.push_back(st_menu_option("FRENCH", true));
        options.push_back(st_menu_option("SPANISH", true));
        options.push_back(st_menu_option("ITALIAN", true));
        options.push_back(st_menu_option("PORTUGUESE"));
    }

    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    while (selected_option != -1) {
        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option != -1) {
            game_config.selected_language = selected_option;
            fio.save_config(game_config);
            show_config_ask_restart();
            main_config_picker.draw();
        }
    }
}

void game_menu::show_config_performance()
{
    short selected_option = 0;
    while (selected_option != -1) {
        st_position config_text_pos;
        config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
        config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
        input.clean();
        timer.delay(300);
        std::vector<std::string> options;

        if (game_config.graphics_performance_mode == PERFORMANCE_MODE_LOW) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_low, game_config.selected_language));
        } else if (game_config.graphics_performance_mode == PERFORMANCE_MODE_NORMAL) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_medium, game_config.selected_language));
        } else {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_high, game_config.selected_language));
        }

        if (gameControl.get_show_fps_enabled()) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_show_fps, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_on, game_config.selected_language));
        } else {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_show_fps, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_off, game_config.selected_language));
        }



        option_picker main_config_picker(false, config_text_pos, options, true);

        selected_option = main_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            game_config.graphics_performance_mode++;
            if (game_config.graphics_performance_mode > PERFORMANCE_MODE_HIGH) {
                game_config.graphics_performance_mode = PERFORMANCE_MODE_LOW;
            }
        } else if (selected_option == 1) {
            gameControl.set_show_fps_enabled(!gameControl.get_show_fps_enabled());
        }
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    }

    fio.save_config(game_config);
}

void game_menu::show_config_warning_android_play_services()
{
    input.clean();
    timer.delay(300);
    st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_play_services1, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_play_services2, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_play_services3, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_play_services4, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+60, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart, game_config.selected_language));
    draw_lib.update_screen();
    input.wait_keypress();
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    draw_lib.update_screen();
}

void game_menu::show_config_warning_android_cloud_save()
{
    input.clean();
    timer.delay(300);
    st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_cloud_save1, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_cloud_save2, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_cloud_save3, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_android_cloud_save4, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+60, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart, game_config.selected_language));
    draw_lib.update_screen();
    input.wait_keypress();
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    draw_lib.update_screen();
}

void game_menu::show_config_warning_android_hide_controls()
{
    input.clean();
    timer.delay(300);
    st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3, game_config.selected_language));
    graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart, game_config.selected_language));
    draw_lib.update_screen();
    input.wait_keypress();
    graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    draw_lib.update_screen();
}

void game_menu::show_config_extras()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_extras_music_player, game_config.selected_language));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_extras_sound_player, game_config.selected_language));
    //options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_extras_art_gallery, game_config.selected_language));

    short selected_option = 0;
    option_picker config_extras_picker(false, config_text_pos, options, true);
    while (selected_option != -1) {
        selected_option = config_extras_picker.pick(selected_option+1);
        if (selected_option == 0) {
            music_player();
        } else if (selected_option == 1) {
            sfx_player();
        }
        config_extras_picker.draw();
    }
}

void game_menu::config_int_value(Uint8 &value_ref, int min, int max)
{
    int config_text_pos_x = graphLib.get_config_menu_pos().x + 24;
    int config_text_pos_y = graphLib.get_config_menu_pos().y + 40;
    graphLib.clear_area(config_text_pos_x-1, config_text_pos_y-1, 300, 100, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    input.clean();
    timer.delay(10);
    char value[3]; // for now, we handle only 0-999

    graphLib.draw_text(config_text_pos_x, config_text_pos_y, "< ");
    graphLib.draw_text(config_text_pos_x+34, config_text_pos_y, " >");

    while (true) {
        input.read_input();

        if (value_ref < 10) {
            sprintf(value, "00%d", value_ref);
        } else if (value_ref < 100) {
            sprintf(value, "0%d", value_ref);
        } else {
            sprintf(value, "%d", value_ref);
        }
        graphLib.clear_area(config_text_pos_x+11, config_text_pos_y-1, 30, 12, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        graphLib.draw_text(config_text_pos_x+12, config_text_pos_y, std::string(value));
        if (input.p1_input[BTN_ATTACK] == 1 || input.p1_input[BTN_START] == 1 || input.p1_input[BTN_DOWN]) {
            break;
        } else if (input.p1_input[BTN_LEFT] == 1) {
            value_ref--;
        } else if (input.p1_input[BTN_RIGHT] == 1) {
            value_ref++;
        }
        if (value_ref < min) {
            value_ref = min;
        }
        if (value_ref > max) {
            value_ref = max;
        }
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }

}

void game_menu::music_player()
{
    Uint8 music_number = 0;
    CURRENT_FILE_FORMAT::file_stages stages_data;
    CURRENT_FILE_FORMAT::file_io fio;
    fio.read_all_stages(stages_data);

    int config_text_pos_x = graphLib.get_config_menu_pos().x + 24;
    int config_text_pos_y = graphLib.get_config_menu_pos().y + 40;
    graphLib.clear_area(config_text_pos_x-1, config_text_pos_y-1, 300, 100, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    input.clean();
    timer.delay(10);
    char value[3]; // for now, we handle only 0-999

    int max = CASTLE1_STAGE5;
    // check stages that have music for max-value, default is castle 1.5
    for (int i=0; i<FS_MAX_STAGES; i++) {
        std::string filename(stages_data.stages[i].bgmusic_filename);
        std::cout << "stage[" << i << "].music[" << filename << "]" << std::endl;
        if (filename.length() == 0) {
            max = i-1;
            break;
        }
    }

    while (true) {
        input.read_input();

        if (music_number < 10) {
            sprintf(value, "00%d", music_number);
        } else if (music_number < 100) {
            sprintf(value, "0%d", music_number);
        } else {
            sprintf(value, "%d", music_number);
        }
        graphLib.clear_area(config_text_pos_x, config_text_pos_y-1, RES_W, RES_H, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        graphLib.draw_text(config_text_pos_x, config_text_pos_y, std::string("MUSIC PLAYER"));
        graphLib.draw_text(config_text_pos_x, config_text_pos_y+24, "< ");
        graphLib.draw_text(config_text_pos_x+12, config_text_pos_y+24, std::string(value));
        graphLib.draw_text(config_text_pos_x+34, config_text_pos_y+24, " >");
        graphLib.draw_text(config_text_pos_x+62, config_text_pos_y+24, std::string(stages_data.stages[music_number].name));

        graphLib.draw_text(config_text_pos_x, config_text_pos_y+48, std::string("(A) PLAY"));
        graphLib.draw_text(config_text_pos_x, config_text_pos_y+60, std::string("(B) LEAVE"));
        if (input.p1_input[BTN_ATTACK] == 1 || input.p1_input[BTN_QUIT] == 1) {
            break;
        } else if (input.p1_input[BTN_JUMP] == 1 || input.p1_input[BTN_START] == 1) {
            soundManager.stop_music();
            soundManager.load_stage_music(stages_data.stages[music_number].bgmusic_filename);
            soundManager.play_music();
        } else if (input.p1_input[BTN_LEFT] == 1) {
            if (music_number == 0) {
                music_number = max;
            } else {
                music_number--;
            }
        } else if (input.p1_input[BTN_RIGHT] == 1) {
            music_number++;
        }
        if (music_number > max) {
            music_number = INTRO_STAGE;
        }
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
    graphLib.clear_area(config_text_pos_x, config_text_pos_y-1, RES_W, RES_H, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
}

void game_menu::sfx_player()
{
    Uint8 sfx_number = 0;
    CURRENT_FILE_FORMAT::file_stages stages_data;
    CURRENT_FILE_FORMAT::file_io fio;
    fio.read_all_stages(stages_data);

    int config_text_pos_x = graphLib.get_config_menu_pos().x + 24;
    int config_text_pos_y = graphLib.get_config_menu_pos().y + 40;
    graphLib.clear_area(config_text_pos_x-1, config_text_pos_y-1, 300, 100, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    input.clean();
    timer.delay(10);
    char value[3]; // for now, we handle only 0-999

    int max = SFX_GOT_ENERGY_BIG;

    soundManager.stop_music();

    while (true) {
        input.read_input();

        if (sfx_number < 10) {
            sprintf(value, "00%d", sfx_number);
        } else if (sfx_number < 100) {
            sprintf(value, "0%d", sfx_number);
        } else {
            sprintf(value, "%d", sfx_number);
        }
        graphLib.clear_area(config_text_pos_x, config_text_pos_y-1, RES_W, RES_H, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        graphLib.draw_text(config_text_pos_x, config_text_pos_y, std::string("SFX PLAYER"));
        graphLib.draw_text(config_text_pos_x, config_text_pos_y+24, "< ");
        graphLib.draw_text(config_text_pos_x+12, config_text_pos_y+24, std::string(value));
        graphLib.draw_text(config_text_pos_x+34, config_text_pos_y+24, " >");

        graphLib.draw_text(config_text_pos_x, config_text_pos_y+48, std::string("(A) PLAY"));
        graphLib.draw_text(config_text_pos_x, config_text_pos_y+60, std::string("(B) LEAVE"));
        if (input.p1_input[BTN_ATTACK] == 1 || input.p1_input[BTN_QUIT] == 1) {
            break;
        } else if (input.p1_input[BTN_JUMP] == 1 || input.p1_input[BTN_START] == 1) {
            soundManager.play_sfx(sfx_number);
        } else if (input.p1_input[BTN_LEFT] == 1) {
            if (sfx_number == 0) {
                sfx_number = max;
            } else {
                sfx_number--;
            }
        } else if (input.p1_input[BTN_RIGHT] == 1) {
            sfx_number++;
        }
        if (sfx_number > max) {
            sfx_number = INTRO_STAGE;
        }
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
    graphLib.clear_area(config_text_pos_x, config_text_pos_y-1, RES_W, RES_H, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    soundManager.play_music();
}
