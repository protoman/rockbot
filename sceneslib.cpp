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

#include "game.h"
extern game gameControl;

#ifdef ANDROID
#include <android/log.h>
#include "ports/android/android_game_services.h"
extern android_game_services game_services;
#endif


#include "scenes/sceneshow.h"

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
#define BOSS_CREDITS_LINES_N 4

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
    graphLib.zoom_image(st_position(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20), intro_screen, false);
    //gfx_sin_wave gfx_wave_obj(&intro_screen);
    //gfx_wave_obj.show(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20);


    graphLib.draw_text(8, 8, VERSION_NUMBER);

    graphLib.draw_text(40-graphLib.RES_DIFF_W, (RES_H-35), strings_map::get_instance()->get_ingame_string(strings_ingame_copyrightline));
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
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_newgame)));
    if (fio.have_one_save_file() == true) {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame)));
    } else {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame), true));
    }
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_about)));

    option_picker main_picker(false, st_position(20, (RES_H*0.5)-graphLib.RES_DIFF_H), options, false);

    main_picker.enable_check_input_reset_command();
    main_picker.enable_check_input_cheat_command();


    bool have_save = fio.have_one_save_file();

    // IF HAVE NO SAVE, TRY TO LOAD IT FROM CLOUD //

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
        } else if (picked_n == 1) { // LOAD GAME //
            short selected_save = select_save(false);
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
        } else if (picked_n == 2) { // CONFIG //
            menu.show_main_config(0, false);
			draw_main();
			main_picker.draw();
        } else if (picked_n == 3) { // ABOUT/CREDITS //
            // only wait for keypress if user did not interrupted credits
            draw_lib.show_about();
            draw_main();
            main_picker.draw();
        } else if (picked_n == 5) { // CHEATS //
            show_cheats_menu();
            draw_main();
            main_picker.draw();
        } else if (picked_n == MAIN_MENU_CHEAT_RETURN) { // LEAVING CHEATS MENU ITEM //
            picked_n = 5;
            main_picker.add_option_item(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_menu_item_CHEATS)));
            draw_main();
            main_picker.draw();
        }
	}
    draw_lib.update_screen();

    if (picked_n == 0) { // NEW GAME //
        game_save.difficulty = select_difficulty();
        game_save.selected_player = select_player();
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
    int current_player = game_save.selected_player;
    while (selected_option != -1) {

        options.clear();

        std::string invencibleStr = strings_map::get_instance()->get_ingame_string(strings_ingame_config_off);
        if (GAME_FLAGS[FLAG_INVENCIBLE]) {
            invencibleStr = strings_map::get_instance()->get_ingame_string(strings_ingame_config_on);
        }
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_menu_item_CHEATS_INVENCIBLE) + ": " + invencibleStr));

        std::string allStagesStr = strings_map::get_instance()->get_ingame_string(strings_ingame_config_off);
        if (GAME_FLAGS[FLAG_ALLWEAPONS]) {
            allStagesStr = strings_map::get_instance()->get_ingame_string(strings_ingame_config_on);
        }
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_menu_item_CHEATS_ALLBEATEN) + ": " + allStagesStr));

        char char_n[50];
        sprintf(char_n, "%s", GameMediator::get_instance()->player_list_v3_1[current_player].name);
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_menu_item_CHEATS_CHARACTER) + ": " + std::string(char_n)));

        option_picker cheat_config_picker(false, config_text_pos, options, true);
        graphLib.show_config_bg();
        cheat_config_picker.draw();
        selected_option = cheat_config_picker.pick(selected_option+1);
        if (selected_option == 0) {
            GAME_FLAGS[FLAG_INVENCIBLE] = !GAME_FLAGS[FLAG_INVENCIBLE];
        } else if (selected_option == 1) {
            GAME_FLAGS[FLAG_ALLWEAPONS] = !GAME_FLAGS[FLAG_ALLWEAPONS];
        } else if (selected_option == 2) {
            current_player++;
            if (current_player >= 4) {
                current_player = 0;
            }
            if (current_player == 0) {
                GAME_FLAGS[FLAG_PLAYER1] = true;
                GAME_FLAGS[FLAG_PLAYER2] = false;
                GAME_FLAGS[FLAG_PLAYER3] = false;
                GAME_FLAGS[FLAG_PLAYER4] = false;
            } else if (current_player == 1) {
                GAME_FLAGS[FLAG_PLAYER1] = false;
                GAME_FLAGS[FLAG_PLAYER2] = true;
                GAME_FLAGS[FLAG_PLAYER3] = false;
                GAME_FLAGS[FLAG_PLAYER4] = false;
            } else if (current_player == 2) {
                GAME_FLAGS[FLAG_PLAYER1] = false;
                GAME_FLAGS[FLAG_PLAYER2] = false;
                GAME_FLAGS[FLAG_PLAYER3] = true;
                GAME_FLAGS[FLAG_PLAYER4] = false;
            } else if (current_player == 3) {
                GAME_FLAGS[FLAG_PLAYER1] = false;
                GAME_FLAGS[FLAG_PLAYER2] = false;
                GAME_FLAGS[FLAG_PLAYER3] = false;
                GAME_FLAGS[FLAG_PLAYER4] = true;
            }
        }
    }


}


void scenesLib::show_game_scene(e_game_scenes_types n)
{
    sceneShow show;
    show.show_scene(game_scenes_map[n]);
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

    soundManager.stop_music();
    soundManager.load_music("rockbot_end_animation.mod");
    soundManager.play_music();


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

    graphLib.blank_screen();
    draw_lib.show_boss_intro_bg();

    soundManager.stop_music();
    if (fio.file_exists(FILEPATH + "/music/rockbot_enemy_parade.mod")) {
        soundManager.load_music("rockbot_enemy_parade.mod");
    } else {
        soundManager.load_shared_music("rockbot_enemy_parade.mod");
    }
    soundManager.play_music();

    // get all stage bosses and add to a vector, and show non-bosses
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
        if (stage_boss_id_list.find(i) != stage_boss_id_list.end()) {
            continue;
        }
        // non-boss
        std::string name = std::string(GameMediator::get_instance()->get_enemy(i)->name);
        ending_show_single_enemy(i, name);
    }

    show_bosses_ending();

    // final boss
    ending_show_single_enemy(game_data.final_boss_id, GameMediator::get_instance()->get_enemy(game_data.final_boss_id)->name);

    graphLib.blank_screen();
    graphLib.updateScreen();
    soundManager.stop_music();
    timer.delay(2000);


}

void scenesLib::ending_show_single_enemy(int id, std::string name)
{
    std::string temp_filename = FILEPATH + "images/sprites/enemies/" + GameMediator::get_instance()->get_enemy(id)->graphic_filename;
    graphicsLib_gSurface npc_sprite_surface;
    graphLib.surfaceFromFile(temp_filename, &npc_sprite_surface);

    draw_lib.show_boss_intro_bg();
    draw_lib.show_boss_intro_sprites(id, false);
    graphLib.draw_centered_text(BOSS_INTRO_BG_TEXT_Y, name);
    draw_lib.update_screen();
    timer.delay(3000);
}

void scenesLib::show_bosses_ending()
{

    // TODO: error handling //


    graphLib.blank_screen();
    // read bosses strings
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<std::string> boss_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/boss_credits.txt");

    for (short i=0; i<CASTLE1_STAGE5; i++) {


        CURRENT_FILE_FORMAT::file_stage stage_data_obj;
        fio.read_stage(stage_data_obj, i);


        int boss_id = stage_data_obj.boss.id_npc;

        draw_lib.show_boss_intro_bg();
        graphLib.updateScreen();


        // BOSS POSITION IN DATA - 40y, 111h
        draw_lib.show_boss_intro_sprites(boss_id, false);
        unsigned int boss_pos = i*BOSS_CREDITS_LINES_N;



        if (boss_pos >= boss_credits_data.size()) {
            continue;
        }



        std::string boss_n = boss_credits_data.at(boss_pos) + ":";
        std::string concept_creator = strings_map::get_instance()->get_ingame_string(STRING_ENDING_CONCEPT) + ":";
        std::string design_creator = strings_map::get_instance()->get_ingame_string(STRING_ENDING_DESIGN) + ":";
        int delay = 60;


        graphLib.draw_progressive_text(5, 170, boss_n, false, delay);
        graphLib.draw_progressive_text(90, 170, boss_credits_data.at(i*4+1), false, delay);
        draw_lib.update_screen();


        graphLib.draw_progressive_text(5, 185, concept_creator.c_str(), false, delay);
        graphLib.draw_progressive_text(90, 185, boss_credits_data.at(i*4+2), false, delay);
        draw_lib.update_screen();


        graphLib.draw_progressive_text(5, 201, design_creator.c_str(), false, delay);
        graphLib.draw_progressive_text(90, 201, boss_credits_data.at(i*4+3), false, delay);
        draw_lib.update_screen();


        timer.delay(2000);
    }


}

short scenesLib::pick_stage(int last_stage)
{
    if (game_save.stages[INTRO_STAGE] == 0) {
        return 0;
    }

    graphLib.blank_screen();
    stage_select selection(STAGE_SELECT_SURFACES);

    short pos_n = selection.pick_stage(last_stage);

    timer.delay(100);
    soundManager.stop_music();

    return pos_n;

}

Uint8 scenesLib::select_player() {
    int selected = 1;
    graphicsLib_gSurface bg_surface;

    int max_loop = 2;
    if (SharedData::get_instance()->game_config.game_finished == true) {
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
    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(strings_ingame_config_select_player));
    graphLib.draw_centered_text(176, GameMediator::get_instance()->player_list_v3_1[0].name);
    graphLib.draw_centered_text(217, strings_map::get_instance()->get_ingame_string(strings_ingame_config_press_start_to_select));
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

void scenesLib::boss_intro(short stage_n)
{
    /*
    if (stage_n < CASTLE1_STAGE1 && stage_data.boss.id_npc == -1) {
        return;
    }
    */
    //std::string botname = GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc)->name;
    std::string botname = stage_data.boss.name;

    soundManager.play_sfx(SFX_STAGE_SELECTED);
    graphLib.blank_screen();

    char decimal = '\0';
    if (stage_n < 10) {
        decimal = '0';
    }
    std::string filename;
    filename.append(FILEPATH).append("/images/backgrounds/boss_intro/");
    if (stage_n < 10) {
        filename.append("0");
    }
    char filename_chr[256];
    sprintf(filename_chr, "%d.png", stage_n);
    filename.append(std::string(filename_chr));

    if (!fio.file_exists(filename)) {
        return;
    }

    graphicsLib_gSurface boss_intro_bg;

    graphLib.surfaceFromFile(FILEPATH + "/images/backgrounds/boss_intro/boss_intro_bg.png", &boss_intro_bg);
    graphLib.showSurfaceAt(&boss_intro_bg, st_position(0, 0), false);


    graphicsLib_gSurface boss_intro_img;
    graphLib.surfaceFromFile(filename, &boss_intro_img);
    graphLib.showSurfaceRegionAt(&boss_intro_img, st_rectangle(2, 0, boss_intro_img.width-4, boss_intro_img.height), st_position(2, 10));
    graphLib.draw_centered_text(217, botname);
    graphLib.wait_and_update_screen(4200);
}

void scenesLib::game_scenes_show_unbeaten_intro()
{
    sceneShow show;
    show.show_scene(game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_UNBEATEN]);
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

    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_easy));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_normal));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_difficulty_hard));

    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
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
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, RES_W,  180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
        draw_lib.update_screen();
    }
    return res;
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


    strings_map::get_instance()->get_ingame_string(strings_menu_pick_save_slot_PICK_OPTION);
    if (is_new_game == true) {
        graphLib.draw_text(10, 10, strings_map::get_instance()->get_ingame_string(strings_menu_pick_save_slot_NEW_GAME));
    } else {
        graphLib.draw_text(10, 10, strings_map::get_instance()->get_ingame_string(strings_menu_pick_save_slot_LOAD_GAME));
    }
    graphLib.draw_text(10, RES_H-12, strings_map::get_instance()->get_ingame_string(strings_menu_pick_save_slot_PICK_OPTION));

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
                graphLib.draw_text(10, i*40+34, strings_map::get_instance()->get_ingame_string(strings_menu_pick_save_slot_NO_SAVE));
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

void scenesLib::show_ending_scenes(e_PLAYERS selected_player)
{
    sceneShow show;
    if (selected_player == PLAYER_1) {
        show.show_scene(game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER1]);
    } else if (selected_player == PLAYER_2) {
        show.show_scene(game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER2]);
    } else if (selected_player == PLAYER_3) {
        show.show_scene(game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER3]);
    } else if (selected_player == PLAYER_4) {
        show.show_scene(game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER4]);
    }

}

void scenesLib::draw_save_details(int n, CURRENT_FILE_FORMAT::st_save save)
{
    // intro stage is rock buster icon, other are weapons icons
    int y_pos = n*40+34;
    for (int i=0; i<=8; i++) {
        st_position pos((i+1)*18-14, y_pos);
        if (save.stages[i] == 1) {
            graphLib.draw_weapon_tooltip_icon(i, pos, true);
        } else {
            // @TODO: draw disabled version
            graphLib.draw_weapon_tooltip_icon(i, pos, false);
        }
    }
    int item_adjust_x = 10;
    char buffer[3];

    // character
    st_position pos_lifes(9*18+item_adjust_x-4, y_pos);
    graphLib.draw_weapon_tooltip_icon(14+save.selected_player, pos_lifes, true);
    std::string difficulty = "[M]";
    if (save.difficulty == DIFFICULTY_EASY) {
        difficulty = "[E]";
    } else if (save.difficulty == DIFFICULTY_HARD) {
        difficulty = "[H]";
    }
    graphLib.draw_text(10*18+item_adjust_x-6, y_pos+5, std::string(difficulty));

    // e-tank
    st_position pos_etank(11*18+item_adjust_x, y_pos);
    graphLib.draw_weapon_tooltip_icon(11, pos_etank, true);
    sprintf(buffer, "x%d", save.items.energy_tanks);
    graphLib.draw_text(12*18+item_adjust_x-1, y_pos+5, std::string(buffer));

    // w-tank
    st_position pos_wtank(13*18+item_adjust_x, y_pos);
    graphLib.draw_weapon_tooltip_icon(12, pos_wtank, true);
    sprintf(buffer, "x%d", save.items.weapon_tanks);
    graphLib.draw_text(14*18+item_adjust_x-1, y_pos+5, std::string(buffer));

    // s-tank
    st_position pos_stank(15*18+item_adjust_x, y_pos);
    graphLib.draw_weapon_tooltip_icon(13, pos_stank, true);
    sprintf(buffer, "x%d", save.items.special_tanks);
    graphLib.draw_text(16*18+item_adjust_x-1, y_pos+5, std::string(buffer));
}

