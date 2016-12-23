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

    char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
    sprintf(eyes_filename_char, "images/faces/p%d_eyes.png", (game_save.selected_player+1));
    filename = FILEPATH + std::string(eyes_filename_char);

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
    botname = botname.substr(0, FS_CHAR8_NAME_SIZE);
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

    posX = posX+80+63;
    posY = posY+64+32-1;
	if (erase_eyes) {
        char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
        sprintf(eyes_filename_char, "player%d.png", (game_save.selected_player+1));
        place_face(std::string(eyes_filename_char), "", st_position(1, 1));

    } else {
        graphLib.copyArea(st_position(posX, posY), &eyes_surface, &graphLib.gameScreen);
	}
}

struct st_position stage_select::select() {
    struct st_position spacing;
	int selection_end=0;

    soundManager.load_music(game_data.stage_select_music_filename);
	soundManager.play_music();
	input.clean();
	timer.delay(300);

	select_pos.x = 1;
	select_pos.y = 1;
	spacing.x = 80;
    spacing.y = 68;
    highlight_pos.x = 135-graphLib.RES_DIFF_W;
    highlight_pos.y = 89;
	graphLib.blank_screen();
	light_mode = &s_light;

	graphLib.copyArea(st_position(0, 0), &background, &graphLib.gameScreen);

    int stage_n = 1;
    for (int i=0; i<=2; i++) {
        for (int j=0; j<=2; j++) {
            if (i == 1 && j == 1) {
                continue;
            }
if (gameControl.is_free_version() == true) {
            if (stage_n != DEMO_VERSION_STAGE1 && stage_n != DEMO_VERSION_STAGE2) {
                stage_n++;
                continue;
            }
}

            if (game_save.stages[stage_n] == 0) {
                // @TODO: name could miss \n, so get only 8 characters
                place_face(game_data.stage_face_filename[stage_n], game_data.stages_face_name[stage_n], st_position(j, i));
            } else {
                place_face(std::string(""), game_data.stages_face_name[stage_n], st_position(j, i));
            }
            stage_n++;
        }
    }


    if (finished_stages() < 9) {
        char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
        sprintf(eyes_filename_char, "player%d.png", (game_save.selected_player+1));
        place_face(std::string(eyes_filename_char), "", st_position(1, 1));
	} else {
		place_face("dr_destrin.png", "Dr. D.", st_position(1, 1));
	}

	light_mode = &s_light;

    draw_lib.update_screen();
    std::string press_start_string = "STAGE SELECT - PRESS START";
    graphLib.draw_text(RES_W*0.5-(FONT_SIZE*press_start_string.length())/2, 227, press_start_string);
if (gameControl.is_free_version() == true) {
    graphLib.clear_area(0, 3, RES_W, 11, 0, 0, 0);
    graphLib.draw_centered_text(5, "FREE VERSION", graphLib.gameScreen, st_color(255, 130, 0));
}

	input.clean();
    if (finished_stages() < 9) {
		draw_eyes(select_pos.x, select_pos.y, false);
	}
	timer.delay(200);

	while (selection_end == 0) {
        input.read_input();
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
            timer.delay(200);
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
            timer.delay(200);
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
            timer.delay(200);
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
            timer.delay(200);
            input.clean();
        } else if (input.p1_input[BTN_START] && finished_stages() < 9 && (select_pos.x != 1 || select_pos.y != 1)) {
if (gameControl.is_free_version() == true) {
            int pos_n = select_pos.x + 1 + select_pos.y*3;

            std::cout << ">>>>>>>>>>>>>>>>>> pos_n: " << pos_n << std::endl;

            if (pos_n == DEMO_VERSION_STAGE1 || pos_n == DEMO_VERSION_STAGE2) {
                selection_end = 1;
            } else {
                soundManager.play_sfx(SFX_NPC_HIT);
            }
} else {
            selection_end = 1;
}
        } else if (input.p1_input[BTN_START] && finished_stages() >= 9) {
if (gameControl.is_free_version() == true) {
            selection_end = 0;
} else {
			selection_end = 1;
}
		}
		animate_highlight();
		timer.delay(10);
	}

    //std::cout << "stage_select::select - c.x: " << select_pos.x << ", c.y: " << select_pos.y << std::endl;

	graphLib.blink_screen(255, 255, 255);
	return select_pos;
}
