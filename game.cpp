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

#ifdef ANDROID
#include <android/log.h>
#include "ports/android/android_game_services.h"
extern android_game_services game_services;
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
game::game() : loaded_stage(-1, NULL), _show_boss_hp(false), player1(0)
{
    currentStage = INTRO_STAGE;
	_frame_duration = 1000/80; // each frame must use this share of time
    invencible_old_value = false;
    _dark_mode = false;
    is_showing_boss_intro = false;
    current_save_slot = 0;
    show_fps_enabled = false;
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

    fps_manager.initialize();

}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::show_game(bool can_characters_move, bool can_scroll_stage)
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

    // must jump a frame
    if (fps_manager.get_frame_drop_n() > 0 && fps_manager.get_current_frame_n() > 0) {
        int modulus = fps_manager.get_frame_drop_n() % fps_manager.get_current_frame_n();
        //std::cout << "MUST JUMP-FRAMES AT [" << fps_manager.get_frame_drop_n() << "], current-frame[" << fps_manager.get_current_frame_n() << "], modulus[" << modulus << "]" << std::endl;
        if (modulus == 0 && show_fps_enabled == true) {
            //std::cout << "JUMP FRAME[" << fps_manager.get_current_frame_n() << "]" << std::endl;
            fps_manager.fps_count();
            return;
        }
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
        loaded_stage.show_stage();
    }

    if (can_characters_move == true) {
        player1.execute();
        loaded_stage.move_npcs();
    }

    /// @TODO - move this to the player, so we don't need to check every single loop
    if (player1.is_dead() == true) {
        //std::cout << "### DEAD - RESTART_STAGE ###" << std::endl;
        restart_stage();
        return;
    }

    if (_dark_mode == false) {
        loaded_stage.show_objects();
        loaded_stage.show_npcs();
        player1.show();
        loaded_stage.show_above_objects();
        loaded_stage.showAbove();
    } else {
        graphLib.blank_screen();
    }


    //std::cout << "GFX_MODE[" << (int)loaded_stage.get_current_map_gfx_mode() << "]" << std::endl;

    if (loaded_stage.get_current_map_gfx_mode() == SCREEN_GFX_MODE_OVERLAY) {
        draw_lib.show_gfx();
    }

    // draw HUD
    draw_lib.show_hud(player1.get_current_hp(), 1, player1.get_selected_weapon(), player1.get_selected_weapon_value());

    if (show_fps_enabled == true) {
        fps_manager.fps_count();
    }
    fps_manager.limit();

    /*
    // REMOVED BECAUSE IT IS NOT USER FRIENDLY //
    if (fps_manager.get_failed_min_fps() == true) {
        if (game_config.graphics_performance_mode == PERFORMANCE_MODE_HIGH) {
            game_config.graphics_performance_mode = PERFORMANCE_MODE_NORMAL;
        } else if (game_config.graphics_performance_mode == PERFORMANCE_MODE_NORMAL) {
            game_config.graphics_performance_mode = PERFORMANCE_MODE_LOW;
        }
        fps_manager.reset_failed_min_fps();
    }
    */
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
    player1.reset_jump();

    player1.clean_projectiles();
    player1.set_direction(ANIM_DIRECTION_RIGHT);
    player1.refill_weapons();
    player1.reset_hp();

    loaded_stage.show_stage();
    loaded_stage.showAbove();
    //draw_lib.update_screen();
    draw_lib.fade_in_screen(0, 0, 0);

    game_unpause();

    soundManager.play_music();

    for (int i=0; i<AUTOSCROLL_START_DELAY_FRAMES; i++) { // extra delay to show dialogs
        input.clean_all();
        loaded_stage.show_stage();
        loaded_stage.showAbove();
        draw_lib.update_screen();
        timer.delay(20);
    }
    loaded_stage.add_autoscroll_delay();

    show_player_teleport(PLAYER_INITIAL_X_POS);
    if (!game_save.stages[currentStage]) {
        game_dialogs.show_stage_dialog(currentStage);
    }
    show_game(false, false);
    // reset timers for objects
    loaded_stage.reset_objects_timers();


}

void game::set_player_position_teleport_in(int initial_pos_x)
{
    int first_unlocked_from_bottom = loaded_stage.get_current_map()->get_first_lock_on_bottom(initial_pos_x, player1.get_size().width, player1.get_size().height);

    std::cout << ">>>>>>>>>> GAME::set_player_position_teleport_in::first_unlocked_from_bottom[" << first_unlocked_from_bottom << "]" << std::endl;

    player1.set_position(st_position(initial_pos_x, (first_unlocked_from_bottom+1)*TILESIZE-player1.get_size().height));
    player1.char_update_real_position();
    player1.set_animation_type(ANIM_TYPE_TELEPORT);
    player1.set_animation_frame(0);

    std::cout << ">>>>>>>>>> GAME::set_player_position_teleport_in::DONE" << std::endl;
}

void game::show_player_teleport(int pos_x)
{
    //std::cout << "GAME::show_player_telport #2" << std::endl;

    // find ground for player
    set_player_position_teleport_in(pos_x);
    long end_time = timer.getTimer() + 1500;

    //std::cout << "GAME::show_player_telport #2" << std::endl;

    while (timer.getTimer() < end_time) {
        loaded_stage.show_stage();
        loaded_stage.showAbove();
        if (player1.animation_has_restarted()) {
            player1.set_animation_frame(1);
            player1.set_animation_has_restarted(false);
        }
        player1.show();
        draw_lib.update_screen();
        timer.delay(20);
    }

    //std::cout << "GAME::show_player_telport #3" << std::endl;
    player1.set_animation_frame(2);
    player1.show();
    draw_lib.update_screen();
    timer.delay(20);
    //std::cout << "GAME::show_player_telport #4" << std::endl;

    show_ready();
    //std::cout << "GAME::show_player_telport #5" << std::endl;
    // force stand to avoid gravity not doing it for any reason
    player1.set_animation_type(ANIM_TYPE_STAND);

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

    player1.reset_hp();
    player1.reset_jump();
    player1.cancel_slide();
    loaded_stage.reset_stage_maps();

    game_unpause();

    loaded_stage.show_stage();
    loaded_stage.showAbove();
	graphLib.set_screen_adjust(st_position(0, 0));
    draw_lib.update_screen();
    // if was on stage-boss, mneeds to reload music
    if (soundManager.get_is_playing_boss_music() == true) {
        soundManager.load_stage_music(stage_data.bgmusic_filename);
    }
    soundManager.restart_music();
    if (checkpoint.y == -1) { // did not reached any checkpoint, use the calculated value from stage start
        // find teleport stop point
        show_player_teleport(PLAYER_INITIAL_X_POS);
    } else {
        show_player_teleport(checkpoint.x);
    }

    while (player1.get_anim_type() == ANIM_TYPE_TELEPORT) {
        input.clean_all();
        show_game(true, false);
        draw_lib.update_screen();
    }
    for (int i=0; i<AUTOSCROLL_START_DELAY_FRAMES; i++) { // extra delay to teleport without moving screen
        input.clean_all();
        show_game(false, false);
        draw_lib.update_screen();
        timer.delay(20);
    }

}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool game::show_game_intro()
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



    currentStage = INTRO_STAGE;

	initGame();

    if (game_save.stages[INTRO_STAGE] == 0 && !GAME_FLAGS[FLAG_ALLWEAPONS]) {
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
    graphLib.draw_centered_text(60, "THIS IS A TEST VERSION OF ROCKBOT,");
    graphLib.draw_centered_text(75, "IT DOES CONTAIN ERRORS AND IS NOT");
    graphLib.draw_centered_text(90, "COMPLETE MISSING SOME FEATURES.");

    graphLib.draw_centered_text(120, "SOFTWARE IS PROVIDED \"AS IS\"");
    graphLib.draw_centered_text(135, "WITHOUT WARRANTY OF ANY KIND,");
    graphLib.draw_centered_text(150, "EXPRESS OR IMPLIED FROM AUTHOR.");

    graphLib.draw_centered_text(170, "REPORT ANY FOUND ISSUES TO");
    graphLib.draw_centered_text(185, "bugs@upperland.net");
    graphLib.draw_centered_text(210, "PRESS A BUTTON OR KEY TO CONTINUE.");
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::show_free_version_warning()
{
    graphLib.blank_screen();
    draw_lib.update_screen();
    input.clean();
    timer.delay(100);

    graphLib.draw_centered_text(10, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning_title, game_config.selected_language), graphLib.gameScreen, st_color(255, 130, 0));
    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning1, game_config.selected_language));
    graphLib.draw_centered_text(45, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning2, game_config.selected_language));
    graphLib.draw_centered_text(60, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning3, game_config.selected_language));
    graphLib.draw_centered_text(75, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning4, game_config.selected_language));
    graphLib.draw_centered_text(90, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning5, game_config.selected_language));
    graphLib.draw_centered_text(105, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning6, game_config.selected_language));
    graphLib.draw_centered_text(130, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning7, game_config.selected_language));
    graphLib.draw_centered_text(145, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning8, game_config.selected_language));
    graphLib.draw_centered_text(160, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning9, game_config.selected_language));
    graphLib.draw_centered_text(175, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning10, game_config.selected_language));

    graphLib.draw_centered_text(205, strings_map::get_instance()->get_ingame_string(string_intro_demo_warning11, game_config.selected_language));
    graphLib.draw_centered_text(220, strings_map::get_instance()->get_ingame_string(string_press_key_or_button, game_config.selected_language));
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::show_notice()
{
    graphLib.blank_screen();
    draw_lib.update_screen();

    graphicsLib_gSurface upperland_surface;
    graphLib.surfaceFromFile(GAMEPATH + "/shared/images/upperland.png", &upperland_surface);

    st_position logo_pos(RES_W/2 - (upperland_surface.width/6)/2, RES_H/2 - upperland_surface.height/2);


    graphLib.draw_centered_text(logo_pos.y + upperland_surface.height + 7, strings_map::get_instance()->get_ingame_string(string_intro_upperland_studios, game_config.selected_language), graphLib.gameScreen, st_color(199, 215, 255));
    graphLib.draw_centered_text(logo_pos.y + upperland_surface.height + 19, strings_map::get_instance()->get_ingame_string(string_intro_presents, game_config.selected_language), graphLib.gameScreen, st_color(199, 215, 255));


    //std::cout << ">> logo_pos.x: " << logo_pos.x << ", logo_pos.y: " << logo_pos.y << std::endl;
    graphLib.copyArea(st_rectangle(0, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);
    graphLib.draw_centered_text(220, "HTTP://ROCKBOT.UPPERLAND.NET");
    draw_lib.update_screen();
    input.clean_and_wait_scape_time(400);
    for (int i=1; i<6; i++) {
        graphLib.copyArea(st_rectangle((upperland_surface.width/6)*i, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);
        draw_lib.update_screen();
        input.wait_scape_time(30);
    }
    graphLib.copyArea(st_rectangle(0, 0, upperland_surface.width/6, upperland_surface.height), logo_pos, &upperland_surface, &graphLib.gameScreen);


    draw_lib.update_screen();

    input.clean_and_wait_scape_time(1200);

    graphLib.blank_screen();

    graphLib.draw_centered_text(30, strings_map::get_instance()->get_ingame_string(string_intro_engine1, game_config.selected_language), graphLib.gameScreen, st_color(199, 215, 255));
    graphLib.draw_centered_text(60, strings_map::get_instance()->get_ingame_string(string_intro_engine2, game_config.selected_language));
    graphLib.draw_centered_text(80, strings_map::get_instance()->get_ingame_string(string_intro_engine3, game_config.selected_language));
    graphLib.draw_centered_text(100, strings_map::get_instance()->get_ingame_string(string_intro_engine4, game_config.selected_language));
    graphLib.draw_centered_text(120, strings_map::get_instance()->get_ingame_string(string_intro_engine5, game_config.selected_language));
    graphLib.draw_centered_text(140, strings_map::get_instance()->get_ingame_string(string_intro_engine6, game_config.selected_language));
    graphLib.draw_centered_text(180, strings_map::get_instance()->get_ingame_string(string_intro_engine7, game_config.selected_language));
    graphLib.draw_centered_text(200, strings_map::get_instance()->get_ingame_string(string_intro_engine8, game_config.selected_language));

    draw_lib.update_screen();

    input.clean_and_wait_scape_time(4000);

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

    graphLib.set_screen_adjust(st_position(0, 0));
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
        draw_lib.fade_out_screen(0, 0, 0, 300);
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
    remove_temp_objects();

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
            show_game(false, true);
            timer.delay(50);
            draw_lib.fade_in_screen(0, 0, 0);
        } else {
            test_player->set_position(st_position(stage_data.links[j].pos_destiny.x*TILESIZE,  stage_data.links[j].pos_destiny.y*TILESIZE));
            test_player->set_animation_type(ANIM_TYPE_JUMP);
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


void game::map_present_boss(bool show_dialog, bool is_static_boss)
{
	is_showing_boss_intro = true;

    soundManager.stop_music();
    soundManager.unload_music();

    // 1. keep showing game screen until player reaches ground
    player1.clear_move_commands();
	bool loop_run = true;
	while (loop_run == true) {
        loaded_stage.show_stage();
        player1.charMove();
        int anim_type = player1.get_anim_type();
        if (player1.hit_ground() == true && anim_type == ANIM_TYPE_STAND) {
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
    classnpc* boss_ref = loaded_stage.get_near_boss();
    if (boss_ref != NULL) {
        if (is_static_boss == false) {
            loop_run = true;
            while (loop_run == true) {
                if (loaded_stage.boss_hit_ground(boss_ref) == true) {
                    loop_run = false;
                    show_stage(0, false);
                } else {
                    show_stage(0, true);
                }
            }
        } else {
            // TODO //
        }
    }
    show_stage(8, false);


	if (show_dialog == true) {
		// 4. show boss dialog
        dialogs boss_dialog;
        if (!game_save.stages[loaded_stage.get_number()]) {
            boss_dialog.show_boss_dialog(loaded_stage.get_number());
        }
	}

    show_stage(8, false);


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

void game::remove_temp_objects()
{
    loaded_stage.get_current_map()->remove_temp_objects();
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
    temp_map->set_foreground_postion(loaded_stage.get_current_map()->get_foreground_postion());

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
    loaded_stage.show_stage();


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


            int temp_map_3rdlevel_pos = (RES_H+TILESIZE*0.5) - i*TRANSITION_STEP - 8;
            if (type == TRANSITION_TOP_TO_BOTTOM) {
                loaded_stage.get_current_map()->show_objects(-i*TRANSITION_STEP);
                temp_map->show_objects(temp_map_3rdlevel_pos, adjust_x);
            } else {
                temp_map_3rdlevel_pos = -(RES_H+TILESIZE*0.5) + i*TRANSITION_STEP + 8; // 8 is a adjust for some error I don't know the reason
                loaded_stage.get_current_map()->show_objects(i*TRANSITION_STEP);
                temp_map->show_objects(temp_map_3rdlevel_pos, adjust_x);
            }

			pObj->char_update_real_position();
			pObj->show();


			if (type == TRANSITION_TOP_TO_BOTTOM) {
                loaded_stage.showAbove(-i*TRANSITION_STEP, false);
                loaded_stage.get_current_map()->show_above_objects(-i*TRANSITION_STEP);
                temp_map->show_above_objects(temp_map_3rdlevel_pos, adjust_x);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			} else {
                loaded_stage.showAbove(i*TRANSITION_STEP, false);
                loaded_stage.get_current_map()->show_above_objects(i*TRANSITION_STEP);
                temp_map->show_above_objects(temp_map_3rdlevel_pos, adjust_x);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			}

            // draw HUD
            draw_lib.show_hud(player1.get_current_hp(), 1, player1.get_selected_weapon(), player1.get_selected_weapon_value());


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

    // when transition finished, spawned npcs and objects such as jet/coild, must be removed
    //temp_map->remove_temp_objects();


    temp_screen.freeGraphic();
	pObj->set_teleporter(-1);
	pObj->char_update_real_position();
}


void game::horizontal_screen_move(short direction, bool is_door, short tileX, short tileY)
{
    int i = 0;
    int upTile = 0;
    int downTile = 0;
    st_float_position scroll_move;

    game_pause();

    classMap* temp_map = loaded_stage.get_current_map();

    graphLib.set_screen_adjust(st_position(0, 0));

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
            game_unpause();
            return;
        }
        loaded_stage.show_stage();
	}



    int move_limit = (RES_W/abs((float)scroll_move.x)) - TILESIZE/abs((float)scroll_move.x);
    float player_move_x = (float)(TILESIZE*2.5)/(float)move_limit; // player should move two tilesize, to avoid doors
    if (scroll_move.x < 0) {
        player_move_x = player_move_x * -1;
    }
    std::cout << "player_move_x[" << player_move_x << "]" << std::endl;
    int static_scroll_x = loaded_stage.getMapScrolling().x;
    for (int i=0; i<move_limit; i++) {
        //loaded_stage.changeScrolling(scroll_move, false);
        loaded_stage.change_map_scroll(scroll_move, false, true);
        loaded_stage.show_stage();
        if (loaded_stage.must_show_static_bg() == false) {
            loaded_stage.show_npcs();
        } else {
            loaded_stage.show_npcs_to_left(static_scroll_x+RES_W);
        }
        player1.show();
        loaded_stage.showAbove();
        // draw HUD
        draw_lib.show_hud(player1.get_current_hp(), 1, player1.get_selected_weapon(), player1.get_selected_weapon_value());
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
        remove_players_slide();
    }
    timer.delay(6);
    game_unpause();
    loaded_stage.add_autoscroll_delay();
    loaded_stage.show_stage();
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

        soundManager.stop_music();
		// fall to ground
        soundManager.play_sfx(SFX_BIG_EXPLOSION);
        player1.fall();
        player1.set_animation_type(ANIM_TYPE_GOT_WEAPON);

        std::vector<st_color> color_list;
        color_list.push_back(st_color(251, 225, 101));
        color_list.push_back(st_color(101, 105, 251));
        color_list.push_back(st_color(112, 251, 101));
        color_list.push_back(st_color(251, 101, 101));
        soundManager.play_sfx(SFX_GOT_WEAPON);
        for (int i=0; i<2; i++) {
            for (int j=0; j<color_list.size(); j++) {
                graphLib.clear_area(0, 0, RES_W, RES_H, color_list.at(j).r, color_list.at(j).g, color_list.at(j).b);
                player1.show();
                draw_lib.update_screen();
                timer.delay(300);
            }
        }
        long end_timer = timer.getTimer() + 3500;
        while (timer.getTimer() < end_timer) {
            loaded_stage.show_stage();
            player1.show();
            loaded_stage.showAbove();
            draw_lib.update_screen();
            timer.delay(250);
        }

		/// @TODO
		// show the "you got" screen
		graphLib.blank_screen();
        soundManager.load_music(game_data.got_weapon_music_filename);
		graphLib.blink_screen(255, 255, 255);
		graphLib.blank_screen();
        soundManager.play_music();

        graphLib.show_config_bg();

        player1.set_position(st_position(20, (RES_H * 0.5 - player1.get_size().height/2)));
        player1.set_animation_type(ANIM_TYPE_ATTACK);
        loaded_stage.set_scrolling(st_float_position(0, 0));
        player1.char_update_real_position();
        player1.show();

        std::string weapon_name(game_data.weapons[currentStage].name);
		for (std::string::iterator p = weapon_name.begin(); weapon_name.end() != p; ++p) {
			*p = toupper(*p);
		}


        // line 1, weapon name; line 2 extra item; line 3 was acquired
		std::string extra_name = "";
        if (currentStage == COIL_GOT_STAGE) {
            std::string item_name = strings_map::get_instance()->toupper(std::string(GameMediator::get_instance()->object_list.at(game_data.player_items[0]).name));
            extra_name = strings_map::get_instance()->get_ingame_string(strings_ingame_and, game_config.selected_language) + std::string(" ") + item_name;
        } else if (currentStage == JET_GOT_STAGE) {
            std::string item_name = strings_map::get_instance()->toupper(std::string(GameMediator::get_instance()->object_list.at(game_data.player_items[1]).name));
            extra_name = strings_map::get_instance()->get_ingame_string(strings_ingame_and, game_config.selected_language) + std::string(" ") + item_name;
        }
        std::string phrase = std::string(strings_map::get_instance()->get_ingame_string(strings_ingame_yougot_singular, game_config.selected_language) + " ");
        graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 - 4), weapon_name, false);
        short line3_pos = RES_H * 0.5 + 8;
        if (extra_name.length() > 0) {
            graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 + 8), extra_name, false);
            line3_pos = RES_H * 0.5 + 20;
            phrase = std::string(strings_map::get_instance()->get_ingame_string(strings_ingame_yougot_plural, game_config.selected_language) + " ");
        }
        graphLib.draw_progressive_text((RES_W * 0.5 - 90), line3_pos, phrase, false);



        player1.show();

        graphLib.wait_and_update_screen(5000);
	}

    game_save.stages[currentStage] = 1;


    if (is_free_version() && game_save.stages[DEMO_VERSION_STAGE1] == 1 && game_save.stages[DEMO_VERSION_STAGE2] == 1) {
        show_demo_ending();
    }

    leave_stage();
}

void game::leave_stage()
{
    save_game();
    draw_lib.set_flash_enabled(false);
    freeze_weapon_effect = FREEZE_EFFECT_NONE;
    GAME_FLAGS[FLAG_INVENCIBLE] = invencible_old_value;

    input.clean();
    timer.delay(200);

    // return to stage selection
    player1.reset_charging_shot();
    player1.set_weapon(WEAPON_DEFAULT, false);
    // @TODO: last stage must be set by game_data.final_boss_id //
    if (currentStage >= CASTLE1_STAGE1 && currentStage < CASTLE1_STAGE5) {
        currentStage = scenes.pick_stage(currentStage+1);
    } else {
        currentStage = scenes.pick_stage(currentStage);
    }
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

void game::return_to_intro_screen()
{
    save_game();

    draw_lib.set_flash_enabled(false);
    freeze_weapon_effect = FREEZE_EFFECT_NONE;
    GAME_FLAGS[FLAG_INVENCIBLE] = invencible_old_value;

    input.clean();
    timer.delay(200);

    // return to stage selection
    player1.reset_charging_shot();
    player1.set_weapon(WEAPON_DEFAULT, false);

    scenes.main_screen();
    currentStage = INTRO_STAGE;
    leave_game = false;

    if (game_save.stages[INTRO_STAGE] == 0 && !GAME_FLAGS[FLAG_ALLWEAPONS]) {
        currentStage = INTRO_STAGE;
        player1.initialize();
        loaded_stage = stage(currentStage, &player1);
        player1.initFrames();
        player1.set_is_player(true);
        player1.reset_hp();
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
    player1.reset_sprite_animation_timer();
    loaded_stage.reset_stage_timers();
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


    save_game();

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


    graphLib.show_config_bg();

    graphicsLib_gSurface dialog_img;
    std::string filename = FILEPATH + "images/backgrounds/dialog.png";
    graphLib.surfaceFromFile(filename, &dialog_img);
    graphLib.copyArea(st_rectangle(0, 0, dialog_img.get_surface()->w, dialog_img.get_surface()->h), st_position(RES_W/2-dialog_img.get_surface()->w/2, RES_H/2-dialog_img.get_surface()->h/2), &dialog_img, &graphLib.gameScreen);

    graphLib.draw_centered_text(RES_H/2-6, strings_map::get_instance()->get_ingame_string(strings_ingame_gameover, game_config.selected_language));

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
    scenes.show_player_walking_ending();

    scenes.show_enemies_ending();

    scenes.show_game_scene(GAME_SCENE_TYPES_ENDING_GAME_CREDITS);
    draw_lib.show_credits(false);

    return_to_intro_screen();
}

void game::show_demo_ending()
{
    input.clean();
    timer.delay(200);
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.draw_centered_text(10, "-THANKS FOR PLAYING-", st_color(255, 130, 0));
    graphLib.draw_centered_text(30, "YOU FINISHED ROCKBOT'S FREE VERSION.");
    graphLib.draw_centered_text(45, "THE COMPLETE GAME WILL BE AVAILABLE");
    graphLib.draw_centered_text(60, "AS AN OPEN BETA VERSION FOR");
    graphLib.draw_centered_text(60, "PURSHASE BY 2017'S ENDING.");
    graphLib.draw_centered_text(RES_H-40, "PRESENTED BY UPPERLAND STUDIOS.");
    graphLib.draw_centered_text(RES_H-20, "PRESS A BUTTON TO CONTINUE PLAYING.");
    draw_lib.update_screen();
    input.wait_keypress();
}

void game::quick_load_game()
{
    GAME_FLAGS[FLAG_ALLWEAPONS] = true;
    if (fio.save_exists(current_save_slot)) {
        fio.read_save(game_save, current_save_slot);
    }

    game_save.difficulty = DIFFICULTY_HARD;
    game_save.selected_player = PLAYER_4;

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

    // TEST //
    GAME_FLAGS[FLAG_ALLWEAPONS] = true;
    currentStage = scenes.pick_stage(INTRO_STAGE);
    //currentStage = CASTLE1_STAGE1;


    // DEBUG //
    std::cout << "############### currentStage[" << (int)currentStage << "]" << std::endl;

    initGame();

    // DEBUG //
    //show_ending();

    scenes.boss_intro(currentStage);

    //game_save.armor_pieces[ARMOR_TYPE_LEGS] = true;

    start_stage();

    //got_weapon();
}

void game::set_player_direction(ANIM_DIRECTION dir)
{
    player1.set_direction(dir);
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

    //ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize
    st_float_position anim_pos = st_float_position(centerX-23+get_current_map_obj()->get_map_scrolling_ref()->x, centerY-23);
    get_current_map_obj()->add_animation(ANIMATION_STATIC, &graphLib.explosion_player_death, anim_pos, st_position(0, 0), 100, 6, player1.get_direction(), st_size(47, 47));

    while (timer.getTimer() < timerInit+2000) {
        loaded_stage.show_stage();
        if (show_players) {
            player1.show();
        }
        loaded_stage.showAbove();
        draw_lib.update_screen();
        timer.delay(10);
    }
    timer.delay(300);
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

void game::show_player_at(int x, int y)
{
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "### GAME::show_player_at[%d, %d] ###", x, y);
#endif
    //std::cout << "show_player_at[" << x << ", " << y << "]" << std::endl;
    player1.show_at(st_position(x, y));
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

void game::walk_character_to_screen_point_x(character *char_obj, short pos_x)
{
	/// @TODO: jump obstacles
	if (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
        char_obj->set_animation_type(ANIM_TYPE_WALK);
		char_obj->set_direction(ANIM_DIRECTION_LEFT);
		while (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
			char_obj->set_position(st_position(char_obj->getPosition().x-2, char_obj->getPosition().y));
            loaded_stage.show_stage();
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
            loaded_stage.show_stage();
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

    std::cout << "################### SET PLAYER TELEPORTER ###################" << std::endl;

	_player_teleporter.active = true;
	_player_teleporter.finished = false;
    _player_teleporter.old_map_scroll = loaded_stage.getMapScrolling();
    _player_teleporter.old_map_n = loaded_stage.get_current_map_n();
}

bool game::is_player_on_teleporter()
{
    std::cout << "######## is_player_on_teleporter[" << _player_teleporter.active << "] ########" << std::endl;
    return _player_teleporter.active;
}

short game::get_last_castle_stage()
{
    if (fio.can_access_castle(game_save) == false && !GAME_FLAGS[FLAG_ALLWEAPONS]) {
        return STAGE8;
    }
    int pos_n = CASTLE1_STAGE1; // stage 1 is accessible when all initial stages are completed
    for (int i=CASTLE1_STAGE1; i<=CASTLE1_STAGE5; i++) {
        //std::cout << "CASTLE1_STAGE1[" << CASTLE1_STAGE1 << "], stage[" << i << "]: (" << game_save.stages[i] << ")" << std::endl;
        if (game_save.stages[i] == 0 && !GAME_FLAGS[FLAG_ALLWEAPONS]) {
            break;
        }
        pos_n = i+1;
    }
    std::cout << "game::get_last_castle_stage[" << pos_n << "]" << std::endl;
    if (pos_n > CASTLE1_STAGE5) {
        pos_n = CASTLE1_STAGE5;
    }
    return pos_n;
}

short game::get_current_save_slot()
{
    return current_save_slot;
}

void game::set_current_save_slot(short n)
{
    current_save_slot = n;
}

void game::save_game()
{
    if (fio.write_save(game_save, current_save_slot) == false) {
        show_savegame_error();
        return;
    }
#ifdef ANDROID
    // if config is set to use cloud
    /*
    if (game_config.android_use_play_services == true && game_config.android_use_cloud_save) {
        graphicsLib_gSurface bg_copy;
        graphLib.initSurface(st_size(RES_W, RES_H), &bg_copy);
        graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &bg_copy);

        graphLib.blank_screen();
        graphLib.draw_text(10, 10, "WRITTING SAVE DATA TO GOOGLE DRIVE,");
        graphLib.draw_text(10, 20, "PLEASE WAIT AND BE SURE TO HAVE");
        graphLib.draw_text(10, 30, "AN AVAILABLE NETWORK CONNECTION.");
        graphLib.updateScreen();
        game_services.cloud_save_game(current_save_slot);
        timer.delay(200);
        graphLib.copyArea(st_position(0, 0), &bg_copy, &graphLib.gameScreen);
        graphLib.updateScreen();
    }
    */
#endif
}

void game::set_show_fps_enabled(bool enabled)
{
    show_fps_enabled = enabled;
}

bool game::get_show_fps_enabled()
{
    return show_fps_enabled;
}


#ifdef ANDROID
bool game::load_save_data_from_cloud()
{
    graphicsLib_gSurface bg_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &bg_copy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &bg_copy);

    // show loading from network dialog
    graphLib.blank_screen();
    graphLib.draw_text(10, 10, "LOADING SAVE DATA FROM GOOGLE DRIVE,");
    graphLib.draw_text(10, 20, "PLEASE WAIT AND BE SURE TO HAVE");
    graphLib.draw_text(10, 30, "AN AVAILABLE NETWORK CONNECTION.");
    graphLib.updateScreen();
    for (int i=0; i<SAVE_MAX_SLOT_NUMBER; i++) {
        game_services.cloud_load_game(i);
    }
    graphLib.copyArea(st_position(0, 0), &bg_copy, &graphLib.gameScreen);
    graphLib.updateScreen();
    return fio.have_one_save_file();
}
#endif



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
    graphLib.show_config_bg();
    graphLib.draw_text(10, 20, "SELECT GAME:");

    option_picker main_picker(false, st_position(30, 40), game_list, false);
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
    return true;
#else
    return false;
#endif
}


void game::finish_player_teleporter()
{
    remove_all_projectiles();
    remove_players_slide();
    player1.recharge(ENERGY_TYPE_HP, ENERGY_ITEM_BIG);
    draw_lib.fade_out_screen(0, 0, 0, 500);
    timer.delay(1000);

    std::cout << "################### RESET PLAYER TELEPORTER ###################" << std::endl;
    _player_teleporter.active = false;
    _last_stage_used_teleporters.insert(pair<int,bool>(_player_teleporter.teleporter_n, true));
	// teleport out
	soundManager.play_sfx(SFX_TELEPORT);
    draw_lib.fade_out_screen(0, 0, 0, 500);
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
        loaded_stage.show_stage();
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
    draw_lib.fade_out_screen(0, 0, 0, 500);
    draw_lib.update_screen();
    timer.delay(500);

    set_current_map(dest_map);

    int new_scroll_pos = loaded_stage.get_first_lock_on_left(dest_pos.x);
    loaded_stage.set_scrolling(st_float_position(new_scroll_pos, 0));
    classPlayer* test_player = &player1;
    test_player->set_position(st_position(dest_pos.x*TILESIZE, 0));
    test_player->char_update_real_position();

    loaded_stage.get_current_map()->reset_scrolled();

    draw_lib.update_screen();
}



bool game::show_config(short finished_stage)
{
    game_menu menu;
    if (menu.show_main_config(finished_stage, true) == 1) {
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
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror1, game_config.selected_language));
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror2, game_config.selected_language));
    msgs.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_savegameerror3, game_config.selected_language));
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
    loaded_stage.show_stage();
    loaded_stage.showAbove();
    draw_lib.update_screen();
}

void game::set_current_stage(int stage)
{
    currentStage = stage;
}



void game::showGotArmorDialog(e_ARMOR_PIECES armor_type)
{
    game_dialogs.showGotArmorDialog(armor_type);
}
