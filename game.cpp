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

#include "scenes/ending.h"
#include "file/file_io.h"

extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::file_io fio;


extern struct format_v_2_0_1::st_checkpoint checkpoint;


extern string FILEPATH;

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;

extern bool GAME_FLAGS[FLAG_COUNT];

extern bool leave_game;

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;

// ********************************************************************************************** //
// class constructor                                                                              //
// ********************************************************************************************** //
game::game() : loaded_stage(NULL), _show_boss_hp(false), fps_timer(0)
{
	currentStage = 1;
	fps_counter = 0;
	_frame_duration = 1000/80; // each frame must use this share of time
    invencible_old_value = false;
    _dark_mode = false;
    is_showing_boss_intro = false;
}

// ********************************************************************************************** //
// class destructor                                                                               //
// ********************************************************************************************** //
game::~game()
{
    // clean allocated stages
    //delete loaded_stage;
}

// ********************************************************************************************** //
// initializar game, can't be on constructor because it needs other objects (circular)            //
// ********************************************************************************************** //
void game::initGame()
{
	stringstream player_name;
    player_name << "p" << game_save.selected_player;
    players.push_back(classPlayer(player_name.str(), game_save.selected_player));
    // @TODO - optimixzation: make this uneeded
    // always insert all possible players, as it is used in cutscenes and such
    for (int i=0; i<FS_MAX_PLAYERS; i++) {
        if (i != game_save.selected_player) {
            stringstream player_name2;
            player_name2 << "p" << i;
            players.push_back(classPlayer(player_name2.str(), i));
        }
    }
    config_manager.set_player_ref(&players.at(0));
    unload_stage();
    loaded_stage = new stage(currentStage, players);
    players.at(0).set_is_player(true);
	players.at(0).reset_hp();
    players.at(1).set_is_player(true);
    players.at(1).reset_hp();
    invencible_old_value = GAME_FLAGS[FLAG_INVENCIBLE];

}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::showGame(bool can_characters_move)
{
    if (leave_game == true) {
        exit_game();
    }
    if (players.at(0).is_teleporting() == false) { // ignore input while player is teleporting because it caused some issues
        input.readInput();
    } else {
        input.clean();
    }

    if (config_manager.execute_ingame_menu()) { // game is paused
        return;
    }

    /// @TODO - move this to the player, so we don't need to check every single loop
    if (players.at(0).is_dead() == true) {
        restart_stage();
        return;
    }

    if (test_teleport(&players.at(0))) {
        return;
    }

    update_stage_scrolling();


    loaded_stage->move_objects();

    if (_dark_mode == false) {
        loaded_stage->showStage();
    }

    if (can_characters_move == true) {
        players.at(0).execute();
        loaded_stage->move_npcs();
    }

    if (_dark_mode == false) {
        loaded_stage->show_npcs();
        players.at(0).show();
        loaded_stage->show_objects();
        loaded_stage->showAbove();
    } else {
        graphLib.blank_screen();
    }
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 game::getMapPointLock(struct st_position pos)
{
    return loaded_stage->getMapPointLock(pos);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_position game::checkScrolling()
{
	st_position move;
    st_position mapScroll = loaded_stage->getMapScrolling();
    st_position p1Pos(players.at(0).getPosition().x,  players.at(0).getPosition().y);

	//cout << "game::checkScrolling - p1Pos.x: " << p1Pos.x << ", mapScroll.x: " << mapScroll.x << ", RES_W/2: " << RES_W/2 << "\n";


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

    graphLib.set_colormap(currentStage);

	_show_boss_hp = false;
    input.clean();
    loaded_stage->reset_current_map();


	// TODO - this must be on a single method in soundlib
	players.at(0).set_position(st_position(80, -TILESIZE));

	soundManager.stop_music();
    soundManager.load_stage_music(stage_data.bgmusic_filename);


    loaded_stage->reload_stage();

    players.at(0).reset_charging_shot();
    players.at(0).cancel_slide();

    draw_lib.update_colorcycle();

    loaded_stage->showStage();
    loaded_stage->showAbove();
    draw_lib.update_screen();


    players.at(0).clean_projectiles();
	players.at(0).set_animation_type(ANIM_TYPE_TELEPORT);
    players.at(0).set_direction(ANIM_DIRECTION_RIGHT);
    players.at(0).set_map(loaded_stage->get_current_map());
	players.at(0).refill_weapons();
	players.at(0).reset_hp();


    // find teleport stop point
    int min_y = loaded_stage->get_teleport_minimal_y(95); // x = 80 + half a player width (30)
    players.at(0).set_teleport_minimal_y((min_y-3)*TILESIZE);


    loaded_stage->showStage(); // to fix colorcycle colors
    loaded_stage->showAbove();
    draw_lib.update_screen();




    show_ready(); /// @TODO - not updating colorcycle for some reason...

    soundManager.play_music();

	/// @TODO: do not show twice
	if (GAME_FLAGS[FLAG_QUICKLOAD] == false) {
		while (players.at(0).get_anim_type() == ANIM_TYPE_TELEPORT) {
			showGame();
            draw_lib.update_screen();
		}
		for (int i=0; i<15; i++) { // extra delay to show dialogs
			showGame(false);
            draw_lib.update_screen();
		}
		if (game_save.stages[currentStage] == 0) {
            game_dialogs.show_stage_dialog();
            // reset timers for objects
            loaded_stage->reset_objects_timers();
		}
    }
}

void game::show_ready() const
{
    for (int i=0; i<4; i++) {
        draw_lib.show_ready();
        timer.delay(400);
        draw_lib.update_screen();

        loaded_stage->showStage(); // to fix colorcycle colors
        loaded_stage->show_objects();
        loaded_stage->showAbove();
        draw_lib.update_screen();
        timer.delay(400);
    }
}


/*
void graphicsLib::show_ready()
{
    st_position dest_pos((RES_W/2)-26, (RES_H/2)-6);

    for (int i=0; i<4; i++) {
        copyArea(dest_pos, &ready_message, &gameScreen);
        updateScreen();
        timer.delay(400);

        updateScreen();
        timer.delay(400);
    }
}
*/

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::restart_stage()
{

    // remove any used teleporter
    players.at(0).set_teleporter(-1);
    _player_teleporter.active = false;

	_show_boss_hp = false;
	input.clean();
    loaded_stage->reset_current_map();
	// TODO - this must be on a single method in soundlib
	players.at(0).set_position(st_position(checkpoint.x, -TILESIZE));

    soundManager.stop_music();
    soundManager.unload_music();
    soundManager.load_stage_music(stage_data.bgmusic_filename);
	players.at(0).clean_projectiles();
	players.at(0).set_animation_type(ANIM_TYPE_TELEPORT);
    std::cout << ">> checkpoint..x: " << checkpoint.x << ", checkpoint.y: " << checkpoint.y << std::endl;

    if (checkpoint.y == -1) { // did not reached any checkpoint, use the calculated value from stage start
        // find teleport stop point
        int min_y = loaded_stage->get_teleport_minimal_y(95); // x = 80 + half a player width (30)
        players.at(0).set_teleport_minimal_y((min_y-3)*TILESIZE);
    } else {
        players.at(0).set_teleport_minimal_y(checkpoint.y-TILESIZE/2);
    }
    players.at(0).set_map(loaded_stage->get_current_map());
	players.at(0).reset_hp();
    loaded_stage->reset_stage_maps();
    loaded_stage->showStage();
    loaded_stage->showAbove();
	graphLib.set_screen_adjust(st_position(0, 0));
    //graphLib.set_colormap(currentStage);
    draw_lib.update_screen();
    soundManager.restart_music();
    show_ready();
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool game::showIntro()
{
#ifdef PSP
    std::cout << "showIntro::RAM::BFR='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif

    scenes.preloadScenes();
    scenes.intro();
	graphLib.set_colormap(-1);
	scenes.main_screen();
    scenes.unload_intro();
	currentStage = 0;

#ifdef PSP
    std::cout << "showIntro::RAM::AFT='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif


	initGame();
    //std::cout << "game::showIntro - game_save.stages[INTRO_STAGE]: " << game_save.stages[INTRO_STAGE] << std::endl;
	if (game_save.stages[INTRO_STAGE] == 0) {
		input.clean();
		start_stage();
	} else {
        currentStage = scenes.pick_stage();
        unload_stage();
        loaded_stage = new stage(currentStage, players);
        // show boss intro with stars, if needed
        soundManager.stop_music();
        if (game_save.stages[currentStage] == 0) {
            scenes.boss_intro(currentStage);
        }
		start_stage();
	}
    //std::cout << "game::showIntro - selected_stage.x: " << selected_stage.x << ", selected_stage.y: " << selected_stage.y << ", currentStage: " << currentStage << std::endl;

#ifdef PSP
    std::cout << "showIntro::RAM::STAGE_LOADED='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif


    return true;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void game::fps_count()
{
	fps_counter++;
	if (fps_timer <= timer.getTimer()) {
        sprintf(_fps_buffer, "fps: %d", fps_counter);
		fps_counter = 0;
		fps_timer = timer.getTimer()+1000;
	}
    if (fps_counter > 1) {
        std::string temp_str(_fps_buffer);
        graphLib.draw_text(10, 10, temp_str);
    }
}






// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool game::test_teleport(classPlayer *test_player) {


	if (players.at(0).get_anim_type() == ANIM_TYPE_TELEPORT) {
		return false;
	}


    stage *temp_stage = loaded_stage;
    int currentMap = loaded_stage->get_current_map_number();
	int temp_x, temp_y;
	int temp_map_n=0;
	int player_x = 0;
	int transition_type = 0;
	int lim1, lim2, lim3, lim4;
    int i=0;
	bool MUST_TELEPORT = false;
	int teleporter_dist = 0;
	int link_type = -1;

    int px = test_player->getPosition().x + (test_player->get_size().width*0.5);
    int py = test_player->getPosition().y + (test_player->get_size().height*0.5) + (test_player->get_size().height*0.25);

    int j = 0;
    for (j=0; j<STAGE_MAX_LINKS; j++) {
        if (stage_data.links[j].id_map_origin == -1 || stage_data.links[j].id_map_destiny == -1) {
            continue;
        }

        if (stage_data.links[j].id_map_origin != -1) {
            if (currentStage == SKULLCASTLE5 && _last_stage_used_teleporters.find(i) != _last_stage_used_teleporters.end()) {
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


            } else if (stage_data.links[j].id_map_destiny == currentMap && stage_data.links[j].bidirecional == true && stage_data.links[j].pos_destiny.x != -1) {
                temp_x = stage_data.links[j].pos_destiny.x;
                temp_y = stage_data.links[j].pos_destiny.y;
                temp_map_n = stage_data.links[j].id_map_origin;
                player_x = stage_data.links[j].pos_origin.x;
                if (stage_data.links[j].pos_origin.y < stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_TOP_TO_BOTTOM;
                } else if (stage_data.links[j].pos_origin.y > stage_data.links[j].pos_destiny.y) {
					transition_type = TRANSITION_BOTTOM_TO_TOP;
				}
			} else {
                i++;
				continue;
			}

			lim1 = temp_x*TILESIZE;
            lim2 = temp_x*TILESIZE + stage_data.links[j].size*TILESIZE;
			lim3 = (temp_y)*TILESIZE;
            lim4 = ((temp_y)*TILESIZE)+TILESIZE;

            // give extra pixels in the END-Y, if top to bottom ot bottom to top
            if (stage_data.links[j].type != LINK_TELEPORTER && stage_data.links[j].type != LINK_BOSS_TELEPORTER) {
                if (transition_type == TRANSITION_TOP_TO_BOTTOM) {
                    lim4 += TILESIZE;
                } else if (transition_type == TRANSITION_BOTTOM_TO_TOP) {
                    lim3 -= TILESIZE;
                }
            }

            //std::cout << "GAME::test_teleport - found a LINK for this map" << std::endl;

            //std::cout << "LINK[" << j << "] - px: " << px << ", lim1: " << lim1 << ", lim2: " << lim2 << ", py: " << py << ", lim3: " << lim3 << ", lim4: " << lim4 << std::endl;

			if ((px >= lim1 && px <= lim2) && ((py > lim3 && py < lim4))) {

				if (test_player->get_teleporter() == -1) {

					// for transition up/down, only execute if player is partially out of screen
                    if (stage_data.links[j].type != LINK_TELEPORTER && stage_data.links[j].type != LINK_BOSS_TELEPORTER && (transition_type == TRANSITION_TOP_TO_BOTTOM || transition_type == TRANSITION_BOTTOM_TO_TOP)) {
						short int p_posy = test_player->getPosition().y;
                        if (p_posy > 0 && p_posy+test_player->get_size().height < RES_H-4) {
                            //std::cout << "GAME::test_teleport - player position is not out-of-screen - t1: " << (p_posy+test_player->get_size().height) << " < t2: " << (RES_H-4) << std::endl;
                            i++;
							continue;
						}
					}
					teleporter_dist = lim1 - player_x*TILESIZE - 8;
					MUST_TELEPORT = true;
                    if (stage_data.links[j].type == LINK_BOSS_TELEPORTER && currentStage == SKULLCASTLE5 && currentMap == 2) {
                        test_player->set_direction(ANIM_DIRECTION_RIGHT);
                        test_player->set_teleporter(i);
                    } else if (stage_data.links[j].type != LINK_TELEPORTER) {
                        test_player->set_teleporter(i);
                    }
                    link_type = stage_data.links[j].type;
                    set_player_teleporter(i, st_position(players.at(0).getPosition().x, players.at(0).getPosition().y), false);
					break;
				}
			// only clean teleport when player is out of the teleporter
			} else {
                if (i == test_player->get_teleporter()) {
                    if (link_type != LINK_BOSS_TELEPORTER || currentStage != SKULLCASTLE5 || currentMap != 2) { // only clean link if not teleporter nor is on final stage/map
                        test_player->set_teleporter(-1);
                    }
				}
			}


		}
		i++;
	}

    if (!MUST_TELEPORT) {
        return false;
    }


    remove_all_projectiles();
    reset_beam_objects(); // beam/ray objects must be reset when changing maps
    // --------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // TODO - define according to the condition, try to maintain it
    //playerObj->sprite->anim_type = ANIM_STAIRS;
    // --------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int dest_x = (player_x*TILESIZE) - temp_stage->getMapScrolling().x;


    int max_pos_x = MAP_W*TILESIZE - RES_W;
    if (dest_x > max_pos_x) {
       dest_x = max_pos_x;
    }
    if (dest_x < 0) {
       dest_x = 0;
    }

    //std::cout << ">>>>>>>>>>>> TELEPORT - temp_map_n: " << temp_map_n << ", dest_x: " << dest_x << " <<<<<<<<<<<<" << std::endl;


    // must move the map, so that the dest position in screen is equal to player_real_pos_x
    int new_map_pos_x;
    //int playerRealXPos = test_player->getPosition().x + temp_stage->getMapScrolling().x;

    //new_map_pos_x = ((player_x*TILESIZE)) - playerRealXPos + teleporter_dist;
    //std::cout << "game::test_teleport - playerRealXPos: " << playerRealXPos << ", teleporter_dist: " << teleporter_dist << ", link.dest.x: " << player_x << std::endl;
    new_map_pos_x = temp_stage->getMapScrolling().x - teleporter_dist;
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

    //std::cout << "******************* game::test_teleport - teleporter_dist: " << teleporter_dist << ", new_map_pos_x: " << new_map_pos_x << std::endl;


    if (link_type == LINK_TELEPORTER || link_type == LINK_BOSS_TELEPORTER) {
        //std::cout << "<<<<<<<<<<<< TELEPORTER LINK >>>>>>>>>>>>>>" << std::endl;
        test_player->teleport_out();
        graphLib.blank_screen();
        draw_lib.update_screen();
        input.waitTime(500);
        currentMap = temp_map_n;
        new_map_pos_x = (stage_data.links[j].pos_destiny.x * TILESIZE) - TILESIZE*2;
    } else {
        //std::cout << "<<<<<<<<<<<< TRANSITION LINK >>>>>>>>>>>>>>" << std::endl;
        transitionScreen(transition_type, temp_map_n, new_map_pos_x, test_player);
    }


    set_current_map(temp_map_n);

    if (link_type == LINK_TELEPORTER || link_type == LINK_BOSS_TELEPORTER) {
        int new_scroll_pos = loaded_stage->get_first_lock_on_left(stage_data.links[j].pos_destiny.x);
        //std::cout << ">>>>>>>>>> teleporter.new_scroll_pos: " << new_scroll_pos << std::endl;
        loaded_stage->set_scrolling(st_position(new_scroll_pos, temp_stage->getMapScrolling().y));
        test_player->set_position(st_position(stage_data.links[j].pos_destiny.x*TILESIZE, 0));
    } else {
        loaded_stage->set_scrolling(st_position(new_map_pos_x, temp_stage->getMapScrolling().y));
        test_player->set_position(st_position(abs((float)test_player->get_real_position().x+new_map_pos_x), test_player->getPosition().y));
    }
    test_player->char_update_real_position();


    loaded_stage->get_current_map()->reset_scrolled();

    draw_lib.update_screen();

    return true;
}

void game::set_current_map(int temp_map_n)
{
	//std::cout << "game::set_current_map - temp_map_n: " << temp_map_n << std::endl;
    loaded_stage->set_current_map(temp_map_n);
    loaded_stage->reset_current_map_objects();
    players.at(0).set_map(loaded_stage->get_current_map());

    if (loaded_stage->get_current_map() != players.at(0).map) {
		std::cout << ">>>>>>>>>>>>>> ERROR" << std::endl;
        graphLib.show_debug_msg("EXIT #03");
		exit(-1);
	}

/*
loaded_stage->get_current_map() != players.at(0).map
*/
}

Uint8 game::get_current_map() const
{
    return loaded_stage->get_current_map_n();;
}


void game::map_present_boss(bool show_dialog)
{
    std::cout << "game::map_present_boss::START" << std::endl;
	is_showing_boss_intro = true;

    soundManager.stop_music();
    soundManager.unload_music();
    soundManager.play_boss_music();

    // 1. keep showing game screen until player reaches ground
	players.at(0).clear_move_commands();
	bool loop_run = true;
	while (loop_run == true) {
        //std::cout << "game::map_present_boss::LOOP #1" << std::endl;
        loaded_stage->showStage();
		players.at(0).charMove();
        bool hit_ground = players.at(0).hit_ground();
        int anim_type = players.at(0).get_anim_type();
        //std::cout << "game::map_present_boss - hit_ground: " << hit_ground << ", anim_type: " << anim_type << std::endl;
        if (hit_ground == true && anim_type == ANIM_TYPE_STAND) {
			loop_run = false;
		}
		players.at(0).show();
        loaded_stage->showAbove();
		input.waitTime(8);
        draw_lib.update_screen();
	}

	// 2. blink screen
	graphLib.blink_screen(255, 255, 255);

	// 3. move boss from top to ground
	loop_run = true;
	while (loop_run == true) {
        //std::cout << "game::map_present_boss::LOOP #2" << std::endl;
        //std::cout << ">> game::boss_intro - waiting for hit_ground... <<" << std::endl;
        if (loaded_stage->boss_hit_ground() == true) {
			loop_run = false;
		}
        show_stage(0, true);
	}
	show_stage(8, true);


	if (show_dialog == true) {
		// 4. show boss dialog
        if (strlen(stage_data.boss_dialog.line1[0]) > 0 && game_save.stages[currentStage] == 0) {
			dialogs boss_dialog;
            boss_dialog.show_boss_dialog();
            //std::cout << "PSP DEBUG - #A" << std::endl;
		}
        //std::cout << "PSP DEBUG - #B" << std::endl;
	}


	show_stage(8, false);

    std::cout << "game::map_present_boss::PASS #1" << std::endl;

    //std::cout << "PSP DEBUG - #1" << std::endl;
	fill_boss_hp_bar();
    //std::cout << "PSP DEBUG - #2" << std::endl;
	_show_boss_hp = true;
	is_showing_boss_intro = false;

    //std::cout << "PSP DEBUG - #3" << std::endl;
}

character *game::get_player()
{
    return (character*)&players.at(0);
}

object* game::get_player_platform()
{
	return players.at(0).get_platform();
}

void game::check_player_return_teleport()
{
	if (is_player_on_teleporter() == true) {
		std::cout << ">>>>>>> RETURN from teleporter <<<<<<<<<" << std::endl;
        finish_player_teleporter();
    }
}

bool game::must_show_boss_hp() const
{
	return _show_boss_hp;
}

void game::fill_boss_hp_bar() const
{
	//soundManager.play_repeated_sfx(SFX_GOT_ENERGY, 5);
	soundManager.play_timed_sfx(SFX_GOT_ENERGY, 60*PLAYER_INITIAL_HP+100);
	for (int i=0; i<PLAYER_INITIAL_HP; i++) {
		graphLib.draw_hp_bar(i, -1, -1);
        draw_lib.update_screen();
		timer.delay(60);
	}
    timer.delay(200);
}

void game::fill_player_weapon(short weapon_n)
{
    soundManager.play_timed_sfx(SFX_GOT_ENERGY, 60*PLAYER_INITIAL_HP+100);
    for (int i=0; i<PLAYER_INITIAL_HP; i++) {
        graphLib.draw_hp_bar(i, players.at(0).get_number(), weapon_n);
        draw_lib.update_screen();
        timer.delay(60);
    }
    timer.delay(200);
}



void game::reset_stage_maps() const
{
    loaded_stage->reset_stage_maps();
}




// ********************************************************************************************** //
// remove the projectiles from the list of all players and npcs                                   //
// ********************************************************************************************** //
void game::remove_all_projectiles()
{
	std::vector<classPlayer>::iterator p_it;
	for (p_it=players.begin(); p_it!=players.end(); p_it++) {
		(*p_it).clean_projectiles();
	}
    loaded_stage->get_current_map()->clean_map_npcs_projectiles();
}

void game::reset_beam_objects()
{
    loaded_stage->get_current_map()->reset_beam_objects();
}

void game::remove_players_slide()
{
    std::vector<classPlayer>::iterator p_it;
    for (p_it=players.begin(); p_it!=players.end(); p_it++) {
        (*p_it).cancel_slide();
    }
}


//TRANSITION_TOP_TO_BOTTOM, TRANSITION_BOTTOM_TO_TOP
void game::transitionScreen(Uint8 type, Uint8 map_n, short int adjust_x, classPlayer *pObj) {
    timer.pause();
	graphicsLib_gSurface temp_screen;
    short i = 0;
    stage* temp_stage = loaded_stage;
    graphLib.initSurface(st_size(RES_W, RES_H*2), &temp_screen);
    classMap* temp_map = temp_stage->maps[map_n];
    temp_map->set_bg1_scroll(loaded_stage->get_current_map()->get_bg1_scroll());
    temp_map->set_bg2_scroll(loaded_stage->get_current_map()->get_bg2_scroll());
    temp_map->set_scrolling(st_position(adjust_x, 0));

    /*
    if (type == TRANSITION_TOP_TO_BOTTOM) {
        temp_map->draw_dynamic_backgrounds_into_surface(temp_screen, RES_H);
    } else {
        temp_map->draw_dynamic_backgrounds_into_surface(temp_screen, 0);
    }
    */
    graphLib.copyArea(st_rectangle(0, i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);

	// draw map in the screen, erasing all players/objects/npcs
    loaded_stage->showStage();

	//std::cout << "transitionScreen::START - p.x: " << pObj->getPosition().x << ", p.y: " << pObj->getPosition().y << std::endl;

    // draw the offscreen with the new loaded map

	if (type == TRANSITION_TOP_TO_BOTTOM || type == TRANSITION_BOTTOM_TO_TOP) {
        // copy current screen to temp
		if (type == TRANSITION_TOP_TO_BOTTOM) {
            graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &temp_screen);
		} else if (type == TRANSITION_BOTTOM_TO_TOP) {
            graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, RES_H), &temp_screen);
        }
		// copy the new screen to the temp_area
        graphicsLib_gSurface temp_map_area = temp_map->get_map_area_surface();
		if (type == TRANSITION_TOP_TO_BOTTOM) {
            graphLib.copy_area_with_colormap_update(st_rectangle(0, 0, RES_W, RES_H), st_position(0, RES_H), &temp_map_area, &temp_screen);
		} else if (type == TRANSITION_BOTTOM_TO_TOP) {
            graphLib.copy_area_with_colormap_update(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &temp_map_area, &temp_screen);
		}

        graphLib.set_colormap_original(&temp_screen);
        graphLib.set_colormap_current(&temp_screen);


		// now, show the transition
        short int extra_y = 0;
		for (i=0; i<(RES_H+TILESIZE*0.5)/TRANSITION_STEP; i++) {
            graphLib.set_colormap_current(&temp_screen);
            if (type == TRANSITION_TOP_TO_BOTTOM) {
                graphLib.copyArea(st_rectangle(0, i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);
            } else if (type == TRANSITION_BOTTOM_TO_TOP) {
                graphLib.copyArea(st_rectangle(0, RES_H-i*TRANSITION_STEP, RES_W, RES_H), st_position(0, 0), &temp_screen, &graphLib.gameScreen);
			}

			if (i % 2 == 0) {
				extra_y = 1;
			} else {
				extra_y = 0;
			}

			if (type == TRANSITION_TOP_TO_BOTTOM) {
				if (pObj->getPosition().y > 6) {
					pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y - TRANSITION_STEP + extra_y));
				}
			} else if (type == TRANSITION_BOTTOM_TO_TOP) {
				if (pObj->getPosition().y < RES_H-TILESIZE*2) {
					pObj->set_position(st_position(pObj->getPosition().x, pObj->getPosition().y + TRANSITION_STEP - extra_y));
				}
			}
			pObj->char_update_real_position();
			pObj->show();

			if (type == TRANSITION_TOP_TO_BOTTOM) {
                loaded_stage->showAbove(-i*TRANSITION_STEP);
                loaded_stage->get_current_map()->show_objects(-i*TRANSITION_STEP);
                int temp_map_3rdlevel_pos = (RES_H+TILESIZE*0.5) - i*TRANSITION_STEP - 8;
                temp_map->show_objects(temp_map_3rdlevel_pos);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			} else {
                loaded_stage->showAbove(i*TRANSITION_STEP);
                loaded_stage->get_current_map()->show_objects(i*TRANSITION_STEP);
                int temp_map_3rdlevel_pos = -(RES_H+TILESIZE*0.5) + i*TRANSITION_STEP + 8; // 8 is a adjust for some error I don't know the reason
                temp_map->show_objects(temp_map_3rdlevel_pos);
                temp_map->showAbove(temp_map_3rdlevel_pos, adjust_x);
			}


            draw_lib.update_screen();
#if !defined(PLAYSTATION2) && !defined(ANDROID)
            input.waitTime(8);
#endif
		}
        if (type == TRANSITION_TOP_TO_BOTTOM) {
            temp_map->changeScrolling(st_position(temp_map->getMapScrolling().x, 0));
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
    timer.unpause();

    //std::cout << "transitionScreen::END" << std::endl;

}


void game::horizontal_screen_move(short direction, bool is_door, short tileX, short tileY)
{
    int i = 0;
    int upTile = 0;
    int downTile = 0;
    classMap* temp_map = loaded_stage->get_current_map();
	st_position scroll_move;
	if (direction == ANIM_DIRECTION_LEFT) {
		scroll_move.x = -2;
	} else {
		scroll_move.x = 2;
	}

    timer.pause();
	if (is_door == true) {
        remove_all_projectiles();
        remove_players_slide();
        /// @TODO - interrupt slides, charged shots, etc

        // if there is a subboss alive, near left, you can't open
        if (subboss_alive_on_left(tileX) == true) {
            std::cout << "Oh não! Porta não pode ser aberta porque existe um sub-boss vivo à esquerda dela!" << std::endl;
            return;
        }

        loaded_stage->showStage();
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
        loaded_stage->redraw_boss_door(false, (downTile-upTile+1), tileX, tileY, players.at(0).get_number());//bool is_close, int nTiles, int tileX
	}


	int move_limit = (RES_W/abs((float)scroll_move.x))-TILESIZE/abs((float)scroll_move.x);
	//int move_limit = (RES_W/abs((float)scroll_move.x));
	for (int i=0; i<move_limit; i++) {
        loaded_stage->changeScrolling(scroll_move, false);
        loaded_stage->showStage();
        loaded_stage->show_npcs();
		players.at(0).show();
        loaded_stage->showAbove();
#if defined(PC)
        input.waitTime(3);
#endif
        draw_lib.update_screen();
		if (i%(TILESIZE/2) == 0) {
			players.at(0).set_position(st_position(players.at(0).getPosition().x+scroll_move.x, players.at(0).getPosition().y));
			players.at(0).char_update_real_position();
		}
	}
    if (is_door == true) {
        soundManager.play_sfx(SFX_DOOR_OPEN);
        loaded_stage->redraw_boss_door(true, (downTile-upTile+1), tileX, tileY, players.at(0).get_number());
        players.at(0).reset_charging_shot();
        players.at(0).cancel_slide();
    }
    std::cout << "horizontal_screen_move END" << std::endl;
	input.waitTime(6);
    timer.unpause();
    loaded_stage->showStage();
}




void game::got_weapon()
{
    bool must_show_got_weapon = false;
	if (game_save.stages[currentStage] == 0) {
        must_show_got_weapon = true;
		game_save.finished_stages++;
    }
    invencible_old_value = GAME_FLAGS[FLAG_INVENCIBLE]; // store old value in order to not set the flag to false if it is on my command-line parameter
    //std::cout << "**** SAVE#1::invencible_old_value: " << invencible_old_value << ", GAME_FLAGS[FLAG_INVENCIBLE]: " << GAME_FLAGS[FLAG_INVENCIBLE] << std::endl;
    GAME_FLAGS[FLAG_INVENCIBLE] = true;
    //std::cout << "game::got_weapon - npc_name: " << npc_name << std::endl;


    // remove any projectiles, charged shots, slides, etc
    players.at(0).clean_projectiles();
    players.at(0).clear_move_commands();

    if (must_show_got_weapon == true && currentStage != 0 && currentStage <= 8) {
		// check witch is the boss that was killed
		//if (npc_name ==)

		/// @TODO: save game

		/// @TODO: teletransport if capsules

		// fall to ground
		players.at(0).fall();


		/// @TODO: walk to boss room middle, if inside a boss room (must create a way to check this), otherwise, just jump up
		walk_character_to_screen_point_x(&players.at(0), RES_W/2);

		// jump up, implosion, jump down and teleport out of the screen
		players.at(0).execute_jump_up();
        draw_implosion(players.at(0).get_real_position().x+players.at(0).get_size().width/2-6, players.at(0).get_real_position().y+players.at(0).get_size().height/2-4);
        players.at(0).show();
        draw_lib.update_screen();

        players.at(0).set_weapon(currentStage);
        fill_player_weapon(players.at(0).get_selected_weapon());
		players.at(0).fall();
		soundManager.play_sfx(SFX_GOT_WEAPON);
		input.waitTime(5000);

        // fall to ground
        players.at(0).fall();
        input.waitTime(500);


		soundManager.play_sfx(SFX_TELEPORT);
		players.at(0).teleport_out();
        players.at(0).set_show_hp(false);
        input.waitTime(1000);

		/// @TODO
		// show the "you got" screen
		graphLib.blank_screen();
        soundManager.load_music("got_weapon.mod");
		graphLib.blink_screen(255, 255, 255);
		graphLib.blank_screen();
        soundManager.play_music();
		graphicsLib_gSurface temp_bg;
        graphLib.surfaceFromFile(FILEPATH+"images/backgrounds/stage_boss_intro.png", &temp_bg);
		graphLib.showSurface(&temp_bg);

        players.at(0).set_position(st_position(20, (RES_H * 0.5 - players.at(0).get_size().height/2)));
		players.at(0).set_animation_type(ANIM_TYPE_ATTACK);
        loaded_stage->set_scrolling(st_position(0, 0));
		players.at(0).char_update_real_position();
		players.at(0).show();

        graphLib.start_stars_animation();

        std::string weapon_name(game_data.weapons[currentStage].name);
		for (std::string::iterator p = weapon_name.begin(); weapon_name.end() != p; ++p) {
			*p = toupper(*p);
		}

		std::string phrase = std::string("YOU GOT ") + weapon_name;

		graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 - 4), phrase, false);
		std::string extra_name = "";
        if (currentStage == COIL_GOT_STAGE) {
			extra_name = "AND FROG COIL ADAPTOR";
        } else if (currentStage == JET_GOT_STAGE) {
			extra_name = "AND EAGLE JET ADAPTOR";
		}
		graphLib.draw_progressive_text((RES_W * 0.5 - 90), (RES_H * 0.5 + 8), extra_name, false);

		players.at(0).show();

        graphLib.wait_and_update_screen(5000);
        graphLib.stop_stars_animation();
    } else {
		players.at(0).teleport_out();
	}
    players.at(0).set_show_hp(true);

    game_save.stages[currentStage] = 1;

    leave_stage();
}

void game::leave_stage()
{
    if (fio.write_save(game_save) == false) {
        show_savegame_error();
    }

    graphLib.reset_tileset_colormap();

    //std::cout << "[[[freeze_weapon_effect(RESET #2)]]]" << std::endl;
    draw_lib.set_flash_enabled(false);
    freeze_weapon_effect = FREEZE_EFFECT_NONE;
    GAME_FLAGS[FLAG_INVENCIBLE] = invencible_old_value;
    //std::cout << "**** LOAD::invencible_old_value: " << invencible_old_value << ", GAME_FLAGS[FLAG_INVENCIBLE]: " << GAME_FLAGS[FLAG_INVENCIBLE] << std::endl;

    // show password
    input.clean();
    timer.delay(200);
    scenes.show_password();


    // return to stage selection
    players.at(0).reset_charging_shot();
    players.at(0).set_weapon(WEAPON_DEFAULT);
    currentStage = scenes.pick_stage();
    unload_stage();
    loaded_stage = new stage(currentStage, players);
    // show boss intro with stars, if needed
    soundManager.stop_music();
    if (game_save.stages[currentStage] == 0) {
        scenes.boss_intro(currentStage);
    }
    checkpoint.map = 0;
    checkpoint.map_scroll_x = 0;
    checkpoint.x = 50;
    checkpoint.y = -1;
    start_stage();
}

void game::exit_game()
{
    //std::cout << ">>> LEAVEGAME <<<" <<std::endl;

    dialogs dialogs_obj;
    if (dialogs_obj.show_leave_game_dialog() != true) { // cuidar se isso não cria loophole
        //std::cout << "++ DON'T LEAVE GAME" << std::endl;
        return;
    }

    //std::cout << "++ YES, LEAVE GAME" << std::endl;
    if (fio.write_save(game_save) == false) {
        std::cout << "*** Show savegame Error" << std::endl;
        show_savegame_error();
    }

#ifdef PSP
    sceKernelExitGame();
#else
    SDL_Quit();
#endif
    exit(0);
}

void game::game_over()
{
    _last_stage_used_teleporters.clear();

    input.waitTime(200);
    input.clean();
    soundManager.load_music("game_over.mod");
    soundManager.play_music();
    graphLib.blank_screen();

    graphicsLib_gSurface password_screen;
    std::string filename = FILEPATH + "images/backgrounds/config.png";
    graphLib.surfaceFromFile(filename, &password_screen);
    graphLib.copyArea(st_rectangle(0, 0, password_screen.gSurface->w, password_screen.gSurface->h), st_position(0, 0), &password_screen, &graphLib.gameScreen);

    graphicsLib_gSurface dialog_img;
    filename = FILEPATH + "images/backgrounds/dialog.png";
    graphLib.surfaceFromFile(filename, &dialog_img);
    graphLib.copyArea(st_rectangle(0, 0, dialog_img.gSurface->w, dialog_img.gSurface->h), st_position(RES_W/2-dialog_img.gSurface->w/2, RES_H/2-dialog_img.gSurface->h/2), &dialog_img, &graphLib.gameScreen);

    graphLib.draw_centered_text(RES_H/2-6, "GAME OVER", graphLib.gameScreen, st_color(235, 235, 235));

    draw_lib.update_screen();
    input.waitTime(400);
    input.wait_keypress();
    if (currentStage != INTRO_STAGE) {
        leave_stage();
    } else {
        scenes.show_password();
        soundManager.stop_music();
        soundManager.load_stage_music(stage_data.bgmusic_filename);
        soundManager.play_music();
        restart_stage();
    }
}

void game::show_ending(st_position boss_pos)
{
    // save the data indicating game was finished, so user can see ending later or get access to more features
    game_config.game_finished = true;
    fio.save_config(game_config);

    players.at(0).set_show_hp(false);
    // reset player colors to original
    players.at(0).set_weapon(0);
	ending game_ending;
    game_ending.set_boss_pos(boss_pos);
	game_ending.start();
    leave_game = true;
}

void game::quick_load_game()
{
    if (fio.save_exists()) {
        fio.read_save(game_save);
    }
    currentStage = DAISIEBOT;
    game_save.selected_player = PLAYER_ROCKBOT;
    if (GAME_FLAGS[FLAG_PLAYER_ROCKBOT]) {
        game_save.selected_player = PLAYER_ROCKBOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_BETABOT]) {
        game_save.selected_player = PLAYER_BETABOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_CANDYBOT]) {
        game_save.selected_player = PLAYER_CANDYBOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_KITTYBOT]) {
        game_save.selected_player = PLAYER_KITTYBOT;
    }
    //std::cout << ">> game_save.selected_player: " << game_save.selected_player << std::endl;
    //std::cout << "p4.arms: " << game_data.armor_pieces[3].special_ability[ARMOR_ARMS] << std::endl;


    scenes.preloadScenes();
	initGame();
    start_stage();
}

void game::update_stage_scrolling()
{
    loaded_stage->changeScrolling(checkScrolling());
}



void game::draw_explosion(short int centerX, short int centerY, bool show_players) {
	unsigned int timerInit;
	int distance=0, mode=0;
	int accel=1;

	timerInit = timer.getTimer();

/*
    if (show_players) {
		players.at(0).set_animation_type(ANIM_TYPE_STAND);
	}
*/


    draw_lib.update_screen();
	soundManager.stop_music();
	soundManager.play_sfx(SFX_PLAYER_DEATH);
	// x = a + r * cos(t)
	// y = b + r * sin(t)
	while (timer.getTimer() < timerInit+2000) {
        loaded_stage->showStage();
        if (show_players) {
            players.at(0).show();
        }
        loaded_stage->showAbove();
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
		input.waitTime(10);
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
		//std::cout << "distance: " << distance << std::endl;
        loaded_stage->showStage();
		players.at(0).show();
        loaded_stage->showAbove();
        //loaded_stage->showAbove();
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
        input.waitTime(15);
	}
}

void game::show_player(Uint8 player_n)
{
	players.at(player_n).show();
}

void game::set_player_position(st_position pos, Uint8 player_n)
{
	players.at(player_n).set_position(pos);
    players.at(player_n).char_update_real_position();
}

void game::change_player_position(short xinc, short yinc, short player_n)
{
	players.at(player_n).change_position(xinc, yinc);
	players.at(player_n).char_update_real_position();
}

void game::set_player_anim_type(ANIM_TYPE anim_type, Uint8 player_n)
{
    players.at(player_n).set_animation_type(anim_type);
}

st_position game::get_player_position(Uint8 player_n)
{
    return st_position(players.at(player_n).getPosition().x, players.at(player_n).getPosition().y);
}

st_size game::get_player_size(Uint8 player_n)
{
	return players.at(player_n).get_size();
}

void game::set_player_direction(Uint8 direction, Uint8 player_n)
{
	players.at(player_n).set_direction(direction);
}

void game::player_fall(Uint8 player_n)
{
	players.at(player_n).fall();
}

void game::walk_character_to_screen_point_x(character *char_obj, short pos_x)
{
	/// @TODO: jump obstacles
	if (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
        char_obj->set_animation_type(ANIM_TYPE_WALK);
		char_obj->set_direction(ANIM_DIRECTION_LEFT);
		while (char_obj->get_real_position().x+char_obj->get_size().width/2 > pos_x) {
			char_obj->set_position(st_position(char_obj->getPosition().x-2, char_obj->getPosition().y));
            loaded_stage->showStage();
            loaded_stage->showAbove();
            loaded_stage->show_npcs();
			players.at(0).show();
            draw_lib.update_screen();
			timer.delay(20);
		}
	} else if (char_obj->get_real_position().x+char_obj->get_size().width/2 < pos_x) {
		char_obj->set_direction(ANIM_DIRECTION_RIGHT);
        char_obj->set_animation_type(ANIM_TYPE_WALK);
		while (char_obj->get_real_position().x+char_obj->get_size().width/2 < pos_x) {
			char_obj->set_position(st_position(char_obj->getPosition().x+2, char_obj->getPosition().y));
            loaded_stage->showStage();
            loaded_stage->showAbove();
            loaded_stage->show_npcs();
			players.at(0).show();
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
	//std::cout << "----->> set_player_teleporter - x: " << _player_teleporter.old_player_pos.x << ", y: " << _player_teleporter.old_player_pos.y << std::endl;
	_player_teleporter.active = true;
	_player_teleporter.finished = false;
    //std::cout << "game::set_player_teleporter - scroll: " << loaded_stage->getMapScrolling().x << std::endl;
    _player_teleporter.old_map_scroll = loaded_stage->getMapScrolling();
    _player_teleporter.old_map_n = loaded_stage->get_current_map_n();
}

bool game::is_player_on_teleporter() const
{
    return _player_teleporter.active;
}

void game::remove_current_teleporter_from_list()
{
    if (_player_teleporter.teleporter_n != -1) {
        _last_stage_used_teleporters.erase(_player_teleporter.teleporter_n);
    }
    players.at(0).set_teleporter(-1);
}

std::string game::select_game_screen()
{
    std::vector<std::string> game_list = fio.read_game_list();
    if (game_list.size() < 2) {
        return std::string("data/");
    }
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.draw_text(10, 10, "- Select Game -");

    option_picker main_picker(false, st_position(40, 50), game_list, false);
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
    input.waitTime(200);

    std::string game_dir = std::string("/data/");
    if (picked_n != 0) {
        game_dir = std::string("/games/") + game_list.at(picked_n) + std::string("/");
    }

    return std::string(game_dir);
}

void game::finish_player_teleporter()
{
    remove_all_projectiles();
    remove_players_slide();
    players.at(0).recharge(ENERGY_TYPE_HP, ENERGY_ITEM_BIG);
    players.at(0).teleport_out();
    timer.delay(1000);

    _player_teleporter.active = false;
    _last_stage_used_teleporters.insert(pair<int,bool>(_player_teleporter.teleporter_n, true));
	// teleport out
	soundManager.play_sfx(SFX_TELEPORT);
	players.at(0).teleport_out();
    _player_teleporter.old_player_pos.y -= 5;
    players.at(0).set_position(_player_teleporter.old_player_pos);
    loaded_stage->set_current_map(_player_teleporter.old_map_n);
    if (_last_stage_used_teleporters.size() == 8) {
        // search for the final-boss teleporter capsule and start it
        loaded_stage->activate_final_boss_teleporter();
    }
    players.at(0).set_map(loaded_stage->get_current_map());
    //std::cout << "game::finish_player_teleporter - scroll.x: " << _player_teleporter.old_map_scroll.x << ", scroll.y: " << _player_teleporter.old_map_scroll.y << std::endl;
    loaded_stage->set_scrolling(st_position(_player_teleporter.old_map_scroll));
    players.at(0).set_animation_type(ANIM_TYPE_STAND);
    if (_player_teleporter.is_object == true) {
        loaded_stage->get_current_map()->finish_object_teleporter(_player_teleporter.teleporter_n);
    }
    players.at(0).set_teleporter(-1);
    soundManager.stop_music();
    soundManager.load_stage_music(stage_data.bgmusic_filename);
    soundManager.play_music();
}

void game::show_stage(int wait_time, bool move_npcs)
{
    if (_dark_mode == false) {
        loaded_stage->showStage();
    }
	if (move_npcs == true) {
        loaded_stage->move_npcs();
	}
    if (_dark_mode == false) {
        loaded_stage->show_npcs();
        players.at(0).show();
        loaded_stage->showAbove();
    }
	if (wait_time > 0) {
		input.waitTime(wait_time);
	}
    draw_lib.update_screen();
}

bool game::subboss_alive_on_left(short tileX)
{
    return loaded_stage->subboss_alive_on_left(tileX);
}

void game::object_teleport_boss(st_position dest_pos, Uint8 dest_map, Uint8 teleporter_id)
{

    //std::cout << "############# teleporter_id: " << teleporter_id << std::endl;

    // checa se já foi usado
    if (_last_stage_used_teleporters.find(teleporter_id) != _last_stage_used_teleporters.end()) {
        //std::cout << "[TELEPORT-OBJ] teleporter_id [" << teleporter_id << "] already used" << std::endl;
        return;
    }
    set_player_teleporter(teleporter_id, st_position(players.at(0).getPosition().x, players.at(0).getPosition().y), true);
    classPlayer* test_player = &players.at(0);
    //std::cout << "<<<<<<<<<<<< TELEPORTER LINK >>>>>>>>>>>>>>" << std::endl;
    test_player->teleport_out();
    graphLib.blank_screen();
    draw_lib.update_screen();
    input.waitTime(500);

    set_current_map(dest_map);

    int new_scroll_pos = loaded_stage->get_first_lock_on_left(dest_pos.x);
    //std::cout << ">>>>>>>>>> teleporter.new_scroll_pos: " << new_scroll_pos << std::endl;
    loaded_stage->set_scrolling(st_position(new_scroll_pos, 0));
    test_player->set_position(st_position(dest_pos.x*TILESIZE, 0));
    test_player->char_update_real_position();

    loaded_stage->get_current_map()->reset_scrolled();

    draw_lib.update_screen();
}



bool game::show_config(short finished_stage)
{
    if (scenes.show_main_config(finished_stage) == 1) {
        input.clean();
        input.waitTime(50);
        config_manager.disable_ingame_menu();
        leave_stage();
        return true;
    }
    return false;
}

void game::unload_stage()
{
    if (loaded_stage == NULL) {
        return;
    }
/*
#ifdef PSP
    std::cout << "unload_stage::RAM::BFR='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
*/
    delete loaded_stage;
    loaded_stage = NULL;
/*
#ifdef PSP
    std::cout << "unload_stage::RAM::AFT='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
*/
}

void game::show_savegame_error()
{
    std::vector<std::string> msgs;
    msgs.push_back("ERROR WHILE SAVING GAME,");
    msgs.push_back("PLEASE CHECK THAT THE DEVICE OR");
    msgs.push_back("FILE IS NOT WRITE-PROTECTED.");
    draw_lib.show_ingame_warning(msgs);
}

void game::get_drop_item_ids()
{
    for (int i=0; i<DROP_ITEM_COUNT; i++) {
        _drop_item_list[i] = -1;
    }
    //std::cout << "**** game::get_drop_item_ids::START ******" << std::endl;
    for (int i=0; i<GAME_MAX_OBJS; i++) {
        //std::cout << "**** game::get_drop_item_ids OBJ_ENERGY_PILL_BIG: " << OBJ_ENERGY_PILL_BIG << ", i: " << i << ", obj.name: '" << game_data.objects[i].name << "'', obj.type: " << game_data.objects[i].type << std::endl;
        if (game_data.objects[i].type == OBJ_LIFE) {
            //std::cout << "**** game::get_drop_item_ids DROP_ITEM_1UP: " << i << " ******" << std::endl;
            _drop_item_list[DROP_ITEM_1UP] = i;
        } else if (game_data.objects[i].type == OBJ_ENERGY_PILL_SMALL) {
            //std::cout << "**** game::get_drop_item_ids OBJ_ENERGY_PILL_SMALL: " << i << " ******" << std::endl;
            _drop_item_list[DROP_ITEM_ENERGY_SMALL] = i;
        } else if (game_data.objects[i].type == OBJ_ENERGY_PILL_BIG) {
            //std::cout << "**** game::get_drop_item_ids OBJ_ENERGY_PILL_BIG: " << i << " ******" << std::endl;
            _drop_item_list[DROP_ITEM_ENERGY_BIG] = i;
        } else if (game_data.objects[i].type == OBJ_WEAPON_PILL_SMALL) {
            //std::cout << "**** game::get_drop_item_ids OBJ_WEAPON_PILL_SMALL: " << i << " ******" << std::endl;
            _drop_item_list[DROP_ITEM_WEAPON_SMALL] = i;
        } else if (game_data.objects[i].type == OBJ_WEAPON_PILL_BIG) {
            //std::cout << "**** game::get_drop_item_ids OBJ_WEAPON_PILL_BIG: " << i << " ******" << std::endl;
            _drop_item_list[DROP_ITEM_WEAPON_BIG] = i;
        }
    }
}

classPlayer *game::get_player(Uint8 n)
{
    return &players.at(n);
}

st_position game::get_current_stage_scroll() const
{
    return loaded_stage->getMapScrolling();
}

void game::reset_scroll() const
{
    loaded_stage->reset_scrolling();
}

short game::get_drop_item_id(short type) const
{
    return _drop_item_list[type];
}

void game::show_map() const
{
    loaded_stage->showStage();
    loaded_stage->showAbove();
    draw_lib.update_screen();
}



void game::showGotArmorDialog(e_ARMOR_PIECES armor_type)
{
    game_dialogs.showGotArmorDialog(armor_type);
}
