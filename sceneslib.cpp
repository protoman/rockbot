#include <cstdlib>
#include "sceneslib.h"
#include "soundlib.h"
#include "file/fio_scenes.h"
#include "strings_map.h"

#include "game_mediator.h"

extern string FILEPATH;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;

#include "graphic/gfx_sin_wave.h"


#include "stage_select.h"
#include "graphic/option_picker.h"
#include "file/version.h"
#include "file/file_io.h"
#include "file/fio_strings.h"

#include "options/key_map.h"

#include "graphic/animation.h"

#include "docs/game_manual.h"

#include "game.h"
extern game gameControl;

#ifdef ANDROID
#include <android/log.h>
#include "ports/android/android_game_services.h"
extern android_game_services game_services;
#endif


#include "scenes/sceneshow.h"

extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_io fio;

extern timerLib timer;

extern bool leave_game;

#include "aux_tools/fps_control.h"
extern fps_control fps_manager;

extern bool GAME_FLAGS[FLAG_COUNT];

#define TIME_SHORT 120
#define TIME_LONG 300
#define INTRO_DIALOG_DURATION_TIME 4000

// ********************************************************************************************** //
// ScenesLib handles all scinematic like intro and ending                                         //
// ********************************************************************************************** //
scenesLib::scenesLib() : _timer(0), _state(0)
{
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void scenesLib::preloadScenes()
{
    game_scenes_map = fio_scn.load_game_scenes();
    soundManager.load_boss_music(game_data.boss_music_filename);
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void scenesLib::unload_stage_select() {
	int i;

	for (i=0; i<STAGE_SELECT_COUNT; i++) {
        if (STAGE_SELECT_SURFACES[i].get_surface()) {
            SDL_FreeSurface(STAGE_SELECT_SURFACES[i].get_surface());
            delete STAGE_SELECT_SURFACES[i].get_surface();
		}
    }
}




void scenesLib::draw_main()
{
    graphLib.blank_screen(CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
    draw_lib.update_screen();

	// PARTE 1 - TITLE SCREEN
    graphicsLib_gSurface intro_screen;
    std::string intro_path = FILEPATH + "/images/logo.png";
    graphLib.surfaceFromFile(intro_path, &intro_screen);
    //graphLib.copyArea(st_position(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20), &intro_screen, &graphLib.gameScreen);
    //graphLib.zoom_image(intro_screen, false);
    gfx_sin_wave gfx_wave_obj(&intro_screen);
    gfx_wave_obj.show(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20);


    graphLib.draw_text(8, 8, VERSION_NUMBER);

    if (gameControl.is_free_version() == true) {
        graphLib.draw_text(RES_W-12*9, 8, "FREE VERSION", st_color(255, 130, 0)); // 12 chars, font-spacing 9
    } else {
        graphLib.draw_text(RES_W-12*9, 8, "FULL VERSION", st_color(255, 130, 0)); // 12 chars, font-spacing 9
    }
    graphLib.draw_text(40-graphLib.RES_DIFF_W, (RES_H-35), strings_map::get_instance()->get_ingame_string(strings_ingame_copyrightline, game_config.selected_language));
    graphLib.draw_centered_text(220, "http://rockbot.upperland.net");

}

// ********************************************************************************************** //
// mostra tela de introdução, até alguém apertar start/enter
// a partir daí, mostra tela de seleção de personagem
// ********************************************************************************************** //
void scenesLib::main_screen()
{

    leave_game = false;
    input.clean();
    timer.delay(100);
    soundManager.stop_music();
    soundManager.load_music(game_data.game_start_screen_music_filename);
    soundManager.play_music();
	draw_main();

    std::vector<st_menu_option> options;
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_newgame, game_config.selected_language)));
    if (fio.have_one_save_file() == true) {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame, game_config.selected_language)));
    } else {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame, game_config.selected_language), true));
    }
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config, game_config.selected_language)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_manual, game_config.selected_language)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_about, game_config.selected_language)));

    option_picker main_picker(false, st_position(20, (RES_H*0.5)-graphLib.RES_DIFF_H), options, false);


    main_picker.enable_check_input_reset_command();
    main_picker.enable_check_input_cheat_command();


    bool have_save = fio.have_one_save_file();

    // IF HAVE NO SAVE, TRY TO LOAD IT FROM CLOUD //
#ifdef ANDROID
    // if config player services is set, and no save is found, get it from cloud
    /*
    if (have_save == false && game_config.android_use_play_services == true && game_config.android_use_cloud_save == true) {
        gameControl.load_save_data_from_cloud();
        have_save = fio.have_one_save_file();
    }
    */
#endif



    int picked_n = 0;

    if (have_save) {
        picked_n = 1;
    }
	bool repeat_menu = true;
	while (repeat_menu == true) {

        picked_n = main_picker.pick(picked_n);
		if (picked_n == -1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        } else if (picked_n == 0) { // NEW GAME
            short selected_save = select_save(true);
            if (selected_save != -1) {
                repeat_menu = false;
                gameControl.set_current_stage(INTRO_STAGE);
                gameControl.set_current_save_slot(selected_save);
                game_save.reset();
            } else {
                input.clean_all();
                draw_main();
                main_picker.draw();
            }
        } else if (picked_n == 1) { // LOAD GAME
            short selected_save = select_save(true);
            if (selected_save != -1) {
                gameControl.set_current_save_slot(selected_save);
                if (have_save == true) {
                    fio.read_save(game_save, gameControl.get_current_save_slot());
                    repeat_menu = false;
                }
            } else {
                input.clean_all();
                draw_main();
                main_picker.draw();
            }
        } else if (picked_n == 2) {
            show_main_config(0, false);
			draw_main();
			main_picker.draw();
        } else if (picked_n == 3) {
            game_manual manual;
            manual.execute();
            draw_main();
            main_picker.draw();
        } else if (picked_n == 4) {
            // only wait for keypress if user did not interrupted credits
            if (draw_lib.show_credits(true) == 0) {
                input.wait_keypress();
            }
            draw_main();
            main_picker.draw();
        } else if (picked_n == 5) {
            show_cheats_menu();
            draw_main();
            main_picker.draw();
        } else if (picked_n == MAIN_MENU_CHEAT_RETURN) {
            picked_n = 5;
            main_picker.add_option_item(st_menu_option("CHEATS"));
            draw_main();
            main_picker.draw();
        }
	}
    draw_lib.update_screen();

    if (picked_n == 0) { // NEW GAME //
        game_save.difficulty = select_difficulty();
        std::cout << "game_save.difficulty[" << (int)game_save.difficulty << "]" << std::endl;
        // demo do not have player selection, only rockbot is playable
        if (gameControl.is_free_version() == false) {
                game_save.selected_player = select_player();
        } else {
                game_save.selected_player = PLAYER_1;
        }
        gameControl.save_game();
    }
}

void scenesLib::show_cheats_menu()
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
    while (selected_option != -1) {

        options.clear();

        std::string invencibleStr = "OFF";
        if (GAME_FLAGS[FLAG_INVENCIBLE]) {
            invencibleStr = "ON";
        }
        options.push_back(st_menu_option("INVENCIBLE: " + invencibleStr));

        std::string allStagesStr = "OFF";
        if (GAME_FLAGS[FLAG_ALLWEAPONS]) {
            allStagesStr = "ON";
        }
        options.push_back(st_menu_option("ALL BEATEN: " + allStagesStr));

        char char_n[2];
        sprintf(char_n, "%d", game_save.selected_player);
        options.push_back(st_menu_option("CHARACTER: " + std::string(char_n)));

        option_picker cheat_config_picker(false, config_text_pos, options, true);
        cheat_config_picker.draw();
        selected_option = cheat_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            GAME_FLAGS[FLAG_INVENCIBLE] = !GAME_FLAGS[FLAG_INVENCIBLE];
        } else if (selected_option == 1) {
            GAME_FLAGS[FLAG_ALLWEAPONS] = !GAME_FLAGS[FLAG_ALLWEAPONS];
        } else if (selected_option == 2) {
            game_save.selected_player++;
            if (game_save.selected_player >= 4) {
                game_save.selected_player = 0;
            }
        }
    }


}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
short scenesLib::pick_stage(int last_stage) {
	graphLib.blank_screen();
	stage_select selection(STAGE_SELECT_SURFACES);

    int pos_n = selection.pick_stage(last_stage);

    timer.delay(100);
    return pos_n;
}


short scenesLib::show_main_config(short stage_finished, bool called_from_game) // returns 1 if must leave stage, 0 if nothing
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
        if (selected_option == 0) { // CONFIG AUDIO
			show_config_audio();
        } else if (selected_option == 1) { // CONFIG INPUT
            key_map key_mapper;
            key_mapper.config_input();
        } else if (selected_option == 2) { // CONFIG PLATFORM
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
        } else if (selected_option == 3) { // LANGUAGE
            show_config_language();
        } else if (selected_option == 4) { // PERFORMANCE
            show_config_performance();
        } else if (selected_option == 5) { // LEAVE STAGE
            res = 1;
            break;
        } else if (selected_option == 6) { // QUIT GAME
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



void scenesLib::game_scenes_show_unbeaten_intro()
{
    sceneShow show;
    show.show_scene(game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_UNBEATEN]);
    //std::cout << "game_scenes_show_unbeaten_intro::DONE";
}

void scenesLib::show_game_scene(e_game_scenes_types n)
{
    sceneShow show;
    show.show_scene(game_scenes_map[n]);
}

void scenesLib::show_player_ending()
{
    switch (game_save.selected_player) {
        case PLAYER_1:
            show_game_scene(GAME_SCENE_TYPES_ENDING_PLAYER1);
            break;
        case PLAYER_2:
            show_game_scene(GAME_SCENE_TYPES_ENDING_PLAYER2);
            break;
        case PLAYER_3:
            show_game_scene(GAME_SCENE_TYPES_ENDING_PLAYER3);
            break;
        case PLAYER_4:
            show_game_scene(GAME_SCENE_TYPES_ENDING_PLAYER4);
            break;
        default:
            break;
    }
}

void scenesLib::show_player_walking_ending()
{
    float bg1_speed = 1.1;
    float bg2_speed = 0.5;
    float bg3_speed = 0.1;
    float bg1_pos = 0;
    float bg2_pos = 0;
    float bg3_pos = 0;
    float player_speed = -0.2;
    float player_pos = RES_W + TILESIZE;

    graphLib.blank_screen();
    std::string filename = FILEPATH + "images/ending/player_walking_layer1.png";
    graphicsLib_gSurface bg1;
    graphLib.surfaceFromFile(filename, &bg1);

    filename = FILEPATH + "images/ending/player_walking_layer2.png";
    graphicsLib_gSurface bg2;
    graphLib.surfaceFromFile(filename, &bg2);

    filename = FILEPATH + "images/ending/player_walking_layer3.png";
    graphicsLib_gSurface bg3;
    graphLib.surfaceFromFile(filename, &bg3);

    int bg3_y_pos = 10;
    int bg1_y_pos = bg3.height + bg3_y_pos - bg1.height;
    int bg2_y_pos = bg1_y_pos - bg2.height;

    gameControl.set_player_direction(ANIM_DIRECTION_LEFT);
    gameControl.set_player_anim_type(ANIM_TYPE_WALK);
    int player_y_pos = bg1_y_pos - gameControl.get_player_size().height;


    while (player_pos > -TILESIZE*3) {


        bg1_pos += bg1_speed;
        bg2_pos += bg2_speed;
        bg3_pos += bg3_speed;
        player_pos += player_speed;
        if (bg1_pos >= RES_W) {
            bg1_pos = 0;
        }
        if (bg2_pos >= RES_W) {
            bg2_pos = 0;
        }
        if (bg3_pos >= RES_W) {
            bg3_pos = 0;
        }

        // draw part1
        graphLib.showSurfaceAt(&bg3, st_position(bg3_pos, bg3_y_pos), false);
        graphLib.showSurfaceRegionAt(&bg3, st_rectangle(RES_W-bg3_pos, 0, bg3_pos, bg3.height), st_position(0, bg3_y_pos));
        graphLib.showSurfaceAt(&bg2, st_position(bg2_pos, bg2_y_pos), false);
        graphLib.showSurfaceRegionAt(&bg2, st_rectangle(RES_W-bg2_pos, 0, bg2_pos, bg2.height), st_position(0, bg2_y_pos));
        graphLib.showSurfaceAt(&bg1, st_position(bg1_pos, bg1_y_pos), false);
        graphLib.showSurfaceRegionAt(&bg1, st_rectangle(bg1.width-bg1_pos, 0, bg1_pos, bg1.height), st_position(0, bg1_y_pos));
        // draw second part (left)



        // show player
        gameControl.show_player_at(player_pos, player_y_pos);


        //std::cout << "bg1_pos[" << bg1_pos << "], bg1.left[" << bg1.width-bg1_pos << "]" << std::endl;

        graphLib.updateScreen();
        timer.delay(10);
    }
    graphLib.blank_screen();
    graphLib.updateScreen();
    timer.delay(1000);

}

// show all enemies from the game //
void scenesLib::show_enemies_ending()
{
    st_color font_color(250, 250, 250);
    std::pair<int, std::string> final_boss_data = std::pair<int, std::string>(-1, "");

    graphLib.blank_screen();
    draw_lib.show_boss_intro_bg();

    soundManager.stop_music();
    if (fio.file_exists(FILEPATH + "/music/ending_bosses.mod")) {
        soundManager.load_music("ending_bosses.mod");
    } else {
        soundManager.load_shared_music("ending_bosses.mod");
    }
    soundManager.play_music();

    // get all stage bosses
    std::map<int, std::string> stage_boss_id_list;
    for (int i=0; i<FS_MAX_STAGES; i++) {
        CURRENT_FILE_FORMAT::file_stage stage_data_obj;
        fio.read_stage(stage_data_obj, i);
        int boss_id = stage_data_obj.boss.id_npc;
        if (boss_id != -1) {
            std::pair<int, std::string> temp_data(boss_id, std::string(stage_data_obj.boss.name));
            stage_boss_id_list.insert(temp_data);
        }
    }

    for (int i=0; i<GameMediator::get_instance()->get_enemy_list_size(); i++) {
        if (GameMediator::get_instance()->get_enemy(i)->id == -1) {
            continue;
        }

        if (stage_boss_id_list.find(GameMediator::get_instance()->get_enemy(i)->id) != stage_boss_id_list.end()) {
            continue;
        }
        // final boss
        std::string name = std::string(GameMediator::get_instance()->get_enemy(i)->name);

        if (GameMediator::get_instance()->get_enemy(i)->id == game_data.final_boss_id) {
            final_boss_data = std::pair<int, std::string>(GameMediator::get_instance()->get_enemy(i)->id, name);
        } else {
            ending_show_single_enemy(GameMediator::get_instance()->get_enemy(i)->id, name);
        }

    }
    show_bosses_ending();
    if (final_boss_data.first != -1) {
        ending_show_single_enemy(final_boss_data.first, final_boss_data.second);
    }

    graphLib.blank_screen();
    graphLib.updateScreen();
    soundManager.stop_music();
    timer.delay(2000);


}

void scenesLib::ending_show_single_enemy(int id, std::string name)
{

    std::cout << "### SCENES::show_enemies_ending [enemy[" << id << "].name[" << name << "]" << std::endl;
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "### SCENES::show_enemies_ending [enemy[%d].name[%s] ###", id, name.c_str());
#endif

    int w = GameMediator::get_instance()->get_enemy(id)->frame_size.width;
    int h = GameMediator::get_instance()->get_enemy(id)->frame_size.height;

    std::string temp_filename = FILEPATH + "images/sprites/enemies/" + GameMediator::get_instance()->get_enemy(id)->graphic_filename;
    graphicsLib_gSurface npc_sprite_surface;
    graphLib.surfaceFromFile(temp_filename, &npc_sprite_surface);

    draw_lib.show_boss_intro_bg();
    draw_lib.show_boss_intro_sprites(id, false);
    //graphLib.draw_progressive_text(120, RES_H/2, name, false);
    graphLib.draw_centered_text(BOSS_INTRO_BG_TEXT_Y, name);
    draw_lib.update_screen();
    timer.delay(3000);
}

void scenesLib::show_bosses_ending()
{
    graphLib.blank_screen();
    // read bosses strings
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<std::string> boss_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/boss_credits.txt");

    for (int i=0; i<CASTLE1_STAGE5; i++) {

        CURRENT_FILE_FORMAT::file_stage stage_data_obj;
        fio.read_stage(stage_data_obj, i);

        draw_lib.show_boss_intro_bg();
        graphLib.updateScreen();
        int boss_id = stage_data_obj.boss.id_npc;

        // 40y, 111h
        draw_lib.show_boss_intro_sprites(boss_id, false);
        graphLib.draw_centered_text(170, boss_credits_data.at(i*3));
        draw_lib.update_screen();
        graphLib.draw_centered_text(185, boss_credits_data.at(i*3+1));
        draw_lib.update_screen();
        graphLib.draw_centered_text(201, boss_credits_data.at(i*3+2));
        draw_lib.update_screen();
        //graphLib.draw_progressive_text(130, 164, boss_credits_data.at(i*3), false, 20);
        //graphLib.draw_progressive_text(130, 175, boss_credits_data.at(i*3+1), false, 20);
        //graphLib.draw_progressive_text(130, 186, boss_credits_data.at(i*3+2), false, 20);
        timer.delay(2000);
    }


}




void scenesLib::show_config_android()
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
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language), false));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled,game_config.selected_language), false));
        }
        // OPTION #3: use cloud save (only available if use play services is true)
        if (game_config.android_use_cloud_save == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_usecloudsave, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled, game_config.selected_language), false));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_usecloudsave, game_config.selected_language) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled, game_config.selected_language), false));
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


void scenesLib::show_config_video()
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

void scenesLib::show_config_video_PSP()
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

void scenesLib::show_config_wii()
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

void scenesLib::show_config_PS2()
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

void scenesLib::show_config_ask_restart()
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

void scenesLib::show_config_audio()
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

void scenesLib::show_config_language()
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

void scenesLib::show_config_performance()
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

void scenesLib::show_config_warning_android_play_services()
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

void scenesLib::show_config_warning_android_cloud_save()
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

void scenesLib::show_config_warning_android_hide_controls()
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

void scenesLib::config_int_value(Uint8 &value_ref, int min, int max)
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



void scenesLib::draw_lights_select_player(graphicsLib_gSurface& lights, int selected, int adjustX, int adjustY) {
	int posX, invPosX;

	invPosX = 0;

	if (_timer < timer.getTimer()) {
		_timer = timer.getTimer()+200;
		if (_state == 0) {
			_state = 1;
		} else {
			_state = 0;
		}
	}
	if (_state != 0) {
		posX = 6;
	} else {
		posX = 0;
	}
    int XPos[4];
    XPos[0] = 2;
    XPos[1] = 88;
    XPos[2] = 162;
    XPos[3] = 248;
    int YPos[4];
    YPos[0] = 2;
    YPos[1] = 88;
    YPos[2] = 114;
    YPos[3] = 200;

    // erase previous position
    for (int i=0; i<2; i++) {
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i+1], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i+1], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i+1], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(invPosX, 0, lights.height, lights.height), st_position(adjustX+XPos[i+1], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
    }

    if (selected == PLAYER_1) {
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[0], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[0], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[1], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[1], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
    } else if (selected == PLAYER_2) {
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[2], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[2], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[3], adjustY+YPos[0]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[3], adjustY+YPos[1]), &lights, &graphLib.gameScreen);
    } else if (selected == PLAYER_3) {
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[0], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[0], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[1], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[1], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
    } else if (selected == PLAYER_4) {
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[2], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[2], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[3], adjustY+YPos[2]), &lights, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(posX, 0, lights.height, lights.height), st_position(adjustX+XPos[3], adjustY+YPos[3]), &lights, &graphLib.gameScreen);
    }
    draw_lib.update_screen();
}


Uint8 scenesLib::select_player() {
    int selected = 1;
    graphicsLib_gSurface bg_surface;

    int max_loop = 2;
    if (game_config.game_finished == true) {
        max_loop = 4;
    }


    graphLib.blank_screen();
    std::string filename = FILEPATH + "images/backgrounds/player_selection.png";
    graphLib.surfaceFromFile(filename, &bg_surface);

    filename = FILEPATH + "images/backgrounds/player_select_p1.png";
    graphicsLib_gSurface p1_surface;
    graphLib.surfaceFromFile(filename, &p1_surface);

    filename = FILEPATH + "images/backgrounds/player_select_p2.png";
    graphicsLib_gSurface p2_surface;
    graphLib.surfaceFromFile(filename, &p2_surface);

    filename = FILEPATH + "images/backgrounds/player_select_p3.png";
    graphicsLib_gSurface p3_surface;
    graphLib.surfaceFromFile(filename, &p3_surface);

    filename = FILEPATH + "images/backgrounds/player_select_p4.png";
    graphicsLib_gSurface p4_surface;
    graphLib.surfaceFromFile(filename, &p4_surface);

    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(strings_ingame_config_select_player, game_config.selected_language));
    graphLib.draw_centered_text(176, GameMediator::get_instance()->player_list_v3_1[0].name);
    graphLib.draw_centered_text(217, strings_map::get_instance()->get_ingame_string(strings_ingame_config_press_start_to_select, game_config.selected_language));
    graphLib.copyArea(st_position(0, 50), &p1_surface, &graphLib.gameScreen);
    draw_lib.update_screen();


    input.clean();
    timer.delay(100);

    while (true) {
        input.read_input();
        if (input.p1_input[BTN_LEFT] == 1 || input.p1_input[BTN_RIGHT] == 1) {
            soundManager.play_sfx(SFX_CURSOR);
            if (input.p1_input[BTN_RIGHT] == 1) {
                selected++;
            } else {
                selected--;
            }
            // adjust selected/loop
            if (selected < 1) {
                selected = max_loop;
            } else if (selected > max_loop) {
                selected = 1;
            }
            graphLib.clear_area(0, 49, RES_W, 96, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
            if (selected == 1) {
                graphLib.copyArea(st_position(0, 50), &p1_surface, &graphLib.gameScreen);
            } else if (selected == 2) {
                graphLib.copyArea(st_position(0, 50), &p2_surface, &graphLib.gameScreen);
            } else if (selected == 3) {
                graphLib.copyArea(st_position(0, 50), &p3_surface, &graphLib.gameScreen);
            } else if (selected == 4) {
                graphLib.copyArea(st_position(0, 50), &p4_surface, &graphLib.gameScreen);
            }
            graphLib.clear_area(60, 168, RES_W, 18, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
            graphLib.draw_centered_text(176, GameMediator::get_instance()->player_list_v3_1[selected-1].name);
        } else if (input.p1_input[BTN_QUIT] == 1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        } else if (input.p1_input[BTN_START] == 1) {
            input.clean();
            draw_lib.update_screen();
            timer.delay(80);
            break;
        }
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
    return (selected-1);
}


Uint8 scenesLib::select_difficulty()
{
    short res = 0;
    st_position config_text_pos;
    std::vector<std::string> options;

    graphLib.show_config_bg();
    draw_lib.update_screen();
    input.clean();
    timer.delay(300);

    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_easy, game_config.selected_language));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_normal, game_config.selected_language));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_hard, game_config.selected_language));

    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 60;

    graphLib.draw_text(config_text_pos.x, graphLib.get_config_menu_pos().y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_select, game_config.selected_language).c_str());

    short selected_option = -2;
    while (selected_option == -2) {
        option_picker main_config_picker(false, config_text_pos, options, false);
        selected_option = main_config_picker.pick();
        if (selected_option == -1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        } else {
            res = selected_option;
        }
        std::cout << "############ select_difficulty.selected_option[" << selected_option << "]" << std::endl;
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        draw_lib.update_screen();
    }
    return res;
}



void scenesLib::boss_intro(Uint8 pos_n)
{
    if (pos_n >= CASTLE1_STAGE1) {
        show_castle_boss_intro(pos_n);
        return;
    }
    if (pos_n == INTRO_STAGE) {
        return;
    }

    std::string botname = GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc)->name;


    graphicsLib_gSurface boss_img;
    char boss_filename_chr[12];
    sprintf(boss_filename_chr, "0%d.png", pos_n);
    std::string boss_filename = FILEPATH + "images/backgrounds/boss_intro/" + std::string(boss_filename_chr);
    graphLib.surfaceFromFile(boss_filename, &boss_img);

    int init_pos = -boss_img.width;
    int end_pos = RES_W-boss_img.width;
    int pos_y = 151-boss_img.height;

    soundManager.play_sfx(SFX_STAGE_SELECTED);
    graphLib.blank_screen();
    draw_lib.show_boss_intro_bg();
    draw_lib.update_screen();

    for (int i=init_pos; i<=end_pos; i++) {
        graphLib.clear_area(0, 34, RES_W, 117, 13, 15, 97);
        graphLib.copyArea(st_position(i, pos_y), &boss_img, &graphLib.gameScreen);
        draw_lib.update_screen();
        timer.delay(5);

    }

    CURRENT_FILE_FORMAT::file_stage temp_stage_data;
    fio.read_stage(temp_stage_data, pos_n);
    std::string full_stage_str = botname + " [" + std::string(temp_stage_data.name) + "]";
    // convert name to uppercase
    std::locale settings;
    std::string boss_name;
    int text_x = RES_W-(full_stage_str.length()*8);
    text_x = text_x/2;

    std::cout << "full_stage_str.length()[" << full_stage_str.length() << "], calc.size[" << (full_stage_str.length()*8) << "], text_x[" << text_x << "]" << std::endl;

    for(unsigned int i = 0; i < full_stage_str.size(); ++i) {
        boss_name += (std::toupper(full_stage_str[i], settings));
        graphLib.draw_text(text_x, BOSS_INTRO_BG_TEXT_Y, boss_name);
        draw_lib.update_screen();
        timer.delay(100);
    }
    graphLib.wait_and_update_screen(2500);

}

void scenesLib::show_castle_boss_intro(Uint8 pos_n)
{
    // @TODO: create new intro for castle stages or simply do the same as regular stages
}


short scenesLib::select_save(bool is_new_game)
{
    int selected = 0;
    bool finished = false;


    std::string filename_selector_enabled = FILEPATH + "images/backgrounds/save_selector_enabled.png";
    graphicsLib_gSurface selector_enabled_bg;
    graphLib.surfaceFromFile(filename_selector_enabled, &selector_enabled_bg);

    std::string filename_selector_disabled = FILEPATH + "images/backgrounds/save_selector_disabled.png";
    graphicsLib_gSurface selector_disabled_bg;
    graphLib.surfaceFromFile(filename_selector_disabled, &selector_disabled_bg);

    graphLib.blank_screen();
    input.clean_all();
    timer.delay(300);

    CURRENT_FILE_FORMAT::st_save save_detail_array[5];
    bool save_slot_exists[5];
    for (int i=0; i<5; i++) {
        if (fio.save_exists(i) == false) {
            save_slot_exists[i] = false;
        } else {
            fio.read_save(save_detail_array[i], i);
            save_slot_exists[i] = true;
        }
    }

    if (is_new_game == true) {
        graphLib.draw_text(10, 10, "CREATE NEW GAME");
    } else {
        graphLib.draw_text(10, 10, "LOAD GAME FILE");
    }
    graphLib.draw_text(10, RES_H-12, "PLEASE SELECT SAVE SLOT");

    while (finished == false) {
        // draw screen
        for (int i=0; i<5; i++) {
            graphicsLib_gSurface* surface_ref = &selector_disabled_bg;
            if (i == selected) {
                surface_ref = &selector_enabled_bg;
            }
            graphLib.copyArea(st_position(0, (i*surface_ref->height)+22), surface_ref, &graphLib.gameScreen);
        }
        for (int i=0; i<5; i++) {
            if (save_slot_exists[i] == true) {
                draw_save_details(i, save_detail_array[i]);
            } else {
                graphLib.draw_text(10, i*40+34, "- NO SAVE FILE -");
            }
        }

        graphLib.updateScreen();
        input.read_input();
        if (input.p1_input[BTN_JUMP] == 1 || input.p1_input[BTN_START] == 1) {
            if (is_new_game == false && save_slot_exists[selected] == false) {
                soundManager.play_sfx(SFX_NPC_HIT);
            } else {
                break;
            }
        } else if (input.p1_input[BTN_ATTACK] == 1) {
            return -1;
        } else if (input.p1_input[BTN_UP] == 1) {
            selected--;
            soundManager.play_sfx(SFX_CURSOR);
            if (selected < 0) {
                selected = SAVE_MAX_SLOT_NUMBER;
            }
        } else if (input.p1_input[BTN_DOWN] == 1) {
            selected++;
            soundManager.play_sfx(SFX_CURSOR);
            if (selected > SAVE_MAX_SLOT_NUMBER) {
                selected = 0;
            }
        } else if (input.p1_input[BTN_QUIT] == 1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        }
        input.clean();
        timer.delay(10);
    }

    return selected;

}

void scenesLib::draw_save_details(int n, CURRENT_FILE_FORMAT::st_save save)
{
    // intro stage is rock buster icon, other are weapons icons
    int y_pos = n*40+34;
    for (int i=0; i<=8; i++) {
        st_position pos((i+1)*18, y_pos);
        if (save.stages[i] == 1) {
            graphLib.draw_weapon_tooltip_icon(i, pos, true);
        } else {
            // @TODO: draw disabled version
            graphLib.draw_weapon_tooltip_icon(i, pos, false);
        }
    }
    // lifes
    st_position pos_lifes(9*18, y_pos);
    graphLib.draw_weapon_tooltip_icon(11+save.selected_player, pos_lifes, true);
    char buffer[3];
    sprintf(buffer, "x%d", save.items.lifes);
    graphLib.draw_text(10*18, y_pos+5, std::string(buffer));

    // e-tank
    st_position pos_etank(11*18, y_pos);
    graphLib.draw_weapon_tooltip_icon(15, pos_etank, true);
    sprintf(buffer, "x%d", save.items.energy_tanks);
    graphLib.draw_text(12*18, y_pos+5, std::string(buffer));

    // w-tank
    st_position pos_wtank(13*18, y_pos);
    graphLib.draw_weapon_tooltip_icon(16, pos_wtank, true);
    sprintf(buffer, "x%d", save.items.weapon_tanks);
    graphLib.draw_text(14*18, y_pos+5, std::string(buffer));

    // s-tank
    st_position pos_stank(15*18, y_pos);
    graphLib.draw_weapon_tooltip_icon(17, pos_stank, true);
    sprintf(buffer, "x%d", save.items.special_tanks);
    graphLib.draw_text(16*18, y_pos+5, std::string(buffer));
}

