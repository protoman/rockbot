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

#define faces_y_top 41
#define faces_x_left 64
#define faces_y_spacing 64
#define faces_x_spacing 80

stage_select::stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]) :
    highlight_animation_white(false), timer_intro(0), highlight_pos(135-graphLib.RES_DIFF_W, 90)
{
    int i;
    for (i=0; i<STAGE_SELECT_COUNT; i++) {
        STAGE_SELECT_SURFACES[i] = &stage_ref[i];
	}
    if (GAMENAME == "RockBot1") {
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

    std::string filename = GAMEPATH + "/shared/images/backgrounds/stage_select.png";
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
    return total_stages;
}



//void stage_select::place_face(int face_n, char botname[20], short int posx, short int posy) {
void stage_select::place_face(std::string face_file, std::string botname, st_position pos) const {
	// TODO - calculate the text position according to it's strlen
	int txtPosX, txtPosY;

	if (face_file.size() > 0) {
        graphLib.place_face(face_file, st_position(pos.x*80+64, pos.y*62+36));
    }
    botname = botname.substr(0, FS_CHAR8_NAME_SIZE);
    txtPosX = pos.x*80+78-(botname.size()*7)*0.5;
    txtPosY = pos.y*62+78;
    graphLib.draw_text(txtPosX, txtPosY, botname);
}

std::vector<string> stage_select::build_stage_boss_names_list()
{
    std::vector<std::string> boss_name_list;
    for (int i=INTRO_STAGE; i<STAGE_COUNT; i++) {
        fio.read_stage(temp_stage_data, i);
        std::string boss_name(temp_stage_data.boss.name);
        boss_name_list.push_back(boss_name);
    }
    return boss_name_list;
}

std::vector<string> stage_select::build_stage_names_list()
{
    std::vector<string> stage_name_list;
    for (int i=0; i<STAGE_COUNT; i++) {
        fio.read_stage(temp_stage_data, i);
        std::string stage_name(temp_stage_data.name);
        stage_name = std::string(temp_stage_data.name);
        stage_name_list.push_back(stage_name);
    }
    return stage_name_list;
}

void stage_select::build_stage_faces_surfaces(graphicsLib_gSurface face_list[])
{
    for (int i=0; i<STAGE_COUNT; i++) {
        fio.read_stage(temp_stage_data, i);
        char face_filename[512];
        if (game_data.game_style == GAME_STYLE_VINTAGE && i <=8) {
            sprintf(face_filename, "%s/images/faces/boss_%d.png", FILEPATH.c_str(), i);
        } else {
            sprintf(face_filename, "%s/images/faces/%d.png", FILEPATH.c_str(), i);
            if (i < 10)  {
                sprintf(face_filename, "%s/images/faces/0%d.png", FILEPATH.c_str(), i);
            }
        }
        if (fio.file_exists(StringUtils::clean_filename(face_filename))) {
            graphLib.surfaceFromFile(StringUtils::clean_filename(face_filename), &face_list[i]);
        } else {
            graphLib.initSurface(st_size(32, 64), &face_list[i]);
        }
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

    graphicsLib_gSurface face_surface;
    graphLib.surfaceFromFile(face_filename, &face_surface);
    graphLib.clear_area(x, y, face_surface.width, face_surface.height, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
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




int stage_select::pick_stage(int stage_n)
{
    if (game_data.game_style == GAME_STYLE_VINTAGE) {
        return pick_stage_classic_style(stage_n);
    }
    bool can_access_castle = fio.can_access_castle(game_save);
    bool blink_state = false;
    long blink_timer = timer.getTimer() + 300;
    if (GAME_FLAGS[FLAG_ALLWEAPONS]) {
        can_access_castle = true;
    }
    int max_stage = gameControl.get_last_castle_stage();

    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();

    graphLib.showSurface(&background);
    graphLib.draw_text(8, 20, strings_map::get_instance()->get_ingame_string(strings_stage_select_pick_mission), graphLib.gameScreen);
    short text_pos_y = 209;
    graphLib.draw_text(26, text_pos_y, "[" + strings_map::get_instance()->get_ingame_string(strings_stage_select_select) + "]", st_color(250, 250, 250));
    graphLib.draw_text(189, text_pos_y, strings_map::get_instance()->get_ingame_string(string_stage_select_enter_stage), st_color(250, 250, 250));

    std::string stage_selection_icon_filename = FILEPATH + "/images/backgrounds/stage_selection_icon.png";
    graphicsLib_gSurface stage_selection_icon;
    graphLib.surfaceFromFile(stage_selection_icon_filename, &stage_selection_icon);

    graphicsLib_gSurface dark_surface;
    graphLib.initSurface(st_size(32, 30), &dark_surface);
    graphLib.clear_surface_area(0, 0, 32, 30, 0, 0, 0, dark_surface);
    graphLib.set_surface_alpha(150, &dark_surface);

    graphLib.updateScreen();

    timer.delay(200);
    input.clean();

    int icon_y = 40;
    int icon_x = 24;
    int icon_spacing_y = 48;

    std::vector<std::string> boss_name_list = build_stage_boss_names_list();
    std::vector<std::string> stage_name_list = build_stage_names_list();
    graphicsLib_gSurface face_list[STAGE_COUNT];
    build_stage_faces_surfaces(face_list);

    bool finished = false;
    while (finished == false) {
        bool moved = false;

        int cursor_level_n = 0;
        for (int j=0; j<3; j++) {
            for (int i=0; i<6; i++) {
                if (cursor_level_n < CASTLE1_STAGE1 || (can_access_castle && cursor_level_n<= max_stage)) {
                    if (cursor_level_n == stage_n) { // current selection
                        if (blink_state == true) {
                            graphLib.showSurfaceRegionAt(&stage_selection_icon, st_rectangle(72, 0, 36, 36), st_position(icon_x+(i*48), (j*icon_spacing_y)+icon_y));
                        } else {
                            graphLib.showSurfaceRegionAt(&stage_selection_icon, st_rectangle(36, 0, 36, 36), st_position(icon_x+(i*48), (j*icon_spacing_y)+icon_y));
                        }
                    } else if (game_save.stages[cursor_level_n] == 1) { // finished stage
                        graphLib.showSurfaceRegionAt(&stage_selection_icon, st_rectangle(36, 0, 36, 36), st_position(icon_x+(i*48), (j*icon_spacing_y)+icon_y));
                    } else { // non-finished-stage
                        graphLib.showSurfaceRegionAt(&stage_selection_icon, st_rectangle(0, 0, 36, 36), st_position(icon_x+(i*48), (j*icon_spacing_y)+icon_y));
                    }
                    graphLib.showSurfaceRegionAt(&face_list[cursor_level_n], st_rectangle(0, 0, 32, 30), st_position(icon_x+2+(i*48), (j*icon_spacing_y)+3+icon_y));
                    if (game_save.stages[cursor_level_n] == 1) {
                        graphLib.showSurfaceAt(&dark_surface, st_position(icon_x+2+(i*48), (j*icon_spacing_y)+3+icon_y), false);
                    }
                }
                cursor_level_n++;
                if (cursor_level_n >= 14) {
                    break;
                }
            }
        }

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
            soundManager.stop_music();
            return stage_n;


        } else if (input.p1_input[BTN_UP]) {
            moved = true;
            stage_n -= 6;
        } else if (input.p1_input[BTN_DOWN]) {
            moved = true;
            stage_n += 6;
        } else if (input.p1_input[BTN_LEFT]) {
            moved = true;
            stage_n--;
        } else if (input.p1_input[BTN_RIGHT]) {
            moved = true;
            stage_n++;
        }

        if (stage_n > max_stage) {
            stage_n = 0;
        } else if (stage_n < 0) {
            stage_n = max_stage;
        }
        if (moved) {
            soundManager.play_sfx(SFX_CURSOR);
            input.clean();
            timer.delay(20);
            fio.read_stage(temp_stage_data, stage_n);
        }
        // show cursor
        int cursorX = 10;
        if (stage_n > STAGE8) {
            cursorX = RES_W/2 + 10;
        }


        int cursorY = 14*stage_n+22;
        if (stage_n > STAGE8) {
            cursorY = 14*(stage_n-9)+22;
        }

        // draw boss info
        char bossname[512];
        sprintf(bossname, "%s %s", strings_map::get_instance()->get_ingame_string(strings_stage_select_boss).c_str(), boss_name_list.at(stage_n).c_str());
        graphLib.clear_area(124, 140, 190, 44, 8, 25, 42);
        graphLib.draw_text(124, 140, std::string(bossname), graphLib.gameScreen);
        graphLib.draw_text(124, 160, stage_name_list.at(stage_n), graphLib.gameScreen);

        if (timer.getTimer() >= blink_timer) {
            blink_timer = timer.getTimer() + 300;
            blink_state = !blink_state;
        }

        //graphLib.draw_text(cursorX, cursorY, ">", graphLib.gameScreen);
        graphLib.updateScreen();
    }



    return stage_n;
}

short stage_select::pick_stage_classic_style(int stage_n)
{
    bool can_access_castle = fio.can_access_castle(game_save);
    if (GAME_FLAGS[FLAG_ALLWEAPONS]) {
        can_access_castle = true;
    }
    // @TODO: use the max_stage to limit selection
    int max_stage = gameControl.get_last_castle_stage();
    int total_stages = fio.get_last_stage();
    std::cout << "STAGE_SELECT::pick_stage_classic_style - total_stages[" << total_stages << "], max_stage[" << max_stage << "]" << std::endl;

    soundManager.load_music(game_data.stage_select_music_filename);
    soundManager.play_music();

    graphicsLib_gSurface classic_style_stage_select_bg_surface;
    std::string classic_style_stage_select_bg_filename = FILEPATH + "/images/backgrounds/classic_style_stage_select_bg.png";
    graphLib.surfaceFromFile(classic_style_stage_select_bg_filename, &classic_style_stage_select_bg_surface);
    graphLib.showSurface(&classic_style_stage_select_bg_surface);

    std::vector<std::string> boss_name_list = build_stage_boss_names_list();
    std::vector<std::string> stage_name_list = build_stage_names_list();
    graphicsLib_gSurface face_list[STAGE_COUNT];
    build_stage_faces_surfaces(face_list);

    std::string stage_selection_icon_filename = FILEPATH + "/images/backgrounds/stage_selection_icon.png";
    graphicsLib_gSurface stage_selection_icon;
    graphLib.surfaceFromFile(stage_selection_icon_filename, &stage_selection_icon);

    std::string stage_selection_cursor_filename = FILEPATH + "/images/backgrounds/classic_style_stage_select_cursor.png";
    graphicsLib_gSurface stage_selection_cursor_surface;
    graphLib.surfaceFromFile(stage_selection_cursor_filename, &stage_selection_cursor_surface);

    char player_face_filename_char[FS_CHAR_FILENAME_SIZE];
    sprintf(player_face_filename_char, "images/faces/p%d.png", (game_save.selected_player+1));
    std::string stage_selection_player_filename = FILEPATH + player_face_filename_char;
    graphicsLib_gSurface stage_selection_player_surface;
    graphLib.surfaceFromFile(stage_selection_player_filename, &stage_selection_player_surface);

    graphicsLib_gSurface player_face_eyes_surface;
    char eyes_filename_char[FS_CHAR_FILENAME_SIZE];
    sprintf(eyes_filename_char, "images/faces/p%d_eyes.png", (game_save.selected_player+1));
    std::string filename = FILEPATH + std::string(eyes_filename_char);
    graphLib.surfaceFromFile(filename, &player_face_eyes_surface);

    std::string stage_selection_castle_filename = FILEPATH + "/images/faces/stage_select_castle.png";
    graphicsLib_gSurface stage_selection_castle_surface;
    graphLib.surfaceFromFile(stage_selection_castle_filename, &stage_selection_castle_surface);

    graphLib.updateScreen();

    timer.delay(200);
    input.clean();

    // TODO: if can access castle, make the castle-stages flag true

    // draw faces
    int adjust_stage_pos = 0;
    draw_faces_classic_style(face_list);

    int old_cursor_pos = STAGE1;
    bool finished = false;
    int cursor_opacity = 0;
    bool cursor_opacity_inc = true;
    int complete_stage_n = STAGE1;
    if (stage_n < STAGE1) {
        stage_n = STAGE1;
    }
    int stage_pos_x = (stage_n-1) - std::floor(stage_n / 3);
    int stage_pos_y = std::floor((stage_n-1) / 3);
    st_position selection_pos = st_position(stage_pos_x, stage_pos_y);

    while (finished == false) {
        // clean cursor in old position
        graphLib.set_surface_alpha(255, stage_selection_cursor_surface);
        int cursor_pos_x = (selection_pos.x * faces_x_spacing) + faces_x_left - 8;
        int cursor_pos_y = (selection_pos.y * faces_y_spacing) + faces_y_top - 9;
        graphLib.showSurfaceRegionAt(&stage_selection_cursor_surface, st_rectangle(0, 0, 48, 48), st_position(cursor_pos_x, cursor_pos_y));
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
            if (stage_n == 5) {
                if (can_access_castle == true) {
                    soundManager.stop_music();
                    return max_stage;
                }
            } else {
                soundManager.stop_music();
                return complete_stage_n;
            }
        } else if (input.p1_input[BTN_UP]) {
            selection_pos.y--;
            old_cursor_pos = stage_n;
            moved = true;
        } else if (input.p1_input[BTN_DOWN]) {
            old_cursor_pos = stage_n;
            moved = true;
            selection_pos.y++;
        } else if (input.p1_input[BTN_LEFT]) {
            old_cursor_pos = stage_n;
            moved = true;
            selection_pos.x--;
        } else if (input.p1_input[BTN_RIGHT]) {
            old_cursor_pos = stage_n;
            moved = true;
            selection_pos.x++;
        }

        if (moved == true) {
            if (selection_pos.x < 0) {
                selection_pos.x = 2;
            } else if (selection_pos.x > 2) {
                selection_pos.x = 0;
            }
            if (selection_pos.y < 0) {
                selection_pos.y = 2;
            } else if (selection_pos.y > 2) {
                selection_pos.y = 0;
            }
            stage_n = selection_pos.y*3 + selection_pos.x + 1;
        }

        if (stage_n > 5) {
            adjust_stage_pos = -1;
        } else {
            adjust_stage_pos = 0;
        }

        // TODO: show press r to access castle/first stages

        if (moved) {
            soundManager.play_sfx(SFX_CURSOR);
            input.clean();
            timer.delay(20);
            if (stage_n == 5 && can_access_castle) {
                complete_stage_n = max_stage;
            } else {
                complete_stage_n = stage_n + adjust_stage_pos;
            }
            fio.read_stage(temp_stage_data, complete_stage_n);
            //std::cout << "MOVED[" << complete_stage_n << "]" << std::endl;
        }

        // draw boss info
        char bossname[512];
        sprintf(bossname, "%s %s", strings_map::get_instance()->get_ingame_string(strings_stage_select_boss).c_str(), boss_name_list.at(complete_stage_n).c_str());
        std::string complete_stage_name = strings_map::get_instance()->get_ingame_string(string_stage_select_stage) + " " + stage_name_list.at(complete_stage_n);
        graphLib.showSurfaceRegionAt(&classic_style_stage_select_bg_surface, st_rectangle(0, 208, RES_W, 32), st_position(0, 208));

        if (can_access_castle == true || stage_n != 5) {
            graphLib.draw_text(20, 211, std::string(bossname), graphLib.gameScreen);
            graphLib.draw_text(20, 223, complete_stage_name, graphLib.gameScreen);
        }

        if (cursor_opacity_inc == true) {
            cursor_opacity += 4;
        } else {
            cursor_opacity -= 4;
        }
        if (cursor_opacity_inc == true && cursor_opacity >= 255) {
            cursor_opacity = 255;
            cursor_opacity_inc = false;
        } else if (cursor_opacity_inc == false && cursor_opacity <= 0) {
            cursor_opacity_inc = true;
            cursor_opacity = 0;
        }
        // show cursor
        cursor_pos_x = (selection_pos.x * faces_x_spacing) + faces_x_left - 8;
        cursor_pos_y = (selection_pos.y * faces_y_spacing) + faces_y_top - 9;

        graphLib.set_surface_alpha(255, stage_selection_cursor_surface);
        graphLib.showSurfaceRegionAt(&stage_selection_cursor_surface, st_rectangle(0, 0, 48, 48), st_position(cursor_pos_x, cursor_pos_y));
        graphLib.set_surface_alpha(cursor_opacity, stage_selection_cursor_surface);
        graphLib.showSurfaceRegionAt(&stage_selection_cursor_surface, st_rectangle(48, 0, 48, 48), st_position(cursor_pos_x, cursor_pos_y));


        if (can_access_castle == false) {
            graphLib.showSurfaceRegionAt(&stage_selection_player_surface, st_rectangle(0, 0, 32, 32), st_position(144, 105));
            st_position eyes_pos_adjust = st_position(0, 0);
            if (selection_pos.x == 0) {
                eyes_pos_adjust.x = -2;
            } else if (selection_pos.x == 2) {
                eyes_pos_adjust.x = 2;
            }
            if (selection_pos.y == 0) {
                eyes_pos_adjust.y = -2;
            } else if (selection_pos.y == 2) {
                eyes_pos_adjust.y = 2;
            }
            graphLib.showSurfaceAt(&player_face_eyes_surface, st_position(154 + eyes_pos_adjust.x, 122 + eyes_pos_adjust.y), false);
        } else {
            graphLib.showSurfaceRegionAt(&stage_selection_castle_surface, st_rectangle(0, 0, 32, 32), st_position(144, 105));
        }

        graphLib.updateScreen();
    }

    return complete_stage_n;
}

void stage_select::draw_faces_classic_style(graphicsLib_gSurface face_list[STAGE_COUNT])
{
    int adjust_stage_pos = 0;
    int initial_stage_pos = 0;
    graphicsLib_gSurface dark_surface;
    graphLib.initSurface(st_size(32, 32), &dark_surface);
    graphLib.clear_surface_area(0, 0, 32, 32, 0, 0, 0, dark_surface);
    graphLib.set_surface_alpha(150, &dark_surface);
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            int cursor_level_n = i + (j * 3);
            if (cursor_level_n == 4) {
                continue;
            }
            if (cursor_level_n > 4) {
                adjust_stage_pos = -1;
            } else {
                adjust_stage_pos = 0;
            }
            int pos_x = faces_x_left + (i * faces_x_spacing);
            int pos_y = faces_y_top + (j * faces_y_spacing);
            int face_n = cursor_level_n+adjust_stage_pos+initial_stage_pos + 1;
            graphLib.showSurfaceRegionAt(&face_list[face_n], st_rectangle(0, 0, 32, 30), st_position(pos_x, pos_y));
            if (game_save.stages[face_n] == 1) {
                graphLib.showSurfaceAt(&dark_surface, st_position(pos_x, pos_y), false);
            }
        }
    }
}

void stage_select::classic_style_castle_intro(short stage_id)
{
    std::string filename = FILEPATH + "images/backgrounds/classic_style_castle.png";
    graphicsLib_gSurface skull_castle_bg;
    graphLib.surfaceFromFile(filename, &skull_castle_bg);
    graphLib.showSurfaceAt(&skull_castle_bg, st_position(0, 0), false);

    filename = FILEPATH + "images/backgrounds/classic_style_castle_point.png";
    graphicsLib_gSurface skull_castle_point;
    graphLib.surfaceFromFile(filename, &skull_castle_point);
    graphLib.showSurfaceAt(&skull_castle_point, st_position(11, 223), false);

    graphLib.updateScreen();
    timer.delay(3000);
}




