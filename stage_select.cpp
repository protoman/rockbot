#include "stage_select.h"


#include "file/format.h"
#include "file/file_io.h"
#include "aux_tools/stringutils.h"
#include "strings_map.h"

// INTERNAL GLOBALS
extern timerLib timer;
extern inputLib input;
extern soundLib soundManager;
extern graphicsLib graphLib;
extern std::string FILEPATH;

extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::file_game game_data;

extern bool GAME_FLAGS[FLAG_COUNT];

extern bool leave_game;

#include "graphic/draw.h"
extern draw draw_lib;

#include "game.h"
extern game gameControl;

CURRENT_FILE_FORMAT::file_stage temp_stage_data;

#define STAGE_SELECT_STAGE_TEXT_X 9
#define STAGE_SELECT_STAGE_TEXT_Y 99

#define STAGE_SELECT_LAIR_TEXT_X 227
#define STAGE_SELECT_LAIR_TEXT_Y 99


stage_select::stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]) :
    highlight_animation_white(false), timer_intro(0), highlight_pos(135-graphLib.RES_DIFF_W, 90)
{
    int i;
    for (i=0; i<STAGE_SELECT_COUNT; i++) {
        STAGE_SELECT_SURFACES[i] = &stage_ref[i];
	}
}

short stage_select::finished_stages() const
{
    short total_stages = 0;
    for (int i=0; i<STAGE_COUNT; i++) {
        if (game_save.stages[i] == 1 || GAME_FLAGS[FLAG_ALLWEAPONS]) {
            total_stages++;
        }
    }
    //std::cout << "stage_select::finished_stages: " << total_stages << std::endl;
    return total_stages;
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





int stage_select::pick_stage(int stage_n)
{
    bool finished = false;
    CURRENT_FILE_FORMAT::file_io fio;
    bool can_access_castle = fio.can_access_castle(game_save);
    int max_stage = gameControl.get_last_castle_stage();

    std::string bg_filename = FILEPATH + "/images/backgrounds/stage_select.png";
    graphicsLib_gSurface bg_surface;
    graphLib.surfaceFromFile(bg_filename, &bg_surface);

    std::string beaten_filename = GAMEPATH + "/shared/images/stage_beaten_cross.png";
    graphicsLib_gSurface beaten_surface;
    graphLib.surfaceFromFile(beaten_filename, &beaten_surface);
    graphLib.set_surface_alpha(180, &beaten_surface);

    // adjust to always show castle #1 images
    int area_pos = stage_n*RES_W;
    if (stage_n > CASTLE1_STAGE1) {
        area_pos = CASTLE1_STAGE1*RES_W;
    }

    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);
    if (stage_n <= 8 && (game_save.stages[stage_n] != 0 || GAME_FLAGS[FLAG_ALLWEAPONS])) {
        graphLib.copyArea(st_position(175, 15), &beaten_surface, &graphLib.gameScreen);
    }


    graphicsLib_gSurface background_face_surface;
    graphLib.surfaceFromFile(FILEPATH + "/images/faces/background.png", &background_face_surface);

    graphicsLib_gSurface light_face_surface;
    graphLib.surfaceFromFile(FILEPATH + "/images/faces/background_light.png", &light_face_surface);
    graphicsLib_gSurface dark_face_surface;
    graphLib.surfaceFromFile(FILEPATH + "/images/faces/background_dark.png", &dark_face_surface);

    st_position face_pos = st_position(30, 60+60);
    // draw faces
    for (int i=0; i<=8; i++) {
        face_pos = calc_face_pos(i);
        std::string face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[i];
        graphicsLib_gSurface face_surface;
        graphLib.surfaceFromFile(face_filename, &face_surface);
        graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &background_face_surface, &graphLib.gameScreen);
        graphLib.copyArea(st_position(face_pos.x, face_pos.y), &face_surface, &graphLib.gameScreen);
        if (i <= 8 && (game_save.stages[i] != 0 || GAME_FLAGS[FLAG_ALLWEAPONS])) {
            graphLib.copyArea(st_position(face_pos.x, face_pos.y), &beaten_surface, &graphLib.gameScreen);
        }
    }
    // skull castle
    face_pos = calc_face_pos(9);
    graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &background_face_surface, &graphLib.gameScreen);
    if (can_access_castle) {
        std::string face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[9];
        graphicsLib_gSurface face_surface;
        graphLib.surfaceFromFile(face_filename, &face_surface);
        graphLib.copyArea(st_position(face_pos.x, face_pos.y), &face_surface, &graphLib.gameScreen);
    }

    face_pos = calc_face_pos(stage_n);
    graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &light_face_surface, &graphLib.gameScreen);

    draw_stage_select_text_info(stage_n);
    graphLib.updateScreen();

    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();

    timer.delay(200);
    input.clean();


    while (finished == false) {
        bool moved = false;
        input.read_input();
        if (input.p1_input[BTN_QUIT]) {
            // show leave dialog
#if !defined(PLAYSTATION2) && !defined(PSP) && !defined(WII) && !defined(DREAMCAST)
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                soundManager.stop_music();
                SDL_Quit();
                exit(0);
            }
#endif
        } else if (input.p1_input[BTN_JUMP] || input.p1_input[BTN_START]) {
            if (gameControl.is_free_version() == true && stage_n != INTRO_STAGE && stage_n != DEMO_VERSION_STAGE1 && stage_n != DEMO_VERSION_STAGE2 && stage_n != DEMO_VERSION_STAGE3 && stage_n != DEMO_VERSION_STAGE4) {
                soundManager.play_sfx(SFX_PLAYER_HIT);
            } else {
                soundManager.stop_music();
                std::cout << "stage_select::pick_stage[" << stage_n << "]" << std::endl;
                return stage_n;
            }
        // @TODO: move all houses util not PATH //
        } else if (input.p1_input[BTN_LEFT]) {
            graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &dark_face_surface, &graphLib.gameScreen);
            stage_n--;
            moved = true;
        } else if (input.p1_input[BTN_RIGHT]) {
            graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &dark_face_surface, &graphLib.gameScreen);
            stage_n++;
            moved = true;
        }

        if (moved) {


            soundManager.play_sfx(SFX_CURSOR);

            std::cout << "stage_n[" << stage_n << "], max_stage[" << max_stage << "]" << std::endl;

            if (stage_n < 0) {
                stage_n = max_stage;
            }
            if (stage_n > max_stage) {
                stage_n = 0;
            }

            face_pos = calc_face_pos(stage_n);
            graphLib.copyArea(st_position(face_pos.x-8, face_pos.y-8), &light_face_surface, &graphLib.gameScreen);

            // adjust to always show castle #1 images
            area_pos = stage_n*RES_W;
            if (stage_n > CASTLE1_STAGE1) {
                area_pos = CASTLE1_STAGE1*RES_W;
            }


            input.clean();
            timer.delay(100);
            //graphLib.copyArea(st_rectangle(area_pos, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);
            draw_stage_select_text_info(stage_n);

        }

        graphLib.updateScreen();
        timer.delay(10);

    }

}

st_position stage_select::calc_face_pos(int stage_n)
{
    st_position face_pos;
    if (stage_n >= 5) {
        if (stage_n > 9) {
            face_pos.x = 60*(9-5)+30;
        } else {
            face_pos.x = 60*(stage_n-5)+30;
        }
    } else {
        face_pos.x = 60*(stage_n-1)+90;
    }
    if (stage_n == 1 || stage_n == 3) {
        face_pos.y = 40;
    } else if (stage_n == 2) {
        face_pos.y = 32;
    } else if (stage_n == 6 || stage_n == 8) {
        face_pos.y = RES_H - 30-40;
    } else if (stage_n == 5 || stage_n >= 9) {
        face_pos.y = RES_H - 30-48;
    } else if (stage_n == 7) {
        face_pos.y = RES_H - 30-32;
    } else {
        face_pos.y = 48;
    }
    return face_pos;
}



void stage_select::draw_stage_select_text_info(int stage_n)
{
    // @TODO - if text is more than 11 characters, and have space, break it
    CURRENT_FILE_FORMAT::file_io fio;
    fio.read_stage(temp_stage_data, stage_n);

    // clear panel
    graphLib.clear_area(6, 96, 91, 48, 71, 69, 68);
    graphLib.clear_area(224, 96, 91, 48, 71, 69, 68);

    std::vector<std::string> stage_name_list;
    std::string stage_name(temp_stage_data.name);
    if (stage_name.length() <= 11) {
        stage_name_list.push_back(stage_name);
    } else if (stage_name.find(" ") != std::string::npos) {
        stage_name_list = StringUtils::split(stage_name, " ");
    } else {
        stage_name_list.push_back(stage_name.substr(0, 11));
    }

    graphLib.draw_text(STAGE_SELECT_STAGE_TEXT_X, STAGE_SELECT_STAGE_TEXT_Y, strings_map::get_instance()->get_ingame_string(string_stage_select_stage, game_config.selected_language));
    for (int i=0; i<stage_name_list.size(); i++) {
        graphLib.draw_text(STAGE_SELECT_STAGE_TEXT_X, STAGE_SELECT_STAGE_TEXT_Y+12*(i+1), stage_name_list.at(i));
    }

    std::string boss_name(temp_stage_data.boss.name);
    std::vector<std::string> boss_name_list;
    if (boss_name.length() <= 11) {
        boss_name_list.push_back(boss_name);
    } else if (boss_name.find(" ") != std::string::npos) {
        boss_name_list = StringUtils::split(boss_name, " ");
    } else {
        boss_name_list.push_back(boss_name.substr(0, 11));
    }

    graphLib.draw_text(STAGE_SELECT_LAIR_TEXT_X, STAGE_SELECT_LAIR_TEXT_Y, strings_map::get_instance()->get_ingame_string(string_stage_select_lair_of, game_config.selected_language));
    for (int i=0; i<boss_name_list.size(); i++) {
        graphLib.draw_text(STAGE_SELECT_LAIR_TEXT_X, STAGE_SELECT_LAIR_TEXT_Y+12*(i+1), boss_name_list.at(i));
    }

    // clear selection text
    int text_pos_y = 227;
    graphLib.clear_area(21, text_pos_y, 76, 16, 19, 2, 36);
    graphLib.clear_area(114, text_pos_y, 206, 16, 19, 2, 36);
    graphLib.draw_text(21, text_pos_y, "[SELECT]", st_color(250, 250, 250));
    if (gameControl.is_free_version() == true && stage_n != INTRO_STAGE && stage_n != DEMO_VERSION_STAGE1 && stage_n != DEMO_VERSION_STAGE2 && stage_n != DEMO_VERSION_STAGE3 && stage_n != DEMO_VERSION_STAGE4) {
        graphLib.draw_text(114, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_unavailable, game_config.selected_language), st_color(250, 250, 250));
    } else {
        graphLib.draw_text(114, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_enter_stage, game_config.selected_language), st_color(250, 250, 250));
    }

}

bool stage_select::walk_path(int incx, int incy, st_position& pos, format_v4::file_stage_select map_data)
{
    if (map_data.points[pos.x+incx][pos.y+incy] == STAGE_SELECT_EDIT_MODE_LOCKED) {
        soundManager.play_sfx(SFX_PLAYER_HIT);
        return false;
    }


    while (map_data.points[pos.x+incx][pos.y+incy] != STAGE_SELECT_EDIT_MODE_LOCKED) {
        pos.x += incx;
        pos.y += incy;
        // stop in stage points
        if (map_data.points[pos.x][pos.y] > 10 || map_data.points[pos.x][pos.y] == STAGE_SELECT_EDIT_MODE_CASTLE) {
            return true;
        }
        // stop in bifurcations
        int forks = 0;
        if (map_data.points[pos.x+1][pos.y] == STAGE_SELECT_EDIT_MODE_PATH) {
            forks++;
        }
        if (map_data.points[pos.x-1][pos.y] == STAGE_SELECT_EDIT_MODE_PATH) {
            forks++;
        }
        if (map_data.points[pos.x][pos.y-1] == STAGE_SELECT_EDIT_MODE_PATH) {
            forks++;
        }
        if (map_data.points[pos.x][pos.y+1] == STAGE_SELECT_EDIT_MODE_PATH) {
            forks++;
        }
        if (forks > 2) {
            return true;
        }
    }

    return true;
}



