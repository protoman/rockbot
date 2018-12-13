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

extern std::string GAMENAME;

CURRENT_FILE_FORMAT::file_stage temp_stage_data;

#define STAGE_SELECT_STAGE_TEXT_X 9
#define STAGE_SELECT_STAGE_TEXT_Y 13

#define STAGE_SELECT_LAIR_TEXT_X 9
#define STAGE_SELECT_LAIR_TEXT_Y 201


stage_select::stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]) :
    highlight_animation_white(false), timer_intro(0), highlight_pos(135-graphLib.RES_DIFF_W, 90)
{
    int i;
    for (i=0; i<STAGE_SELECT_COUNT; i++) {
        STAGE_SELECT_SURFACES[i] = &stage_ref[i];
	}
    if (GAMENAME == "Rockbot1") {
        // regular stages, rockbot1 order
        stage_select_point_list.push_back(st_position(254, 82));    // KYOTO
        stage_select_point_list.push_back(st_position(177, 118));   // CENTRAL AFRICA
        stage_select_point_list.push_back(st_position(126, 132));   // BRAZIL
        stage_select_point_list.push_back(st_position(260, 140));   // AUSTRALIA
        stage_select_point_list.push_back(st_position(178, 92));    // EGYPT
        stage_select_point_list.push_back(st_position(156, 69));    // UK
        stage_select_point_list.push_back(st_position(222, 96));    // INDIA
        stage_select_point_list.push_back(st_position(141, 52));    // GROELANDIA
        stage_select_point_list.push_back(st_position(108, 80));    // USA/NY
    } else {
        stage_select_point_list.push_back(st_position(254, 82));    // KYOTO
        stage_select_point_list.push_back(st_position(108, 80));    // USA/NY
        stage_select_point_list.push_back(st_position(222, 96));    // INDIA
        stage_select_point_list.push_back(st_position(260, 140));   // AUSTRALIA
        stage_select_point_list.push_back(st_position(126, 132));   // BRAZIL
        stage_select_point_list.push_back(st_position(177, 118));   // CENTRAL AFRICA
        stage_select_point_list.push_back(st_position(156, 69));    // UK
        stage_select_point_list.push_back(st_position(175, 81));    // GREECE
        stage_select_point_list.push_back(st_position(141, 52));    // GROELANDIA
    }

    // castle stages
    stage_select_point_list.push_back(st_position(127, 168));   // ANTARCTICA
    stage_select_point_list.push_back(st_position(192, 66));    // MOSCOW
    stage_select_point_list.push_back(st_position(102, 96));    // CUBA
    stage_select_point_list.push_back(st_position(140, 84));    // AÇORES
    stage_select_point_list.push_back(st_position(221, 131));   // PACIFIC-RIM

    std::string stage_select_map_point_filename = GAMEPATH + "/shared/images/backgrounds/stage_select_point.png";
    graphLib.surfaceFromFile(stage_select_map_point_filename, &stage_select_map_point_surface);

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

short stage_select::finished_stages() const
{
    short total_stages = 0;
    for (int i=0; i<MAX_STAGES; i++) {
        if (game_save.stages[i] == 1 || GAME_FLAGS[FLAG_ALLWEAPONS]) {
            total_stages++;
        }
    }
    //std::cout << "stage_select::finished_stages: " << total_stages << std::endl;
    return total_stages;
}


void stage_select::move_highlight(Sint8 x_inc, Sint8 y_inc) {

    //std::cout << "STAGESELECT::move_highlight  - xinc: " << (int)x_inc << ", yinc: " << (int)y_inc << std::endl;
    // CLEAR AREA

	graphLib.copyArea(highlight_pos, &s_dark, &graphLib.gameScreen);
	highlight_pos.x += x_inc;
	highlight_pos.y += y_inc;


    graphLib.clear_area(1, 11, RES_W, 15, 25, 77, 129);
    if (select_pos.x != 1 || select_pos.y != 1) {
        CURRENT_FILE_FORMAT::file_io fio;
        short stage_n = (select_pos.x + select_pos.y*3) + 1;
        if (select_pos.y == 2 || (select_pos.y == 1 && select_pos.x > 1)) {
            stage_n = (select_pos.x + select_pos.y*3);
        }
        fio.read_stage(temp_stage_data, stage_n);
        std::string stage_name(temp_stage_data.name);
        std::string boss_name(temp_stage_data.boss.name);
        graphLib.draw_text(3, 12, strings_map::get_instance()->get_ingame_string(string_stage_select_stage, game_config.selected_language) + " " + stage_name, graphLib.gameScreen);
        //graphLib.draw_text(9, 5, strings_map::get_instance()->get_ingame_string(string_stage_select_lair_of, game_config.selected_language) + " " + boss_name, graphLib.gameScreen);
    }

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
        graphLib.place_face(face_file, st_position(pos.x*80+64, pos.y*62+36));
    }
    botname = botname.substr(0, FS_CHAR8_NAME_SIZE);
    txtPosX = pos.x*80+78-(botname.size()*7)*0.5;
    txtPosY = pos.y*62+78;
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
        timer_intro = timer.getTimer()+100;
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

    posX = posX+80+64;
    posY = posY+64+32+1;
	if (erase_eyes) {
        char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
        sprintf(eyes_filename_char, "player%d.png", (game_save.selected_player+1));
        place_face(std::string(eyes_filename_char), "", st_position(1, 1));

    } else {
        graphLib.copyArea(st_position(posX, posY), &eyes_surface, &graphLib.gameScreen);
	}
}


int stage_select::pick_stage(int selected_stage_n) {
    struct st_position spacing;
    int selection_end=0;

    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<std::string> boss_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/boss_credits.txt");

#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "### ROCKBOT2 ###", "### stage_select::pick_stage::boss_credits_data.size[%d] ###", boss_credits_data.size());
#endif


    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();
    input.clean();
    timer.delay(300);

    select_pos.x = 1;
    select_pos.y = 1;
    spacing.x = 80;
    spacing.y = 62;
    highlight_pos.x = 136-graphLib.RES_DIFF_W;
    highlight_pos.y = 90;
    graphLib.blank_screen();
    light_mode = &s_light;

    int stage_n = 1;

    //graphLib.copyArea(st_position(0, 0), &background, &graphLib.gameScreen);
    std::string bg_filename = FILEPATH + "/images/backgrounds/stage_select.png";
    graphicsLib_gSurface bg_surface;
    graphLib.surfaceFromFile(bg_filename, &bg_surface);
    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);

    short text_pos_y = 217;
    graphLib.draw_text(21, text_pos_y, "[SELECT]", st_color(250, 250, 250));
    if (gameControl.is_free_version() == true && stage_n != INTRO_STAGE && stage_n != DEMO_VERSION_STAGE1 && stage_n != DEMO_VERSION_STAGE2 && stage_n != DEMO_VERSION_STAGE3 && stage_n != DEMO_VERSION_STAGE4) {
        graphLib.draw_text(184, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_unavailable, game_config.selected_language), st_color(250, 250, 250));
    } else {
        graphLib.draw_text(184, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_enter_stage, game_config.selected_language), st_color(250, 250, 250));
    }
    /*
    std::string stage_name(temp_stage_data.name);
    std::string boss_name(temp_stage_data.boss.name);
    graphLib.draw_text(9, 5, strings_map::get_instance()->get_ingame_string(string_stage_select_lair_of, game_config.selected_language) + " " + boss_name, graphLib.gameScreen);
    */


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


    if (gameControl.is_free_version() == true || finished_stages() < 9) {
        char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
        sprintf(eyes_filename_char, "player%d.png", (game_save.selected_player+1));
        place_face(std::string(eyes_filename_char), "", st_position(1, 1));
    } else {
        place_face("dr_destrin.png", "Dr. D.", st_position(1, 1));
    }

    light_mode = &s_light;

    draw_lib.update_screen();
    if (gameControl.is_free_version() == true) {
        graphLib.clear_area(0, 3, RES_W, 11, 0, 0, 0);
        graphLib.draw_centered_text(5, "FREE VERSION", graphLib.gameScreen, st_color(255, 130, 0));
    }

    input.clean();
    if (gameControl.is_free_version() == true || finished_stages() < 9) {
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
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.y++;
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(0, spacing.y);
            timer.delay(200);
            input.clean();
        } else if (select_pos.y > 0 && input.p1_input[BTN_UP]) {
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.y--;
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(0, -spacing.y);
            timer.delay(200);
            input.clean();
        } else if (select_pos.x > 0 && input.p1_input[BTN_LEFT]) {
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.x--;
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(-spacing.x, 0);
            timer.delay(200);
            input.clean();
        } else if (select_pos.x < 2 && input.p1_input[BTN_RIGHT]) {
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, true);
            }
            select_pos.x++;
            if (gameControl.is_free_version() == true || finished_stages() < 9) {
                draw_eyes(select_pos.x, select_pos.y, false);
            }
            soundManager.play_sfx(SFX_CURSOR);
            move_highlight(spacing.x, 0);
            timer.delay(200);
            input.clean();
        } else if ((input.p1_input[BTN_START] || input.p1_input[BTN_JUMP]) && (gameControl.is_free_version() == true || finished_stages() < 9) && (select_pos.x != 1 || select_pos.y != 1)) {
            if (gameControl.is_free_version() == true) {
                int pos_n = select_pos.x + 1 + select_pos.y*3;

                std::cout << ">>>>>>>>>>>>>>>>>> pos_n: " << pos_n << std::endl;

                if (pos_n == DEMO_VERSION_STAGE1 || pos_n == DEMO_VERSION_STAGE2+1) { // +1 because of middle/center castle selection
                    selection_end = 1;
                } else {
                    soundManager.play_sfx(SFX_NPC_HIT);
                }
            } else {
                selection_end = 1;
            }
        } else if ((input.p1_input[BTN_START] || input.p1_input[BTN_JUMP]) && finished_stages() >= 9) {
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

    // center face
    if (select_pos.y == 1 && select_pos.x ==1) {
        short castle_stage_n = gameControl.get_last_castle_stage();
        if (castle_stage_n > STAGE8) {
            return castle_stage_n;
        }
        // @TODO //
    }

    // after center face, subtract one
    if (select_pos.y == 2 || (select_pos.y == 1 && select_pos.x > 1)) {
        return (select_pos.x + select_pos.y*3);
    }

    return (select_pos.x + select_pos.y*3) + 1;
}


int stage_select::pick_stage_new(int stage_n)
{
    bool finished = false;
    CURRENT_FILE_FORMAT::file_io fio;
    bool can_access_castle = fio.can_access_castle(game_save);
    if (GAME_FLAGS[FLAG_ALLWEAPONS]) {
        can_access_castle = true;
    }
    int max_stage = gameControl.get_last_castle_stage();

    std::string bg_filename = GAMEPATH + "/shared/images/backgrounds/stage_select.png";
    graphicsLib_gSurface bg_surface;
    graphLib.surfaceFromFile(bg_filename, &bg_surface);

    std::string stage_select_bar_img_str = GAMEPATH + "/shared/images/backgrounds/stage_select_bar.png";
    graphicsLib_gSurface stage_select_bar_img;
    graphLib.surfaceFromFile(stage_select_bar_img_str, &stage_select_bar_img);

    std::string beaten_filename = GAMEPATH + "/shared/images/stage_beaten_cross.png";

    graphLib.surfaceFromFile(beaten_filename, &beaten_surface);
    graphLib.set_surface_alpha(180, &beaten_surface);

    // adjust to always show castle #1 images
    int area_pos = stage_n*RES_W;
    if (stage_n > CASTLE1_STAGE1) {
        area_pos = CASTLE1_STAGE1*RES_W;
    }

    graphLib.copyArea(st_position(0, 0), &bg_surface, &graphLib.gameScreen);

    // draw map points
    for (int i=0; i<stage_select_point_list.size(); i++) {
        graphLib.copyArea(st_rectangle(0, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(i).x-2, stage_select_point_list.at(i).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
        if (i >= CASTLE1_STAGE1 && game_save.stages[i] == 0) { // break after loop, because stage is available before beaten
            break;
        }
    }


    std::string face_bg_filename = FILEPATH + "/images/faces/background.png";
    graphicsLib_gSurface face_bg_surface;
    graphLib.surfaceFromFile(face_bg_filename, &face_bg_surface);
    graphLib.copyArea(st_position(4, 36), &face_bg_surface, &graphLib.gameScreen);



    // top bar
    graphLib.copyArea(st_position(0, 0), &stage_select_bar_img, &graphLib.gameScreen);
    // bottom bar
    graphLib.copyArea(st_position(0, RES_H-stage_select_bar_img.height-20), &stage_select_bar_img, &graphLib.gameScreen);


    graphicsLib_gSurface background_face_surface;
    graphLib.surfaceFromFile(FILEPATH + "/images/faces/background.png", &background_face_surface);

    st_position face_pos = st_position(12, 44);

    show_stage_face(face_pos.x, face_pos.y, stage_n);


    draw_stage_select_text_info(stage_n);
    graphLib.updateScreen();

    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();

    timer.delay(200);
    input.clean();

    // red point for selected stage
    graphLib.copyArea(st_rectangle(stage_select_map_point_surface.width/2, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);

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
            // erase yellow point
            if (stage_n >= 0 && stage_n < stage_select_point_list.size()) {
                graphLib.copyArea(st_rectangle(0, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
            }
            stage_n--;
            moved = true;
        } else if (input.p1_input[BTN_RIGHT]) {
            // erase yellow point
            if (stage_n >= 0 && stage_n < stage_select_point_list.size()) {
                graphLib.copyArea(st_rectangle(0, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
            }
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

            show_stage_face(face_pos.x, face_pos.y, stage_n);

            // adjust to always show castle #1 images
            area_pos = stage_n*RES_W;
            if (stage_n > CASTLE1_STAGE1) {
                area_pos = CASTLE1_STAGE1*RES_W;
            }

            // yellow point
            if (stage_n >= 0 && stage_n < stage_select_point_list.size()) {
                graphLib.copyArea(st_rectangle(stage_select_map_point_surface.width/2, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
            }

            input.clean();
            timer.delay(100);
            //graphLib.copyArea(st_rectangle(area_pos, 0, RES_W, RES_H), st_position(0, 0), &bg_surface, &graphLib.gameScreen);
            draw_stage_select_text_info(stage_n);

        }

        // animate point
        if (map_point_animation.timer < timer.getTimer()) {
            map_point_animation.timer = timer.getTimer() + MAP_POINT_ANIMATION_DELAY;
            if (map_point_animation.state == 0) {
                map_point_animation.state = 1;
            } else {
                map_point_animation.state = 0;
            }
            if (stage_n >= 0 && stage_n < stage_select_point_list.size()) {
                if (map_point_animation.state == 0) {
                    graphLib.copyArea(st_rectangle(0, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
                } else {
                    graphLib.copyArea(st_rectangle(stage_select_map_point_surface.width/2, 0, stage_select_map_point_surface.width/2, stage_select_map_point_surface.height), st_position(stage_select_point_list.at(stage_n).x-2, stage_select_point_list.at(stage_n).y-2), &stage_select_map_point_surface, &graphLib.gameScreen);
                }
            }

        }

        graphLib.updateScreen();
        timer.delay(10);

    }

}

void stage_select::show_stage_face(int x, int y, int stage_n)
{
    std::string face_filename;
    if (stage_n < CASTLE1_STAGE1) {
        face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[stage_n];
    } else {
        face_filename = FILEPATH + "/images/faces/" + game_data.stage_face_filename[9];
    }

    std::cout << ">>>>> face_filename[" << face_filename << "]" << std::endl;

    graphicsLib_gSurface face_surface;
    graphLib.surfaceFromFile(face_filename, &face_surface);
    graphLib.clear_area(x, y, face_surface.width, face_surface.height, 8, 25, 42);
    graphLib.copyArea(st_position(x, y), &face_surface, &graphLib.gameScreen);

    if (stage_n <= 8 && game_save.stages[stage_n] != 0) {
        graphLib.copyArea(st_position(x, y), &beaten_surface, &graphLib.gameScreen);
    }

}

st_position stage_select::calc_face_pos(int stage_n)
{
    st_position face_pos;

    if (stage_n <= 5) { // first row
        face_pos.x = 50*(stage_n)+10;
    } else if (stage_n < CASTLE1_STAGE1) { // second row
        face_pos.x = 80*(stage_n-5)+30;
    } else { // third row - castle stages
        face_pos.x = 60*(stage_n-9)+30;
    }

    if (stage_n >= 0 && stage_n <= 4) {
        face_pos.y = 10;
    } else if (stage_n < CASTLE1_STAGE1) {
        face_pos.y = RES_H - 30-78;
    } else {
        face_pos.y = RES_H - 30;
    }
    return face_pos;
}



void stage_select::draw_stage_select_text_info(int stage_n)
{
    // @TODO - if text is more than 11 characters, and have space, break it
    CURRENT_FILE_FORMAT::file_io fio;
    fio.read_stage(temp_stage_data, stage_n);

    graphicsLib_gSurface top_panel;
    graphicsLib_gSurface bottom_panel;
    graphicsLib_gSurface top_panel_copy;
    graphicsLib_gSurface bottom_panel_copy;
    graphLib.initSurface(st_size(RES_W, 18), &top_panel);
    graphLib.initSurface(st_size(RES_W, 18), &bottom_panel);
    graphLib.initSurface(st_size(RES_W, 18), &top_panel_copy);
    graphLib.initSurface(st_size(RES_W, 18), &bottom_panel_copy);
    graphLib.clear_surface_area(0, 0, top_panel.width, top_panel.height, 8, 25, 42, top_panel);
    graphLib.clear_surface_area(0, 0, bottom_panel.width, bottom_panel.height, 8, 25, 42, bottom_panel);
    graphLib.clear_surface_area(0, 0, top_panel_copy.width, top_panel_copy.height, 8, 25, 42, top_panel_copy);
    graphLib.clear_surface_area(0, 0, bottom_panel_copy.width, bottom_panel_copy.height, 8, 25, 42, bottom_panel_copy);
    graphLib.copyArea(st_rectangle(0, 8, RES_W, 18), st_position(0, 0), &graphLib.gameScreen, &top_panel_copy);
    graphLib.copyArea(st_position(0, RES_H-top_panel.height-20), &graphLib.gameScreen, &bottom_panel_copy);

    // clear panel
    //graphLib.clear_area(0, 7, RES_W, 18, 8, 25, 42);
    //graphLib.clear_area(0, RES_H-45, RES_W, 18, 8, 25, 42);

    std::string stage_name(temp_stage_data.name);
    std::string boss_name(temp_stage_data.boss.name);
    graphLib.draw_text(9, 4, strings_map::get_instance()->get_ingame_string(string_stage_select_stage, game_config.selected_language) + " " + stage_name, top_panel);
    graphLib.draw_text(9, 5, strings_map::get_instance()->get_ingame_string(string_stage_select_lair_of, game_config.selected_language) + " " + boss_name, bottom_panel);

    // animate
    for (int i=0; i<=RES_W; i+=6) {
        // top
        graphLib.copyArea(st_position(i, 8), &top_panel_copy, &graphLib.gameScreen);
        graphLib.copyArea(st_position(-RES_W+i, 8), &top_panel, &graphLib.gameScreen);
        // bottom
        graphLib.copyArea(st_position(-i, RES_H-bottom_panel.height-20-7), &bottom_panel, &graphLib.gameScreen);
        graphLib.copyArea(st_position(RES_W-i, RES_H-bottom_panel.height-20-7), &bottom_panel, &graphLib.gameScreen);

        graphLib.updateScreen();
        timer.delay(1);
    }

    // clear selection text
    short text_pos_y = 227;
    graphLib.clear_area(21, text_pos_y, 76, 16, 19, 2, 36);
    graphLib.clear_area(184, text_pos_y, 206, 16, 19, 2, 36);
    graphLib.draw_text(21, text_pos_y, "[SELECT]", st_color(250, 250, 250));
    if (gameControl.is_free_version() == true && stage_n != INTRO_STAGE && stage_n != DEMO_VERSION_STAGE1 && stage_n != DEMO_VERSION_STAGE2 && stage_n != DEMO_VERSION_STAGE3 && stage_n != DEMO_VERSION_STAGE4) {
        graphLib.draw_text(184, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_unavailable, game_config.selected_language), st_color(250, 250, 250));
    } else {
        graphLib.draw_text(184, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_enter_stage, game_config.selected_language), st_color(250, 250, 250));
    }
}




