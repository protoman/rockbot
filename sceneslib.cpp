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
	graphLib.blank_screen();
    draw_lib.update_screen();

	// PARTE 1 - TITLE SCREEN
    graphicsLib_gSurface intro_screen;
    std::string intro_path = FILEPATH + "/images/logo.png";
    graphLib.surfaceFromFile(intro_path, &intro_screen);
    //graphLib.copyArea(st_position(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20), &intro_screen, &graphLib.gameScreen);
    gfx_sin_wave gfx_wave_obj(&intro_screen);
    gfx_wave_obj.show(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20);


    graphLib.draw_text(8, 8, VERSION_NUMBER);

if (gameControl.is_free_version() == true) {
    graphLib.draw_text(RES_W-12*9, 8, "FREE VERSION", st_color(255, 130, 0)); // 12 chars, font-spacing 9
} else {
    graphLib.draw_text(RES_W-12*9, 8, "FULL VERSION", st_color(255, 130, 0)); // 12 chars, font-spacing 9
}
    graphLib.draw_text(40-graphLib.RES_DIFF_W, (RES_H-35), strings_map::get_instance()->get_ingame_string(strings_ingame_copyrightline));
    graphLib.draw_centered_text(220, "HTTP://ROCKBOT.UPPERLAND.NET", st_color(240, 240, 240));

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
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_newgame)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_manual)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_about)));

    option_picker main_picker(false, st_position(40-graphLib.RES_DIFF_W, (RES_H*0.5)-graphLib.RES_DIFF_H), options, false);


    main_picker.enable_check_input_reset_command();


    int picked_n = 1;

    bool have_save = fio.save_exists();
    int initial_pick_pos = 0;

    if (have_save) {
        initial_pick_pos = 1;
    }
	bool repeat_menu = true;
	while (repeat_menu == true) {

        picked_n = main_picker.pick(initial_pick_pos);
		if (picked_n == -1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        } else if (picked_n == 0) { // NEW GAME
			repeat_menu = false;
            game_save.reset();
        } else if (picked_n == 1) { // LOAD GAME
            if (have_save == true) {
				fio.read_save(game_save);
				repeat_menu = false;
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
        }
	}
    draw_lib.update_screen();

    if (picked_n == 0) {
        game_save.difficulty = select_difficulty();
        std::cout << "game_save.difficulty[" << (int)game_save.difficulty << "]" << std::endl;
        // demo do not have player selection, only rockbot is playable
        if (gameControl.is_free_version() == false) {
                game_save.selected_player = select_player();
        } else {
                game_save.selected_player = PLAYER_1;
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

    if (pos_n == CASTLE1_STAGE1) { // look for the actual stage in skullcastle
        for (int i=CASTLE1_STAGE1; i<CASTLE1_STAGE5; i++) {
            if (game_save.stages[i] == 0) {
                break;
            }
            pos_n = i+1;
        }
    }
    timer.delay(100);
    return pos_n;
}




short scenesLib::show_main_config(short stage_finished, bool called_from_game) // returns 1 if must leave stage, 0 if nothing
{
    short res = 0;
    st_position config_text_pos;
    std::vector<st_menu_option> options;

	graphLib.show_config_bg(0);
    draw_lib.update_screen();
	input.clean();
    timer.delay(300);

    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_audio)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_input)));
#if defined(PC)
    options.push_back(st_menu_option("PC"));
#elif PSP
    options.push_back(st_menu_option("PSP"));
#elif WII
    options.push_back(st_menu_option("WII"));
#elif ANDROID
    options.push_back(st_menu_option("ANDROID"));
#else
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_wii_platformspecific), true));
#endif
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_language), true));

    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config_graphics_performance)));


    if (called_from_game) {
        if (stage_finished) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage), true));
        }
        options.push_back( strings_map::get_instance()->get_ingame_string(strings_ingame_config_quitgame));
    }



	config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
	config_text_pos.y = graphLib.get_config_menu_pos().y + 40;

	short selected_option = 0;
	while (selected_option != -1) {
        option_picker main_config_picker(false, config_text_pos, options, true);
		selected_option = main_config_picker.pick();
		if (selected_option == -1) {
			break;
		}
		graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, 180,  180, 0, 0, 0);
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

		graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, 180,  180, 0, 0, 0);
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
    show_player_walking_ending();
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
        gameControl.show_player_at(player_pos,player_y_pos);


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
    graphicsLib_gSurface bg_surface;
    st_color font_color(250, 250, 250);

    graphLib.blank_screen();
    std::string filename = FILEPATH + "images/backgrounds/stage_boss_intro.png";
    graphLib.surfaceFromFile(filename, &bg_surface);

    soundManager.stop_music();
    if (fio.file_exists(FILEPATH + "/music/ending_bosses.mod")) {
        soundManager.load_music("ending_bosses.mod");
    } else {
        soundManager.load_shared_music("ending_bosses.mod");
    }
    soundManager.play_music();

    // get all stage bosses
    CURRENT_FILE_FORMAT::file_stages stage_data;
    fio.read_all_stages(stage_data);
    std::map<int, std::string> stage_boss_id_list;
    for (int i=0; i<FS_MAX_STAGES; i++) {
        std::pair<int, std::string> temp_data((int)stage_data.stages[i].boss.id_npc, std::string(stage_data.stages[i].boss.name));
        stage_boss_id_list.insert(temp_data);
    }

    for (int i=0; i<GameMediator::get_instance()->get_enemy_list_size(); i++) {
        if (stage_boss_id_list.find(GameMediator::get_instance()->get_enemy(i)->id) != stage_boss_id_list.end()) {
            continue;
        }
        std::string name = GameMediator::get_instance()->get_enemy(i)->name;
        std::cout << "[enemy[" << i << "].name[" << name << "]" << std::endl;
        int w = GameMediator::get_instance()->get_enemy(i)->frame_size.width;
        int h = GameMediator::get_instance()->get_enemy(i)->frame_size.height;

        std::string temp_filename = FILEPATH + "images/sprites/enemies/" + GameMediator::get_instance()->get_enemy(i)->graphic_filename;
        graphicsLib_gSurface npc_sprite_surface;
        graphLib.surfaceFromFile(temp_filename, &npc_sprite_surface);


        graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
        //graphLib.copyArea(st_rectangle(0, 0, w, h), st_position(20, RES_H/2 - h/2), &npc_sprite_surface, &graphLib.gameScreen);
        draw_lib.show_boss_intro_sprites(i, false);
        graphLib.draw_progressive_text(120, RES_H/2, name, false);
        timer.delay(3000);
    }
    show_bosses_ending(bg_surface, stage_data);

    graphLib.blank_screen();
    graphLib.updateScreen();
    soundManager.stop_music();
    timer.delay(2000);


}

void scenesLib::show_bosses_ending(graphicsLib_gSurface& bg_surface, CURRENT_FILE_FORMAT::file_stages& stage_data)
{
    graphLib.blank_screen();
    // read bosses strings
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<std::string> boss_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/boss_credits.txt", LANGUAGE_ENGLISH);

    for (int i=0; i<CASTLE1_STAGE5; i++) {
        graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
        graphLib.updateScreen();
        draw_lib.show_boss_intro_sprites((int)stage_data.stages[i].boss.id_npc, false);
        graphLib.draw_progressive_text(130, RES_H/2-14, boss_credits_data.at(i*3), false, 20);
        graphLib.draw_progressive_text(130, RES_H/2-3, boss_credits_data.at(i*3+1), false, 20);
        graphLib.draw_progressive_text(130, RES_H/2+8, boss_credits_data.at(i*3+2), false, 20);
        timer.delay(2000);
    }


}




void scenesLib::show_config_android()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);
    short selected_option = 0;

    std::vector<st_menu_option> options;

    while (selected_option != -1) {
        options.clear();
        //graphLib.clear_area(menu_pos.x-14, menu_pos.y, 180,  180, 0, 0, 0);

        if (game_config.android_touch_controls_hide == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_hidescreencontrols) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_hidescreencontrols) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled)));
        }
        if (game_config.android_touch_controls_size == 0) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_SMALL)));
        } else if (game_config.android_touch_controls_size == 2) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_BIG)));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_config_android_screencontrolssize_MEDIUM)));
        }
        if (game_config.android_use_play_services == false) {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled),true));
        } else {
            options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_android_useplayservices) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled), true));
        }

        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick();

        if (selected_option == 0) {
            game_config.android_touch_controls_hide = !game_config.android_touch_controls_hide;
        } else if (selected_option == 1) {
            game_config.android_touch_controls_size++;
            if (game_config.android_touch_controls_size > 2) {
                game_config.android_touch_controls_size = 0;
            }
        } else if (selected_option == 2) {
            game_config.android_use_play_services = !game_config.android_use_play_services;
        }
    }
    fio.save_config(game_config);
}


void scenesLib::show_config_video()
{
	st_position config_text_pos;
	config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
	config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
	input.clean();
    timer.delay(300);
	std::vector<std::string> options;

    if (game_config.video_fullscreen == false) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed));
	} else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen));
	}

    if (game_config.video_filter == VIDEO_FILTER_NOSCALE) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_noscale));
    } else if (game_config.video_filter == VIDEO_FILTER_BITSCALE) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_size2x));
    } else if (game_config.video_filter == VIDEO_FILTER_SCALE2x) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale2x));
    }

	short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
	selected_option = main_config_picker.pick();
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
        st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);
        graphLib.clear_area(menu_pos.x-14, menu_pos.y, 195, 100, 0, 0, 0);
        show_config_video();
    }
}

void scenesLib::show_config_video_PSP()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    if (game_config.video_fullscreen == true) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed));
    } else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen));
    }
    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    if (selected_option == 0) {
        game_config.video_fullscreen = !game_config.video_fullscreen;
    }
    if (selected_option != -1) {
        fio.save_config(game_config);
        show_config_ask_restart();
    }
}

void scenesLib::show_config_wii()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_WIIMOTE));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_CLASSIC));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_config_wii_joysticktype_GAMECUBE));
    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    if (selected_option != -1) {
        game_config.wii_joystick_type = selected_option;
    }
}

void scenesLib::show_config_PS2()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
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
    selected_option = main_config_picker.pick();
    if (selected_option != -1) {
        game_config.playstation2_video_mode = selected_option;
        fio.save_config(game_config);
        show_config_ask_restart();
    }
}

void scenesLib::show_config_ask_restart()
{
    input.clean();
    timer.delay(300);
    st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);
    graphLib.clear_area(menu_pos.x, menu_pos.y, 180,  180, 0, 0, 0);
    graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1));
    graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2));
    graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3));
    graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart));
    draw_lib.update_screen();
    input.wait_keypress();
    graphLib.clear_area(menu_pos.x, menu_pos.y, 193,  180, 0, 0, 0);
    draw_lib.update_screen();
}

void scenesLib::show_config_audio()
{
	st_position config_text_pos;
	config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
	config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
	input.clean();
    timer.delay(300);

	std::vector<std::string> options;
    if (game_config.sound_enabled == true) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled));
    } else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled));
    }
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_music));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_sfx));


	short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
	selected_option = main_config_picker.pick();
	if (selected_option == 0) {
        if (game_config.sound_enabled == false) {
            soundManager.enable_sound();
        } else {
            soundManager.disable_sound();
        }
        show_config_audio();
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

void scenesLib::show_config_language()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    if (game_config.selected_language == 1) {           // FRENCH
        options.push_back("ANGLAIS");
        options.push_back("FRANCAIS");
        options.push_back("ESPANOL");
        options.push_back("ITALIEN");
    } else if (game_config.selected_language == 2) {    // SPANISH
        options.push_back("INGLES");
        options.push_back("FRANCES");
        options.push_back("ESPANOL");
        options.push_back("ITALIANO");
    } else if (game_config.selected_language == 3) {    // ITALIAN
        options.push_back("INGLESE");
        options.push_back("FRANCESE");
        options.push_back("SPAGNOLO");
        options.push_back("ITALIANO");
    } else {                                            // ENGLISH
        options.push_back("ENGLISH");
        options.push_back("FRENCH");
        options.push_back("SPANISH");
        options.push_back("ITALIAN");
    }

    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    game_config.selected_language = selected_option;
    fio.save_config(game_config);
}

void scenesLib::show_config_performance()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    std::vector<std::string> options;
    options.push_back("LOW-END");
    options.push_back("NORMAL");
    options.push_back("HIGH-END");

    short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick(game_config.graphics_performance_mode+1);
    game_config.graphics_performance_mode = selected_option;

    if (game_config.graphics_performance_mode == PERFORMANCE_MODE_LOW) {
        gameControl.set_max_fps(30);
    } else {
        gameControl.set_max_fps(60);
    }

    fio.save_config(game_config);
}

void scenesLib::config_int_value(Uint8 &value_ref, int min, int max)
{
    int config_text_pos_x = graphLib.get_config_menu_pos().x + 74;
    int config_text_pos_y = graphLib.get_config_menu_pos().y + 40;
    graphLib.clear_area(config_text_pos_x-1, config_text_pos_y-1, 100, 100, 0, 0, 0);
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
        graphLib.clear_area(config_text_pos_x+11, config_text_pos_y-1, 30, 12, 0, 0, 0);
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
    st_color font_color(250, 250, 250);
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
    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(strings_ingame_config_select_player), font_color);
    graphLib.draw_centered_text(176, GameMediator::get_instance()->player_list[0].name, font_color);
    graphLib.draw_centered_text(217, strings_map::get_instance()->get_ingame_string(strings_ingame_config_press_start_to_select), font_color);
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
            graphLib.clear_area(0, 49, 320, 96, 27, 63, 95);
            if (selected == 1) {
                graphLib.copyArea(st_position(0, 50), &p1_surface, &graphLib.gameScreen);
            } else if (selected == 2) {
                graphLib.copyArea(st_position(0, 50), &p2_surface, &graphLib.gameScreen);
            } else if (selected == 3) {
                graphLib.copyArea(st_position(0, 50), &p3_surface, &graphLib.gameScreen);
            } else if (selected == 4) {
                graphLib.copyArea(st_position(0, 50), &p4_surface, &graphLib.gameScreen);
            }
            graphLib.clear_area(60, 168, 204, 18, 0, 0, 0);
            graphLib.draw_centered_text(176, GameMediator::get_instance()->player_list[selected-1].name, font_color);
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


/*
Uint8 scenesLib::select_player() {
	int adjustX, adjustY;
    int selected = 1;

	graphLib.blank_screen();

    if (game_config.game_finished == true) {
        adjustX = (RES_W-256)*0.5;
        adjustY = (RES_H-208)*0.5;
    } else {
        adjustX = (RES_W-256)*0.5;
        adjustY = 74;
    }


    graphicsLib_gSurface bg_surface;
    std::string filename = FILEPATH + "images/backgrounds/player_select.png";
    if (game_config.game_finished == true) {
        filename = FILEPATH + "images/backgrounds/player_select4.png";
    }
    graphLib.surfaceFromFile(filename, &bg_surface);
    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
    draw_lib.update_screen();

    int x = 0;
    int y = 0;

	input.clean();
    timer.delay(100);


    graphicsLib_gSurface lights_surface;
    std::string filename_lights = FILEPATH + "images/backgrounds/lights.png";
    graphLib.surfaceFromFile(filename_lights, &lights_surface);

    while (true) {
        input.readInput();
        if (game_config.game_finished == true && (input.p1_input[BTN_DOWN] == 1 || input.p1_input[BTN_UP] == 1)) {
            soundManager.play_sfx(SFX_CURSOR);
            y = !y;
        } else if (input.p1_input[BTN_LEFT] == 1 || input.p1_input[BTN_RIGHT] == 1) {
            soundManager.play_sfx(SFX_CURSOR);
            x = !x;
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
        // convert x/y into selected
        selected = x + y*2;

        draw_lights_select_player(lights_surface, selected, adjustX, adjustY);

        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
    return selected;
}
*/

Uint8 scenesLib::select_difficulty()
{
    short res = 0;
    st_position config_text_pos;
    std::vector<std::string> options;

    graphLib.show_config_bg(0);
    draw_lib.update_screen();
    input.clean();
    timer.delay(300);

    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_easy));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_normal));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_hard));

    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 60;

    graphLib.draw_text(config_text_pos.x, graphLib.get_config_menu_pos().y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_select).c_str());

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
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, 180,  180, 0, 0, 0);
        draw_lib.update_screen();
    }
    return res;
}


void scenesLib::boss_intro(Uint8 pos_n) const {

    if (pos_n < CASTLE1_STAGE1 && stage_data.boss.id_npc == -1) {
        std::cout << "WARNING: Ignoring boss intro, as boss is not set." << std::endl;
        return;
    }

    std::cout << "scenesLib::boss_intro[" << (int)pos_n << "]" << std::endl;

    graphicsLib_gSurface spriteCopy;
    unsigned int intro_frames_n = 1;
    int text_x = RES_W*0.5 - 60;
    unsigned int i;
    std::string filename;
    std::string botname;

    // set skullcastole number accoring to the save
    if (pos_n == CASTLE1_STAGE1) {
        if (game_save.stages[CASTLE1_STAGE5] != 0 || game_save.stages[CASTLE1_STAGE4] != 0) {
            pos_n = CASTLE1_STAGE5;
        } else if (game_save.stages[CASTLE1_STAGE3] != 0) {
            pos_n = CASTLE1_STAGE4;
        } else if (game_save.stages[CASTLE1_STAGE2] != 0) {
            pos_n = CASTLE1_STAGE3;
        } else if (game_save.stages[CASTLE1_STAGE1] != 0) {
            pos_n = CASTLE1_STAGE2;
        }
    }

    if (pos_n == CASTLE1_STAGE1) {
        graphLib.blank_screen();
        /// @TODO - use scenes here
        //show_destrin_ship_intro();
    }

    botname = GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc)->name;

    intro_frames_n = 0;
    for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
        if (GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc)->sprites[ANIM_TYPE_INTRO][i].used == true) {
            intro_frames_n++;
        }
    }

    if (pos_n == CASTLE1_STAGE1 || pos_n >= CASTLE1_STAGE2) {
        filename = FILEPATH + "images/backgrounds/skull_castle.png";
        graphLib.surfaceFromFile(filename, &spriteCopy);
        graphLib.copyArea(st_position(0, 0), &spriteCopy, &graphLib.gameScreen);

        soundManager.play_sfx_from_file("skull_castle_intro.wav", 1);
        graphLib.wait_and_update_screen(6000);
        graphLib.blink_surface_into_screen(spriteCopy);

        CURRENT_FILE_FORMAT::file_castle castle_data;
        fio.read_castle_data(castle_data);



        if (pos_n == CASTLE1_STAGE2) {
            draw_lib.draw_castle_path(true, castle_data.points[0], castle_data.points[1]);
        } else if (pos_n == CASTLE1_STAGE3) {
            draw_lib.draw_castle_path(true, castle_data.points[0], castle_data.points[1]);
            draw_lib.draw_castle_path(true, castle_data.points[1], castle_data.points[2]);
        } else if (pos_n == CASTLE1_STAGE4) {
            draw_lib.draw_castle_path(true, castle_data.points[0], castle_data.points[1]);
            draw_lib.draw_castle_path(true, castle_data.points[1], castle_data.points[2]);
            draw_lib.draw_castle_path(true, castle_data.points[2], castle_data.points[3]);
        } else if (pos_n == CASTLE1_STAGE5) {
            graphicsLib_gSurface castle_skull_point;
            filename = FILEPATH + "images/backgrounds/castle_skull_point.png";
            graphLib.surfaceFromFile(filename, &castle_skull_point);
            graphLib.copyArea(castle_data.points[4], &castle_skull_point, &graphLib.gameScreen);
            draw_lib.draw_castle_path(true, castle_data.points[0], castle_data.points[1]);
            draw_lib.draw_castle_path(true, castle_data.points[1], castle_data.points[2]);
            draw_lib.draw_castle_path(true, castle_data.points[2], castle_data.points[3]);
            draw_lib.draw_castle_path(false, castle_data.points[3], castle_data.points[4]);
        }

        draw_lib.update_screen();
        timer.delay(1000);


        /// @TODO - instant path for drawing previous ones (do not need a for-loop)
        if (pos_n == CASTLE1_STAGE2) {
            draw_lib.draw_castle_path(false, castle_data.points[0], castle_data.points[1]);
        } else if (pos_n == CASTLE1_STAGE3) {
            draw_lib.draw_castle_path(false, castle_data.points[1], castle_data.points[2]);
        } else if (pos_n == CASTLE1_STAGE4) {
            draw_lib.draw_castle_path(false, castle_data.points[2], castle_data.points[3]);
        } else if (pos_n == CASTLE1_STAGE5) {
            draw_lib.draw_castle_path(false, castle_data.points[3], castle_data.points[4]);
        }
        timer.delay(1500);

        return;

    } else if (game_save.stages[pos_n] != 0) {
        std::cout << "boss_intro - stage already finished" << std::endl;
    }


    soundManager.play_sfx(SFX_STAGE_SELECTED);
    graphLib.blank_screen();


    graphLib.start_stars_animation();

    draw_lib.show_boss_intro_sprites(stage_data.boss.id_npc, true);

    for (i=0; i<botname.size(); i++) {
        graphLib.wait_and_update_screen(100);
        // convert name to uppercase
        std::string str = &botname.at(i);
        std::locale settings;
        std::string boss_name;
        for(unsigned int i = 0; i < str.size(); ++i) {
            boss_name += (std::toupper(str[i], settings));
        }

        graphLib.draw_text(text_x, 118, boss_name);
        text_x += 8;
    }
    graphLib.wait_and_update_screen(2500);
    graphLib.stop_stars_animation();

}


void scenesLib::draw_castle_path(bool vertical_first, st_position initial_point, st_position final_point, short total_duration) const
{
    if (total_duration > 0) {
        soundManager.play_sfx(SFX_CHARGING2);
    }
    st_position middle_point(final_point.x, initial_point.y);
    if (vertical_first == true) {
        middle_point.x = initial_point.x;
        middle_point.y = final_point.y;
    }
    graphLib.draw_path(initial_point, middle_point, total_duration/2);
    graphLib.draw_path(middle_point, final_point, total_duration/2);
}

