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
    CURRENT_FILE_FORMAT::file_io fio;
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

    std::vector<std::string> boss_name_list;


    std::string stage_selection_icon_filename = FILEPATH + "/images/backgrounds/stage_selection_icon.png";
    graphicsLib_gSurface stage_selection_icon;
    graphLib.surfaceFromFile(stage_selection_icon_filename, &stage_selection_icon);




    for (int i=INTRO_STAGE; i<=STAGE8; i++) {
        fio.read_stage(temp_stage_data, i);
        std::string stage_name(temp_stage_data.name);
        std::string boss_name(temp_stage_data.boss.name);
        boss_name_list.push_back(boss_name);
        /*
        if (game_save.stages[i] == 0) {
            graphLib.draw_text(22, 14*i+22, stage_name, st_color(230, 230, 122));
        } else {
            graphLib.draw_text(22, 14*i+22, stage_name, graphLib.gameScreen);
        }
        */
    }
    for (int i=CASTLE1_STAGE1; i<=CASTLE1_STAGE5; i++) {
        fio.read_stage(temp_stage_data, i);
        std::string stage_name(temp_stage_data.name);
        std::string boss_name(temp_stage_data.boss.name);
        boss_name_list.push_back(boss_name);
        if (can_access_castle && i<= max_stage) {
            /*
            if (game_save.stages[i] == 0) {
                graphLib.draw_text(RES_W/2+22, 14*(i-9)+22, stage_name, st_color(230, 230, 122));
            } else {
                graphLib.draw_text(RES_W/2+22, 14*(i-9)+22, stage_name, graphLib.gameScreen);
            }
            */
        }
    }
    graphLib.updateScreen();

    timer.delay(200);
    input.clean();

    int icon_y = 40;
    int icon_x = 24;
    int icon_spacing_y = 48;



    std::vector<std::string> stage_name_list;
    graphicsLib_gSurface face_list[STAGE_COUNT];
    for (int i=0; i<STAGE_COUNT; i++) {
        fio.read_stage(temp_stage_data, i);
        std::string stage_name(temp_stage_data.name);
        stage_name = std::string(temp_stage_data.name);
        stage_name_list.push_back(stage_name);
        char face_filename[512];
        sprintf(face_filename, "%s/images/faces/%d.png", FILEPATH.c_str(), i);
        if (i < 10)  {
            sprintf(face_filename, "%s/images/faces/0%d.png", FILEPATH.c_str(), i);
        }

        if (fio.file_exists(StringUtils::clean_filename(face_filename))) {
            graphLib.surfaceFromFile(StringUtils::clean_filename(face_filename), &face_list[i]);
        } else {
            graphLib.initSurface(st_size(32, 32), &face_list[i]);
        }
    }

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




