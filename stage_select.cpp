#include <cstdlib>
#include "stage_select.h"

#include <string>

#include "file/format.h"

// INTERNAL GLOBALS
extern timerLib timer;
extern inputLib input;
extern soundLib soundManager;
extern graphicsLib graphLib;
extern std::string FILEPATH;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_game_config game_config;

extern CURRENT_FILE_FORMAT::file_game game_data;

extern bool leave_game;

#include "graphic/draw.h"
extern draw draw_lib;

#include "game.h"
extern game gameControl;


stage_select::stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]) :
    highlight_animation_white(false), timer_intro(0), highlight_pos(135-graphLib.RES_DIFF_W, 90)
{
	int i;
	for (i=0; i<STAGE_SELECT_COUNT; i++) {
		STAGE_SELECT_SURFACES[i] = &stage_ref[i];
	}
    load_graphics();
}

short stage_select::finished_stages() const
{
    short total_stages = 0;
    for (int i=0; i<STAGE_COUNT; i++) {
        if (game_save.stages[i] == 1) {
            total_stages++;
        }
    }
    //std::cout << "stage_select::finished_stages: " << total_stages << std::endl;
    return total_stages;
}

void stage_select::load_graphics() {
	std::string filename;
    filename = FILEPATH + "images/backgrounds/stage_select_highlighted.png";
	graphLib.surfaceFromFile(filename, &s_light);

    filename = FILEPATH + "images/backgrounds/stage_select_darkned.png";
	graphLib.surfaceFromFile(filename, &s_dark);

    if (game_save.selected_player == PLAYER_BETABOT) {
        filename = FILEPATH + "images/faces/yellow_eyes.png";
    } else if (game_save.selected_player == PLAYER_ROCKBOT) {
        filename = FILEPATH + "images/faces/dark_eyes.png";
    } else {
        filename = FILEPATH + "images/faces/big_eyes.png";
    }

	graphLib.surfaceFromFile(filename, &eyes_surface);

    filename = FILEPATH + "images/backgrounds/stage_select.png";
	graphLib.surfaceFromFile(filename, &background);
}



void stage_select::move_highlight(Sint8 x_inc, Sint8 y_inc) {

    std::cout << "STAGESELECT::move_highlight  - xinc: " << (int)x_inc << ", yinc: " << (int)y_inc << std::endl;

	graphLib.copyArea(highlight_pos, &s_dark, &graphLib.gameScreen);
	highlight_pos.x += x_inc;
	highlight_pos.y += y_inc;
	graphLib.copyArea(highlight_pos, &s_light, &graphLib.gameScreen);
    draw_lib.update_screen();
}


void stage_select::show_highlight() {
	graphLib.copyArea(highlight_pos, light_mode, &graphLib.gameScreen);
    draw_lib.update_screen();
}


//void stage_select::place_face(int face_n, char botname[20], short int posx, short int posy) {
void stage_select::place_face(std::string face_file, std::string botname, st_position pos) const {
	// TODO - calculate the text position according to it's strlen
	int txtPosX, txtPosY;

    //std::cout << "stage_select::place_face - face_file: '" << face_file << "'" << std::endl;

	if (face_file.size() > 0) {
        graphLib.place_face(face_file, st_position(pos.x*80+63, pos.y*68+28));
    }
    txtPosX = pos.x*80+76-(botname.size()*7)*0.5;
    txtPosY = pos.y*68+71;
	graphLib.draw_text(txtPosX, txtPosY, botname);
}


void stage_select::animate_highlight() {
	if (timer.getTimer() > timer_intro) {
		if (light_mode == &s_light) {
			light_mode = &s_dark;
		} else {
			light_mode = &s_light;
		}
		show_highlight();
		timer_intro = timer.getTimer()+200;
	}
}

void stage_select::draw_eyes(Uint8 x, Uint8 y, bool erase_eyes) {
	int posX, posY;
	if (x == 1) {
		posX = 11;
	} else if (x == 0) {
		posX = 10;
	} else {
		posX = 12;
	}
	if (y == 1) {
		posY = 18;
	} else if (y == 0) {
		posY = 17;
	} else {
		posY = 19;
	}
	/*
    if (game_config.selected_player == PLAYER_BETABOT) {
		posY -= 3;
	}
	*/
    posX = posX+80+63;
    posY = posY+64+32;
	if (erase_eyes) {
        if (game_save.selected_player == PLAYER_ROCKBOT) {
            place_face("player1.png", "", st_position(1, 1));
        } else if (game_save.selected_player == PLAYER_BETABOT) {
            place_face("player2.png", "", st_position(1, 1));
        } else if (game_save.selected_player == PLAYER_CANDYBOT) {
            place_face("player3.png", "", st_position(1, 1));
        } else {
            place_face("player4.png", "", st_position(1, 1));
        }
	} else {
        if (game_save.selected_player == PLAYER_ROCKBOT) {
			graphLib.copyArea(st_position(posX, posY), &eyes_surface, &graphLib.gameScreen);
        } else if (game_save.selected_player == PLAYER_BETABOT) {
			graphLib.copyArea(st_position(posX, posY-2), &eyes_surface, &graphLib.gameScreen);
        } else {
            graphLib.copyArea(st_position(posX-1, posY), &eyes_surface, &graphLib.gameScreen);
        }
	}
}

struct st_position stage_select::select() {
    struct st_position spacing;
	int selection_end=0;

    soundManager.load_music(game_data.stage_select_music_filename);
	soundManager.play_music();
	input.clean();
	input.waitTime(300);

	select_pos.x = 1;
	select_pos.y = 1;
	spacing.x = 80;
    spacing.y = 68;
    highlight_pos.x = 135-graphLib.RES_DIFF_W;
    highlight_pos.y = 89;
	graphLib.blank_screen();
	light_mode = &s_light;

	graphLib.copyArea(st_position(0, 0), &background, &graphLib.gameScreen);

	if (game_save.stages[STAGE1] == 0) {
        place_face(game_data.stage_face_filename[STAGE1], "APE", st_position(0, 0));
	} else {
		place_face(std::string(""), "APE", st_position(0, 0));
	}

	if (game_save.stages[STAGE2] == 0) {
        place_face(game_data.stage_face_filename[STAGE2], "DAISIE", st_position(1, 0));
	} else {
		place_face(std::string(""), "DAISIE", st_position(1, 0));
	}

	if (game_save.stages[STAGE3] == 0) {
        place_face(game_data.stage_face_filename[STAGE3], "SEAHORSE", st_position(2, 0));
	} else {
		place_face(std::string(""), "SEAHORSE", st_position(2, 0));
	}

	if (game_save.stages[STAGE4] == 0) {
        place_face(game_data.stage_face_filename[STAGE4], "MUMMY", st_position(0, 1));
	} else {
		place_face(std::string(""), "MUMMY", st_position(0, 1));
	}

	if (game_save.stages[STAGE5] == 0) {
        place_face(game_data.stage_face_filename[STAGE5], "MAGE", st_position(2, 1));
	} else {
		place_face(std::string(""), "MAGE", st_position(2, 1));
	}

	if (game_save.stages[STAGE6] == 0) {
        place_face(game_data.stage_face_filename[STAGE6], "DYNAMITE", st_position(0, 2));
	} else {
		place_face(std::string(""), "DYNAMITE", st_position(0, 2));
	}

	if (game_save.stages[STAGE7] == 0) {
        place_face(game_data.stage_face_filename[STAGE7], "SPIKE", st_position(1, 2));
	} else {
		place_face(std::string(""), "SPIKE", st_position(1, 2));
	}

	if (game_save.stages[STAGE8] == 0) {
        place_face(game_data.stage_face_filename[STAGE8], "TECHNO", st_position(2, 2));
	} else {
		place_face(std::string(""), "TECHNO", st_position(2, 2));
	}

    if (finished_stages() < 9) {
        if (game_save.selected_player == PLAYER_ROCKBOT) {
            place_face("player1.png", "", st_position(1, 1));
        } else if (game_save.selected_player == PLAYER_BETABOT) {
            place_face("player2.png", "", st_position(1, 1));
        } else if (game_save.selected_player == PLAYER_CANDYBOT) {
            place_face("player3.png", "", st_position(1, 1));
        } else {
            place_face("player4.png", "", st_position(1, 1));
        }
	} else {
		place_face("dr_destrin.png", "Dr. D.", st_position(1, 1));
	}

	light_mode = &s_light;

    draw_lib.update_screen();
    std::string press_start_string = "STAGE SELECT - PRESS START";
    graphLib.draw_text(RES_W*0.5-(FONT_SIZE*press_start_string.length())/2, 227, press_start_string);

	input.clean();
    if (finished_stages() < 9) {
		draw_eyes(select_pos.x, select_pos.y, false);
	}
	input.waitTime(200);

	while (selection_end == 0) {
		input.readInput();

        if (input.p1_input[BTN_QUIT]) {
#if !defined(PLAYSTATION2) && !defined(PSP) && !defined(WII) && !defined(DREAMCAST)
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
#endif
		}

        if (select_pos.y < 2 && input.p1_input[BTN_DOWN]) {
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.y++;
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(0, spacing.y);
            input.waitTime(200);
            input.clean();
        } else if (select_pos.y > 0 && input.p1_input[BTN_UP]) {
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.y--;
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(0, -spacing.y);
            input.waitTime(200);
            input.clean();
        } else if (select_pos.x > 0 && input.p1_input[BTN_LEFT]) {
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.x--;
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(-spacing.x, 0);
            input.waitTime(200);
            input.clean();
        } else if (select_pos.x < 2 && input.p1_input[BTN_RIGHT]) {
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.x++;
            if (finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(spacing.x, 0);
            input.waitTime(200);
            input.clean();
        } else if (input.p1_input[BTN_START] && finished_stages() < 9 && (select_pos.x != 1 || select_pos.y != 1)) {
			selection_end = 1;
        } else if (input.p1_input[BTN_START] && finished_stages() >= 9) {
			selection_end = 1;
		}
		animate_highlight();
		input.waitTime(10);
	}

    //std::cout << "stage_select::select - c.x: " << select_pos.x << ", c.y: " << select_pos.y << std::endl;

	graphLib.blink_screen(255, 255, 255);
	return select_pos;
}
