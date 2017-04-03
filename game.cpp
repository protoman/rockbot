#include <cstring>
#include <cstdlib>
#include "game.h"
#include <fstream>

#ifdef PSP
	#include <pspkernel.h>
	#include <pspdebug.h>
	#include <pspctrl.h>
	#include <pspdebug.h>
#endif

#include "timerlib.h"
extern timerLib timer;

#include <math.h>

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "soundlib.h"
extern soundLib soundManager;

#include "inputlib.h"
extern inputLib input;

#include "graphic/option_picker.h"
#include "file/format.h"
#include "defines.h"
#include "file/file_io.h"
#include "file/fio_strings.h"

#include "strings_map.h"

extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::file_io fio;


extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;
extern bool run_game;

#ifdef ANDROID
#include <android/log.h>
#endif


extern string FILEPATH;

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;

extern bool GAME_FLAGS[FLAG_COUNT];

extern bool leave_game;

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;

extern std::map<Uint8, Uint8> game_scenes_map;

#include "aux_tools/fps_control.h"
extern fps_control fps_manager;

// ********************************************************************************************** //
// class constructor                                                                              //
// ********************************************************************************************** //
game::game() : loaded_stage(-1, NULL), _show_boss_hp(false), fps_timer(0), player1(0)
{
	currentStage = 1;
	fps_counter = 0;
	_frame_duration = 1000/80; // each frame must use this share of time
    invencible_old_value = false;
    _dark_mode = false;
    is_showing_boss_intro = false;
    must_break_npc_loop = false;
}

// ********************************************************************************************** //
// class destructor                                                                               //
// ********************************************************************************************** //
game::~game()
{
}

// ********************************************************************************************** //
// initializar game, can't be on constructor because it needs other objects (circular)            //
// ********************************************************************************************** //
void game::initGame()
{
    player1.initialize();
    loaded_stage = stage(currentStage, &player1);
    player1.initFrames();
    player1.set_is_player(true);
    player1.reset_hp();
    config_manager.set_player_ref(&player1);

    invencible_old_value = GAME_FLAGS[FLAG_INVENCIBLE];

}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::showGame(bool can_characters_move, bool can_scroll_stage)
{
    if (leave_game == true) {
        exit_game();
    }
    if (player1.is_teleporting() == false) { // ignore input while player is teleporting because it caused some issues
        input.read_input();
    }

    if (config_manager.execute_ingame_menu()) { // game is paused
        return;
    }

    /// @TODO - move this to the player, so we don't need to check every single loop
    if (player1.is_dead() == true) {
        //std::cout << "### DEAD - RESTART_STAGE ###" << std::endl;
        restart_stage();
        return;
    }

    if (test_teleport(&player1)) {
        return;
    }

    if (timer.is_paused() == false) {
        if (can_scroll_stage == true) {
            update_stage_scrolling();
        }
        loaded_stage.move_objects();
    }

    if (_dark_mode == false) {
        loaded_stage.showStage();
    }

    if (can_characters_move == true) {
        player1.execute();
        loaded_stage.move_npcs();
    }

    if (_dark_mode == false) {
        loaded_stage.show_npcs();
        player1.show();
        loaded_stage.show_objects();
        loaded_stage.showAbove();
    } else {
        graphLib.blank_screen();
    }


    //std::cout << "GFX_MODE[" << (int)loaded_stage.get_current_map_gfx_mode() << "]" << std::endl;

    if (loaded_stage.get_current_map_gfx_mode() == SCREEN_GFX_MODE_OVERLAY) {
        draw_lib.show_gfx();
    }

    // draw HUD
    draw_lib.show_player_hp(player1.get_current_hp(), 1, player1.get_selected_weapon(), player1.get_selected_weapon());

}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 game::getMapPointLock(struct st_position pos)
{
    return loaded_stage.getMapPointLock(pos);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_float_position game::checkScrolling()
{
    st_float_position move;
    st_float_position mapScroll = loaded_stage.getMapScrolling();
    st_float_position p1Pos(player1.getPosition().x,  player1.getPosition().y);

    move.x += (p1Pos.x - mapScroll.x) - RES_W/2;

    if (mapScroll.x + move.x < 0 || mapScroll.x + move.x > MAP_W*TILESIZE) {
        move.x = 0;
	}

	return move;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::start_stage()
{



	_show_boss_hp = false;
    input.clean();


    loaded_stage.reset_current_map();

    /// @TODO - this must be on a single method in soundlib
    player1.set_position(st_position(RES_W/2 - 29/2, -TILESIZE));

	soundManager.stop_music();
    soundManager.load_stage_music(stage_data.bgmusic_filename);

    loaded_stage.reload_stage();

    player1.cancel_slide();


    loaded_stage.showStage();
    loaded_stage.showAbove();
    draw_lib.update_screen();


    player1.clean_projectiles();
    player1.set_animation_type(ANIM_TYPE_TELEPORT);
    player1.set_direction(ANIM_DIRECTION_RIGHT);
    player1.refill_weapons();
    player1.reset_hp();

    // find teleport stop point
    int min_y = loaded_stage.get_teleport_minimal_y_tile(85); // x = 70 + half a player width (30)
    player1.set_teleport_minimal_y(min_y*TILESIZE);

    game_unpause();

    show_ready();

    soundManager.play_music();


    while (player1.get_anim_type() == ANIM_TYPE_TELEPORT) {
        input.clean_all();
        showGame(true, false);
        draw_lib.update_screen();
    }
    for (int i=0; i<AUTOSCROLL_START_DELAY_FRAMES; i++) { // extra delay to show dialogs
        input.clean_all();
        showGame(false, false);
        draw_lib.update_screen();
        timer.delay(20);
    }

	/// @TODO: do not show twice
    //if (GAME_FLAGS[FLAG_QUICKLOAD] == false) {
		if (game_save.stages[currentStage] == 0) {
            game_dialogs.show_stage_dialog(currentStage);
            // reset timers for objects
            loaded_stage.reset_objects_timers();
		}
    //}
}

void game::show_ready()
{
    //std::cout << "SHOW READY CALL" << std::endl;
    draw_lib.show_ready();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::restart_stage()
{

    input.clean_all();

    //std::cout << "### RESTART_STAGE::START ###" << std::endl;

    if (checkpoint.x < TILESIZE*4) {
        checkpoint.x = TILESIZE*4;
    }
    // remove any used teleporter
    draw_lib.fade_out_screen(0, 0, 0, 500);

    player1.set_teleporter(-1);
    _player_teleporter.active = false;

    remove_all_projectiles();
    remove_players_slide();

	_show_boss_hp = false;
	input.clean();
    loaded_stage.reset_current_map();
	// TODO - this must be on a single method in soundlib

    player1.clean_projectiles();
    player1.set_animation_type(ANIM_TYPE_TELEPORT);

    int min_y = checkpoint.y;
    if (checkpoint.y == -1) { // did not reached any checkpoint, use the calculated value from stage start
        // find teleport stop point
        min_y = loaded_stage.get_teleport_minimal_y_tile(85);
    }
    // to avoid errors, set as 1/3 of the screen
    if (min_y >= MAP_H) {
        min_y = MAP_H/2;
    }
    player1.set_teleport_minimal_y(min_y*TILESIZE);


    player1.reset_hp();
    loaded_stage.reset_stage_maps();

    player1.set_position(st_position(checkpoint.x, -TILESIZE));
    player1.char_update_real_position();
    std::cout << ">>>>>>>>>>>>> min_y: " << min_y << ", p.x: " << (int)player1.getPosition().x << std::endl;

    game_unpause();

    loaded_stage.showStage();
    loaded_stage.showAbove();
	graphLib.set_screen_adjust(st_position(0, 0));
    draw_lib.update_screen();
    soundManager.restart_music();
    player1.set_animation_type(ANIM_TYPE_TELEPORT);
    show_ready();

    while (player1.get_anim_type() == ANIM_TYPE_TELEPORT) {
        input.clean_all();
        showGame(true, false);
        draw_lib.update_screen();
    }
    for (int i=0; i<AUTOSCROLL_START_DELAY_FRAMES; i++) { // extra delay to teleport without moving screen
        input.clean_all();
        showGame(false, false);
        draw_lib.update_screen();
        timer.delay(20);
    }

}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool game::showIntro()
{

    show_notice();

#ifdef BETA_VERSION
    show_beta_version_warning();
#endif

    if (is_free_version()) {
        show_free_version_warning();
    }

    scenes.preloadScenes();

    /// @TODO - add scene intro here
    scenes.game_scenes_show_unbeaten_intro();
	scenes.main_screen();



	currentStage = 0;

	initGame();

	if (game_save.stages[INTRO_STAGE] == 0) {
		input.clean();
		start_stage();
	} else {
        currentStage = scenes.pick_stage(INTRO_STAGE);
        loaded_stage = stage(currentStage, &player1);
        // show boss intro with stars, if needed
        soundManager.stop_music();
        if (game_save.stages[currentStage] == 0 || currentStage >= CASTLE1_STAGE1) {
            scenes.boss_intro(currentStage);
        }
		start_stage();
	}

    return true;
}

void game::show_beta_version_warning()
{
    graphLib.blank_screen();
    draw_lib.update_screen();
    input.clean();
    timer.delay(100);

    graphLib.draw_centered_text(30, "-- BETA VERSION WARNING --", graphLib.gameScreen, st_color(255, 130, 0));
    graphLib.draw_centered_text(60, "THIS IS A TEST VERSION OF ROCKBOT,", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(75, "IT DOES CONTAIN ERRORS AND IS NOT", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(90, "COMPLETE MISSING SOME FEATURES.", graphLib.gameScreen, st_color(255, 255, 255));

    graphLib.draw_centered_text(120, "SOFTWARE IS PROVIDED \"AS IS\"", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(135, "WITHOUT WARRANTY OF ANY KIND,", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(150, "EXPRESS OR IMPLIED FROM AUTHOR.", graphLib.gameScreen, st_color(255, 255, 255));

    graphLib.draw_centered_text(170, "REPORT ANY FOUND ISSUES TO", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(185, "protoman@upperland.net", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(210, "PRESS A BUTTON OR KEY TO CONTINUE.", graphLib.gameScreen, st_color(255, 255, 255));
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::show_free_version_warning()
{
    graphLib.blank_screen();
    draw_lib.update_screen();
    input.clean();
    timer.delay(100);

    graphLib.draw_centered_text(30, "-- FREE VERSION WARNING --", graphLib.gameScreen, st_color(255, 130, 0));
    graphLib.draw_centered_text(60, "THIS IS ROCKBOT 2'S FREE VERSION.", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(75, "IT CONTAINS THREE STAGES", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(90, "AND LACKS SUPPORT FOR", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(105, "A FEW FEATURES.", graphLib.gameScreen, st_color(255, 255, 255));

    graphLib.draw_centered_text(130, "FULL VERSION WILL BE RELEASED", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(145, "LATER DURING THE YEAR 2017.", graphLib.gameScreen, st_color(255, 255, 255));

    graphLib.draw_centered_text(185, "ENJOY!", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(210, "PRESS A BUTTON OR KEY TO CONTINUE.", graphLib.gameScreen, st_color(255, 255, 255));
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::show_notice()
{
    graphLib.blank_screen();
    draw_lib.update_screen();

    graphicsLib_gSurface upperland_surface;
    graphLib.surfaceFromFile(GAMEPATH + "/shared/images/upperland.png", &upperland_surface);

    graphicsLib_gSurface presents_surface;
    graphLib.surfaceFromFile(GAMEPATH + "/shared/images/presents.png", &presents_surface);

    st_position logo_pos(RES_W/2 - (upperland_surface.width/6)/2, RES_H/2 - upperland_surface.height/2);
    graphLib.copyArea(st_rectangle(0, 0, presents_surface.width, presents_surface.height), st_position(RES_W*0.5-presents_surface.width*0.5, logo_pos.y + upperland_surface.height + 7), &presents_surface, &graphLib.gameScreen);


    //std::cout << ">> logo_pos.x: " << logo_pos.x << ", logo_pos.y: " << logo_pos.y << std::endl;
    graphLib.copyArea(st_rectangle(0, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);
    graphLib.draw_centered_text(220, "HTTP://ROCKBOT.UPPERLAND.NET", st_color(240, 240, 240));
    draw_lib.update_screen();
    input.waitScapeTime(400);
    for (int i=1; i<6; i++) {
        graphLib.copyArea(st_rectangle((upperland_surface.width/6)*i, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);
        draw_lib.update_screen();
        input.waitScapeTime(30);
    }
    graphLib.copyArea(st_rectangle(0, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);


    draw_lib.update_screen();

    timer.delay(1200);

    graphLib.blank_screen();

    graphLib.draw_centered_text(30, "ROCKBOT ENGINE", graphLib.gameScreen, st_color(199, 215, 255));
    graphLib.draw_centered_text(60, "THIS GAME RUNS WITH", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(80, "UPPERLAND'S ROCKBOT ENGINE.", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(120, "GAME'S SOURCE-CODE IS LICENSED UNDER", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(132, "THE GPL AND IS FREELY DISTRIBUTABLE.", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(144, "GAME CONTENT IS UNDER DIFFERENT", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(156, "LICENCES BY ITS RESPECTIVE CREATORS", graphLib.gameScreen, st_color(255, 255, 255));
    graphLib.draw_centered_text(268, "AND USED WITH AUTHORIZATION.", graphLib.gameScreen, st_color(255, 255, 255));

    draw_lib.update_screen();

    timer.delay(3000);

}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::fps_count()
{
    if (timer.is_paused()) {
        return;
    }
    fps_counter++;
	if (fps_timer <= timer.getTimer()) {
        sprintf(_fps_buffer, "FPS: %d", fps_counter);
		fps_counter = 0;
		fps_timer = timer.getTimer()+1000;
	}
    if (fps_counter > 1) {
        std::string temp_str(_fps_buffer);
        graphLib.draw_text(12, 2, temp_str);
    }
}






// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool game::test_teleport(classPlayer *test_player) {


    if (player1.get_anim_type() == ANIM_TYPE_TELEPORT) {
		return false;
	}


    int currentMap = loaded_stage.get_current_map_number();
	int temp_x, temp_y;
	int temp_map_n=0;
	int player_x = 0;
	int transition_type = 0;
    int i=0;
	bool MUST_TELEPORT = false;
    int teleporter_dist = 0;
    int link_type = -1;


    int j = 0;
    for (j=0; j<STAGE_MAX_LINKS; j++) {
        if (stage_data.links[j].id_map_origin == -1 || stage_data.links[j].id_map_destiny == -1) {
            continue;
        }

        if (stage_data.links[j].id_map_origin != -1) {
            // @TODO: replace CASTLE1_STAGE5 by a flag that indicates last level
            if (currentStage == CASTLE1_STAGE5 && _last_stage_used_teleporters.find(i) != _last_stage_used_teleporters.end()) {
				i++;
				continue;
			}


            if ((stage_data.links[j].id_map_origin == currentMap && stage_data.links[j].pos_origin.x != -1)) {
                temp_x = stage_data.links[j].pos_origin.x;
                temp_y = stage_data.links[j].pos_origin.y;
                temp_map_n = stage_data.links[j].id_map_destiny;
                player_x = stage_data.links[j].pos_destiny.x;
                if (stage_data.links[j].pos_origin.y > stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_TOP_TO_BOTTOM;
                } else if (stage_data.links[j].pos_origin.y < stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_BOTTOM_TO_TOP;
				}
                MUST_TELEPORT = check_player_is_on_teleport(test_player, currentMap, temp_x, temp_y, j, transition_type, i, teleporter_dist, player_x, link_type);
                //std::cout << "MUST_TELEPORT #1[" << MUST_TELEPORT << "]" << std::endl;
            }
            if (MUST_TELEPORT == false && stage_data.links[j].id_map_destiny == currentMap && stage_data.links[j].bidirecional == true && stage_data.links[j].pos_destiny.x != -1) {
                temp_x = stage_data.links[j].pos_destiny.x;
                temp_y = stage_data.links[j].pos_destiny.y;
                temp_map_n = stage_data.links[j].id_map_origin;
                player_x = stage_data.links[j].pos_origin.x;
                if (stage_data.links[j].pos_origin.y < stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_TOP_TO_BOTTOM;
                } else if (stage_data.links[j].pos_origin.y > stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_BOTTOM_TO_TOP;
				}
                MUST_TELEPORT = check_player_is_on_teleport(test_player, currentMap, temp_x, temp_y, j, transition_type, i, teleporter_dist, player_x, link_type);
                //std::cout << "MUST_TELEPORT #2[" << MUST_TELEPORT << "]" << std::endl;
            }
            if (MUST_TELEPORT == false) {
                i++;
				continue;
            } else {
                break;
            }
		}
		i++;
	}

    if (!MUST_TELEPORT) {
        return false;
    }

    //std::cout << "START TELEPORT!!!!" << std::endl;
    game_pause();

    remove_all_projectiles();
    reset_beam_objects(); // beam/ray objects must be reset when changing maps


    // must move the map, so that the dest position in screen is equal to player_real_pos_x
    int new_map_pos_x;
    new_map_pos_x = loaded_stage.getMapScrolling().x - teleporter_dist;

    if (new_map_pos_x < 0) {
       new_map_pos_x = 0;
    } else if (new_map_pos_x > MAP_W*TILESIZE) {
       new_map_pos_x = MAP_W*TILESIZE;
    }

    int diff_h=6;

    if (test_player->get_size().width > 30) {
       diff_h = abs((float)test_player->get_size().width-30);
    }
    new_map_pos_x -= diff_h +2;


    if (is_link_teleporter(stage_data.links[j].type) == true) {
        if (link_type == LINK_FADE_TELEPORT) {
            draw_lib.fade_out_screen(0, 0, 0, 300);
        } else {
            test_player->teleport_out();
        }
        graphLib.blank_screen();
        draw_lib.update_screen();
        timer.delay(500);
        int calc_pos_x = ((int)stage_data.links[j].pos_destiny.x * TILESIZE) - TILESIZE*2;

        if (link_type == LINK_TELEPORT_LEFT_LOCK) {
            //std::cout << "%%%% LEFT %%%%" << std::endl;
            new_map_pos_x = loaded_stage.get_first_lock_on_left(calc_pos_x/TILESIZE) - TILESIZE;
        } else if (link_type == LINK_TELEPORT_RIGHT_LOCK) {
            //std::cout << "%%%% RIGHT %%%%" << std::endl;
            new_map_pos_x = loaded_stage.get_first_lock_on_right(calc_pos_x/TILESIZE);
        } else {
            new_map_pos_x = calc_pos_x;
        }
        //std::cout << "##### dest-x[" << (int)stage_data.links[j].pos_destiny.x << "], calc_pos_x[" << calc_pos_x << "], new_map_pos_x[" << new_map_pos_x << "]" << std::endl;
    } else {
        transition_screen(transition_type, temp_map_n, new_map_pos_x, test_player);
    }

    st_float_position bg1_pos = loaded_stage.get_current_map()->get_bg_scroll();

    set_current_map(temp_map_n);

    if (is_link_teleporter(stage_data.links[j].type) == true) {
        int left_wall_lock = loaded_stage.get_first_lock_on_left(stage_data.links[j].pos_destiny.x);
        int diff = RES_W/2 - player1.get_size().width;
        if (left_wall_lock <= new_map_pos_x-diff) { // if no wall on left in near-screen, move scroll to center
            new_map_pos_x -= diff;
        }
        //loaded_stage.set_scrolling(st_float_position(new_scroll_pos, loaded_stage.getMapScrolling().y));
        loaded_stage.set_scrolling(st_float_position(new_map_pos_x, loaded_stage.getMapScrolling().y));

        loaded_stage.get_current_map()->reset_scrolled();
        if (link_type == LINK_FADE_TELEPORT) {
            test_player->set_position(st_position(stage_data.links[j].pos_destiny.x*TILESIZE, stage_data.links[j].pos_destiny.y*TILESIZE));
            test_player->char_update_real_position();
            test_player->set_animation_type(ANIM_TYPE_JUMP);
            showGame(false, true);
            timer.delay(50);
            draw_lib.fade_in_screen(0, 0, 0);
        } else {
            test_player->set_position(st_position(stage_data.links[j].pos_destiny.x*TILESIZE, 0));
            test_player->char_update_real_position();
        }
    } else {
        loaded_stage.set_scrolling(st_float_position(new_map_pos_x, loaded_stage.getMapScrolling().y));
        loaded_stage.get_current_map()->set_bg_scroll(bg1_pos);
        test_player->set_position(st_position(abs((float)test_player->get_real_position().x+new_map_pos_x), test_player->getPosition().y));
        test_player->char_update_real_position();
        loaded_stage.get_current_map()->reset_scrolled();
    }

    timer.delay(100);

    game_unpause();

    draw_lib.update_screen();

    return true;
}

bool game::check_player_is_on_teleport(classPlayer *test_player, int currentMap, int temp_x, int temp_y, int link_n, int transition_type, int &teleport_count, int &teleporter_dist, int &player_x, int &link_type)
{
    int lim1 = temp_x*TILESIZE;
    int lim2 = temp_x*TILESIZE + stage_data.links[link_n].size*TILESIZE;
    int lim3 = (temp_y)*TILESIZE + (TILESIZE*0.5);
    int lim4 = ((temp_y)*TILESIZE)+TILESIZE;

    int px = test_player->getPosition().x + (test_player->get_size().width*0.5);
    int py = test_player->getPosition().y + (test_player->get_size().height*0.5) + (test_player->get_size().height*0.25);

    // if teleporter is out of screen, ignore it
    st_float_position scroll = loaded_stage.getMapScrolling();
    int min = scroll.x-RES_W/2;
    int max = scroll.x+RES_W*1.5;
    if (abs(px) < min || abs(px) > max) {
        //std::cout << "IGN - px[" << px << "], scroll.x[" << scroll.x << "], min[" << min << "], max[" << max << "]" << std::endl;
        return false;
    }


    // give extra pixels in the END-Y, if top to bottom ot bottom to top
    if (is_link_teleporter(stage_data.links[link_n].type) == false) {
        if (transition_type == TRANSITION_TOP_TO_BOTTOM) {
            lim4 += TILESIZE*1.5;
        } else if (transition_type == TRANSITION_BOTTOM_TO_TOP) {
            lim3 -= TILESIZE*1.5;
        }
    }

    //std::cout << "GAME::check_player_is_on_teleport - lim3[" << lim3 << "], lim4[" << lim4 << "], py[" << py << "]" << std::endl;

    if ((px >= lim1 && px <= lim2) && ((py > lim3 && py < lim4))) {

        if (test_player->get_teleporter() == -1) {

            // avoid using same teleporter to return
            if ((is_link_teleporter(stage_data.links[link_n].type) == true) && teleport_count == _player_teleporter.teleporter_n) {
                std::cout << ">>>>>>>>>> IGNORE TELEPORT <<<<<<<<<<<<<<<<<" << std::endl;
                teleport_count++;
                // TODO: return false in structure
                return false;
            }
            // for transition up/down, only execute if player is partially out of screen
            if (is_link_teleporter(stage_data.links[link_n].type) == false && (transition_type == TRANSITION_TOP_TO_BOTTOM || transition_type == TRANSITION_BOTTOM_TO_TOP)) {
                short int p_posy = test_player->getPosition().y;
                if (p_posy > 0 && p_posy+test_player->get_size().height < RES_H-4) {
                    teleport_count++;
                    // TODO: return false in structure
                    return false;
                }
            }
            teleporter_dist = lim1 - player_x*TILESIZE - 8;
            if (is_link_teleporter(stage_data.links[link_n].type) == false) {
                test_player->set_teleporter(teleport_count);
            }
            link_type = stage_data.links[link_n].type;
            //std::cout << "## TELEPORT #1, teleporter_dist[" << teleporter_dist << "]" << std::endl;
            set_player_teleporter(teleport_count, st_position(player1.getPosition().x, player1.getPosition().y), false);
            return true;
        }
    // only clean teleport when player is out of the teleporter
    } else {
        if (teleport_count == test_player->get_teleporter()) {
            if (currentStage != CASTLE1_STAGE5 || currentMap != 2) { // only clean link if not teleporter nor is on final stage/map
                test_player->set_teleporter(-1);
            }
        }
    }
    return false;
}

bool game::is_link_teleporter(int type)
{
    if (type == LINK_TELEPORTER || type == LINK_TELEPORT_LEFT_LOCK || type == LINK_TELEPORT_RIGHT_LOCK || type == LINK_FADE_TELEPORT) {
        return true;
    }
    return false;
}

void game::set_current_map(int temp_map_n)
{
    loaded_stage.set_current_map(temp_map_n);
    loaded_stage.reset_current_map_objects();
}

Uint8 game::get_current_map()
{
    return loaded_stage.get_current_map_n();;
}


void game::map_present_boss(bool show_dialog)
{
	is_showing_boss_intro = true;

    soundManager.stop_music();
    soundManager.unload_music();

    // 1. keep showing game screen until player reaches ground
    player1.clear_move_commands();
	bool loop_run = true;
	while (loop_run == true) {
        loaded_stage.showStage();
        player1.charMove();
        bool hit_ground = player1.hit_ground();
        int anim_type = player1.get_anim_type();
        if (hit_ground == true && anim_type == ANIM_TYPE_STAND) {
			loop_run = false;
		}
        player1.show();
        loaded_stage.showAbove();
        timer.delay(8);
        draw_lib.update_screen();
	}

	// 2. blink screen
	graphLib.blink_screen(255, 255, 255);

	// 3. move boss from top to ground
	loop_run = true;
	while (loop_run == true) {
        if (loaded_stage.boss_hit_ground() == true) {
			loop_run = false;
		}
        show_stage(0, true);
	}
	show_stage(8, true);


	if (show_dialog == true) {
		// 4. show boss dialog
        dialogs boss_dialog;
        boss_dialog.show_boss_dialog(loaded_stage.get_number());
	}

	show_stage(8, false);


	fill_boss_hp_bar();

    soundManager.play_boss_music();

    timer.delay(100);

	_show_boss_hp = true;
	is_showing_boss_intro = false;

}

object* game::get_player_platform()
{
    return player1.get_platform();
}

void game::check_player_return_teleport()
{
    remove_all_projectiles();
    remove_players_slide();
	if (is_player_on_teleporter() == true) {
        finish_player_teleporter();
    }
}

bool game::must_show_boss_hp()
{
	return _show_boss_hp;
}

void game::fill_boss_hp_bar()
{
    soundManager.play_timed_sfx(SFX_GOT_ENERGY, 60*BOSS_INITIAL_HP+100);
    for (int i=0; i<BOSS_INITIAL_HP; i++) {
        graphLib.draw_hp_bar(i, -1, -1, BOSS_INITIAL_HP);
        draw_lib.update_screen();
		timer.delay(60);
	}
    timer.delay(200);
}

void game::fill_player_weapon(short weapon_n)
{
    soundManager.play_timed_sfx(SFX_GOT_ENERGY, 60*PLAYER_INITIAL_HP+100);
    for (int i=0; i<PLAYER_INITIAL_HP; i++) {
        graphLib.draw_hp_bar(i, player1.get_number(), weapon_n, fio.get_heart_pieces_number(game_save));
        draw_lib.update_screen();
        timer.delay(60);
    }
    timer.delay(200);
}



void game::reset_stage_maps()
{
    loaded_stage.reset_stage_maps();
}




// ********************************************************************************************** //
// remove the projectiles from the list of all players and npcs                                   //
// ********************************************************************************************** //
void game::remove_all_projectiles()
{
    player1.clean_projectiles();
    loaded_stage.get_current_map()->clean_map_npcs_projectiles();
    player1.remove_freeze_effect();
}

void game::reset_beam_objects()
{
    loaded_stage.get_current_map()->reset_beam_objects();
}

void game::remove_players_slide()
{
    player1.cancel_slide();
}


//TRANSITION_TOP_TO_BOTTOM, TRANSITION_BOTTOM_TO_TOP
void game::transition_screen(Uint8 type, Uint8 map_n, short int adjust_x, classPlayer *pObj) {
	graphicsLib_gSurface temp_screen;
    short i = 0;
    graphLib.initSurface(st_size(RES_W, RES_H*2), &temp_screen);

    classMap* temp_map = &loaded_stage.maps[map_n];
    temp_map->set_bg_scroll(loaded_stage.get_current_map()->get_bg_scroll());

    graphLib.copyArea(st_rectangle(0, i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);

    // if map destiny and map origin are the same, adjust player's X position
    if (loaded_stage.get_current_map_number() == map_n) {
        //std::cout << "p.x[" << (int)test_player->getPosition().x << "], p.real.x[" << test_player->get_real_position().x << "]" << std::endl;
        pObj->set_position(st_position(pObj->get_real_position().x+adjust_x, pObj->get_real_position().y));
        //adjust_x += TILESIZE;
    }

    // TODO: adjust player X position when changing from the same map
    // pegar posição relativa do jogador em relação à tela
    // posição nova é o scroll-x novo mais essa diferença

    // draw map in the screen, erasing all players/objects/npcs/GFX
    draw_lib.set_gfx(SCREEN_GFX_NONE, BG_SCROLL_MODE_NONE);
    loaded_stage.showStage();


    // draw the offscreen with the new loaded map
	if (type == TRANSITION_TOP_TO_BOTTOM || type == TRANSITION_BOTTOM_TO_TOP) {
        // copy current screen to temp
		if (type == TRANSITION_TOP_TO_BOTTOM) {
            graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &temp_screen);
		} else if (type == TRANSITION_BOTTOM_TO_TOP) {
            graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, RES_H), &temp_screen);
        }

        temp_map->set_scrolling(st_float_position(adjust_x, 0));

		// copy the new screen to the temp_area
        graphicsLib_gSurface temp_map_area;
        temp_map->get_map_area_surface(temp_map_area);
		if (type == TRANSITION_TOP_TO_BOTTOM) {
            graphLib.copyArea(st_rectangle(0, 0, RES_W, RES_H), st_position(0, RES_H), &temp_map_area, &temp_screen);
		} else if (type == TRANSITION_BOTTOM_TO_TOP) {
            graphLib.copyArea(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &temp_map_area, &temp_screen);
		}
        temp_map_area.freeGraphic();

		// now, show the transition
        short int extra_y = 0;
        for (i=0; i<(RES_H)/TRANSITION_STEP; i++) {
            if (type == TRANSITION_TOP_TO_BOTTOM) {
                graphLib.copyArea(st_rectangle(0, i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);
            } else if (type == TRANSITION_BOTTOM_TO_TOP) {
                graphLib.copyArea(st_rectangle(0, RES_H-i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);
			}

            if (i % 5 == 0) {
				extra_y = 1;
			} else {
                extra_y = 0;
			}

			if (type == TRANSITION_TOP_TO_BOTTOM) {
                //std::cout << "TRANSITION_TOP_TO_BOTTOM, px[" << (int)pObj->getPosition().x << "], py[" << (int)pObj->getPosition().y << "]" << std::endl;
				if (pObj->getPosition().y > 6) {
					pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y - TRANSITION_STEP + extra_y));
				}
			} else if (type == TRANSITION_BOTTOM_TO_TOP) {
                //std::cout << "TRANSITION_BOTTOM_TO_TOP, px[" << (int)pObj->getPosition().x << "], py[" << (int)pObj->getPosition().y << "]" << std::endl;
				if (pObj->getPosition().y < RES_H-TILESIZE*2) {
					pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y + TRANSITION_STEP - extra_y));
				}
			}
			pObj->char_update_real_position();
			pObj->show();


			if (type == TRANSITION_TOP_TO_BOTTOM) {
                loaded_stage.showAbove(-i*TRANSITION_STEP);
                loaded_stage.get_current_map()->show_objects(-i*TRANSITION_STEP);
                int temp_map_3rdlevel_pos = (RES_H+TILESIZE*0.5) - i*TRANSITION_STEP - 8;
                temp_map->show_objects(temp_map_3rdlevel_pos, adjust_x);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			} else {
                loaded_stage.showAbove(i*TRANSITION_STEP);
                loaded_stage.get_current_map()->show_objects(i*TRANSITION_STEP);
                int temp_map_3rdlevel_pos = -(RES_H+TILESIZE*0.5) + i*TRANSITION_STEP + 8; // 8 is a adjust for some error I don't know the reason
                temp_map->show_objects(temp_map_3rdlevel_pos, adjust_x);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			}


            draw_lib.update_screen();
#if !defined(PLAYSTATION2) && !defined(ANDROID)
            timer.delay(6);
            //timer.delay(100); // DEBUG //
#endif
		}

        if (type == TRANSITION_TOP_TO_BOTTOM) {
            temp_map->changeScrolling(st_float_position(temp_map->getMapScrolling().x, 0));
        }
        if (type == TRANSITION_TOP_TO_BOTTOM) {
            if (pObj->getPosition().y > TILESIZE) {
                pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y - TRANSITION_STEP - 2));
            }
        } else if (type == TRANSITION_BOTTOM_TO_TOP) {
            if (pObj->getPosition().y < RES_H-TILESIZE*2) {
                pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y + TRANSITION_STEP));
            }
        }
	}

    temp_screen.freeGraphic();
	pObj->set_teleporter(-1);
	pObj->char_update_real_position();
}


void game::horizontal_screen_move(short direction, bool is_door, short tileX, short tileY)
{
    int i = 0;
    int upTile = 0;
    int downTile = 0;

    classMap* temp_map = loaded_stage.get_current_map();
    st_float_position scroll_move;
	if (direction == ANIM_DIRECTION_LEFT) {
        scroll_move.x = -TRANSITION_STEP;
	} else {
        scroll_move.x = TRANSITION_STEP;
	}

	if (is_door == true) {
        remove_all_projectiles();
        // if there is a subboss alive, near left, you can't open
        if (subboss_alive_on_left(tileX) == true) {
            std::cout << "[ERROR]: Oh no! Door can't be open because there is an alive sub-boss on its left side." << std::endl;
            return;
        }

        loaded_stage.showStage();
		upTile = tileY;
		for (i=tileY; i>=0; i--) {
			if (temp_map->getMapPointLock(st_position(tileX, i)) == TERRAIN_DOOR) {
				upTile = i;
			} else {
				break;
			}
		}
		downTile = tileY;
		for (i=tileY; i<MAP_H; i++) {
			if (temp_map->getMapPointLock(st_position(tileX, i)) == TERRAIN_DOOR) {
				downTile = i;
			} else {
				break;
			}
		}
        soundManager.play_sfx(SFX_DOOR_OPEN);
        loaded_stage.redraw_boss_door(false, (downTile-upTile+1), tileX, tileY, player1.get_number());//bool is_close, int nTiles, int tileX
	}

    game_pause();


    int move_limit = (RES_W/abs((float)scroll_move.x)) - TILESIZE/abs((float)scroll_move.x);
    float player_move_x = (float)(TILESIZE*2.5)/(float)move_limit; // player should move two tilesize, to avoid doors
    if (scroll_move.x < 0) {
        player_move_x = player_move_x * -1;
    }
    std::cout << "player_move_x[" << player_move_x << "]" << std::endl;
    for (int i=0; i<move_limit; i++) {
        //loaded_stage.changeScrolling(scroll_move, false);
        loaded_stage.change_map_scroll(scroll_move, false, true);
        loaded_stage.showStage();
        loaded_stage.show_npcs();
        player1.show();
        loaded_stage.showAbove();
#if defined(PC)
        timer.delay(1);
#endif
        draw_lib.update_screen();

        player1.inc_position(player_move_x, 0);
        /*
        if (i%(TILESIZE/4) == 0) {
            player1.set_position(st_position(player1.getPosition().x+scroll_move.x, player1.getPosition().y));
            player1.char_update_real_position();
		}
        */
	}
    if (is_door == true) {
        soundManager.play_sfx(SFX_DOOR_OPEN);
        loaded_stage.redraw_boss_door(true, (downTile-upTile+1), tileX, tileY, player1.get_number());
        remove_players_slide();
    }
    timer.delay(6);
    game_unpause();
    loaded_stage.showStage();
}




void game::got_weapon()
{
    bool must_show_got_weapon = false;
	if (game_save.stages[currentStage] == 0) {
        must_show_got_weapon = true;
		game_save.finished_stages++;
    }
    invencible_old_value = GAME_FLAGS[FLAG_INVENCIBLE]; // store old value in order to not set the flag to false if it is on my command-line parameter
    GAME_FLAGS[FLAG_INVENCIBLE] = true;


    // remove any projectiles, charged shots, slides, etc
    player1.clean_projectiles();
    player1.clear_move_commands();

    if (must_show_got_weapon == true && currentStage != 0 && currentStage <= 8) {
		// check witch is the boss that was killed

		/// @TODO: save game

		/// @TODO: teletransport if capsules

		// fall to ground
        player1.fall();


		/// @TODO: walk to boss room middle, if inside a boss room (must create a way to check this), otherwise, just jump up
        walk_character_to_screen_point_x(&player1, RES_W/2);

		// jump up, implosion, jump down and teleport out of the screen
        player1.execute_jump_up();
        draw_implosion(player1.get_real_position().x+player1.get_size().width/2-6, player1.get_real_position().y+player1.get_size().height/2-4);
        player1.show();
        draw_lib.update_screen();

        player1.set_weapon(currentStage, false);
        fill_player_weapon(player1.get_selected_weapon());
        player1.fall();
		soundManager.play_sfx(SFX_GOT_WEAPON);
        timer.delay(5000);

        // fall to ground
        player1.fall();
        timer.delay(500);


		soundManager.play_sfx(SFX_TELEPORT);
        player1.teleport_out();
        timer.delay(1000);

		/// @TODO
		// show the "you got" screen
		graphLib.blank_screen();
        soundManager.load_music(game_data.got_weapon_music_filename);
		graphLib.blink_screen(255, 255, 255);
		graphLib.blank_screen();
        soundManager.play_music();
		graphicsLib_gSurface temp_bg;
        graphLib.surfaceFromFile(FILEPATH+"images/backgrounds/stage_boss_intro.png", &temp_bg);
		graphLib.showSurface(&temp_bg);

        player1.set_position(st_position(20, (RES_H * 0.5 - player1.get_size().height/2)));
        player1.set_animation_type(ANIM_TYPE_ATTACK);
        loaded_stage.set_scrolling(st_float_position(0, 0));
        player1.char_update_real_position();
        player1.show();

        graphLib.start_stars_animation();

        std::string weapon_name(game_data.weapons[currentStage].name);
		for (std::string::iterator p = weapon_name.begin(); weapon_name.end() != p; ++p) {
			*p = toupper(*p);
		}

		std::string phrase = std::string("YOU GOT ") + weapon_name;

		graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 - 4), phrase, false);
		std::string extra_name = "";
        if (currentStage == COIL_GOT_STAGE) {
            std::string item_name = strings_map::get_instance()->toupper(std::string(GameMediator::get_instance()->object_list.at(game_data.player_items[0]).name));
            extra_name = strings_map::get_instance()->get_ingame_string(strings_ingame_and) + std::string(" ") + item_name;
        } else if (currentStage == JET_GOT_STAGE) {
            std::string item_name = strings_map::get_instance()->toupper(std::string(GameMediator::get_instance()->object_list.at(game_data.player_items[1]).name));
            extra_name = strings_map::get_instance()->get_ingame_string(strings_ingame_and) + std::string(" ") + item_name;
        }
		graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 + 8), extra_name, false);

        player1.show();

        graphLib.wait_and_update_screen(5000);
        graphLib.stop_stars_animation();
    } else {
        player1.teleport_out();
	}

    game_save.stages[currentStage] = 1;


    if (is_free_version() && game_save.stages[DEMO_VERSION_STAGE1] == 1 && game_save.stages[DEMO_VERSION_STAGE2] == 1) {
        show_demo_ending();
    }

    leave_stage();
}

void game::leave_stage()
{
    if (fio.write_save(game_save) == false) {
        show_savegame_error();
    }

    draw_lib.set_flash_enabled(false);
    freeze_weapon_effect = FREEZE_EFFECT_NONE;
    GAME_FLAGS[FLAG_INVENCIBLE] = invencible_old_value;

    input.clean();
    timer.delay(200);

    // return to stage selection
    player1.reset_charging_shot();
    player1.set_weapon(WEAPON_DEFAULT, false);
    currentStage = scenes.pick_stage(currentStage);
    loaded_stage = stage(currentStage, &player1);
    // show boss intro with stars, if needed
    soundManager.stop_music();
    if (game_save.stages[currentStage] == 0) {
        scenes.boss_intro(currentStage);
    }
    checkpoint.map = 0;
    checkpoint.map_scroll_x = 0;
    checkpoint.reset();
    start_stage();
}

void game::game_pause()
{
    timer.pause();
    // @TODO - save-player-input
    player1.save_input();
}

void game::game_unpause()
{
    timer.unpause();
    input.read_input();
    player1.restore_input();
}

void game::exit_game()
{

    dialogs dialogs_obj;
    if (dialogs_obj.show_leave_game_dialog() != true) { // cuidar se isso não cria loophole
        leave_game = false;
        return;
    }


#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "### GAME::exit_game ###");
#endif


    if (fio.write_save(game_save) == false) {
        show_savegame_error();
    }

    run_game = false;

}

void game::game_over()
{
    _last_stage_used_teleporters.clear();

    timer.delay(200);
    input.clean();
    soundManager.load_music(game_data.game_over_music_filename);
    soundManager.play_music();
    graphLib.blank_screen();

    graphicsLib_gSurface config_bg;
    std::string filename = FILEPATH + "images/backgrounds/config.png";
    graphLib.surfaceFromFile(filename, &config_bg);
    graphLib.copyArea(st_rectangle(0, 0, config_bg.get_surface()->w, config_bg.get_surface()->h), st_position(0, 0), &config_bg, &graphLib.gameScreen);

    graphicsLib_gSurface dialog_img;
    filename = FILEPATH + "images/backgrounds/dialog.png";
    graphLib.surfaceFromFile(filename, &dialog_img);
    graphLib.copyArea(st_rectangle(0, 0, dialog_img.get_surface()->w, dialog_img.get_surface()->h), st_position(RES_W/2-dialog_img.get_surface()->w/2, RES_H/2-dialog_img.get_surface()->h/2), &dialog_img, &graphLib.gameScreen);

    graphLib.draw_centered_text(RES_H/2-6, strings_map::get_instance()->get_ingame_string(strings_ingame_gameover), graphLib.gameScreen, st_color(235, 235, 235));

    draw_lib.update_screen();
    timer.delay(400);
    input.wait_keypress();
    if (currentStage != INTRO_STAGE) {
        leave_stage();
    } else {
        soundManager.stop_music();
        soundManager.load_stage_music(stage_data.bgmusic_filename);
        // restart stage will play the music
        restart_stage();
    }
}

void game::show_ending()
{
    // save the data indicating game was finished, so user can see ending later or get access to more features
    game_config.game_finished = true;
    fio.save_config(game_config);

    // reset player colors to original
    player1.set_weapon(0, false);

    scenes.show_player_ending();
    scenes.show_enemies_ending();

    scenes.show_game_scene(GAME_SCENE_TYPES_ENDING_GAME_CREDITS);
    draw_lib.show_credits(false);

    /// @TODO add scene ending
    leave_game = true;
}

void game::show_demo_ending()
{
    input.clean();
    timer.delay(200);
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.draw_centered_text(10, "-THANKS FOR PLAYING-", st_color(255, 130, 0));
    graphLib.draw_centered_text(30, "YOU FINISHED ROCKBOT'S FREE VERSION.", st_color(240, 240, 240));
    graphLib.draw_centered_text(45, "THE COMPLETE GAME WILL BE AVAILABLE", st_color(240, 240, 240));
    graphLib.draw_centered_text(60, "FOR PURSHASE LATER DURING IN 2017.", st_color(240, 240, 240));
    graphLib.draw_centered_text(RES_H-40, "PRESENTED BY UPPERLAND.", st_color(240, 240, 240));
    graphLib.draw_centered_text(RES_H-20, "PRESS A BUTTON TO CONTINUE PLAYING.", st_color(240, 240, 240));
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::quick_load_game()
{
    if (fio.save_exists()) {
        fio.read_save(game_save);
    }

    currentStage = INTRO_STAGE;
    game_save.difficulty = DIFFICULTY_HARD;
    game_save.selected_player = PLAYER_1;

    /*
    // DEBUG //
    game_save.armor_pieces[ARMOR_ARMS] = true;
    game_save.armor_pieces[ARMOR_BODY] = true;
    game_save.armor_pieces[ARMOR_LEGS] = true;
    */

    if (GAME_FLAGS[FLAG_PLAYER1]) {
        game_save.selected_player = PLAYER_1;
    } else if (GAME_FLAGS[FLAG_PLAYER2]) {
        game_save.selected_player = PLAYER_2;
    } else if (GAME_FLAGS[FLAG_PLAYER3]) {
        game_save.selected_player = PLAYER_3;
    } else if (GAME_FLAGS[FLAG_PLAYER4]) {
        game_save.selected_player = PLAYER_4;
    }


    scenes.preloadScenes();

    // DEBUG //
    std::cout << "############### currentStage[" << (int)currentStage << "]" << std::endl;

    initGame();

    // TEST //
    scenes.pick_stage(0);

    scenes.boss_intro(currentStage);
    start_stage();
}

void game::update_stage_scrolling()
{
    if (timer.is_paused() == true) {
        return;
    }
    loaded_stage.change_map_scroll(checkScrolling(), true, false);
    st_position p_pos = player1.get_real_position();
    //std::cout << "p_pos.x: " << p_pos.x << std::endl;
    if (p_pos.x < 0.0) {
        player1.change_position_x(1);
        // out of screen, probably because was pushed out on a autoscroll stage
        if (p_pos.x < -(TILESIZE*2)) {
                player1.damage(999, true);
            }
    }
}



void game::draw_explosion(short int centerX, short int centerY, bool show_players) {
	unsigned int timerInit;
	int distance=0, mode=0;
	int accel=1;

	timerInit = timer.getTimer();

    draw_lib.update_screen();
	soundManager.stop_music();
	soundManager.play_sfx(SFX_PLAYER_DEATH);
	// x = a + r * cos(t)
	// y = b + r * sin(t)
	while (timer.getTimer() < timerInit+2000) {
        loaded_stage.showStage();
        if (show_players) {
            player1.show();
        }
        loaded_stage.showAbove();
        for (int i=0; i<6; i++) {
			graphLib.copyArea(st_rectangle(TILESIZE*mode, 0, graphLib.small_explosion.width/3, graphLib.small_explosion.height), st_position(centerX+distance*cos(static_cast<double>(i*45)), centerY+distance*sin(static_cast<double>(i*45))), &graphLib.small_explosion, &graphLib.gameScreen);
		}
		if (distance > 50) {
			for (int i=0; i<6; i++) {
				graphLib.copyArea(st_rectangle(TILESIZE*mode, 0, graphLib.small_explosion.width/3, graphLib.small_explosion.height), st_position(centerX+(distance-50)*cos(static_cast<double>(i*45)), centerY+(distance-50)*sin(static_cast<double>(i*45))), &graphLib.small_explosion, &graphLib.gameScreen);
			}
		}
		distance += 3;
		if (distance % 6 == 0) {
			if (mode+1 <= 2) {
				mode++;
			} else {
				mode = 0;
			}
			if (accel +1 <= 5) {
				accel++;
			}
		}
        draw_lib.update_screen();
        timer.delay(10);
	}
}

void game::draw_implosion(short int centerX, short int centerY) {
	int distance=RES_W*0.5, mode=0;
	int accel=1;
    int second_distance = 100;

    draw_lib.update_screen();
	soundManager.stop_music();
    soundManager.play_repeated_sfx(SFX_IMPLOSION, 1);
	// x = a + r * cos(t)
	// y = b + r * sin(t)
    while (distance > -second_distance) {
        loaded_stage.showStage();
        player1.show();
        loaded_stage.showAbove();
        if (distance > 0) {
            for (int i=0; i<6; i++) {
				graphLib.copyArea(st_rectangle(TILESIZE*mode, 0, graphLib.small_explosion.width/3, graphLib.small_explosion.height), st_position(centerX+distance*cos(static_cast<double>(i*45)), centerY+distance*sin(static_cast<double>(i*45))), &graphLib.small_explosion, &graphLib.gameScreen);
            }
        }
		if (distance < RES_W*0.5-50) {
            for (int i=0; i<6; i++) {
				graphLib.copyArea(st_rectangle(TILESIZE*mode, 0, graphLib.small_explosion.width/3, graphLib.small_explosion.height), st_position(centerX+(distance+second_distance)*cos(static_cast<double>(i*45)), centerY+(distance+second_distance)*sin(static_cast<double>(i*45))), &graphLib.small_explosion, &graphLib.gameScreen);
			}
		}
		distance -= 3;
		if ((int)abs((float)distance) % 4 == 0) {
			if (mode+1 <= 2) {
				mode++;
			} else {
				mode = 0;
			}
			if (accel +1 <= 5) {
				accel++;
			}
		}
        draw_lib.update_screen();
        timer.delay(15);
	}
}

void game::show_player()
{
    player1.show();
}

void game::set_player_position(st_position pos)
{
    player1.set_position(pos);
    player1.char_update_real_position();
}

void game::change_player_position(short xinc, short yinc)
{
    player1.change_position(xinc, yinc);
    player1.char_update_real_position();
}

void game::set_player_anim_type(ANIM_TYPE anim_type)
{
    player1.set_animation_type(anim_type);
}

st_position game::get_player_position()
{
    return st_position(player1.getPosition().x, player1.getPosition().y);
}

st_size game::get_player_size()
{
    return player1.get_size();
}

void game::set_player_direction(Uint8 direction)
{
    player1.set_direction(direction);
}

void game::player_fall()
{
    player1.fall();
}

void game::walk_character_to_screen_point_x(character *char_obj, short pos_x)
{
	/// @TODO: jump obstacles
	if (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
        char_obj->set_animation_type(ANIM_TYPE_WALK);
		char_obj->set_direction(ANIM_DIRECTION_LEFT);
		while (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
			char_obj->set_position(st_position(char_obj->getPosition().x-2, char_obj->getPosition().y));
            loaded_stage.showStage();
            loaded_stage.showAbove();
            loaded_stage.show_npcs();
            player1.show();
            draw_lib.update_screen();
			timer.delay(20);
		}
	} else if (char_obj->get_real_position().x+char_obj->get_size().width/2 < pos_x) {
		char_obj->set_direction(ANIM_DIRECTION_RIGHT);
        char_obj->set_animation_type(ANIM_TYPE_WALK);
		while (char_obj->get_real_position().x+char_obj->get_size().width/2 < pos_x) {
			char_obj->set_position(st_position(char_obj->getPosition().x+2, char_obj->getPosition().y));
            loaded_stage.showStage();
            loaded_stage.showAbove();
            loaded_stage.show_npcs();
            player1.show();
            draw_lib.update_screen();
			timer.delay(20);
		}
	}

}

void game::set_player_teleporter(short set_teleport_n, st_position set_player_pos, bool is_object)
{
    _player_teleporter.is_object = is_object;
	_player_teleporter.teleporter_n = set_teleport_n;
	_player_teleporter.old_player_pos.x = set_player_pos.x;
	_player_teleporter.old_player_pos.y = set_player_pos.y;
	_player_teleporter.active = true;
	_player_teleporter.finished = false;
    _player_teleporter.old_map_scroll = loaded_stage.getMapScrolling();
    _player_teleporter.old_map_n = loaded_stage.get_current_map_n();
}

bool game::is_player_on_teleporter()
{
    return _player_teleporter.active;
}

void game::remove_current_teleporter_from_list()
{
    if (_player_teleporter.teleporter_n != -1) {
        _last_stage_used_teleporters.erase(_player_teleporter.teleporter_n);
    }
    player1.set_teleporter(-1);
}

void game::select_game_screen()
{
    std::vector<std::string> game_list = fio.read_game_list();
    if (game_list.size() < 1) {
        _selected_game = std::string("");
        return;
    } else if (game_list.size() == 1) {
        _selected_game = game_list.at(0);
        return;
    }
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.draw_text(10, 10, "SELECT GAME:");

    option_picker main_picker(false, st_position(40, 50), game_list, false);
    main_picker.enable_check_input_reset_command();
    draw_lib.update_screen();
    bool repeat_menu = true;
    int picked_n = -1;
    while (repeat_menu == true) {
        picked_n = main_picker.pick();
        if (picked_n >= 0 && picked_n < game_list.size()) {
            repeat_menu = false;
        }
        main_picker.draw();
    }
    input.clean();
    timer.delay(200);

    //std::string game_dir = std::string("/games/") + game_list.at(picked_n) + std::string("/");

    _selected_game = game_list.at(picked_n);
}

string game::get_selected_game()
{
    return _selected_game;
}

bool game::is_free_version()
{
#ifdef DEMO_VERSION
    if (_selected_game == "Rockbot2") {
        return true;
    }
#endif
    return false;
}

float game::get_fps_speed_multiplier()
{
    float speed = fps_manager.get_fps_speed_multiplier();
    return speed;
}

void game::finish_player_teleporter()
{
    remove_all_projectiles();
    remove_players_slide();
    player1.recharge(ENERGY_TYPE_HP, ENERGY_ITEM_BIG);
    player1.teleport_out();
    timer.delay(1000);

    _player_teleporter.active = false;
    _last_stage_used_teleporters.insert(pair<int,bool>(_player_teleporter.teleporter_n, true));
	// teleport out
	soundManager.play_sfx(SFX_TELEPORT);
    player1.teleport_out();
    _player_teleporter.old_player_pos.y -= 5;
    player1.set_position(_player_teleporter.old_player_pos);
    loaded_stage.set_current_map(_player_teleporter.old_map_n);
    if (_last_stage_used_teleporters.size() == 8) {
        // search for the final-boss teleporter capsule and start it
        loaded_stage.activate_final_boss_teleporter();
    }
    loaded_stage.set_scrolling(st_float_position(_player_teleporter.old_map_scroll));
    std::cout << "CHAR::RESET_TO_STAND #Y.5" << std::endl;
    player1.set_animation_type(ANIM_TYPE_STAND);
    if (_player_teleporter.is_object == true) {
        loaded_stage.get_current_map()->finish_object_teleporter(_player_teleporter.teleporter_n);
    }
    player1.set_teleporter(-1);
    soundManager.stop_music();
    soundManager.load_stage_music(stage_data.bgmusic_filename);
    soundManager.play_music();
}

void game::show_stage(int wait_time, bool move_npcs)
{
    if (_dark_mode == false) {
        loaded_stage.showStage();
    }
	if (move_npcs == true) {
        loaded_stage.move_npcs();
	}
    if (_dark_mode == false) {
        loaded_stage.show_npcs();
        player1.show();
        loaded_stage.showAbove();
    }
	if (wait_time > 0) {
        timer.delay(wait_time);
	}
    draw_lib.update_screen();
}

bool game::subboss_alive_on_left(short tileX)
{
    return loaded_stage.subboss_alive_on_left(tileX);
}

classMap *game::get_current_map_obj()
{
    return loaded_stage.get_current_map();
}

void game::object_teleport_boss(st_position dest_pos, Uint8 dest_map, Uint8 teleporter_id)
{

    // checa se já foi usado
    if (_last_stage_used_teleporters.find(teleporter_id) != _last_stage_used_teleporters.end()) {
        return;
    }
    std::cout << "############################################ TELEPORT #2" << std::endl;
    set_player_teleporter(teleporter_id, st_position(player1.getPosition().x, player1.getPosition().y), true);
    classPlayer* test_player = &player1;
    test_player->teleport_out();
    graphLib.blank_screen();
    draw_lib.update_screen();
    timer.delay(500);

    set_current_map(dest_map);

    int new_scroll_pos = loaded_stage.get_first_lock_on_left(dest_pos.x);
    loaded_stage.set_scrolling(st_float_position(new_scroll_pos, 0));
    test_player->set_position(st_position(dest_pos.x*TILESIZE, 0));
    test_player->char_update_real_position();

    loaded_stage.get_current_map()->reset_scrolled();

    draw_lib.update_screen();
}



bool game::show_config(short finished_stage)
{
    if (scenes.show_main_config(finished_stage, true) == 1) {
        input.clean();
        timer.delay(50);
        config_manager.disable_ingame_menu();
        leave_stage();
        return true;
    }
    return false;
}

void game::show_savegame_error()
{
    std::vector<std::string> msgs;
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror1));
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror2));
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror3));
    draw_lib.show_ingame_warning(msgs);
}

void game::get_drop_item_ids()
{
    for (int i=0; i<DROP_ITEM_COUNT; i++) {
        _drop_item_list[i] = -1;
    }
    for (int i=0; i<GameMediator::get_instance()->object_list.size(); i++) {
        if (GameMediator::get_instance()->object_list.at(i).type == OBJ_LIFE) {
            _drop_item_list[DROP_ITEM_1UP] = i;
        } else if (GameMediator::get_instance()->object_list.at(i).type == OBJ_ENERGY_PILL_SMALL) {
            _drop_item_list[DROP_ITEM_ENERGY_SMALL] = i;
        } else if (GameMediator::get_instance()->object_list.at(i).type == OBJ_ENERGY_PILL_BIG) {
            _drop_item_list[DROP_ITEM_ENERGY_BIG] = i;
        } else if (GameMediator::get_instance()->object_list.at(i).type == OBJ_WEAPON_PILL_SMALL) {
            _drop_item_list[DROP_ITEM_WEAPON_SMALL] = i;
        } else if (GameMediator::get_instance()->object_list.at(i).type == OBJ_WEAPON_PILL_BIG) {
            _drop_item_list[DROP_ITEM_WEAPON_BIG] = i;
        }
    }
}

character *game::get_player()
{
    return &player1;
}

st_float_position game::get_current_stage_scroll()
{
    return loaded_stage.getMapScrolling();
}

void game::reset_scroll()
{
    loaded_stage.reset_scrolling();
}

short game::get_drop_item_id(short type)
{
    return _drop_item_list[type];
}

void game::show_map()
{
    loaded_stage.showStage();
    loaded_stage.showAbove();
    draw_lib.update_screen();
}



void game::showGotArmorDialog(e_ARMOR_PIECES armor_type)
{
    game_dialogs.showGotArmorDialog(armor_type);
}
