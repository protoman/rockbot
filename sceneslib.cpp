#include <cstdlib>
#include "sceneslib.h"
#include "soundlib.h"
#include "scenes/password_generator.h"
#include "file/fio_scenes.h"
#include "strings_map.h"

#include "game_mediator.h"

extern string FILEPATH;
extern soundLib soundManager;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;


#include "stage_select.h"
#include "graphic/option_picker.h"
#include "file/version.h"
#include "file/file_io.h"

#include "options/key_map.h"

#include "graphic/animation.h"

#include "game.h"
extern game gameControl;

extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_io fio;

extern timerLib timer;

extern bool have_save;

extern bool leave_game;

#define TIME_SHORT 120
#define TIME_LONG 300
#define INTRO_DIALOG_DURATION_TIME 4000

// ********************************************************************************************** //
// ScenesLib handles all scinematic like intro and ending                                         //
// ********************************************************************************************** //
scenesLib::scenesLib() : _timer(0), _state(0)
{
	for (int i=0; i<PASSWORD_GRID_SIZE; i++) {
		for (int j=0; j<PASSWORD_GRID_SIZE; j++) {
			_password_selected_balls[i][j] = -1;
		}
	}
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void scenesLib::preloadScenes() const
{
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
    graphLib.copyArea(st_position(-graphLib.RES_DIFF_W, -graphLib.RES_DIFF_H+20), &intro_screen, &graphLib.gameScreen);

    graphLib.draw_text(8, 8, VERSION_NUMBER);

    graphLib.draw_text(8, 18, "FREE VERSION");

    graphLib.draw_text(40-graphLib.RES_DIFF_W, (RES_H-35), strings_map::get_instance()->get_ingame_string(strings_ingame_copyrightline));
}

// ********************************************************************************************** //
// mostra tela de introdução, até alguém apertar start/enter
// a partir daí, mostra tela de seleção de personagem
// ********************************************************************************************** //
void scenesLib::main_screen()
{

    soundManager.stop_music();
    soundManager.load_music(game_data.game_start_screen_music_filename);
    soundManager.play_music();
	draw_main();

    std::vector<st_menu_option> options;
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_newgame)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_loadgame)));
#ifdef DEMO_VERSION
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_password), true));
#else
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_password)));
#endif
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_config)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_about)));
    option_picker main_picker(false, st_position(40-graphLib.RES_DIFF_W, (RES_H*0.5)-graphLib.RES_DIFF_H), options, false);


    int picked_n = 1;

    if (have_save) {
		main_picker.set_picker_initial_pos(1);
    } else {
        main_picker.set_picker_initial_pos(0);
    }
	bool repeat_menu = true;
	while (repeat_menu == true) {

		picked_n = main_picker.pick();
		if (picked_n == -1) {
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
        } else if (picked_n == 0) { // NEW GAME
			repeat_menu = false;
        } else if (picked_n == 1) { // LOAD GAME
            if (have_save == true) {
				fio.read_save(game_save);
				repeat_menu = false;
			}
        } else if (picked_n == 2) { // PASSWORD
#ifndef DEMO_VERSION
            if (show_password_input() == true) {
				repeat_menu = false;
			} else {
				draw_main();
				main_picker.draw();
			}
#else
            soundManager.play_sfx(SFX_NPC_HIT);
#endif
		} else if (picked_n == 3) {
            show_main_config(0);
			draw_main();
			main_picker.draw();
        } else if (picked_n == 4) {
            draw_lib.show_credits();
            input.wait_keypress();
            draw_main();
            main_picker.draw();
        }
	}
    draw_lib.update_screen();

    if (picked_n == 0) {
        game_save.difficulty = select_difficulty();
        // demo do not have player selection, only rockbot is playable
#ifndef DEMO_VERSION
        game_save.selected_player = select_player();
#else
        game_save.selected_player = PLAYER_1;
#endif
    }
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
short scenesLib::pick_stage() {
	graphLib.blank_screen();
	stage_select selection(STAGE_SELECT_SURFACES);

	selected_stage = selection.select();


	int pos_n = selected_stage.x + 1 + selected_stage.y*3;
    if (pos_n == 5) { // hack for skullcastle position
        pos_n = CASTLE1_STAGE1;
    } else if (pos_n >= 6) {
        pos_n--;
    }

    if (pos_n == CASTLE1_STAGE1) { // look for the actual stage in skullcastle
        for (int i=CASTLE1_STAGE1; i<CASTLE1_STAGE5; i++) {
            if (game_save.stages[i] == 0) {
                break;
            }
            pos_n = i+1;
        }
    }
	input.waitTime(100);
    return pos_n;
}




short scenesLib::show_main_config(short stage_finished) // returns 1 if must leave stage, 0 if nothing
{
    short res = 0;
    st_position config_text_pos;
    std::vector<st_menu_option> options;

	graphLib.show_config_bg(0);
    draw_lib.update_screen();
	input.clean();
	input.waitTime(300);

    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_audio)));
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_input)));
#if defined(PC) || defined (PSP)
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_video)));
#else
    options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_video), true));
#endif
    if (stage_finished) {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage)));
    } else {
        options.push_back(st_menu_option(strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage), true));
    }

    options.push_back( strings_map::get_instance()->get_ingame_string(strings_ingame_config_quitgame));


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
        } else if (selected_option == 2) { // CONFIG VIDEO
#ifdef PC
                show_config_video();
#elif PSP
                show_config_video_PSP();
#endif
        } else if (selected_option == 3) { // LEAVE STAGE
            res = 1;
            break;
        } else if (selected_option == 4) { // QUIT GAME
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






void scenesLib::show_config_video()
{
	st_position config_text_pos;
	config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
	config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
	input.clean();
	input.waitTime(300);

	std::vector<std::string> options;
	if (game_config.video_fullscreen == true) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed));
	} else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen));
	}

    if (game_config.video_filter == VIDEO_FILTER_NOSCALE) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_noscale) + std::string("[X]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_size2x) + std::string("[ ]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale2x) + std::string("[ ]"));
    } else if (game_config.video_filter == VIDEO_FILTER_BITSCALE) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_noscale) + std::string("[ ]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_size2x) + std::string("[X]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale2x) + std::string("[ ]"));
    } else if (game_config.video_filter == VIDEO_FILTER_SCALE2x) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_noscale) + std::string("[ ]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_size2x) + std::string("[ ]"));
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_scale2x) + std::string("[X]"));
    }

	short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
	selected_option = main_config_picker.pick();
	if (selected_option == 0) {
        game_config.video_fullscreen = !game_config.video_fullscreen;
    } else if (selected_option == 1) {
        game_config.video_filter = VIDEO_FILTER_NOSCALE;
	} else if (selected_option == 2) {
        game_config.video_filter = VIDEO_FILTER_BITSCALE;
	} else if (selected_option == 3) {
        game_config.video_filter = VIDEO_FILTER_SCALE2x;
    } else {
        std::cout << "SET video_filter - UNKNOWN mode!!" << std::endl;
    }
    if (selected_option != -1) {
        fio.save_config(game_config);
        input.clean();
        input.waitTime(300);
        st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);
        graphLib.clear_area(menu_pos.x, menu_pos.y, 180,  180, 0, 0, 0);
        graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1));
        graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2));
        graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3));
        graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart));
        input.wait_keypress();
    }
}

void scenesLib::show_config_video_PSP()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    input.waitTime(300);

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
        input.clean();
        input.waitTime(300);
        st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);
        graphLib.clear_area(menu_pos.x, menu_pos.y, 180,  180, 0, 0, 0);
        graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1));
        graphLib.draw_text(menu_pos.x, menu_pos.y+10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2));
        graphLib.draw_text(menu_pos.x, menu_pos.y+20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3));
        graphLib.draw_text(menu_pos.x, menu_pos.y+40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart));
        input.wait_keypress();
    }
}

void scenesLib::show_config_audio()
{
	st_position config_text_pos;
	config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
	config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
	input.clean();
	input.waitTime(300);

	std::vector<std::string> options;
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_enabled));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_disabled));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_music));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_audio_volume_sfx));


	short selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
	selected_option = main_config_picker.pick();
	if (selected_option == 0) {
		soundManager.enable_sound();
	} else if (selected_option == 1) {
		soundManager.disable_sound();
    } else if (selected_option == 2) {
        config_int_value(game_config.volume_music, 1, 128);
        soundManager.update_volumes();
        fio.save_config(game_config);
    } else if (selected_option == 3) {
        config_int_value(game_config.volume_sfx, 1, 128);
        soundManager.update_volumes();
        fio.save_config(game_config);
    }
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
        input.readInput();

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

bool scenesLib::password_ball_selector()
{
    std::cout << "password_ball_selector::START" << std::endl;
	int selected_ball = 0; // blue, 1 is red
	st_position blue_ball_pos(236, 60);
	st_position red_ball_pos(252, 60);
    graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
    draw_lib.update_screen();

	input.clean();
    timer.delay(10);

	while (true) {
		input.readInput();
		if (input.p1_input[BTN_LEFT] == 1 || input.p1_input[BTN_RIGHT] == 1) {
			soundManager.play_sfx(SFX_CURSOR);
			if (selected_ball == 0) {
				// erase mark on blue
                graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
				// draw mark on red
                graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), red_ball_pos, &_password_selector, &graphLib.gameScreen);
				selected_ball = 1;
			} else {
				// erase mark on red
                graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), red_ball_pos, &_password_selector, &graphLib.gameScreen);
				// draw mark on blue
                graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
				selected_ball = 0;
			}
		} else if (input.p1_input[BTN_JUMP] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), red_ball_pos, &_password_selector, &graphLib.gameScreen);
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
			password_number_selector(selected_ball);
			if (selected_ball == 0) {
                graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
			} else {
                graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), red_ball_pos, &_password_selector, &graphLib.gameScreen);
			}
		} else if (input.p1_input[BTN_ATTACK] == 1 || input.p1_input[BTN_START] == 1 || input.p1_input[BTN_DOWN]) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), red_ball_pos, &_password_selector, &graphLib.gameScreen);
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), blue_ball_pos, &_password_selector, &graphLib.gameScreen);
            bool res = password_end_selector();
            return res;
		}
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
}

bool scenesLib::password_set()
{
	for (int y=0; y<PASSWORD_GRID_SIZE; y++) {
		for (int x=0; x<PASSWORD_GRID_SIZE; x++) {
            //if (_password_selected_balls[x][y] != -1) { std::cout << "password at[" << x << "][" << y << "]: " << _password_selected_balls[x][y] << std::endl; }

            /*

              NUMBER IS X, LETTER IS Y

            [1, 0]a2 blue -> char #3 (candy?)
            [5, 3]d6 blue -> techno - OK
            [5, 5]f6 blue -> daisie - OK
            [3, 2]c4 blue -> mummy - OK
            [4, 5]f5 blue -> spike - OK

            [0, 0]a1 -> castle #1
            [1, 1]b2 -> castle #2
            [0, 4]e1 -> castle #3
            [2, 4]e3 -> castle #4
            */
			if (_password_selected_balls[x][y] == 0) { // blue

                std::cout << "PASSWORD[BLUE] [" << x << "][" << y << "]" << std::endl;

                if (x == 1 && y == 0) {
                    game_save.selected_player = PLAYER_3;
                } else if (x == 5 && y == 3) {
					game_save.stages[STAGE8] = 1;
				} else if (x == 5 && y == 5) {
					game_save.stages[STAGE2] = 1;
                } else if (x == 3 && y == 2) {
					game_save.stages[STAGE4] = 1;
                } else if (x == 4 && y == 5) {
					game_save.stages[STAGE7] = 1;

                } else if (x == 0 && y == 0) {
                    game_save.stages[CASTLE1_STAGE1] = 1;
                } else if (x == 1 && y == 1) {
                    game_save.stages[CASTLE1_STAGE2] = 1;
                } else if (x == 0 && y == 4) {
                    game_save.stages[CASTLE1_STAGE3] = 1;
                } else if (x == 2 && y == 4) {
                    game_save.stages[CASTLE1_STAGE4] = 1;


                // weapon-tank: y 5, x 0
                } else if (x == 0 && y == 5) {
                    game_save.items.weapon_tanks = 1;
                // special tank y 5, x 2
                } else if (x == 2 && y == 5) {
                    game_save.items.special_tanks = 1;

                // armor pieces: linha4, coluna 1 -> armor, 2 -> hands, 4 -> legs
                } else if (x == 1 && y == 4) {
                    game_save.armor_pieces[ARMOR_BODY] = true;
                } else if (x == 2 && y == 4) {
                    game_save.armor_pieces[ARMOR_ARMS] = true;
                } else if (x == 4 && y == 4) {
                    game_save.armor_pieces[ARMOR_LEGS] = true;



				} else {
					std::cout << "password error at blue[" << x << "][" << y << "]" << std::endl;
					return false;
				}
			} else if (_password_selected_balls[x][y] == 1) { // red

                std::cout << "PASSWORD[RED] [" << x << "][" << y << "]" << std::endl;

				/*
                [1, 0]a2 red -> betabot
                [2, 1]b3 red -> p#4 (kitty?)
                [0, 2]c1 red  -> initial-stage
                [2, 0]a3 red  -> magebot
                [4, 1]b5 red  -> dynamite
                [2, 3]d3 red  -> seahorse
                [3, 5]f4 red  -> apebot
				*/
                if (x == 1 && y == 0) {
                    game_save.selected_player = PLAYER_2;
                } else if (x == 2 && y == 1) {
                    game_save.selected_player = PLAYER_4;
                } else if (x == 0 && y == 2) {
					game_save.stages[INTRO_STAGE] = 1;
                } else if (x == 2 && y == 0) {
					game_save.stages[STAGE5] = 1;
                } else if (x == 4 && y == 1) {
					game_save.stages[STAGE6] = 1;
                } else if (x == 2 && y == 3) {
					game_save.stages[STAGE3] = 1;
                } else if (x == 3 && y == 5) {
					game_save.stages[STAGE1] = 1;
				// energy tanks
				/*
                [5, 4]E6 = Start with 1
                [3, 4]E4 = Start with 2
                [3, 1]B4 = Start with 3
                [4, 0]A5 = Start with 4
                [4, 2]C5 = Start with 5
                [1, 3]D2 = Start with 6
				[2, 2]C3 = Start with 7
                [1, 5]F2 = Start with 8
                [5, 0]A6 = Start with 9
				*/
                } else if (x == 5 && y == 4) {
					game_save.items.energy_tanks = 1;
                } else if (x == 3 && y == 4) {
					game_save.items.energy_tanks = 2;
                } else if (x == 3 && y == 1) {
					game_save.items.energy_tanks = 3;
                } else if (x == 4 && y == 0) {
					game_save.items.energy_tanks = 4;
                } else if (x == 4 && y == 3) {
					game_save.items.energy_tanks = 5;
                } else if (x == 1 && y == 3) {
					game_save.items.energy_tanks = 6;
				} else if (x == 2 && y == 2) {
					game_save.items.energy_tanks = 7;
                } else if (x == 1 && y == 5) {
					game_save.items.energy_tanks = 8;
                } else if (x == 5 && y == 0) {
					game_save.items.energy_tanks = 9;
                } else {
                    std::cout << "password error at red[" << x << "][" << y << "]" << std::endl;
                    return false;
                }
			}
		}
	}
	return true;
}

bool scenesLib::password_end_selector()
{
	st_position end_pos1(235, 124);
	st_position end_pos2(258, 124);
    graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w/2, _password_selector.get_surface()->h/2), end_pos1, &_password_selector, &graphLib.gameScreen);
    graphLib.copyArea(st_rectangle( _password_selector.get_surface()->w/2, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), end_pos2, &_password_selector, &graphLib.gameScreen);
    draw_lib.update_screen();
    input.clean();
    timer.delay(10);

    while (true) {
        input.readInput();
		if (input.p1_input[BTN_UP] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w/2, _password_selector.get_surface()->h), end_pos1, &_password_selector, &graphLib.gameScreen);
            graphLib.copyArea(st_rectangle( _password_selector.get_surface()->w/2, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), end_pos2, &_password_selector, &graphLib.gameScreen);
            return password_ball_selector();
		} else if (input.p1_input[BTN_ATTACK] == 1) {
			return false;
		} else if (input.p1_input[BTN_JUMP] == 1 || input.p1_input[BTN_START] == 1) {
			if (password_set() == false) { // show an error and keep trying
                graphLib.clear_area(53, 180, 217, 28, 0, 0, 0);
                graphLib.draw_text(66, 190, strings_map::get_instance()->get_ingame_string(strings_ingame_passwordinvalid), graphLib.gameScreen);
                draw_lib.update_screen();
			} else {
				return true;
			}
		}
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
}




void scenesLib::password_number_selector(int ball_type)
{
	graphicsLib_gSurface ball_img;
	st_position selected_number(0, 0);
	st_rectangle point_zero(60, 52, 16, 16);

    graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), st_position(point_zero.x, point_zero.y), &_password_selector, &graphLib.gameScreen);

	std::string filename;
	if (ball_type == 0) {
        filename = FILEPATH + "images/backgrounds/password_blue_ball.png";
	} else {
        filename = FILEPATH + "images/backgrounds/password_red_ball.png";
	}
	graphLib.surfaceFromFile(filename, &ball_img);

	input.clean();
    timer.delay(10);

	while (true) {
		input.readInput();
		if (input.p1_input[BTN_LEFT] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
			selected_number.x--;
			if (selected_number.x < 0) {
				selected_number.x = 5;
			}
            graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
		} else if (input.p1_input[BTN_RIGHT] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
			selected_number.x++;
			if (selected_number.x > 5) {
				selected_number.x = 0;
			}
            graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
		} else if (input.p1_input[BTN_UP] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
			selected_number.y--;
			if (selected_number.y < 0) {
				selected_number.y = 5;
			}
            graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
		} else if (input.p1_input[BTN_DOWN] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
			selected_number.y++;
			if (selected_number.y > 5) {
				selected_number.y = 0;
			}
            graphLib.copyArea(st_rectangle(0, 0, _password_selector.get_surface()->w, _password_selector.get_surface()->h/2), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
		} else if (input.p1_input[BTN_JUMP] == 1) {
            if (_password_selected_balls[selected_number.x][selected_number.y] == -1) {
                graphLib.copyArea(st_rectangle(0, 0, ball_img.get_surface()->w, ball_img.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x+2, selected_number.y*point_zero.h + point_zero.y+2), &ball_img, &graphLib.gameScreen);
                _password_selected_balls[selected_number.x][selected_number.y] = ball_type;
				std::cout << "set password[" << selected_number.y << "][" << selected_number.x << "] to " << ball_type << std::endl;
			} else {
                graphLib.blank_area(selected_number.x*point_zero.w + point_zero.x+2, selected_number.y*point_zero.h + point_zero.y+2, ball_img.get_surface()->w, ball_img.get_surface()->h, graphLib.gameScreen);
                _password_selected_balls[selected_number.x][selected_number.y] = -1;
			}
		} else if (input.p1_input[BTN_ATTACK] == 1) {
            graphLib.copyArea(st_rectangle(0, _password_selector.get_surface()->h/2, _password_selector.get_surface()->w, _password_selector.get_surface()->h), st_position(selected_number.x*point_zero.w + point_zero.x, selected_number.y*point_zero.h + point_zero.y), &_password_selector, &graphLib.gameScreen);
			break;
		}
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
}




bool scenesLib::show_password_input()
{
	graphicsLib_gSurface password_screen;
    std::string filename = FILEPATH + "images/backgrounds/password.png";
	graphLib.surfaceFromFile(filename, &password_screen);
    graphLib.copyArea(st_rectangle(0, 0, password_screen.get_surface()->w, password_screen.get_surface()->h), st_position(0, 0), &password_screen, &graphLib.gameScreen);

    filename = FILEPATH + "images/backgrounds/password_selector.png";
	graphLib.surfaceFromFile(filename, &_password_selector);

    draw_lib.update_screen();

    bool res = password_end_selector();
    return res;
}

void scenesLib::show_password()
{
    graphicsLib_gSurface password_screen;
    std::string filename = FILEPATH + "images/backgrounds/password.png";
    graphLib.surfaceFromFile(filename, &password_screen);
    graphLib.copyArea(st_rectangle(0, 0, password_screen.get_surface()->w, password_screen.get_surface()->h), st_position(0, 0), &password_screen, &graphLib.gameScreen);

    filename = FILEPATH + "images/backgrounds/password_selector.png";
    graphLib.surfaceFromFile(filename, &_password_selector);
    // add balls
    graphicsLib_gSurface red_ball_img, blue_ball_img;
    st_rectangle point_zero(60, 52, 16, 16);
    filename = FILEPATH + "images/backgrounds/password_blue_ball.png";
    graphLib.surfaceFromFile(filename, &blue_ball_img);
    filename = FILEPATH + "images/backgrounds/password_red_ball.png";
    graphLib.surfaceFromFile(filename, &red_ball_img);

    password_generator pgen(game_save);
    password_matrix res = pgen.run();
    for (int i=0; i<PASSWORD_GRID_SIZE; i++) {
        for (int j=0; j<PASSWORD_GRID_SIZE; j++) {
            if (res.value[i][j] == PASSWORD_BALL_COLOR_RED) {
                graphLib.copyArea(st_rectangle(0, 0, red_ball_img.get_surface()->w, red_ball_img.get_surface()->h), st_position(i*point_zero.w + point_zero.x+2, j*point_zero.h + point_zero.y+2), &red_ball_img, &graphLib.gameScreen);
            } else if (res.value[i][j] == PASSWORD_BALL_COLOR_BLUE) {
                graphLib.copyArea(st_rectangle(0, 0, blue_ball_img.get_surface()->w, blue_ball_img.get_surface()->h), st_position(i*point_zero.w + point_zero.x+2, j*point_zero.h + point_zero.y+2), &blue_ball_img, &graphLib.gameScreen);
            }
        }
    }
    graphLib.draw_text(112, 190, std::string(">") + strings_map::get_instance()->get_ingame_string(strings_ingame_pressstart), graphLib.gameScreen);
    draw_lib.update_screen();
    input.wait_keypress();
    password_screen.freeGraphic();
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


    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(strings_ingame_config_select_player), font_color);
    graphLib.draw_centered_text(170, GameMediator::get_instance()->player_list[0].name, font_color);
    graphLib.draw_centered_text(217, strings_map::get_instance()->get_ingame_string(strings_ingame_config_press_start_to_select), font_color);
    graphLib.copyArea(st_position(0, 50), &p1_surface, &graphLib.gameScreen);
    draw_lib.update_screen();


    input.clean();
    input.waitTime(100);

    while (true) {
        input.readInput();
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
            }
            graphLib.clear_area(60, 168, 204, 18, 0, 0, 0);
            graphLib.draw_centered_text(170, GameMediator::get_instance()->player_list[selected-1].name, font_color);
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
        input.waitTime(10);
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
    input.waitTime(100);


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
        input.waitTime(10);
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
    input.waitTime(300);

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
        }
        graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, 180,  180, 0, 0, 0);
        draw_lib.update_screen();
    }
    return res;
}


void scenesLib::boss_intro(Uint8 pos_n) const {

    if (stage_data.boss.id_npc == -1) {
        std::cout << "WARNING: Ignoring boss intro, as boss is not set." << std::endl;
        return;
    }

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

    botname = GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc).name;

    intro_frames_n = 0;
    for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
        if (GameMediator::get_instance()->get_enemy(stage_data.boss.id_npc).sprites[ANIM_TYPE_INTRO][i].used == true) {
            intro_frames_n++;
        }
    }

    if (pos_n == CASTLE1_STAGE1 || pos_n >= CASTLE1_STAGE2) {
        filename = FILEPATH + "images/backgrounds/skull_castle.png";
        graphLib.surfaceFromFile(filename, &spriteCopy);
        graphLib.copyArea(st_position(0, 0), &spriteCopy, &graphLib.gameScreen);

        graphLib.wait_and_update_screen(500);
        graphLib.blink_surface_into_screen(spriteCopy);

        graphicsLib_gSurface castle_point;
        filename = FILEPATH + "images/backgrounds/castle_point.png";
        graphLib.surfaceFromFile(filename, &castle_point);
        graphLib.copyArea(st_position(74, 228), &castle_point, &graphLib.gameScreen);



        if (pos_n == CASTLE1_STAGE2) {
            graphLib.copyArea(st_position(109, 150), &castle_point, &graphLib.gameScreen);
        } else if (pos_n == CASTLE1_STAGE3) {
            draw_castle_path(true, st_position(76, 228), st_position(105, 152), 0);
            graphLib.copyArea(st_position(109, 150), &castle_point, &graphLib.gameScreen);
            graphLib.copyArea(st_position(177, 138), &castle_point, &graphLib.gameScreen);
        } else if (pos_n == CASTLE1_STAGE4) {
            draw_castle_path(true, st_position(76, 228), st_position(105, 152), 0);
            draw_castle_path(false, st_position(117, 152), st_position(179, 140), 0);
            graphLib.copyArea(st_position(109, 150), &castle_point, &graphLib.gameScreen);
            graphLib.copyArea(st_position(177, 138), &castle_point, &graphLib.gameScreen);
            graphLib.copyArea(st_position(195, 110), &castle_point, &graphLib.gameScreen);
        } else if (pos_n == CASTLE1_STAGE5) {
            draw_castle_path(true, st_position(76, 228), st_position(105, 152), 0);
            draw_castle_path(false, st_position(117, 152), st_position(179, 146), 0);
            draw_castle_path(true, st_position(179, 138), st_position(191, 112), 0);
            graphLib.copyArea(st_position(109, 150), &castle_point, &graphLib.gameScreen);
            graphLib.copyArea(st_position(177, 138), &castle_point, &graphLib.gameScreen);
            graphLib.copyArea(st_position(195, 110), &castle_point, &graphLib.gameScreen);
            graphicsLib_gSurface castle_skull_point;
            filename = FILEPATH + "images/backgrounds/castle_skull_point.png";
            graphLib.surfaceFromFile(filename, &castle_skull_point);
            graphLib.copyArea(st_position(167, 42), &castle_skull_point, &graphLib.gameScreen);
        }

        soundManager.play_sfx(SFX_SKULL_CASTLE_INTRO);
        draw_lib.update_screen();
        input.waitTime(7550);


        /// @TODO - instant path for drawing previous ones (do not need a for-loop)
        if (pos_n == CASTLE1_STAGE2) {
            draw_castle_path(true, st_position(76, 228), st_position(105, 152), 1000);
        } else if (pos_n == CASTLE1_STAGE3) {
            draw_castle_path(false, st_position(117, 152), st_position(179, 146), 1000);
        } else if (pos_n == CASTLE1_STAGE4) {
            draw_castle_path(true, st_position(179, 138), st_position(191, 112), 1000);
        } else if (pos_n == CASTLE1_STAGE5) {
            draw_castle_path(true, st_position(197, 110), st_position(186, 47), 1000);
        }
        input.waitTime(1500);
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

