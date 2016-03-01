#include "character.h"
#include "game.h"
#include "timerlib.h"

extern game gameControl;
#include "timerlib.h"
extern timerLib timer;

#include "soundlib.h"
extern soundLib soundManager;

#include "inputlib.h"
extern inputLib input;

#include "game_mediator.h"

extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;

extern bool GAME_FLAGS[FLAG_COUNT];

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern int freeze_weapon_id;


// initialize static member
std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]> character::character_graphics_list;
std::map<std::string, graphicsLib_gSurface> character::_character_graphics_background_list;
static std::map<std::string, graphicsLib_gSurface> _character_frames_surface;

// init character with default values
// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
character::character() : map(NULL), hitPoints(1, 1), last_hit_time(0), is_player_type(false), _platform(NULL), hit_animation_timer(0), hit_moved_back_n(0), jump_button_released(true), attack_button_released(true), dead(false), charging_color_n(0), charging_color_timer(0), shield_type(0), _moving_platform_timer(0), position(), _number(0), _super_jump(false), _force_jump(false), _teleport_minimal_y(0), _projectile_type_id(-1), _is_falling(false), _dead_state(0), slide_type(0), _water_splash(false), _has_background(false), hit_duration(300), _is_boss(false), _is_stage_boss(false)
{
    _was_animation_reset = false;
    move_speed = 2.0;

    accel_speed_y = 1;
    gravity_y = 0.25;
	position.y = 0;
	position.x = 0;
    can_fly = false;
	attack_state = ATTACK_NOT;
	max_projectiles = 1;
    _debug_char_name = "Bat";
    _frame_pos_adjust.x = 0;
    _frame_pos_adjust.y = 0;
    _stairs_stopped_count = 0;
    _charged_shot_projectile_id = -1;
    _is_last_frame = false;
    _simultaneous_shots = 1;
    _ignore_gravity = false;
    _show_hp = true;
    _always_move_ahead = false;
    _was_hit = false;
    _progressive_appear_pos = 0;
    last_execute_time = 0;
    _check_always_move_ahead = false;
    _dropped_from_stairs = false;
    _jumps_number = 1;
    _dash_button_released = true;
    _damage_modifier = 0;
    _hit_ground = false;
    _dashed_jump = false;
    _can_execute_airdash = true;
    _player_must_reset_colors = false;
    hit_animation_count = 0;
    _attack_frame_n = -1;
    _is_attack_frame = false;
}


// init character with default values
// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
character::~character()
{
	clean_projectiles();
	/// @TODO free map _character_frames_surface
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::char_update_real_position() {
	if (map != NULL) {
        realPosition.x = position.x - (int)map->getMapScrolling().x;
        realPosition.y = position.y - (int)map->getMapScrolling().y;
        //std::cout << ">>>> show::char_update_real_position - realPosition.y: " << realPosition.y << ", pos.y: " << position.y << ", map->getMapScrolling().y: " << map->getMapScrolling().y << std::endl;
    } else {
		realPosition.x = position.x;
		realPosition.y = position.y;
	}
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::charMove() {
    int mapLock = 0;
	bool moved = false;
    float temp_move_speed = move_speed;

    if (timer.is_paused() == true) {
        return;
    }


    bool did_hit_ground = hit_ground();

    if (_dashed_jump == true) {
        if (state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) {
            temp_move_speed = move_speed*2;
            if (did_hit_ground == true) {
                _dashed_jump = false;
            }
        }
    }
    if (did_hit_ground == true) {
        _can_execute_airdash = true;
    }

    //if (is_player()) std::cout << "CHAR::CHARMOVE - _fractional_move_speed: " << _fractional_move_speed << std::endl;


	if (!map) {
		return; // error - can't execute this action without an associated map
	}

    int water_lock = map->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y+6)/TILESIZE));
    if (is_player() == true && water_lock == TERRAIN_WATER) {
        map->add_bubble_animation(st_position(realPosition.x+frameSize.width/2, position.y+6));
    }

    int map_point_x = (position.x+frameSize.width/2)/TILESIZE;
    int map_point_y = (position.y+frameSize.height)/TILESIZE;
    int bottom_point_lock = map->getMapPointLock(st_position(map_point_x, map_point_y));
    //std::cout << "map.x: " << map_point_x << ", map.y: " << map_point_y << ", bottom_point_lock: " << bottom_point_lock << std::endl;


	if (state.frozen == true) {
        std::cout << "CHAR::charMove - timer: " << timer.getTimer() << ", frozen_timer: " << state.frozen_timer << std::endl;
		if (state.frozen_timer < timer.getTimer()) {
			state.frozen = false;
		} else { // if is player, check collision agains NPCs to leave freeze
			if (is_player()) {
                std::cout << "CHAR::charMove::frozen - check collision (is_player)" << std::endl;
				int diff_w = 0;
				int diff_h = 0;
				if (frameSize.width > 21) {
                    diff_w = abs((float)frameSize.height-21);
				}
                if (state.animation_type == ANIM_TYPE_SLIDE) {
                    if (slide_type == 1) {
                        diff_h = abs((float)TILESIZE-1-frameSize.height);
                    } else {
                        diff_h = abs((float)TILESIZE/2-1-frameSize.height);
                    }
                 }
                std::cout << "colision_player_npcs #1" << std::endl;
				short int res_colision_npc = map->colision_player_npcs(this, 0, 0, diff_w, diff_h);
				if (res_colision_npc == 1) {
                    damage(TOUCH_DAMAGE_SMALL, false);
				} else if (res_colision_npc == 2) {
                    damage(TOUCH_DAMAGE_BIG, false);
				}
			}
		}
		return;
	}

	if (state.animation_type == ANIM_TYPE_TELEPORT) {
		gravity(false);
		return;
	}

    if (state.animation_type == ANIM_TYPE_HIT) {

        //if (is_player()) std::cout << "hit_moved_back_n: " << hit_moved_back_n << ", get_hit_push_back_n(): " << get_hit_push_back_n() << std::endl;

        if (hit_moved_back_n < get_hit_push_back_n()) {
            //std::cout << ">>>>>>>>>>>>> ANIM_TYPE_HIT::PUSHBACK #2" << std::endl;
            if (state.direction == ANIM_DIRECTION_LEFT) {
                moveCommands.left = 0;
                moveCommands.right = 1;
            } else {
                moveCommands.left = 1;
                moveCommands.right = 0;
            }
            temp_move_speed = 0.5;
        } else {
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #Z" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
			hit_moved_back_n = 0;
		}
	}

	if (moveCommands.left == 1 && position.x > 0 && state.animation_type != ANIM_TYPE_SLIDE && is_in_stairs_frame() == false) {


        for (float i=temp_move_speed; i>=0.1; i--) {
            st_map_colision map_col = map_colision(-i, 0, map->getMapScrolling());
            mapLock = map_col.block;
            if (state.animation_type == ANIM_TYPE_HIT && hit_ground() == true) {
                hit_moved_back_n += temp_move_speed;
            }
            if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                if (mapLock == BLOCK_UNBLOCKED || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1) || mapLock == BLOCK_Y) {
					position.x -= i + map->get_last_scrolled().x;
				} else {
					position.x -= i*WATER_SPEED_MULT + map->get_last_scrolled().x;
				}
				if (state.animation_type != ANIM_TYPE_HIT) {
					state.direction = ANIM_DIRECTION_LEFT;
				} else {
                    gravity(false);
                    return;
				}
				if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_WALK_ATTACK) {
					state.animation_timer = 0;
				}
                if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_JUMP && state.animation_type != ANIM_TYPE_SLIDE && state.animation_type != ANIM_TYPE_JUMP_ATTACK && state.animation_type != ANIM_TYPE_HIT && (state.animation_type != ANIM_TYPE_WALK_ATTACK || (state.animation_type == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                    set_animation_type(ANIM_TYPE_WALK);
				}
				moved = true;
				break;
			}
		}
	}
	if (moveCommands.left == 1 && state.direction != ANIM_DIRECTION_LEFT && (state.animation_type == ANIM_TYPE_SLIDE || is_in_stairs_frame() == true)) {
        state.direction = ANIM_DIRECTION_LEFT;
    }

	if (moveCommands.right == 1 && state.animation_type != ANIM_TYPE_SLIDE && is_in_stairs_frame() == false) {
        for (float i=temp_move_speed; i>=0.1; i--) {
            if (state.animation_type == ANIM_TYPE_HIT && hit_ground() == true) {
                hit_moved_back_n += temp_move_speed;
            }

            st_map_colision map_col = map_colision(i, 0, map->getMapScrolling());
            mapLock = map_col.block;
            //mapLock =  gameControl.getMapPointLock(st_position((position.x + frameSize.width + i)/TILESIZE, (position.y + frameSize.height/2)/TILESIZE));
            if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                //std::cout << "character::charMove - temp_move_speed: " << temp_move_speed << ", map->get_last_scrolled().x: " << map->get_last_scrolled().x << std::endl;
                if (mapLock == TERRAIN_UNBLOCKED || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1) || mapLock == BLOCK_Y) {
					position.x += i - map->get_last_scrolled().x;
				} else {
					position.x += i*WATER_SPEED_MULT - map->get_last_scrolled().x;
				}
				if (state.animation_type != ANIM_TYPE_HIT) {
					state.direction = ANIM_DIRECTION_RIGHT;
				} else {
                    gravity(false);
                    return;
                }
				if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_WALK_ATTACK) {
					state.animation_timer = 0;
                }
                if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_JUMP && state.animation_type != ANIM_TYPE_SLIDE && state.animation_type != ANIM_TYPE_JUMP_ATTACK && state.animation_type != ANIM_TYPE_HIT && (state.animation_type != ANIM_TYPE_WALK_ATTACK || (state.animation_type == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                    set_animation_type(ANIM_TYPE_WALK);
				}
				moved = true;
				break;
			}
		}
	}
	if (moveCommands.right == 1 && state.direction != ANIM_DIRECTION_RIGHT && (state.animation_type == ANIM_TYPE_SLIDE || is_in_stairs_frame() == true)) {
        state.direction = ANIM_DIRECTION_RIGHT;
    }

    // Ice inertia
    if (bottom_point_lock == TERRAIN_ICE || (_inertia_obj.is_started() && (bottom_point_lock == TERRAIN_UNBLOCKED|| bottom_point_lock == TERRAIN_WATER))) {

        if (moved == true) {
            if (moveCommands.right == 1) {
                st_map_colision map_col = map_colision(1, 0, map->getMapScrolling());
                mapLock = map_col.block;
                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                    position.x++;
                }
            } else if (moveCommands.left == 1) {
                st_map_colision map_col = map_colision(-1, 0, map->getMapScrolling());
                mapLock = map_col.block;
                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                    position.x--;
                }
            }
            _inertia_obj.start();
        } else if (moveCommands.left == 0 && moveCommands.right == 0) {
            int inertia_xinc = _inertia_obj.execute();
            if (inertia_xinc != 0) {
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    if (position.x - inertia_xinc < 0) {
                        std::cout << "INERTIA::STOP #1" << std::endl;
                        _inertia_obj.stop();
                    } else {
                        st_map_colision map_col = map_colision(-inertia_xinc, 0, map->getMapScrolling());
                        mapLock = map_col.block;
                        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                            //std::cout << "inertia CONTINUE, pos.x: " << position.x << ", mapLock: " << mapLock << std::endl;
                            position.x -= inertia_xinc;
                        //} else {
                            //std::cout << "inertia DONT EXECUTE, pos.x: " << position.x << ", mapLock: " << mapLock << std::endl;
                        }
                    }
                } else {
                    if (realPosition.x+inertia_xinc > RES_W) {
                        //std::cout << "INERTIA::STOP #2" << std::endl;
                        _inertia_obj.stop();
                    } else {
                        st_map_colision map_col = map_colision(inertia_xinc, 0, map->getMapScrolling());
                        mapLock = map_col.block;
                        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                            //std::cout << "inertia CONTINUE, pos.x: " << position.x << ", mapLock: " << mapLock << std::endl;
                            position.x += inertia_xinc;
                        //} else {
                            //std::cout << "inertia DONT EXECUTE, pos.x: " << position.x << ", mapLock: " << mapLock << std::endl;
                        }
                    }
                }
            }
        }
        if (map != NULL) {
            gameControl.update_stage_scrolling();
        }
        char_update_real_position();
    } else if (bottom_point_lock != TERRAIN_WATER) {
        _inertia_obj.stop();
    }

    if (_obj_jump.is_started() == true) {
        _inertia_obj.stop();
    }


	// check if character is on stairs
    if (moveCommands.up == 1 && state.animation_type != ANIM_TYPE_STAIRS_ATTACK) { // check stairs on middle
        st_position stairs_pos = is_on_stairs(st_rectangle(position.x, position.y+(frameSize.height/2)-2, frameSize.width, frameSize.height/2-2));
        int top_terrain = map->getMapPointLock(st_position(((stairs_pos.x * TILESIZE - 6)+frameSize.width/2)/TILESIZE, position.y/TILESIZE));
        if (stairs_pos.x != -1) {
			if (state.animation_type != ANIM_TYPE_STAIRS_MOVE) {
                set_animation_type(ANIM_TYPE_STAIRS_MOVE);
			}
            if (top_terrain == TERRAIN_UNBLOCKED || top_terrain == TERRAIN_WATER || top_terrain == TERRAIN_STAIR) {
                position.y -= temp_move_speed/2;
                position.x = stairs_pos.x * TILESIZE - 6;
            }
		// out of stairs
		} else {
            int map_terrain = map->getMapPointLock(st_position(((position.x+frameSize.width/2)/TILESIZE), ((position.y+frameSize.height-4)/TILESIZE)));
            //std::cout << ">> map_terrain: " << map_terrain << ", _dropped_from_stairs: " << _dropped_from_stairs << std::endl;
            if (_dropped_from_stairs == false && map_terrain == TERRAIN_STAIR) { // check stairs bottom (leaving)
                //std::cout << "STAIRS SEMI - SET #1" << std::endl;
                set_animation_type(ANIM_TYPE_STAIRS_SEMI);
				position.y -= temp_move_speed/2;
			} else if (state.animation_type == ANIM_TYPE_STAIRS_SEMI) {
                if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #A" << std::endl;
                set_animation_type(ANIM_TYPE_STAND);
                //std::cout << "LEAVE STAIRS (BOTTOM->UP)" << std::endl;
                position.y -= 2;
			}
		}
    }




    if (moveCommands.down == 1 && state.animation_type != ANIM_TYPE_SLIDE && state.animation_type != ANIM_TYPE_STAIRS_ATTACK) {
        st_position stairs_pos_center = is_on_stairs(st_rectangle(position.x, position.y+frameSize.height/2, frameSize.width, frameSize.height/2));
        bool is_already_on_stairs = is_in_stairs_frame();
        /// @TODO - check that move-speed/2 is not zero

        // is on stairs
        if (is_already_on_stairs == true) {
            // if frame is semi, but already entered whole body, change to full-stairs frame
            if (state.animation_type == ANIM_TYPE_STAIRS_SEMI && stairs_pos_center.x != -1) {
                //std::cout << "STAIRS *MOVE* - SET #2" << std::endl;
                set_animation_type(ANIM_TYPE_STAIRS_MOVE);
            }

            // check that path is clear to move
            if (bottom_point_lock == TERRAIN_WATER || bottom_point_lock == TERRAIN_UNBLOCKED || bottom_point_lock == TERRAIN_STAIR) {
                position.y += temp_move_speed/2;
            }

            // if bottom point is not stairs, leave it
            if (bottom_point_lock != TERRAIN_STAIR) {
                if (stairs_pos_center.x == -1 && (bottom_point_lock == TERRAIN_UNBLOCKED || bottom_point_lock == TERRAIN_WATER)) {
                    set_animation_type(ANIM_TYPE_JUMP);
                }
                if (bottom_point_lock != TERRAIN_UNBLOCKED && bottom_point_lock != TERRAIN_WATER) {
                    if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #B" << std::endl;
                    set_animation_type(ANIM_TYPE_STAND);
                }
            }
        // not in stairs, but over it
        } else if (bottom_point_lock == TERRAIN_STAIR) {
            // over stairs, enter it
            st_position stairs_pos_bottom = is_on_stairs(st_rectangle(position.x, position.y+frameSize.height, frameSize.width, frameSize.height/2));
            if (stairs_pos_bottom.x != -1) {
                //std::cout << "STAIRS SEMI - SET #2" << std::endl;
                set_animation_type(ANIM_TYPE_STAIRS_SEMI);
                position.y += temp_move_speed/2;
                position.x = stairs_pos_bottom.x * TILESIZE - 6;
            }
        }

    }

	// is on stairs without moving
	if (moveCommands.down == 0 && moveCommands.up == 0 && state.animation_type == ANIM_TYPE_STAIRS_MOVE) {
        _stairs_stopped_count++;
        if (_stairs_stopped_count > 10) {
            set_animation_type(ANIM_TYPE_STAIRS);
        }
    } else if (moveCommands.down != 0 || moveCommands.up != 0) {
        _stairs_stopped_count = 0;
        if (state.animation_type == ANIM_TYPE_STAIRS) {
            //std::cout << "STAIRS *MOVE* - SET #3" << std::endl;
            set_animation_type(ANIM_TYPE_STAIRS_MOVE);
        }
    }

    check_reset_stand();

    if (is_player() == false) {
        attack(false, 0, false);
    }

	bool res_slide = slide(map->getMapScrolling());

    bool resJump = false;
    resJump = jump(moveCommands.jump, map->getMapScrolling());
    if (resJump == true || res_slide == true) {
        if (state.animation_type == ANIM_TYPE_HIT) {
            gravity(false);
        }
    /// @TODO: removed a gravity from here in an ELSE. Hope it was not necessary
	}


    if (is_player_type && moved == false && resJump == false && res_slide == false) {
		//if (state.animation_type != ANIM_TYPE_WALK) {
         if (is_in_stairs_frame() == false && state.animation_type != ANIM_TYPE_STAND && state.animation_type != ANIM_TYPE_JUMP && state.animation_type != ANIM_TYPE_JUMP_ATTACK && state.animation_type != ANIM_TYPE_TELEPORT && state.animation_type != ANIM_TYPE_SHIELD && state.animation_type != ANIM_TYPE_TELEPORT && state.animation_type != ANIM_TYPE_HIT && state.animation_type != ANIM_TYPE_SLIDE && (state.animation_type != ANIM_TYPE_ATTACK || (state.animation_type == ANIM_TYPE_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
			//if (is_player()) std::cout << "********* reset to stand - on_stairs_frame: " << is_in_stairs_frame() << ", state.animation_type: " << state.animation_type << std::endl;
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #D" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
		}
		//std::cout << "charMove - ANIM_TYPE_HIT: " << ANIM_TYPE_HIT << ", animation_type: " << state.animation_type << ", timer.getTimer(): " << timer.getTimer() << ", hit_duration+last_hit_time: " << hit_duration+last_hit_time << std::endl;
		if (state.animation_type == ANIM_TYPE_HIT && timer.getTimer() > hit_duration/2+last_hit_time) { // finished hit time
			//if (is_player()) std::cout << "state.animation_type SET to STAND " << std::endl;
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #E" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
		}
	}

    //if (is_player()) std::cout << "pos.x: " << position.x << ", pos.y: " << position.y << std::endl;

    if (_dropped_from_stairs == true) {
        if (timer.getTimer() > hit_duration+last_hit_time) {
            std::cout << "RESET _dropped_from_stairs flag #1" << std::endl;
            _dropped_from_stairs = false;
        } else if (hit_ground() == true) {
            std::cout << "RESET _dropped_from_stairs flag #2" << std::endl;
            _dropped_from_stairs = false;
        }
    }


	map->reset_scrolled();
    _previous_position = position;

}

void character::clear_move_commands()
{
	moveCommands.up = 0;
	moveCommands.down = 0;
	moveCommands.left = 0;
	moveCommands.right = 0;
    //std::cout << ">>> moveCommands.attack::RESET #3" << std::endl;
	moveCommands.attack = 0;
	moveCommands.jump = 0;
    moveCommands.start = 0;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::change_char_color(Sint8 colorkey_n, st_color new_color, bool full_change=true)
{
    if (full_change == false) {
        graphLib.change_surface_color(colorkey_n, new_color, &(character_graphics_list.find(name)->second)[state.direction][state.animation_type][state.animation_state].frameSurface);
	} else {
        for (int i=0; i<2; i++) {
			for (int j=0; j<ANIM_TYPE_COUNT; j++) {
				for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    graphLib.change_surface_color(colorkey_n, new_color, &(character_graphics_list.find(name)->second)[i][j][k].frameSurface);
				}
			}
		}
    }
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
/// @TODO: this must be moved to player, as character attack must be very simple
void character::attack(bool dont_update_colors, short updown_trajectory, bool auto_charged)
{

    if (timer.is_paused()) {
        return;
    }

    if (state.animation_type == ANIM_TYPE_TELEPORT) {
        //std::cout << "character::attack - LEAVE #1" << std::endl;
        return;
    }
    //std::cout << "character::attack - START, name: " << name << ", max_projectiles: " << max_projectiles << ", projectile_list.size(): " << projectile_list.size() << std::endl;
	if (character_graphics_list.find(name) == character_graphics_list.end()) {
		std::cout << "ERROR: could not find projectile graphics" << std::endl;
		return;
	}
    if (attack_state != ATTACK_NOT && (timer.getTimer()-state.attack_timer) >= (character_graphics_list.find(name)->second)[state.direction][state.animation_type][state.animation_state].delay) {
		//std::cout << "character::attack - shoot projectile END" << std::endl;
		attack_state = ATTACK_NOT;
	}

    int attack_id = -1;

    if (moveCommands.attack != 0) {
        //std::cout << "ATTACK 0 - #1 - max_projectiles: " << max_projectiles << std::endl;
        if (max_projectiles > get_projectile_count() && (timer.getTimer()-state.attack_timer) > 100 && attack_button_released == true) {
            //std::cout << "ATTACK 0 - #2" << std::endl;
            if (auto_charged == true) {
                attack_id = game_data.semi_charged_projectile_id;
            } else {
                attack_id = 0;
            }
        }
    }


    if (_charged_shot_projectile_id > 0 && moveCommands.attack == 0 && (timer.getTimer()-state.attack_timer) > CHARGED_SHOT_TIME && attack_button_released == false) {
        attack_id = _charged_shot_projectile_id;
    } else if (_charged_shot_projectile_id > 0 && moveCommands.attack == 0 && (timer.getTimer()-state.attack_timer) > 400 && (timer.getTimer()-state.attack_timer) < CHARGED_SHOT_TIME && attack_button_released == false) {
        attack_id = game_data.semi_charged_projectile_id;
    }

    if (moveCommands.attack == 0 && attack_button_released == false) {
        attack_button_released = true;
    }

    // change player colors if charging attack
    if (_charged_shot_projectile_id > 0 && (timer.getTimer()-state.attack_timer) > CHARGED_SHOT_INITIAL_TIME && (timer.getTimer()-state.attack_timer) < CHARGED_SHOT_TIME && attack_button_released == false) {
		if (is_player() && soundManager.is_playing_repeated_sfx() == false) {
			soundManager.play_repeated_sfx(SFX_CHARGING1, 0);
		}
		if (color_keys[0].r != -1) {
            if (charging_color_timer < timer.getTimer()) {
                charging_color_n++;
                if (charging_color_n > 2) {
                    charging_color_n = 0;
                }
                charging_color_timer = timer.getTimer()+200;
				if (charging_color_n == 0) {
                    change_char_color(0, st_color(171, 0, 19), false);
                    change_char_color(1, st_color(231, 0, 91), false);
                } else if (charging_color_n == 1) {
                    change_char_color(0, st_color(231, 0, 91), false);
                    change_char_color(1, st_color(255, 119, 183), false);
                } else if (charging_color_n == 2) {
                    change_char_color(0, st_color(255, 119, 183), false);
                    change_char_color(1, st_color(171, 0, 19), false);
                }
			}
        }
    }
    if (_charged_shot_projectile_id > 0 && is_player() && (timer.getTimer()-state.attack_timer) >= CHARGED_SHOT_TIME && attack_button_released == false) {
		if (soundManager.is_playing_repeated_sfx() == true && soundManager.get_repeated_sfx_n() == SFX_CHARGING1) {
			soundManager.stop_repeated_sfx();
            soundManager.play_repeated_sfx(SFX_CHARGING2, 255);
		}
        if (color_keys[0].r != -1) {
            if (charging_color_timer < timer.getTimer()) {
                charging_color_n++;
                if (charging_color_n > 2) {
                    charging_color_n = 0;
                }
                charging_color_timer = timer.getTimer()+100;
				if (charging_color_n == 0) {
                    change_char_color(0, st_color(219, 43, 0), false);
                    change_char_color(1, st_color(255, 155, 59), false);
				} else if (charging_color_n == 1) {
                    change_char_color(0, st_color(255, 155, 59), false);
                    change_char_color(1, st_color(255, 234, 0), false);
				} else if (charging_color_n == 2) {
                    change_char_color(0, st_color(255, 234, 0), false);
                    change_char_color(1, st_color(219, 43, 0), false);
				}
			}
        }
    }

    if (state.animation_type == ANIM_TYPE_SLIDE) {
        //std::cout << "character::attack - LEAVE #2" << std::endl;
        return;
    }


    if (attack_id != -1) {
        //std::cout << "character::attack - attack_id: " << attack_id << std::endl;

        //if (!is_player()) { std::cout << "CHAR::attack::attack_id: " << attack_id << std::endl; }
        if (attack_id == _charged_shot_projectile_id || attack_id == 20 || attack_id == 10 || attack_id == game_data.semi_charged_projectile_id) {
			if (is_player() && soundManager.is_playing_repeated_sfx() == true) {
				soundManager.stop_repeated_sfx();
			}
		}


        attack_button_released = false; // coment out this line to get "turbo" in button
		//attack_state == ATTACK_NOT &&
        //std::cout << "character::attack - shoot projectile" << std::endl;
        st_position proj_pos;

        if (state.direction == ANIM_DIRECTION_LEFT) {
            proj_pos = st_position(position.x+TILESIZE/3, position.y+frameSize.height/2);
        } else {
            proj_pos = st_position(position.x+frameSize.width-TILESIZE/2, position.y+frameSize.height/2);
        }

        projectile_list.push_back(projectile(attack_id, state.direction, proj_pos, map, is_player()));
        projectile &temp_proj = projectile_list.back();
        temp_proj.set_is_permanent();
        _player_must_reset_colors = true;


        // second projectile for player that fires multiple ones
        if ((attack_id == 0 || (attack_id == game_data.semi_charged_projectile_id && auto_charged == true)) && is_player() && _number == PLAYER_BETABOT) { /// @TODO - move number of simultaneous shots to character/data-file
            projectile_list.push_back(projectile(attack_id, state.direction, st_position(proj_pos.x-TILESIZE, proj_pos.y+5), map, is_player()));
            projectile &temp_proj2 = projectile_list.back();
            temp_proj2.set_is_permanent();
        }
        if (attack_id == 0) { // handle normal attack differences depending on player
            if (updown_trajectory == 1) {
                temp_proj.set_trajectory(TRAJECTORY_DIAGONAL_UP);
                set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_UP);
            } else if (updown_trajectory == -1) {
                temp_proj.set_trajectory(TRAJECTORY_DIAGONAL_DOWN);
                set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_DOWN);
            } else if (updown_trajectory == 0 && (state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_UP || state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_DOWN)) { // not shooting diagonal, but animation is on diagonal -> reset to normal attack
                set_animation_type(ANIM_TYPE_ATTACK);
            }

        }



        if (GameMediator::get_instance()->get_projectile(attack_id).trajectory == TRAJECTORY_CENTERED) {
            temp_proj.set_owner_direction(&state.direction);
            temp_proj.set_owner_position(&position);
		}
        if (GameMediator::get_instance()->get_projectile(attack_id).trajectory == TRAJECTORY_TARGET_DIRECTION || GameMediator::get_instance()->get_projectile(attack_id).trajectory == TRAJECTORY_TARGET_EXACT || GameMediator::get_instance()->get_projectile(attack_id).trajectory == TRAJECTORY_ARC_TO_TARGET || GameMediator::get_instance()->get_projectile(attack_id).trajectory == TRAJECTORY_FOLLOW) {
            if (!is_player() && map->_player_ref != NULL) {
                character* p_player = map->_player_ref;
                temp_proj.set_target_position(p_player->get_position_ref());
            } else {
                classnpc* temp_npc = map->find_nearest_npc(st_position(position.x, position.y));
                if (temp_npc != NULL) {
                    temp_proj.set_target_position(temp_npc->get_position_ref());
                }
            }
		}

        temp_proj.play_sfx();


		attack_state = ATTACK_START;
		state.attack_timer = timer.getTimer();
		if (state.animation_type == ANIM_TYPE_STAND) {
            set_animation_type(ANIM_TYPE_ATTACK);
		} else if (state.animation_type == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
		} else if (is_in_stairs_frame()) {
            set_animation_type(ANIM_TYPE_STAIRS_ATTACK);
		} else if (state.animation_type == ANIM_TYPE_WALK) {
			//std::cout << "+++++++++++ CHARACTER - set animation to ANIM_TYPE_WALK_ATTACK" << std::endl;
            set_animation_type(ANIM_TYPE_WALK_ATTACK);
		}


        /// @TODO: here is the color bug
        /*
        if (dont_update_colors == false) {
            if (color_keys[0].r != -1) {
                change_char_color(0, color_keys[0]);
            }
            if (color_keys[1].r != -1) {
                change_char_color(1, color_keys[1]);
            }
        }
        */
    }
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::advance_frameset()
{

    //if (is_player()) std::cout << "character::show - direction: " << state.direction << ", type: " << state.animation_type << ", state: " << state.animation_state << "\n";
    //[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]
    if (state.direction > CHAR_ANIM_DIRECTION_COUNT) {
        //if (is_player()) std::cout << "WARNING - character::show - (" << name << ") error, direction value " << state.direction << " is invalid" << std::endl;
		state.direction = ANIM_DIRECTION_LEFT;
		return;
	}
    if (state.animation_type > ANIM_TYPE_COUNT) {
        //if (is_player()) std::cout << "character::show - error, type value " << state.direction << " is invalid" << std::endl;
		return;
	}
    if ((is_player() && state.animation_state > MAX_PLAYER_SPRITES) || (!is_player() && state.animation_state > MAX_NPC_SPRITES)) {
        //if (is_player()) std::cout << "character::show - error, animation_state value " << state.animation_state << " is invalid. state.animation_type: " << state.animation_type << std::endl;
		state.animation_state = 0;
		return;
	}
    if (have_frame_graphic(state.direction, state.animation_type, state.animation_state) == false) {
        //if (!is_player()) std::cout << "CHAR::advance_frameset - reset frameset animation" << std::endl;
        _was_animation_reset = true;
        state.animation_state = 0;
        _is_last_frame = true;
    } else {
        if (have_frame_graphic(state.direction, state.animation_type, state.animation_state+1) == false) {
            //if (!is_player()) std::cout << "CHAR::advance_frameset - _is_last_frame TRUE" << std::endl;
            _is_last_frame = true;
        } else {
            //if (is_player()) std::cout << "CHAR::advance_frameset - _is_last_frame FALSE" << std::endl;
            _is_last_frame = false;
        }
    }
}



void character::reset_jump()
{
    _obj_jump.finish();
}

void character::consume_projectile()
{
    if (projectile_list.size() > 0) {
        projectile_list.at(0).consume_projectile();
    }

}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::show() {

    if (timer.is_paused() == true) {
        return;
    }

	// update real position
	char_update_real_position();
    if (!is_player() && is_on_visible_screen() == false) {
		return;
	}


    // check attack frame

    if (_attack_frame_n != -1 && is_on_attack_frame() && state.animation_state == _attack_frame_n) {
        _is_attack_frame = true;
    } else {
        _is_attack_frame = false;
    }

    // show background, if any
    if (have_background_graphics() == true) {
        graphLib.showSurfaceAt(&(_character_graphics_background_list.find(name)->second), realPosition, false);
    }

    // only advance if time for the current frame has finished
    advance_frameset();

	// turn is a special case, if it does not exist, we must show stand instead
	if ((state.animation_type == ANIM_TYPE_TURN || state.animation_type == ANIM_TYPE_VERTICAL_TURN) && have_frame_graphic(state.direction, state.animation_type, state.animation_state) == false) {
        //std::cout << "show() - TURN graphic FINISHED" << std::endl;
		if (have_frame_graphic(state.direction, ANIM_TYPE_WALK, state.animation_state) == true) {
            show_sprite_graphic(state.direction, ANIM_TYPE_WALK, state.animation_state);
		} else {
            show_sprite_graphic(state.direction, ANIM_TYPE_STAND, state.animation_state);
		}
	// npc teleport use shows stand for now (will have a common graphic to show in the future)
	} else {
        show_sprite_graphic(state.direction, state.animation_type, state.animation_state);
	}
    if (is_player() == false) {
        if (freeze_weapon_effect != FREEZE_EFFECT_NPC || is_weak_to_freeze() == false) {
            show_sprite();
        }
    } else {
        show_sprite();
    }
    show_hp();
}





void character::show_sprite()
{
    if (state.animation_timer < timer.getTimer()) { // time passed the value to advance frame

		// change animation state to next frame
		int frame_inc = 1;
		if (state.animation_inverse == true) {
            frame_inc = frame_inc * -1;
		}
        int new_frame = (state.animation_state + frame_inc);
        if (have_frame_graphic(state.direction, state.animation_type, new_frame)) {
			state.animation_state += frame_inc;
        } else {
            if (state.animation_type == ANIM_TYPE_VERTICAL_TURN) {
				if (state.direction == ANIM_DIRECTION_LEFT) {
					state.direction = ANIM_DIRECTION_RIGHT;
				} else {
					state.direction = ANIM_DIRECTION_LEFT;
				}
                if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #F" << std::endl;
                set_animation_type(ANIM_TYPE_STAND);
			}
			if (state.animation_inverse == false) {
				if (state.animation_state > 0) {
					state.animation_state = 0;
				}
			} else {
				advance_to_last_frame();
            }

            _was_animation_reset = true;
            // some animation types reset to stand/other
            if (state.animation_type == ANIM_TYPE_STAIRS_ATTACK) {
                state.animation_type = ANIM_TYPE_STAIRS;
            }
        }
		if (state.animation_type == ANIM_TYPE_WALK_ATTACK) {
			state.animation_timer = timer.getTimer() + 5000;
		} else {
            short direction = ANIM_DIRECTION_RIGHT;
            int delay = (character_graphics_list.find(name)->second)[direction][state.animation_type][state.animation_state].delay;
            state.animation_timer = timer.getTimer() + delay;
        }
    }
}

void character::show_sprite_graphic(short direction, short type, short frame_n)
{
    st_position frame_pos = realPosition;
    frame_pos.x += _frame_pos_adjust.x;
    frame_pos.y += _frame_pos_adjust.y;

    if (state.invisible == true) {
        return;
    }

    //std::cout << ">>>> CHAR::show_sprite_graphic - direction: " << direction << ", type: " << type << ", frame_n: " << frame_n << ", x: " << frame_pos.x << ", y: " << frame_pos.y << ", _have_right_direction_graphics: " << _have_right_direction_graphics << std::endl;


    // NPCs use stand as teleport
    if (is_player() == false && type == ANIM_TYPE_TELEPORT) {
        type = ANIM_TYPE_STAND;
    }

    std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]>::iterator it_graphic;
    it_graphic = character_graphics_list.find(name);
    if (it_graphic == character_graphics_list.end()) {
        std::cout << "ERROR: #1 character::show_sprite_graphic - Could not find graphic for NPC [" << name << "]" << std::endl;
        return;
    }
    if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic with the given N position
        //std::cout << ">> character::show_sprite_graphic(" << name << ") #1 - no graphic for type (" << type << "):frame_n(" << frame_n << "), set to ZERO pos" << std::endl;
        frame_n = 0;
        state.animation_state = 0;
        _was_animation_reset = true;
        if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic with the given type
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #G" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
            type = ANIM_TYPE_STAND;
            if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic at all
                std::cout << "ERROR: #2 character::show_sprite_graphic - Could not find graphic for NPC [" << name << "] at pos[0][0][0]" << std::endl;
                return;
            }
        }
    }

    /// blinking when hit
    unsigned int now_timer = timer.getTimer();
    if (now_timer < hit_duration+last_hit_time) {

        //if (is_player()) { std::cout << "PLATER::HIT. hit_animation_timer: " << hit_animation_timer << ", now_timer: " << now_timer << std::endl; }

        if (hit_animation_timer > now_timer) {
            graphLib.show_white_surface_at(&it_graphic->second[direction][type][frame_n].frameSurface, frame_pos);
            hit_animation_count = 0;
            return;
        } else if ((hit_animation_timer+HIT_BLINK_ANIMATION_LAPSE) < now_timer) {
            hit_animation_count++;
            if (hit_animation_count > 2) {
                hit_animation_timer = now_timer+HIT_BLINK_ANIMATION_LAPSE;
            }
            return;
        }
    }
    if (_progressive_appear_pos == 0) {
        //std::cout << "#1 - show::frame_pos.x: " << frame_pos.x << ", realPosition.x: " << realPosition.x << ", pos.x: " << position.x << std::endl;
        graphLib.showSurfaceAt(&it_graphic->second[direction][type][frame_n].frameSurface, frame_pos, false);
    } else {
        int diff_y = frameSize.height-_progressive_appear_pos;

        graphLib.showSurfaceRegionAt(&it_graphic->second[direction][type][frame_n].frameSurface, st_rectangle(0, 0, frameSize.width, (frameSize.height-_progressive_appear_pos)), st_position(frame_pos.x, frame_pos.y-diff_y));
        _progressive_appear_pos--;
        if (_progressive_appear_pos == 0) {
            position.y -= frameSize.height;
        }
    }
}

void character::reset_gravity_speed()
{
    accel_speed_y = 0.25;
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool character::gravity(bool boss_demo_mode=false)
{

    /// @TODO: gravity speed is starting at 1.25, it should start at 0.25

    if (_progressive_appear_pos != 0) {
        //std::cout << "CHAR::GRAVITY leave #1" << std::endl;
        reset_gravity_speed();
        return false;
    }
    //if (is_player()) std::cout << "character::gravity - EXECUTE" << std::endl;
	if (!map) {
        std::cout << "ERROR: can't execute gravity without a map" << std::endl;
        reset_gravity_speed();
        return false; // error - can't execute this action without an associated map
	}

    if (state.animation_type == ANIM_TYPE_TELEPORT) {
        unsigned int now_time = timer.getTimer();
        if (last_execute_time < now_time) {
            last_execute_time = now_time + 10;
        } else {
            //std::cout << "CHAR::GRAVITY leave #2" << std::endl;
            reset_gravity_speed();
            return true;
        }
    }

    if (can_air_dash() == true && state.animation_type == ANIM_TYPE_SLIDE) {
        //std::cout << "CHAR::GRAVITY leave #3" << std::endl;
        reset_gravity_speed();
        return false;
    }

	// ------------- NPC gravity ------------------ //
	if (!is_player()) {
		if (!map) {
			return false; // error: is not into a map, can't use gravity
        }
        if (_ignore_gravity == true) {
            return false;
        }
        if (can_fly == false || gameControl.is_showing_boss_intro == true) {
			bool is_moved = false;
			short int limit_speed = move_speed;
			if (boss_demo_mode == true) {
                limit_speed = GRAVITY_MAX_SPEED;
			}
            if (limit_speed < 1) {
                limit_speed = 1;
            }
			for (int i=limit_speed; i>0; i--) {
                bool res_test_move = test_change_position(0, i);
                //std::cout << ">> boss_demo_mode: " << boss_demo_mode << ", position.y: " << position.y << std::endl;
                if ((boss_demo_mode == true && position.y <= TILESIZE*2) || res_test_move == true) {
					position.y += i;
					is_moved = true;
					break;
				}
            }
			return is_moved;
        }
        //std::cout << "CHAR::GRAVITY leave #4" << std::endl;
        reset_gravity_speed();
        return false; // not moved because of IA type
	}


	// ------------ PLAYER gravity --------------------- //
	if (is_player() && position.y > RES_H+1) {
        std::cout << "**** gravity - LEAVE (death)" << std::endl;
		hitPoints.current = 0;
		death();
        std::cout << "CHAR::GRAVITY leave #5" << std::endl;
        reset_gravity_speed();
        return false;
	}


    //std::cout << ">>>>>> CHAR::position.y[BEFORE]: " << position.y << std::endl;


	if (is_in_stairs_frame()) {
        //character* playerObj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y
        short int res_colision_npc = map->colision_player_npcs(this, 0, 0,  9, 0);
        if (res_colision_npc == 1) {
            damage(TOUCH_DAMAGE_SMALL, false);
        } else if (res_colision_npc == 2) {
            damage(TOUCH_DAMAGE_BIG, false);
        }
        //std::cout << "gravity leave #6" << std::endl;
        reset_gravity_speed();
        return false;
	}

    if (_obj_jump.is_started() == false && can_fly == false && position.y < RES_H+TILESIZE+1 + frameSize.height) {

        // tem que inicializar essa variável sempre que for false
        //std::cout << "CHAR::GRAVITY [1] - accel_speed_y: " << accel_speed_y << ", gravity_y: " << gravity_y << std::endl;
        accel_speed_y = accel_speed_y + accel_speed_y*gravity_y;
        //std::cout << "CHAR::GRAVITY [2] - accel_speed_y: " << accel_speed_y << std::endl;

        if (accel_speed_y < 0.25) {
            accel_speed_y = 0.25;
        } else if (accel_speed_y > GRAVITY_MAX_SPEED) {
            accel_speed_y = GRAVITY_MAX_SPEED;
        }

        int adjusted_speed = accel_speed_y;
        if (adjusted_speed < 1) {
            adjusted_speed = 1;
        }



		if (state.animation_type == ANIM_TYPE_TELEPORT) {
            if (_teleport_minimal_y - position.y > TILESIZE) {
                adjusted_speed = GRAVITY_MAX_SPEED;
            } else {
                adjusted_speed = GRAVITY_MAX_SPEED/2;
            }
		}

        //if (is_player()) std::cout << "gravity - accel_speed_y: " << accel_speed_y << ", gravity_y: " << gravity_y << ", adjusted_speed: " << adjusted_speed << std::endl;


        st_map_colision map_col;
        bool was_moved = false;
        for (int i=adjusted_speed; i>0; i--) {
            map_col = map_colision(0, i, map->getMapScrolling());
            int mapLock = map_col.block;


			if (state.animation_type == ANIM_TYPE_TELEPORT && position.y < _teleport_minimal_y-TILESIZE) {
				mapLock = BLOCK_UNBLOCKED;
            } else if (!is_player() && state.animation_type == ANIM_TYPE_TELEPORT && position.y >= _teleport_minimal_y-TILESIZE) {
                _teleport_minimal_y = frameSize.height+TILESIZE*2; // RESET MIN_Y -> remove limit for next telepor
            } else if (position.y > RES_H+TILESIZE) { // out of screen
				mapLock = BLOCK_UNBLOCKED;
			}


			if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_STAIR_X || mapLock == BLOCK_STAIR_Y) {
                //if (is_player()) std::cout << "character::gravity - FALL" << std::endl;
				if (mapLock != BLOCK_WATER || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1)) {
					position.y += i;
				} else {
					position.y += i*WATER_SPEED_MULT;
				}
                if (state.animation_type != ANIM_TYPE_JUMP && state.animation_type != ANIM_TYPE_JUMP_ATTACK && state.animation_type != ANIM_TYPE_TELEPORT && state.animation_type != ANIM_TYPE_SLIDE && state.animation_type != ANIM_TYPE_HIT && (state.animation_type != ANIM_TYPE_JUMP_ATTACK || (state.animation_type == ANIM_TYPE_JUMP_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                    set_animation_type(ANIM_TYPE_JUMP);
				}
				was_moved = true;

				_is_falling = true;
				break;
            }
			if (i == 1) {
                reset_gravity_speed();
			}
		}

		if (was_moved == false && (state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) && state.animation_type != ANIM_TYPE_SLIDE) {
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #H" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
			return true;
		} else if (was_moved == false && state.animation_type == ANIM_TYPE_TELEPORT && position.y >= RES_H/3) {
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #I" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
			return true;
		}
        if (was_moved == false && _is_falling == true) {
            _is_falling = false;
            if (is_player()) {
                if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #J" << std::endl;
                set_animation_type(ANIM_TYPE_STAND);
                //std::cout << "SFX_PLAYER_JUMP #2" << std::endl;
                soundManager.play_sfx(SFX_PLAYER_JUMP);
            }
        }

        check_platform_move(map_col.terrain_type);

		// teleport finished
        //std::cout << "NOT FALLING, RESET ACCEL_SPEED_Y" << std::endl;
    }
    return false;
}

bool character::hit_ground() // indicates if character is standing above ground
{
    // if position did not changed since last execution, return previous hit_ground value
    /*
    if (position == _previous_position) {
        return _hit_ground;
    }
    */
    short map_tile_x = (position.x + frameSize.width/2)/TILESIZE;
    short map_tile_y1 = (position.y + frameSize.height)/TILESIZE;
    short map_tile_y2 = (position.y + frameSize.height/2)/TILESIZE;
    int pointLock1 = gameControl.getMapPointLock(st_position(map_tile_x, map_tile_y1));
    _hit_ground = false;
    if (pointLock1 != TERRAIN_UNBLOCKED && pointLock1 != TERRAIN_WATER && pointLock1 != TERRAIN_STAIR) {
        //if (is_player()) std::cout << "hit_ground #1" << std::endl;
        _hit_ground = true;
    } else {
        int pointLock2 = gameControl.getMapPointLock(st_position(map_tile_x, map_tile_y2));
        if (pointLock1 != pointLock2) {
            //if (is_player()) std::cout << "hit_ground #2" << std::endl;
            _hit_ground = true;
        }

    }
    return _hit_ground;
}

bool character::will_hit_ground(int y_change) const
{
    short map_tile_x = (position.x + frameSize.width/2)/TILESIZE;
    short map_tile_y = (position.y + y_change + frameSize.height)/TILESIZE;
    int pointLock = gameControl.getMapPointLock(st_position(map_tile_x, map_tile_y));
    if (pointLock != TERRAIN_UNBLOCKED && pointLock != TERRAIN_WATER) {
        return true;
    }
    return false;
}

bool character::is_on_screen()
{
    st_float_position scroll(0, 0);
    if (map != NULL) {
        scroll = map->getMapScrolling();
    }
    // is on screen
    if (abs((float)position.x) >= scroll.x && abs((float)position.x) <= scroll.x+RES_W) {
        //if (name == "Dynamite Bot") std::cout << ">>>> character::is_on_screen - visible - pos.x: " << position.x << ", scroll.x: " << scroll.x << ", scroll.x+RES_W: " << scroll.x+RES_W << " <<<<" << std::endl;
        return true;
    }

    // is on left of the screen
    if (abs((float)position.x) > scroll.x-RES_W/2 && abs((float)position.x) < scroll.x) {
        // check wall-lock on the range
        int map_point_start = (scroll.x-RES_W/2)/TILESIZE;
        int map_point_end = scroll.x/TILESIZE;
        bool found_lock = false;
        for (int i=map_point_start; i<=map_point_end; i++) {
            if (map->get_map_point_wall_lock(i) == true) {
                found_lock = true;
            }
        }
        if (found_lock == false) {
            return true;
        }
        if (name == "Dynamite Bot") std::cout << ">>>> character::is_on_screen - left <<<<" << std::endl;
    }

    // is on right to the screen
    if (abs((float)position.x) > scroll.x+RES_W && abs((float)position.x) < scroll.x+RES_W*1.5) {
        int map_point_start = (scroll.x+RES_W)/TILESIZE;
        int map_point_end = (scroll.x*1.5)/TILESIZE;
        bool found_lock = false;
        for (int i=map_point_start; i<=map_point_end; i++) {
            if (map->get_map_point_wall_lock(i) == true) {
                found_lock = true;
            }
        }
        if (found_lock == false) {
            return true;
        }
        if (name == "Dynamite Bot") std::cout << ">>>> character::is_on_screen - right <<<<" << std::endl;
    }
    return false;
}

bool character::is_on_visible_screen()
{
    st_float_position scroll = map->getMapScrolling();
    // entre scroll.x e scroll.x+RES_W

    if (abs((float)position.x + frameSize.width) >= scroll.x && abs((float)position.x) < scroll.x+RES_W) {
        return true;
    }
    return false;
}

bool character::is_invisible() const
{
	return state.invisible;
}


void character::activate_super_jump()
{
    _super_jump = true;
}

void character::activate_force_jump()
{
	_force_jump = true;
}

st_float_position *character::get_position_ref()
{
	return &position;
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_float_position character::getPosition() const
{
	/// @TODO - this is crashing sometimes
	return position;
}

void character::set_position(struct st_position new_pos)
{
    //std::cout << "\\\\\\\\\\\\ character::set_position - pos.x: " << position.x << ", new_pos.x: " << new_pos.x << std::endl;
	//std::cout << "\\\\\\\\\\\\ character::set_position - pos.y: " << position.x << ", new_pos.y: " << new_pos.x << std::endl;
	position.x = new_pos.x;
	position.y = new_pos.y;
	char_update_real_position();
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool character::slide(st_float_position mapScrolling)
{
	if (is_player() == false) {
		return false;
	}

    // change jump button released state, if needed
    if (_dash_button_released == false && moveCommands.dash == 0) {
        _dash_button_released = true;
    }


    // deal with cases player should not slide
    /// @TODO: share common code between jump and slide

    if (state.animation_type == ANIM_TYPE_TELEPORT) {
        //std::cout << "CHAR::slide - RETURN #0" << std::endl;
        return false;
    }

    if ((state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) && can_air_dash() == false) {
        //std::cout << "CHAR::slide - RETURN #1" << std::endl;
        return false;
    }


	if (is_in_stairs_frame()) {
        //std::cout << "CHAR::slide - RETURN #2" << std::endl;
		return false;
	}

	// no need to slide
    if (state.animation_type != ANIM_TYPE_SLIDE && moveCommands.dash != 1) {
        //std::cout << "CHAR::slide - RETURN #3" << std::endl;
		return false;
	}


    st_map_colision map_col = map_colision(0, -6, map->getMapScrolling()); // this is minus six because of +4 adjustments in jump-up colision
    int map_lock =  map_col.block;
    //std::cout << "character::slide - map_lock: " << map_lock << std::endl;

    // releasing down (or dash button) interrupts the slide
    if (moveCommands.dash != 1 && state.animation_type == ANIM_TYPE_SLIDE && (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER)) {
        if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #K" << std::endl;
        set_animation_type(ANIM_TYPE_STAND);
        //std::cout << "CHAR::slide - RETURN #4" << std::endl;
        return false;
    }


	if (state.slide_distance > TILESIZE*5 && (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER)) {
        if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #L" << std::endl;
        if (hit_ground() == true) {
            set_animation_type(ANIM_TYPE_STAND);
        } else {
            set_animation_type(ANIM_TYPE_JUMP);
        }
        state.slide_distance = 0;
        //std::cout << "CHAR::slide - RETURN #5" << std::endl;
        return false;
    }

    //std::cout << "_dash_button_released: " << _dash_button_released << std::endl;

    // start slide
    if (state.animation_type != ANIM_TYPE_SLIDE && _dash_button_released == true) {
        if (moveCommands.dash == 1) {
            bool did_hit_ground = hit_ground();
            if (did_hit_ground == true || (did_hit_ground == false && _can_execute_airdash == true)) {
                _can_execute_airdash = false;
                set_animation_type(ANIM_TYPE_SLIDE);
                state.slide_distance = 0;
                _dash_button_released = false;
                int adjust_x = -3;
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    adjust_x = frameSize.width+3;
                }
                map->add_animation(ANIMATION_STATIC, &graphLib.dash_dust, position, st_position(adjust_x, frameSize.height-8), 160, 0, state.direction, st_size(8, 8));
            }
		}
    }

    if (state.animation_type != ANIM_TYPE_SLIDE) {
        //std::cout << "CHAR::slide - RETURN #6" << std::endl;
        return false;
    }

    // if there is no ground, interrupts slide
    st_map_colision map_col_fall = map_colision(0, 1, map->getMapScrolling());
    int fall_map_lock = map_col_fall.block;
    if (can_air_dash() == false && (fall_map_lock == BLOCK_UNBLOCKED || fall_map_lock == BLOCK_WATER)) {
        set_animation_type(ANIM_TYPE_JUMP);
		state.slide_distance = 0;
        //std::cout << "CHAR::slide - RETURN #7" << std::endl;
		return false;
	}

    // check if trying to leave screen
    if (state.direction == ANIM_DIRECTION_LEFT && position.x <= 0) {
        state.slide_distance = 0;
        return false;
    }
    if (state.direction == ANIM_DIRECTION_RIGHT && position.x + frameSize.width > MAP_W * TILESIZE) {
        state.slide_distance = 0;
        return false;
    }


    int res_move_x = 0;
    int mapLockAfter = BLOCK_UNBLOCKED;
    _obj_jump.finish();

    // reduce progressively the jump-move value in oder to deal with colision
    for (float i=move_speed*2; i>0.0; i--) {

        int temp_i;
        if (state.direction == ANIM_DIRECTION_LEFT) {
            temp_i = -i;
        } else {
            temp_i = i;
        }
        st_map_colision map_col = map_colision(temp_i, 0, mapScrolling);;
        mapLockAfter = map_col.block;
        if (mapLockAfter == BLOCK_UNBLOCKED) {
            res_move_x = temp_i;
            break;
        } else if (mapLockAfter == BLOCK_WATER) {
			if (abs((float)i*WATER_SPEED_MULT) > 0) {
                res_move_x = temp_i*WATER_SPEED_MULT;
            } else {
                res_move_x = temp_i;
            }
            break;
        }
    }


    if (mapLockAfter == BLOCK_UNBLOCKED || mapLockAfter == BLOCK_WATER) {
        position.x += res_move_x;
		state.slide_distance += abs((float)res_move_x);
    }

    return true;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool character::jump(int jumpCommandStage, st_float_position mapScrolling)
{
    // @TODO - can only jump again once set foot on land
    if (jumpCommandStage == 0 && jump_button_released == false) {
        jump_button_released = true;
    }

    if (state.animation_type == ANIM_TYPE_HIT) {
        return false;
    }

    if (_force_jump == true || (jumpCommandStage == 1 && jump_button_released == true)) {
        //std::cout << "char::jump - button pressed" << std::endl;

        if (is_in_stairs_frame()) {
            set_animation_type(ANIM_TYPE_JUMP);
            _is_falling = true;
            return false;
        } else {
            //std::cout << "char::_jumps_number: " << _jumps_number << ", obj::_jumps_number: " << _obj_jump.get_jumps_number() << std::endl;
            if (_is_falling == false && (_obj_jump.is_started() == false || (_jumps_number > _obj_jump.get_jumps_number()))) {
                int water_lock = map->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y+6)/TILESIZE));
                if (water_lock == TERRAIN_WATER) {
                    _obj_jump.start(true);
                } else if (_super_jump == true) {
                    _super_jump = false;
                    _obj_jump.start(true);
                } else {
                    _obj_jump.start(false);
                }
                if (state.animation_type == ANIM_TYPE_SLIDE && slide_type == 0) {
                    _dashed_jump = true;
                }
                set_animation_type(ANIM_TYPE_JUMP);
                jump_button_released = false;
            }
        }
    }


    if (_obj_jump.is_started() == true) {
        int jump_speed = _obj_jump.get_speed();
        bool jump_moved = false;

        // if got into stairs, finish jumping
        if ((is_in_stairs_frame())) {
            _obj_jump.finish();
            return false;
        }

        if (jump_speed < 0 && jumpCommandStage == 0 && _force_jump == false) {
            _obj_jump.interrupt();
        }


        // check collision
        for (int i=abs((float)jump_speed); i>0; i--) {
            int speed_y = 0;
            if (jump_speed > 0) {
                speed_y = i;
            } else {
                speed_y = i*-1;
            }
            st_map_colision map_col = map_colision(0, speed_y, mapScrolling);
            int map_lock = map_col.block;
            //std::cout << "jump::check_collision - i[" << i << "], map_lock["  << map_lock << "]" << std::endl;

            if (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER) {
                std::cout << "jump.speed[" << speed_y << "]" << std::endl;
                position.y += speed_y;
                jump_moved = true;
                break;
            }
        }
        if (jump_speed != 0 && jump_moved == false) {
            //std::cout << "chat::jump - must interrupt because a collision happened" << std::endl;
            if (jump_speed < 0) {
                _obj_jump.interrupt();
            } else {
                _obj_jump.finish();
            }
        }

        _obj_jump.execute();
        if (_obj_jump.is_started() == false) {
            //std::cout << "SFX_PLAYER_JUMP #1" << std::endl;
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            if (_force_jump == true) {
                _force_jump = false;
            }
        } else {
            if (is_player() && position.y > RES_H+1) {
                std::cout << "**** JUMP::LEAVE (death)" << std::endl;
                _obj_jump.finish();
            }
        }
    } else {
        if (_force_jump == true) {
            _force_jump = false;
        }
        //accel_speed_y = GRAVITY_MAX_SPEED;
        gravity();
        return false;
    }


    return true;
}




void character::check_map_colision_point(int &map_block, int &new_map_lock, int mode_xy, st_position map_pos) // mode_xy 0 is x, 1 is y
{
    UNUSED(map_pos);

    int old_map_lock = gameControl.getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y+frameSize.height/2)/TILESIZE));

    if (map_block == BLOCK_UNBLOCKED && new_map_lock == TERRAIN_WATER) {
        map_block = BLOCK_WATER;
    }

    if (old_map_lock != new_map_lock) {
        bool must_block = false;

        if (is_player() == false && new_map_lock == TERRAIN_UNBLOCKED && old_map_lock == TERRAIN_WATER) { // NPCs must not leave water
            must_block = true;
        } else if (is_player() == false && old_map_lock == TERRAIN_UNBLOCKED && new_map_lock == TERRAIN_WATER) { // NPCs must not enter water
            must_block = true;
        } else if (is_player() == false && new_map_lock == TERRAIN_SCROLL_LOCK) {
            must_block = true;
        } else if (new_map_lock == TERRAIN_EASYMODEBLOCK) {
            if (game_save.difficulty == DIFFICULTY_EASY) {
                must_block = true;
            }
        } else if (new_map_lock == TERRAIN_HARDCODEBLOCK) {
            if (game_save.difficulty == DIFFICULTY_HARD) {
                damage(999, true);
                must_block = true;
            }
        } else if (new_map_lock != TERRAIN_UNBLOCKED && new_map_lock != TERRAIN_WATER && new_map_lock != TERRAIN_SCROLL_LOCK && new_map_lock != TERRAIN_CHECKPOINT && new_map_lock != TERRAIN_STAIR) {
            must_block = true;
        }
        if (must_block == true) {
            if (mode_xy == 0) {
                if (map_block != BLOCK_XY) {
                    map_block = BLOCK_X;
                }
            } else {
                if (map_block == BLOCK_X) {
                    map_block = BLOCK_XY;
                } else if (map_block != BLOCK_XY) {
                    map_block = BLOCK_Y;
                }
            }
        }
	}
}

bool character::process_special_map_points(int map_lock, int incx, int incy, st_position map_pos)
{
    UNUSED(incy);
    int direction = ANIM_DIRECTION_LEFT;
    if (incx > 0) {
        direction = ANIM_DIRECTION_RIGHT;
    }
	if (is_player() && incx > 0 && map_lock == TERRAIN_DOOR) {
        gameControl.horizontal_screen_move(direction, true, map_pos.x, map_pos.y);
		return true;
	}
	if (is_player() && incx != 0 && map_lock == TERRAIN_SCROLL_LOCK) {
        gameControl.horizontal_screen_move(direction, false, map_pos.x, map_pos.y);
		return true;
	}
    if (is_player() == true && state.animation_type != ANIM_TYPE_TELEPORT && (map_lock == TERRAIN_SPIKE || (map_lock == TERRAIN_HARDCODEBLOCK && game_save.difficulty == 2))) {
        damage(SPIKES_DAMAGE, false);
        return true;
    }
	if (is_player() && map_lock == TERRAIN_CHECKPOINT) {
		checkpoint.x = position.x;
		checkpoint.y = position.y;
		checkpoint.map = map->get_number();
		checkpoint.map_scroll_x = map->getMapScrolling().x;
	}

    return false;
}

void character::check_platform_move(short map_lock)
{
    float move = 0.0;
    bool can_move = true;

    if (_moving_platform_timer < timer.getTimer()) {
        int pos_y = (position.y + frameSize.height/2) / TILESIZE;
        if (map_lock == TERRAIN_MOVE_LEFT) {
            move = (move_speed-0.5)*-1;
            int pos_x = (position.x + move) / TILESIZE;
            if (is_player() && state.direction == ANIM_DIRECTION_RIGHT) { // add a few pixels because of graphic when turned right
                pos_x = (position.x + 5 + move) / TILESIZE;
            }
            int point_terrain = gameControl.getMapPointLock(st_position(pos_x, pos_y));
            if (point_terrain != TERRAIN_UNBLOCKED && point_terrain != TERRAIN_WATER) {
                can_move = false;
            }
        } else if (map_lock == TERRAIN_MOVE_RIGHT) {
            move = move_speed-0.5;
            int pos_x = (position.x + frameSize.width - 10 + move) / TILESIZE;
            int point_terrain = gameControl.getMapPointLock(st_position(pos_x, pos_y));
            if (point_terrain != TERRAIN_UNBLOCKED && point_terrain != TERRAIN_WATER) {
                can_move = false;
            }
        }
        if (can_move) {
            position.x += move;
            _moving_platform_timer = timer.getTimer()+MOVING_GROUND;
        }
    }
}

st_map_colision character::map_colision(const float incx, const short incy, st_float_position mapScrolling)
{
    int py_adjust = 8;
    int terrain_type = TERRAIN_UNBLOCKED;



    if (state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) {
        py_adjust = 1;
    } else if (state.animation_type == ANIM_TYPE_SLIDE) {
        if (slide_type == 1) {
            py_adjust = TILESIZE;
        } else {
            py_adjust = TILESIZE/2;
        }
    }

    UNUSED(mapScrolling);
	int map_block = BLOCK_UNBLOCKED;


    map->colision_char_object(this, incx, incy, 9, py_adjust);
    object_colision res_colision_object = map->get_obj_colision();




    if (is_player() == true && res_colision_object._block != 0) {
        // deal with teleporter object that have special block-area and effect (9)teleporting)
        if (res_colision_object._object != NULL && (res_colision_object._object->get_type() == OBJ_BOSS_TELEPORTER || (res_colision_object._object->get_type() == OBJ_FINAL_BOSS_TELEPORTER && res_colision_object._object->is_started() == true))) {
            if (is_on_teleporter_capsulse(res_colision_object._object) == true) {
                state.direction = ANIM_DIRECTION_RIGHT;
                gameControl.object_teleport_boss(res_colision_object._object->get_boss_teleporter_dest(), res_colision_object._object->get_boss_teleport_map_dest(), res_colision_object._object->get_obj_map_id());
            }
        } else if (res_colision_object._object != NULL && (res_colision_object._object->get_type() == OBJ_FINAL_BOSS_TELEPORTER && res_colision_object._object->is_started() == false)) {
            // ignore block
        } else if (!get_item(res_colision_object)) {
			map_block = res_colision_object._block;
            if (map_block == BLOCK_Y || map_block == BLOCK_XY) {
                _can_execute_airdash = true;
            }
        }
    } else if (is_player() == false && res_colision_object._block != 0) {
        map_block = res_colision_object._block;
        if (map_block == BLOCK_Y || map_block == BLOCK_XY) {
            _can_execute_airdash = true;
        }
    }

	if (is_player()) {
        if (have_shoryuken() == true && state.animation_type == ANIM_TYPE_SPECIAL_ATTACK) {
            map->colision_player_special_attack(this, incx, incy, 9, py_adjust);
        } else {
            short int res_colision_npc = map->colision_player_npcs(this, incx, incy, 9, py_adjust);
            if (res_colision_npc == 1) {
                damage(TOUCH_DAMAGE_SMALL, false);
            } else if (res_colision_npc == 2) {
                damage(TOUCH_DAMAGE_BIG, false);
            }
        }
	}


	if (incx == 0 && incy == 0) {
        return st_map_colision(BLOCK_UNBLOCKED, TERRAIN_UNBLOCKED);
	}

    if (_always_move_ahead == false && ((incx < 0 && position.x+incx) < 0 || (incx > 0 && position.x+incx > MAP_W*TILESIZE))) {
        if (map_block == BLOCK_UNBLOCKED) {
            map_block = BLOCK_X;
        } else {
            map_block = BLOCK_XY;
        }
	}
    if ((incy < 0 && ((position.y+incy+frameSize.height < 0) || (incx > 0 && position.y+incx+TILESIZE > MAP_W*TILESIZE)))) {
		if (map_block == BLOCK_UNBLOCKED) {
			map_block = BLOCK_Y;
		} else {
			map_block = BLOCK_XY;
		}
    }

    // if we are out of map, return always true
    if (_always_move_ahead == true) {
        if ((incx < 0 && (position.x+incx < 0)) || (incx > 0 && position.x+incx > MAP_W*TILESIZE)) {
            return st_map_colision(BLOCK_UNBLOCKED, TERRAIN_UNBLOCKED);
        }
    }

	/// @TODO - use collision rect for the current frame. Until there, use 3 points check
	int py_top, py_middle, py_bottom;
	int px_left, px_center, px_right;

    py_top = position.y + incy + py_adjust;

    py_middle = position.y + incy + frameSize.height/2;
    py_bottom = position.y + incy + frameSize.height - 2;

	px_center = position.x + incx + frameSize.width/2;
	px_left = position.x + incx + 9;
	px_right = position.x + incx + frameSize.width - 9;


	st_position map_point;
	map_point.x = px_left/TILESIZE;
	int new_map_lock = TERRAIN_UNBLOCKED;
	if (incx > 0) {
		map_point.x = px_right/TILESIZE;
	}

	/// @TODO - use a array-of-array for poijts in order to having a cleaner code

	int map_x_points[3];
	map_x_points[0] = px_left/TILESIZE;
	map_x_points[1] = px_center/TILESIZE;
	map_x_points[2] = px_right/TILESIZE;

	int map_y_points[3];
    map_y_points[0] = py_top/TILESIZE;
	map_y_points[1] = py_middle/TILESIZE;
    map_y_points[2] = py_bottom/TILESIZE;

    // TEST X POINTS
	if (incx != 0) {
		for (int i=0; i<3; i++) {
            if (is_player() && (state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) && i == 0) {
                map_point.y = (py_top+1)/TILESIZE;
            } else {
                map_point.y = map_y_points[i];
            }
            new_map_lock = gameControl.getMapPointLock(map_point);
            check_map_colision_point(map_block, new_map_lock, 0, map_point);
            if (process_special_map_points(new_map_lock, incx, incy, map_point)) {
                return st_map_colision(map_block, new_map_lock);
            }
		}
	}

	// TEST Y POINTS
	if (incy < 0) {
        map_point.y = py_top/TILESIZE;
	} else if (incy > 0) {
		map_point.y = py_bottom/TILESIZE;
	}
	if (incy != 0) {
		for (int i=0; i<3; i++) {
            map_point.x = map_x_points[i];
			new_map_lock = gameControl.getMapPointLock(map_point);

            //if (is_player()) std::cout << "@@@ i: " << i << ", new_map_lock: " << new_map_lock << std::endl;
            check_map_colision_point(map_block, new_map_lock, 1, map_point);
            if (new_map_lock != TERRAIN_UNBLOCKED) {
                terrain_type = new_map_lock;
            }

            if (process_special_map_points(new_map_lock, incx, incy, map_point)) {
                return st_map_colision(map_block, new_map_lock);
            }
			// STAIRS
			if ((map_block == BLOCK_UNBLOCKED || map_block == BLOCK_X || map_block == BLOCK_WATER) && incy > 0 && new_map_lock == TERRAIN_STAIR) { // stairs special case
                int middle_y_point_lock = TERRAIN_UNBLOCKED;
                if (incy == 1) { // gravity
                    middle_y_point_lock = gameControl.getMapPointLock(st_position(map_x_points[i], (py_bottom-1)/TILESIZE));
                } else { // other cases as falling or jump
                    middle_y_point_lock = gameControl.getMapPointLock(st_position(map_x_points[i], map_y_points[1]));
                }

                if (middle_y_point_lock != TERRAIN_STAIR) {
					if (map_block == BLOCK_X) {
						map_block = BLOCK_XY;
					} else {
						map_block = BLOCK_Y;
					}
				}
            }
		}

	}

	if (is_player()) {
        // check water splash
		int point_top = gameControl.getMapPointLock(st_position(map_x_points[1], map_y_points[0]));
		int point_middle = gameControl.getMapPointLock(st_position(map_x_points[1], map_y_points[1]));
		int point_bottom = gameControl.getMapPointLock(st_position(map_x_points[1], map_y_points[2]));

		//std::cout << ">> check water-splash - point_top.x: " <<  map_x_points[1] << ", point_top.y: " << map_y_points[1] << ", pos.x: " << position.x << ", pos.y: " << position.y << ", point_bottom: " << point_bottom << ", point_middle: " << point_middle << ", point_top: " << point_top << std::endl;

		if (incy != 0) {
			if (point_top == TERRAIN_UNBLOCKED && point_middle == TERRAIN_UNBLOCKED && point_bottom == TERRAIN_WATER && _water_splash == false) {
				if (incy < 0) {
					soundManager.play_sfx(SFX_WATER_LEAVE);
				} else {
					soundManager.play_sfx(SFX_WATER_LEAVE);
				}
				//std::cout << ">> ADD water splash animation - adjust_y: " << adjust_y << ", point_bottom: " << point_bottom << ", point_middle: " << point_middle << ", point_top: " << point_top << std::endl;
				_water_splash = true;
				//ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize
                map->add_animation(ANIMATION_STATIC, &graphLib.water_splash, st_float_position(position.x, (map_y_points[2]-1)*TILESIZE), st_position(0, -6), 100, 0, ANIM_DIRECTION_LEFT, st_size(32, 23));
			} else if (point_top == point_bottom && point_top == point_middle && _water_splash == true) {
				//std::cout << ">> RE-ENABLE water splash animation - point_bottom: " << point_bottom << ", point_middle: " << point_middle << ", point_top: " << point_top << std::endl;
				_water_splash = false;
			}
        }
	}


    //if (is_player()) std::cout << "character::map_colision_v2 - map_block: " << map_block << std::endl;


    return st_map_colision(map_block, terrain_type);

}

bool character::is_on_teleporter_capsulse(object *object)
{
    // check se player está dentro da área Y do objeto
    int obj_y = object->get_position().y;
    if (obj_y < position.y && (obj_y + object->get_size().height > position.y + frameSize.height)) {
        // só teleporta quando estiver no centro (1 TILE), caso contrário, ignora block
        double abs_value = TILESIZE/2 - object->get_size().width;
        int obj_center_diff = abs(abs_value)/2;
        int limit_min = object->get_position().x + obj_center_diff;
        int limit_max = object->get_position().x + object->get_size().width - obj_center_diff;
        int px = position.x + frameSize.width/2;
        //std::cout << "px: " << px << ", limit_min: " << limit_min << ", limit_max: " << limit_max << std::endl;
        if (px > limit_min && px < limit_max) {
            return true;
        }
    }
    return false;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::addSpriteFrame(int anim_type, int posX, graphicsLib_gSurface &spritesSurface, int delay)
{
	struct st_rectangle spriteArea;

	spriteArea.x = posX*frameSize.width;
    spriteArea.y = 0;
	spriteArea.w = frameSize.width;
    spriteArea.h = frameSize.height;

    // ANIM_TYPE_STAIRS_MOVE and ANIM_TYPE_STAIRS_SEMI have an extra frame that is the mirror of the first one

    if (is_player()) std::cout << "delay: " << delay << std::endl;

    for (int anim_direction=0; anim_direction<=1; anim_direction++) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) { // find the last free frame
            if ((character_graphics_list.find(name)->second)[anim_direction][anim_type][i].frameSurface.get_surface() == NULL) {

                st_spriteFrame *sprite = &(character_graphics_list.find(name)->second)[anim_direction][anim_type][i];
                graphicsLib_gSurface gsurface = graphLib.surfaceFromRegion(spriteArea, spritesSurface);

                // RIGHT
                if (anim_direction != 0) {
                    sprite->setSurface(gsurface);
                // LEFT
                } else {
                    graphicsLib_gSurface gsurface_flip = graphLib.flip_image(gsurface, flip_type_horizontal);
                    sprite->setSurface(gsurface_flip);
                }


                (character_graphics_list.find(name)->second)[anim_direction][anim_type][i].frameSurface.init_colorkeys();
                (character_graphics_list.find(name)->second)[anim_direction][anim_type][i].delay = delay;

                if (anim_type == ANIM_TYPE_STAIRS_MOVE || anim_type == ANIM_TYPE_STAIRS_SEMI) {
                    st_spriteFrame *sprite = &(character_graphics_list.find(name)->second)[anim_direction][anim_type][i+1];
                    if (anim_direction != 0) {
                        graphicsLib_gSurface gsurface_flip = graphLib.flip_image(gsurface, flip_type_horizontal);
                        sprite->setSurface(gsurface_flip);
                    } else {
                        sprite->setSurface(gsurface);
                    }
                    (character_graphics_list.find(name)->second)[anim_direction][anim_type][i+1].frameSurface.init_colorkeys();
                    (character_graphics_list.find(name)->second)[anim_direction][anim_type][i+1].delay = delay;
                }


                break;
            }
        }
    }
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::set_map(classMap *set_map)
{
	map = set_map;
	//st_position new_map_scroll = set_map->getMapScrolling();
    //std::cout << "////////////// CHARACTER - new_map_scroll.x: " << new_map_scroll.x << std::endl;
}

classMap *character::get_map()
{
    return map;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::set_is_player(bool set_player)
{
	is_player_type = set_player;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool character::is_player() const
{
	return is_player_type;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
string character::getName(void) const
{
	return name;
}

// ********************************************************************************************** //
// Returns true of character is over a staircase                                                  //
// ********************************************************************************************** //
st_position character::is_on_stairs(st_rectangle pos)
{
    if (_dropped_from_stairs == true) { // was dropped from stairs, can't grab again until invencibility time ends
        //if (is_player()) std::cout << "is_on_stairs - FALSE 1" << std::endl;
        return st_position(-1, -1);;
    }
    int map_tile_x, map_tile_y;

    int diff_w = pos.w/3;
	map_tile_x = (pos.x+diff_w)/TILESIZE;
	map_tile_y = (pos.y)/TILESIZE;

	if (map->getMapPointLock(st_position(map_tile_x, map_tile_y)) == TERRAIN_STAIR) {
        //if (is_player()) std::cout << "is_on_stairs - TRUE 1" << std::endl;
		return st_position(map_tile_x, map_tile_y);
    }

    map_tile_x = (pos.x+pos.w-diff_w)/TILESIZE;
    if (map->getMapPointLock(st_position(map_tile_x, map_tile_y)) == TERRAIN_STAIR) {
        //if (is_player()) std::cout << "is_on_stairs - TRUE 2" << std::endl;
        return st_position(map_tile_x, map_tile_y);
    }

    //if (is_player()) std::cout << "is_on_stairs - FALSE 2" << std::endl;
	return st_position(-1, -1);
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_size character::get_size() const
{
	return frameSize;
}


// ********************************************************************************************** //
// adds an entry into character_graphics_list map, if needed                                      //
// ********************************************************************************************** //
void character::add_graphic()
{
    std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]>::iterator it;
    st_spriteFrame temp_sprites[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT];
	const std::string temp_name(name);

    for (int i=0; i<2; i++) {
		for (int j=0; j<ANIM_TYPE_COUNT; j++) {
			for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                temp_sprites[i][j][k].frameSurface.set_surface(NULL);
			}
		}
	}


	it = character_graphics_list.find(name);
	if (it == character_graphics_list.end()) { // there is no graphic with this key yet, add it
        character_graphics_list.insert(pair<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]>(temp_name, temp_sprites));
		//std::cout << "2. character::add_graphic - adding graphic entry for character '" << name << "', list.size: " << character_graphics_list.size() << std::endl;
    }
}


bool character::have_frame_graphics()
{
    std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]>::iterator it;
	it = character_graphics_list.find(name);
	if (it != character_graphics_list.end()) { // there is no graphic with this key yet, add it
        for (int i=0; i<2; i++) {
			for (int j=0; j<ANIM_TYPE_COUNT; j++) {
				for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    if ((character_graphics_list.find(name)->second)[i][k][k].frameSurface.width > 0 && (character_graphics_list.find(name)->second)[i][j][k].frameSurface.get_surface() != NULL) {
						return true;
					}
				}
			}
		}
	}
    return false;
}

void character::clean_character_graphics_list()
{
    if (is_player()) {
        return;
    }
    if (character_graphics_list.size() <= 0) {
        return;
    }

    std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]>::iterator it;
    it = character_graphics_list.find(name);
    if (it != character_graphics_list.end()) {
        std::cout << "CHAR::clean_character_graphics_list[" << name << "]" << std::endl;
        character_graphics_list.erase(it);
    }
}

bool character::have_background_graphics()
{
    static std::map<std::string, graphicsLib_gSurface>::iterator it;
    it = _character_graphics_background_list.find(name);
    if (it != _character_graphics_background_list.end()) { // there is no graphic with this key yet, add it
        return true;
    }
    return false;
}




int character::frames_count()
{
	for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
        if ((character_graphics_list.find(name)->second)[state.direction][state.animation_type][i].frameSurface.width == 0 || (character_graphics_list.find(name)->second)[state.direction][state.animation_type][i].frameSurface.get_surface() == NULL) {
			return i;
		}
	}
	return ANIM_FRAMES_COUNT;
}

void character::advance_to_last_frame()
{
	int frames_n = frames_count();
	if (frames_n > 0) {
		state.animation_state = frames_n - 1;
	} else {
		state.animation_state = 0;
    }
}

void character::show_hp()
{
    // do nothing here
}


bool character::have_frame_graphic(int direction, int type, int pos)
{
    if ((character_graphics_list.find(name)->second)[direction][type][pos].frameSurface.width == 0 || (character_graphics_list.find(name)->second)[direction][type][pos].frameSurface.get_surface() == NULL) {
        //if (name == "Bat") std::cout << "character::have_frame_graphic(" << name << ")[" << direction << "][" << type << "][" << pos << "] - FALSE" << std::endl;
		return false;
	}
    //if (name == "Bat") std::cout << "character::have_frame_graphic(" << name << ")[" << direction << "][" << type << "][" << pos << "] - TRUE" << std::endl;
	return true;
}

bool character::is_in_stairs_frame() const
{
	if (state.animation_type == ANIM_TYPE_STAIRS || state.animation_type == ANIM_TYPE_STAIRS_MOVE || state.animation_type == ANIM_TYPE_STAIRS_SEMI || state.animation_type == ANIM_TYPE_STAIRS_ATTACK) {
		return true;
	}
    return false;
}

bool character::is_on_attack_frame()
{
    if (state.animation_type == ANIM_TYPE_ATTACK || state.animation_type == ANIM_TYPE_STAIRS_ATTACK || state.animation_type == ANIM_TYPE_WALK_ATTACK || state.animation_type == ANIM_TYPE_JUMP_ATTACK || state.animation_type == ANIM_TYPE_THROW || state.animation_type == ANIM_TYPE_SPECIAL_ATTACK || state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_DOWN || state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_UP || state.animation_type == ANIM_TYPE_ATTACK_DOWN || state.animation_type == ANIM_TYPE_ATTACK_UP) {
        return true;
    }
    return false;
}

void character::recharge(e_energy_types _en_type, int value)
{
	//std::cout << "character::recharge";
	if (_en_type == ENERGY_TYPE_HP) {
        if (hitPoints.current < hitPoints.total) {
            if (hitPoints.current + value <= hitPoints.total) {
				hitPoints.current += value;
			} else {
                hitPoints.current = hitPoints.total;
			}

            if (value > ENERGY_ITEM_SMALL) {
                soundManager.play_repeated_sfx(SFX_GOT_ENERGY, 1);
            } else {
                soundManager.play_sfx(SFX_GOT_ENERGY);
            }
        }
	}
}



bool character::get_item(object_colision& obj_info)
{
	bool res = false;
	// deal with non-blocking items
	if (obj_info._object != NULL && obj_info._object->finished() == false) {
		//std::cout << "character::get_item" << std::endl;
		switch (obj_info._object->get_type()) {
            case OBJ_ENERGY_PILL_SMALL:
                //std::cout << "character::get_item - OBJ_ENERGY_PILL_SMALL" << std::endl;
                recharge(ENERGY_TYPE_HP, ENERGY_ITEM_SMALL);
                res = true;
                obj_info._object->set_finished(true);
                break;
            case OBJ_ENERGY_PILL_BIG:
                //std::cout << "character::get_item - OBJ_ENERGY_PILL_BIG" << std::endl;
                recharge(ENERGY_TYPE_HP, ENERGY_ITEM_BIG);
                res = true;
                obj_info._object->set_finished(true);
                break;
            default:
                //std::cout << "character::get_item - unknown item type: " << obj_info._object->get_type() << std::endl;
                break;
		}
//	} else {
//		std::cout << "character::get_item - empty object" << std::endl;
	}
	return res;
}

// returns type, or -1 if none
int character::is_executing_effect_weapon()
{
    vector<projectile>::iterator it;
    for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        int move_type = (*it).get_move_type();
        if (move_type == TRAJECTORY_BOMB) {
            return TRAJECTORY_BOMB;
        } else if (move_type == TRAJECTORY_QUAKE) {
            return TRAJECTORY_QUAKE;
        } else if (move_type == TRAJECTORY_FREEZE) {
            return TRAJECTORY_FREEZE;
        } else if (move_type == TRAJECTORY_CENTERED) {
            return TRAJECTORY_CENTERED;
        } else if (move_type == TRAJECTORY_PUSH_BACK) {
            return TRAJECTORY_PUSH_BACK;
        }
    }
    return -1;
}

// is all projectiles are normal (-1 or 0) return the character's max_shots,
// otherwise, find the lowest between all fired projectiles
Uint8 character::get_projectile_max_shots()
{
    bool all_projectiles_normal = true;
    vector<projectile>::iterator it;
    short max_proj = 9;
    for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        short id = (*it).get_id();
        if (id != -1 && id != 0) {
            all_projectiles_normal = false;
            short proj_max = (*it).get_max_shots();
            if (max_proj > 0 && proj_max < max_proj) {
                max_proj = proj_max;
            }
        }
    }
    if (all_projectiles_normal == true) {
        return max_projectiles;
    }
    return max_proj;
}

void character::push_back(short direction)
{
    int xinc = -(move_speed-0.2);
    if (direction == ANIM_DIRECTION_LEFT) {
        xinc = move_speed-0.2;
    }

    std::cout << "CHAR::PUSH_BACK - xinc: " << xinc << std::endl;

    if (test_change_position(xinc, 0)) {
        position.x += xinc;
    }
}

void character::remove_freeze_effect()
{
    state.frozen = false;
    state.frozen_timer = 0;
}


st_position character::get_int_position()
{
    return st_position((int)position.x, (int)position.y);
}

void character::check_reset_stand()
{
    if (!is_player()) { // NPCs do not need this
        return;
    }
    // is walking without moving, reset to stand
    if (moveCommands.left == 0 && moveCommands.right == 0 && state.animation_type == ANIM_TYPE_WALK) {
        if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #C" << std::endl;
        set_animation_type(ANIM_TYPE_STAND);
    }
    if ((state.animation_type == ANIM_TYPE_ATTACK || state.animation_type == ANIM_TYPE_WALK_ATTACK || state.animation_type == ANIM_TYPE_JUMP_ATTACK || state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_DOWN || state.animation_type == ANIM_TYPE_ATTACK_DIAGONAL_UP) && timer.getTimer() > state.attack_timer+500) {
        switch (state.animation_type) {
            case ANIM_TYPE_WALK_ATTACK:
                state.animation_type = ANIM_TYPE_WALK;
                break;
            case ANIM_TYPE_JUMP_ATTACK:
                state.animation_type = ANIM_TYPE_JUMP;
                break;
            default:
                state.animation_type = ANIM_TYPE_STAND;
                break;
        }

    }

}


unsigned int character::get_projectile_count()
{
	int pcount = 0;
    vector<projectile>::iterator it;
	for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        pcount++;
	}
    //std::cout << "character::get_projectile_count - pcount: " << pcount << ", max_projectiles: " << max_projectiles << std::endl;
	return pcount;
}



// ********************************************************************************************** //
// set the object platform player is over, if any                                                 //
// ********************************************************************************************** //
void character::set_platform(object* obj)
{
	if (obj != NULL) {
		if (state.animation_type == ANIM_TYPE_JUMP) {
            if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #O" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
            _obj_jump.interrupt();
        } else if (state.animation_type == ANIM_TYPE_JUMP_ATTACK) {
            set_animation_type(ANIM_TYPE_ATTACK);
            _obj_jump.interrupt();
        }
        if (name == _debug_char_name) std::cout << "CHAR::RESET_TO_STAND #P" << std::endl;
        set_animation_type(ANIM_TYPE_STAND);
	}
	_platform = obj;
}

object* character::get_platform()
{
    //std::cout << "CHAR::get_platform: " << _platform << std::endl;
	return _platform;
}

int character::get_direction() const
{
	return state.direction;
}

void character::set_direction(int direction)
{
	state.direction = direction;
}

void character::clean_projectiles()
{
	// empty projectile list
    //if (name == "Mage Bot") std::cout << ">>> CHAR::clean_projectiles[" << name << "] - proj_list.size: " << projectile_list.size() << std::endl;

	while (!projectile_list.empty()) {
        if (name == "Mage Bot") std::cout << "removing projectile" << std::endl;
        projectile_list.at(0).finish();
		projectile_list.erase(projectile_list.begin());
	}
}

void character::damage(unsigned int damage_points, bool ignore_hit_timer = false)
{

    //std::cout << "CHAR::DAMAGE::START" << std::endl;

    UNUSED(ignore_hit_timer);
	if (is_player() && GAME_FLAGS[FLAG_INVENCIBLE] == true) {
        //std::cout << "CHAR::DAMAGE::LEAVE #1" << std::endl;
		return;
	}
    if (game_save.difficulty == DIFFICULTY_HARD && is_player() == false) {
        damage_points = damage_points * 0.5;
    }
    if (damage_points < 1) { // minimum damage is 1. if you don't want damage, don't call this method, ok? :)
        damage_points = 1;
    }
    if (damage_points + _damage_modifier > 0) {
        damage_points += _damage_modifier;
    }


	//std::cout << "1. character::damage - damage_points: " << damage_points << ", hitPoints.current: " << hitPoints.current << std::endl;

	if (hitPoints.current <= 0) { /// already dead
		return;
	}

    if (state.frozen == true && is_player()) {
        state.frozen_timer = 0;
		state.frozen = false;
	}

	unsigned int now_timer = timer.getTimer();
    if (now_timer < hit_duration+last_hit_time) { /// is still intangible from last hit
        return;
    }

    _was_hit = true;

    if (is_in_stairs_frame() == true) {
        _dropped_from_stairs = true;
    }

    if (_is_boss == true && hit_animation_timer < now_timer) {
        if (map != NULL) {
            int repeat_times = 4;
            int frame_duration = BOSS_HIT_DURATION / (repeat_times*2); // one time for show, one time for hide
            map->add_animation(ANIMATION_DYNAMIC, &graphLib.hit, position, st_position(0, 5), frame_duration, repeat_times, state.direction, st_size(24, 24));
        }
    }


    last_hit_time = now_timer;
    if (now_timer > hit_duration+last_hit_time) {
        hit_animation_timer = now_timer+HIT_BLINK_ANIMATION_LAPSE;
    }
    if (!is_player() || GAME_FLAGS[FLAG_INFINITE_HP] == false) {
        hitPoints.current -= damage_points;
        std::cout << "CHAR::DAMAGE::damage_points: " << damage_points << ", hitPoints.current: " << hitPoints.current << std::endl;
    }


    if (is_player() == true && state.animation_type != ANIM_TYPE_HIT) {
        set_animation_type(ANIM_TYPE_HIT);
        if (_obj_jump.is_started() == true) {
            hit_moved_back_n = get_hit_push_back_n()/2;
            _obj_jump.finish();
        } else {
            hit_moved_back_n = 0;
        }
        jump_button_released = false;
        if (is_player() == true) { /// @TODO - remove all animations when boss is defeated
            if (map != NULL) {
                int hit_anim_x = 0;
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    hit_anim_x = 3;
                }
                map->add_animation(ANIMATION_DYNAMIC, &graphLib.hit, position, st_position(hit_anim_x, 5), 150, 4, state.direction, st_size(24, 24));
            }
        }
	}

	// TODO: add hit animation
	if (hitPoints.current <= 0) {
		//std::cout << "1. character::damage - DEATH" << std::endl;
		hitPoints.current = 0;
		death();
	}
}

bool character::is_dead() const
{
	return (hitPoints.current <= 0);
}




st_hit_points character::get_hp() const
{
	return hitPoints;
}




Uint8 character::get_current_hp() const
{
	return hitPoints.current;
}

void character::set_current_hp(Uint8 inc)
{
	hitPoints.current += inc;
}


st_position character::get_real_position() const
{
	return realPosition;
}

void character::execute_jump_up()
{
	// fall intil reaching ground
	/// @TODO
	for (int i=0; i<100; i++) {
		char_update_real_position();
		gravity();
		map->showMap();
		show();
		map->showAbove(0);
        draw_lib.update_screen();
	}

    //activate_super_jump();
	// reset command jump, if any
	jump(0, map->getMapScrolling());
	jump(1, map->getMapScrolling());
    //std::cout << "execute_jump::START - " << initial_y << ", position.y: " << position.y << std::endl;
    while (_obj_jump.get_speed() < 0) {
        char_update_real_position();
        jump(1, map->getMapScrolling());
        map->showMap();
		show();
		map->showAbove();
        draw_lib.update_screen();
        timer.delay(20);
	}
    _obj_jump.interrupt();
}

void character::execute_jump()
{
	// fall until reaching ground
	fall();

	// reset command jump, if any
	jump(0, map->getMapScrolling());
    int initial_y = (int)position.y;
	jump(1, map->getMapScrolling());
	std::cout << "execute_jump::START - " << initial_y << ", position.y: " << position.y << std::endl;
    while (position.y != initial_y) {
        //std::cout << "execute_jump::LOOP - " << initial_y << ", position.y: " << position.y << std::endl;
		char_update_real_position();
		bool resJump = jump(1, map->getMapScrolling());
        if (resJump == false) {
			gravity();
		}
		map->showMap();
		show();
		map->showAbove();
        draw_lib.update_screen();
        timer.delay(20);
    }
}


void character::fall()
{
	/// @TODO
    _obj_jump.finish();
    for (int i=0; i<100; i++) {
		char_update_real_position();
		gravity(false);
		if (hit_ground() == true && state.animation_type == ANIM_TYPE_STAND) {
			map->showMap();
			show();
			map->showAbove();
            draw_lib.update_screen();
			return;
		}
		map->showMap();
		show();
		map->showAbove();
        draw_lib.update_screen();
        timer.delay(20);
	}
}

void character::teleport_out() {
	soundManager.play_sfx(SFX_TELEPORT);
    set_animation_type(ANIM_TYPE_TELEPORT);
    _obj_jump.finish();
    while (position.y > -(frameSize.height+TILESIZE)) {
        std::cout << "teleport_out - position.y: " << position.y << std::endl;
        position.y -= GRAVITY_MAX_SPEED;
		char_update_real_position();
		map->showMap();
        map->show_npcs();
		show();
        map->show_objects();
        map->showAbove();
        draw_lib.update_screen();
        timer.delay(10);
	}
    timer.delay(200);
}



bool character::change_position(short xinc, short yinc)
{

    st_map_colision map_col = map_colision(xinc, yinc, map->getMapScrolling());
    short int mapLock = map_col.block;
    if (is_player()) std::cout << "*** character::change_position - x: " << position.x << ", y: " << position.y << ", xinc: " << xinc << ", yinc: " << yinc << ", BLOCKED (" << mapLock << ")" << std::endl;

	if (mapLock != BLOCK_UNBLOCKED && mapLock != BLOCK_WATER) {
		return false;
	}
	position.x += xinc;
	position.y += yinc;
    return true;
}

void character::change_position_x(short xinc)
{
    if (xinc == 0) { // nothing todo
        return;
    }
    for (int i=xinc; i>=0.1; i--) {
        if (state.animation_type == ANIM_TYPE_HIT && hit_ground() == true) {
            hit_moved_back_n += xinc;
        }

        st_map_colision map_col = map_colision(i, 0, map->getMapScrolling());
        int mapLock = map_col.block;
        //mapLock =  gameControl.getMapPointLock(st_position((position.x + frameSize.width + i)/TILESIZE, (position.y + frameSize.height/2)/TILESIZE));
        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
            //std::cout << "character::charMove - temp_move_speed: " << temp_move_speed << ", map->get_last_scrolled().x: " << map->get_last_scrolled().x << std::endl;
            if (mapLock == TERRAIN_UNBLOCKED || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1) || mapLock == BLOCK_Y) {
                position.x += i - map->get_last_scrolled().x;
            } else {
                position.x += i*WATER_SPEED_MULT - map->get_last_scrolled().x;
            }
            if (state.animation_type != ANIM_TYPE_HIT) {
                state.direction = ANIM_DIRECTION_RIGHT;
            } else {
                gravity(false);
                return;
            }
            if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_WALK_ATTACK) {
                state.animation_timer = 0;
            }
            if (state.animation_type != ANIM_TYPE_WALK && state.animation_type != ANIM_TYPE_JUMP && state.animation_type != ANIM_TYPE_SLIDE && state.animation_type != ANIM_TYPE_JUMP_ATTACK && state.animation_type != ANIM_TYPE_HIT && (state.animation_type != ANIM_TYPE_WALK_ATTACK || (state.animation_type == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                set_animation_type(ANIM_TYPE_WALK);
            }
            position.x += xinc;
            break;
        }
    }
}

int character::change_position_y(short yinc)
{
    if (yinc == 0) { // nothing todo
        return 0;
    }
    if (test_change_position(0, yinc)) { // can move max
        return yinc;
    } else { // check decrementing xinc
        if (yinc > 0) {
            for (int i=yinc; i>0; i--) {
                if (test_change_position(0, i)) {
                    return i;
                }
            }
        } else {
            for (int i=yinc; i<0; i++) {
                if (test_change_position(0, i)) {
                    return i;
                }
            }
        }
    }
    return 0;
}

bool character::test_change_position(short xinc, short yinc)
{
    if (yinc < 0 && position.y < 0) {
		return false;
	}
    if (yinc > 0 && position.y > RES_H) {
        return true;
	}
    if (xinc < 0 && position.x <= 0) {
        return false;
    }
    st_map_colision map_col = map_colision(xinc, yinc, map->getMapScrolling());
    short int mapLock = map_col.block;
	if (mapLock != BLOCK_UNBLOCKED && mapLock != BLOCK_WATER) {
		return false;
	}
    return true;
}


bool character::test_change_position_debug(short xinc, short yinc)
{
	if (yinc < 0 && position.y < 0) {
		std::cout << ">>> test_change_position_debug - TRUE #1" << std::endl;
		return false;
	}
	if (yinc > 0 && position.y > RES_H) {
		return false;
	}
    st_map_colision map_col = map_colision(xinc, yinc, map->getMapScrolling());
    short int mapLock = map_col.block;
	//std::cout << "character::test_change_position - pos.x: " << position.x << ", pos.y: " << position.y << ", xinc: " << xinc << ", yinc: " << yinc << ", mapLock: " << mapLock << std::endl;
	if (mapLock != BLOCK_UNBLOCKED && mapLock != BLOCK_WATER) {
		std::cout << ">>> test_change_position_debug - FALSE #1 - mapLock: " << mapLock << std::endl;
		return false;
	}
	std::cout << ">>> test_change_position_debug - TRUE #2" << std::endl;
	return true;
}

bool character::is_shielded(int projectile_direction) const
{
	if (is_player()) {
        if (input.p1_input[BTN_SHIELD] == 1 && state.animation_type == ANIM_TYPE_SHIELD) { // player is on SHIELD animation and is keeping the shield button pressed
            if (shield_type == SHIELD_FULL || shield_type == SHIELD_FRONT) { // player can use shield
                return true;
            }
        }
		return false;
	} else {
		//std::cout << ">> classnpc::is_shielded[" << name << "] - shield_type: " << shield_type << ", projectile_direction: " << projectile_direction << ", state.direction: " << state.direction << std::endl;
		if (shield_type == SHIELD_FULL || (shield_type == SHIELD_FRONT && projectile_direction != state.direction && (state.animation_type == ANIM_TYPE_STAND || state.animation_type == ANIM_TYPE_WALK)) || (shield_type == SHIELD_STAND && state.animation_type == ANIM_TYPE_STAND)) {
			//std::cout << ">> classnpc::is_shielded[" << name << "] - TRUE" << std::endl;
			return true;
		}
		//std::cout << ">> classnpc::is_shielded[" << name << "] - FALSE" << std::endl;
		return false;
	}
}



short character::get_anim_type() const
{
	return state.animation_type;
}

graphicsLib_gSurface *character::get_char_frame(int direction, int type, int frame)
{
	if (character_graphics_list.find(name) == character_graphics_list.end()) {
		return NULL;
	} else {
		return &(character_graphics_list.find(name)->second)[direction][type][frame].frameSurface;
	}
}

st_color character::get_color_key(short key_n) const
{
	return color_keys[key_n];
}

short character::get_number() const
{
	return _number;
}


bool character::is_using_circle_weapon()
{
    if (projectile_list.size() == 1) {
        if (projectile_list.at(0).get_move_type() == TRAJECTORY_CENTERED) {
            std::cout << "CHAR::is_using_circle_weapon[" << name << "] - TRUE" << std::endl;
            return true;
        }
    }
    return false;
}

void character::inc_effect_weapon_status()
{
    if (projectile_list.size() == 1) {
        int move_type = projectile_list.at(0).get_move_type() ;
        if (move_type == TRAJECTORY_CENTERED || move_type == TRAJECTORY_BOMB) {
            projectile_list.at(0).inc_status();
        }
    }
}

void character::set_animation_type(ANIM_TYPE type)
{
    // if is hit, finish jumping
    if (state.animation_type != type && type == ANIM_TYPE_HIT) {
        _obj_jump.finish();
    }
    if (type != state.animation_type) {
        state.animation_state = 0;
        state.animation_type = type;
        _was_animation_reset = false;
    }
    state.animation_timer = timer.getTimer() + (character_graphics_list.find(name)->second)[state.direction][state.animation_type][state.animation_state].delay;
}

void character::set_show_hp(bool show)
{
    _show_hp = show;
}

void character::set_progressive_appear_pos(int pos)
{
    _progressive_appear_pos = pos;
}

bool character::is_stage_boss()
{
    return _is_stage_boss;
}

bool character::is_weak_to_freeze()
{
    if (freeze_weapon_id == -1) {
        return false;
    }
    int wpn_id = -1;
    for (int i=0; i<MAX_WEAPON_N; i++) {
        if (game_data.weapons[i].id_projectile == freeze_weapon_id) {
            wpn_id = i;
        }
    }
    if (wpn_id == -1) {
        return false;
    }
    //if (game_data.game_npcs[_number].weakness[wpn_id].damage_multiplier == 0) {
    if (GameMediator::get_instance()->get_enemy(_number).weakness[wpn_id].damage_multiplier == 0) {
        return false;
    }
    return true;
}

bool character::can_air_dash()
{
    return false;
}



void character::cancel_slide()
{
    state.slide_distance = 999;
    if (state.animation_type == ANIM_TYPE_SLIDE) {
        if (hit_ground() == true) {
            set_animation_type(ANIM_TYPE_STAND);
        } else {
            set_animation_type(ANIM_TYPE_JUMP);
        }
    }
}

float character::get_hit_push_back_n()
{
    return TILESIZE*0.8;
}

bool character::have_shoryuken()
{
    return false;
}

int character::get_armor_arms_attack_id()
{
    return -1;
}



