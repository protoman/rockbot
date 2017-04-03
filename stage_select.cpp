#include <cstdlib>
#include "stage_select.h"

#include <string>

#include "file/format.h"
#include "file/file_io.h"

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

CURRENT_FILE_FORMAT::file_stage temp_stage_data;


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



int stage_select::pick_stage(int start_stage)
{
    bool finished = false;
    std::string boss_name;
    std::string bg_filename = FILEPATH + "/images/backgrounds/stage_select_map.png";
    std::string face_filename;

    graphicsLib_gSurface bg_surface;
    graphicsLib_gSurface face_surface;
    graphicsLib_gSurface stage_icon_beaten;
    std::string stage_icon_filename = FILEPATH + "/images/backgrounds/map_icon_beaten.png";
    graphLib.surfaceFromFile(stage_icon_filename, &stage_icon_beaten);

    graphLib.surfaceFromFile(bg_filename, &bg_surface);
    int bg_frames = 1;
    long bg_frame_timer = timer.getTimer()+100;
    if (bg_surface.width >= RES_W*2) {
        bg_frames = bg_surface.width/RES_W;
    }
    int current_bg_frame = 0;


    CURRENT_FILE_FORMAT::file_io fio;
    CURRENT_FILE_FORMAT::file_stage_select map_data;
    fio.read_stage_select_data(map_data);

    // put stages icons
    for (int i=0; i<RES_W/TILESIZE; i++) {
        for (int j=0; j<RES_H/TILESIZE; j++) {
            if (map_data.points[i][j] > 10) {
                int stage_n = map_data.points[i][j]-10;
                if (stage_n >=1 && stage_n <= 8) {
                    if (game_save.stages[stage_n] == 1) {
                        graphLib.copyArea(st_position(i*TILESIZE, j*TILESIZE), &stage_icon_beaten, &bg_surface);
                    }
                }
            }
        }
    }



    graphLib.copyArea(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);


    st_position pos;
    st_position previous_pos;
    if (start_stage <= 0) {
        pos.x = 1;
        pos.y = 3;
    // search in map array for the start point
    } else {
        for (int i=0; i<RES_W/TILESIZE; i++) {
            for (int j=0; j<RES_H/TILESIZE; j++) {
                if ((start_stage >= CASTLE1_STAGE1 && map_data.points[i][j] == STAGE_SELECT_EDIT_MODE_CASTLE) || map_data.points[i][j]-10 == start_stage) {
                    pos.x = i;
                    pos.y = j;
                    fio.read_stage(temp_stage_data, start_stage);
                    face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[start_stage];
                    boss_name = temp_stage_data.boss.name;
                    graphLib.surfaceFromFile(face_filename, &face_surface);
                    break;
                }
            }
        }
    }
    previous_pos = pos;


    std::string cursor_filename = FILEPATH + "/images/backgrounds/castle_skull_point.png";
    graphicsLib_gSurface cursor_surface;
    graphLib.surfaceFromFile(cursor_filename, &cursor_surface);
    graphLib.copyArea(st_position(pos.x*TILESIZE, pos.y*TILESIZE), &cursor_surface, &graphLib.gameScreen);





    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();


    graphLib.updateScreen();

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
        } else if (input.p1_input[BTN_START]) {
            if (map_data.points[pos.x][pos.y] > 10) {
                int stage_n = map_data.points[pos.x][pos.y]-10;
                soundManager.stop_music();
                return stage_n;
            } else if (map_data.points[pos.x][pos.y] == STAGE_SELECT_EDIT_MODE_CASTLE && fio.can_access_castle(game_save) == true) {
                soundManager.stop_music();
                return CASTLE1_STAGE1;
            } else {
                soundManager.play_sfx(SFX_PLAYER_HIT);
            }
        // @TODO: move all houses util not PATH //
        } else if (input.p1_input[BTN_LEFT]) {
            moved = walk_path(-1, 0, pos, map_data);
            input.clean();
            timer.delay(100);
        } else if (input.p1_input[BTN_RIGHT]) {
            moved = walk_path(1, 0, pos, map_data);
            input.clean();
            timer.delay(100);
        } else if (input.p1_input[BTN_UP]) {
            moved = walk_path(0, -1, pos, map_data);
            input.clean();
            timer.delay(100);
        } else if (input.p1_input[BTN_DOWN]) {
            moved = walk_path(0, 1, pos, map_data);
            input.clean();
            timer.delay(100);
        }

        if (moved == true) {
            soundManager.play_sfx(SFX_CURSOR);
            // show move animation
            if (previous_pos.x != pos.x || previous_pos.y != pos.y) {
                int diffx = previous_pos.x - pos.x;
                int diffy = previous_pos.y - pos.y;

                int adjust = 0;
                for (int i=0; i<abs(diffx*TILESIZE); i+=2) {
                    if (diffx > 0) {
                        adjust-=2;
                    } else {
                        adjust+=2;
                    }
                    graphLib.copyArea(st_rectangle(RES_W*current_bg_frame, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);
                    graphLib.copyArea(st_position(previous_pos.x*TILESIZE+adjust, previous_pos.y*TILESIZE), &cursor_surface, &graphLib.gameScreen);
                    graphLib.clear_area(0, 180, RES_W, 60, 0, 0, 0);
                    graphLib.updateScreen();
                    timer.delay(1);
                }
                adjust = 0;
                for (int i=0; i<abs(diffy*TILESIZE); i+=2) {
                    if (diffy > 0) {
                        adjust-=2;
                    } else {
                        adjust+=2;
                    }
                    graphLib.copyArea(st_rectangle(RES_W*current_bg_frame, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);
                    graphLib.copyArea(st_position(previous_pos.x*TILESIZE, previous_pos.y*TILESIZE+adjust), &cursor_surface, &graphLib.gameScreen);
                    graphLib.clear_area(0, 180, RES_W, 60, 0, 0, 0);
                    graphLib.updateScreen();
                    timer.delay(1);
                }
            }
            previous_pos = pos;
        }

        graphLib.copyArea(st_rectangle(RES_W*current_bg_frame, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);
        graphLib.copyArea(st_position(pos.x*TILESIZE, pos.y*TILESIZE), &cursor_surface, &graphLib.gameScreen);

        // show stage and boss data, if is over a stage point //
        graphLib.clear_area(0, 180, RES_W, 60, 0, 0, 0);
        if (map_data.points[pos.x][pos.y] > 10) {
            int stage_n = map_data.points[pos.x][pos.y]-10;
            if (moved == true) {
                fio.read_stage(temp_stage_data, map_data.points[pos.x][pos.y]-10);
                face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[stage_n];
                boss_name = temp_stage_data.boss.name;
                graphLib.surfaceFromFile(face_filename, &face_surface);
            }
            graphLib.draw_text(10, 190, temp_stage_data.name);
            graphLib.draw_text(10, 205, "LAIR OF");
            graphLib.draw_text(72, 205, boss_name);
            graphLib.copyArea(st_position(RES_W-52, 190), &face_surface, &graphLib.gameScreen);
        // show castle data
        } else if (map_data.points[pos.x][pos.y] == STAGE_SELECT_EDIT_MODE_CASTLE) {
            if (fio.can_access_castle(game_save) == true) {
                if (moved == true) {
                    fio.read_stage(temp_stage_data, CASTLE1_STAGE1);
                    face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[CASTLE1_STAGE1];
                    boss_name = temp_stage_data.boss.name;
                    graphLib.surfaceFromFile(face_filename, &face_surface);
                }
                graphLib.draw_text(10, 190, temp_stage_data.name);
                graphLib.draw_text(10, 205, "LAIR OF");
                graphLib.draw_text(72, 205, boss_name);
                graphLib.copyArea(st_position(RES_W-52, 190), &face_surface, &graphLib.gameScreen);
            } else {
                graphLib.draw_text(10, 190, "LOCKED");
            }
        }

        if (timer.getTimer() > bg_frame_timer) {
            bg_frame_timer = timer.getTimer()+200;
            current_bg_frame++;
            //std::cout << "inc current_bg_frame[" << current_bg_frame << "]" << std::endl;
            if (current_bg_frame >= bg_frames) {
                //std::cout << "reset current_bg_frame" << std::endl;
                current_bg_frame = 0;
            }
        }

        graphLib.updateScreen();
        timer.delay(10);
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



