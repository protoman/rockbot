// character: movement, jump, slide, gravity and collision (split from character.cpp, no change of behaviour)
#include "character.h"
#include "../game.h"
#include "../timerlib.h"

#include "shareddata.h"

extern game gameControl;
extern timerLib timer;

#include "../soundlib.h"
extern soundLib soundManager;

#include "../inputlib.h"
extern inputLib input;

#include "../game_mediator.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#define STAIR_ANIMATION_WAIT_FRAMES 10
#define STAIRS_GRAB_TIMEOUT 200
#define SLIDE_Y_ADJUST 12


extern bool GAME_FLAGS[FLAG_COUNT];

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern int freeze_weapon_id;


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
// left key: turn, walk, slide/stairs turn. Returns false when charMove must end this frame (as it did with the old early return)
bool character::move_horizontal_left(float temp_move_speed, bool &moved)
{
    int mapLock = 0;
	if (moveCommands.left == 1 && position.x > 0 && state.animation_type() != ANIM_TYPE_SLIDE && is_in_stairs_frame() == false) {
        // check inverting direction
        if (state.animation_type() != ANIM_TYPE_HIT && state.direction() != ANIM_DIRECTION_LEFT) {
            set_direction(ANIM_DIRECTION_LEFT);
            return false;
        }

        for (float i=temp_move_speed; i>=0.1; i--) {
            st_map_collision map_col = map_collision(-i, 0, gameControl.get_current_map_obj()->getMapScrolling());
            mapLock = map_col.block;
            if (state.animation_type() == ANIM_TYPE_HIT) {
                hit_moved_back_n += temp_move_speed;
            }
            if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y || mapLock == BLOCK_QUICKSAND) {
                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_Y) {
                    position.x -= i + gameControl.get_current_map_obj()->get_last_scrolled().x;
                } else if (mapLock == BLOCK_WATER) {
                    position.x -= i*WATER_SPEED_MULT + gameControl.get_current_map_obj()->get_last_scrolled().x;
                } else if (mapLock == BLOCK_QUICKSAND) {
                    position.x -= i*QUICKSAND_SPEED_MULT + gameControl.get_current_map_obj()->get_last_scrolled().x;
                }
				if (state.animation_type() != ANIM_TYPE_HIT) {
                    if (is_player() && state.direction() == ANIM_DIRECTION_RIGHT && is_in_stairs_frame() != true) {
                        position.x -= PLAYER_RIGHT_TO_LEFT_DIFF;
                    }
                    set_direction(ANIM_DIRECTION_LEFT);
				} else {
                    gravity(false);
                    return false;
				}
                if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_WALK_ATTACK) {
					state.set_animation_timer(0);
                }
                if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_JUMP && state.animation_type() != ANIM_TYPE_SLIDE && state.animation_type() != ANIM_TYPE_JUMP_ATTACK && state.animation_type() != ANIM_TYPE_HIT && (state.animation_type() != ANIM_TYPE_WALK_ATTACK || (state.animation_type() == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                    set_animation_type(ANIM_TYPE_WALK);
				}
				moved = true;
				break;
			}
		}
	}
    if (moveCommands.right != 1 && moveCommands.left == 1 && state.direction() != ANIM_DIRECTION_LEFT && (state.animation_type() == ANIM_TYPE_SLIDE || is_in_stairs_frame() == true)) {
        if (is_player() && state.direction() == ANIM_DIRECTION_RIGHT && is_in_stairs_frame() != true) {
            position.x -= PLAYER_RIGHT_TO_LEFT_DIFF;
        }
        set_direction(ANIM_DIRECTION_LEFT);
    }
    return true;
}

// right key: same as the left one
bool character::move_horizontal_right(float temp_move_speed, bool &moved)
{
    int mapLock = 0;
    if (moveCommands.left != 1 && moveCommands.right == 1 && state.animation_type() != ANIM_TYPE_SLIDE && is_in_stairs_frame() == false) {
        if (state.animation_type() != ANIM_TYPE_HIT && state.direction() != ANIM_DIRECTION_RIGHT) {
            set_direction(ANIM_DIRECTION_RIGHT);
            return false;
        }
        if (state.animation_type() == ANIM_TYPE_HIT) {
            hit_moved_back_n += temp_move_speed;
        }

        if (is_on_quicksand()) {
            temp_move_speed = QUICKSAND_JUMP_LIMIT/2;
        }

        for (float i=temp_move_speed; i>=0.1; i--) {
            // (moving left has no "too small to change a pixel" skip, and this one made right-side moves under 1px,
            // like the hit knockback, travel less than left-side ones: both sides now move the same)
            if (is_player() == false || (realPosition.x + i + frameSize.width/2) < RES_W) {
                st_map_collision map_col = map_collision(i, 0, gameControl.get_current_map_obj()->getMapScrolling());
                mapLock = map_col.block;

                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y || mapLock == BLOCK_QUICKSAND) {
                    if (mapLock == TERRAIN_UNBLOCKED || mapLock == BLOCK_Y) {
                        position.x += i - gameControl.get_current_map_obj()->get_last_scrolled().x;
                    } else if (mapLock == BLOCK_WATER) {
                        position.x += i*WATER_SPEED_MULT - gameControl.get_current_map_obj()->get_last_scrolled().x;
                    } else if (mapLock == BLOCK_QUICKSAND) {
                        position.x += i*QUICKSAND_SPEED_MULT + gameControl.get_current_map_obj()->get_last_scrolled().x;
                    }
                    if (state.animation_type() != ANIM_TYPE_HIT) {
                        if (is_player() && state.direction() == ANIM_DIRECTION_LEFT && is_in_stairs_frame() != true) {
                            position.x += PLAYER_RIGHT_TO_LEFT_DIFF;
                        }
                        set_direction(ANIM_DIRECTION_RIGHT);
                    } else {
                        gravity(false);
                        return false;
                    }
                    if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_WALK_ATTACK) {
                        state.set_animation_timer(0);
                    }
                    if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_JUMP && state.animation_type() != ANIM_TYPE_SLIDE && state.animation_type() != ANIM_TYPE_JUMP_ATTACK && state.animation_type() != ANIM_TYPE_HIT && (state.animation_type() != ANIM_TYPE_WALK_ATTACK || (state.animation_type() == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                        set_animation_type(ANIM_TYPE_WALK);
                    }
                    moved = true;
                    break;
                } else {
                    moved = false;
                }
            }
		}
	}
	if (moveCommands.right == 1 && state.direction() != ANIM_DIRECTION_RIGHT && (state.animation_type() == ANIM_TYPE_SLIDE || is_in_stairs_frame() == true)) {
        if (is_player() && state.direction() == ANIM_DIRECTION_LEFT && is_in_stairs_frame() != true) {
            position.x += PLAYER_RIGHT_TO_LEFT_DIFF;
        }
        set_direction(ANIM_DIRECTION_RIGHT);
    }
    return true;
}

// sliding on ice / after leaving ice
void character::move_ice_inertia(bool moved, int bottom_point_lock)
{
    int mapLock = 0;
    // Ice inertia
    if (bottom_point_lock == TERRAIN_ICE || (_inertia_obj.is_started() && (bottom_point_lock == TERRAIN_UNBLOCKED|| bottom_point_lock == TERRAIN_WATER))) {

        if (moved == true) {
            if (moveCommands.right == 1) {
                st_map_collision map_col = map_collision(1, 0, gameControl.get_current_map_obj()->getMapScrolling());
                mapLock = map_col.block;
                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                    position.x++;
                }
            } else if (moveCommands.left == 1) {
                st_map_collision map_col = map_collision(-1, 0, gameControl.get_current_map_obj()->getMapScrolling());
                mapLock = map_col.block;
                if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                    position.x--;
                }
            }
            _inertia_obj.start();
        } else if (moveCommands.left == 0 && moveCommands.right == 0) {
            int inertia_xinc = _inertia_obj.execute();
            if (inertia_xinc != 0) {
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    if (position.x - inertia_xinc < 0) {
                        _inertia_obj.stop();
                    } else {
                        st_map_collision map_col = map_collision(-inertia_xinc, 0, gameControl.get_current_map_obj()->getMapScrolling());
                        mapLock = map_col.block;
                        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                            position.x -= inertia_xinc;
                        }
                    }
                } else {
                    if (realPosition.x+inertia_xinc > RES_W) {
                        _inertia_obj.stop();
                    } else {
                        st_map_collision map_col = map_collision(inertia_xinc, 0, gameControl.get_current_map_obj()->getMapScrolling());
                        mapLock = map_col.block;
                        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
                            position.x += inertia_xinc;
                        }
                    }
                }
            }
        }
        if (gameControl.get_current_map_obj() != NULL) {
            gameControl.update_stage_scrolling();
        }
        char_update_real_position();
    } else if (bottom_point_lock != TERRAIN_WATER) {
        _inertia_obj.stop();
    }

    if (_obj_jump.is_started() == true) {
        _inertia_obj.stop();
    }
}

// climbing/descending stairs and their animations
void character::move_stairs_vertical(float temp_move_speed, int bottom_point_lock)
{
	// check if character is on stairs
    if (moveCommands.up == 1 && state.animation_type() != ANIM_TYPE_STAIRS_ATTACK) { // check stairs on middle
        st_position stairs_pos = is_on_stairs(st_rectangle(position.x, position.y+(frameSize.height/2)-2, frameSize.width, frameSize.height/2-2));
        int top_terrain = gameControl.get_current_map_obj()->getMapPointLock(st_position(((stairs_pos.x * TILESIZE - 6)+frameSize.width/2)/TILESIZE, position.y/TILESIZE));
        if (stairs_pos.x != -1) {
            if (state.animation_type() != ANIM_TYPE_STAIRS_MOVE && _stairs_falling_timer < timer.getTimer()) {
                set_animation_type(ANIM_TYPE_STAIRS_MOVE);
			}
            if (is_in_stairs_frame() && (top_terrain == TERRAIN_UNBLOCKED || top_terrain == TERRAIN_WATER || top_terrain == TERRAIN_STAIR)) {
                position.y -= temp_move_speed * STAIRS_MOVE_MULTIPLIER;
                position.x = stairs_pos.x * TILESIZE - (frameSize.width-TILESIZE)/2;
            }
		// out of stairs
		} else {
            int map_terrain = gameControl.get_current_map_obj()->getMapPointLock(st_position(((position.x+frameSize.width/2)/TILESIZE), ((position.y+frameSize.height-4)/TILESIZE)));
            if (_dropped_from_stairs == false && map_terrain == TERRAIN_STAIR) { // check stairs bottom (leaving)
                set_animation_type(ANIM_TYPE_STAIRS_SEMI);
                position.y -= temp_move_speed * STAIRS_MOVE_MULTIPLIER;
			} else if (state.animation_type() == ANIM_TYPE_STAIRS_SEMI) {
                set_animation_type(ANIM_TYPE_STAND);
                position.y -= 2;
			}
		}
    }


    if (moveCommands.down == 1 && state.animation_type() != ANIM_TYPE_SLIDE && state.animation_type() != ANIM_TYPE_STAIRS_ATTACK) {
        st_position stairs_pos_center = is_on_stairs(st_rectangle(position.x, position.y+frameSize.height/2, frameSize.width, frameSize.height/2));
        bool is_already_on_stairs = is_in_stairs_frame();
        /// @TODO - check that move-speed/2 is not zero

        // is on stairs
        if (is_already_on_stairs == true) {
            // if frame is semi, but already entered whole body, change to full-stairs frame
            if (state.animation_type() == ANIM_TYPE_STAIRS_SEMI && stairs_pos_center.x != -1 && _stairs_falling_timer < timer.getTimer()) {
                set_animation_type(ANIM_TYPE_STAIRS_MOVE);
            }

            // check that path is clear to move
            if (is_in_stairs_frame() && (bottom_point_lock == TERRAIN_WATER || bottom_point_lock == TERRAIN_UNBLOCKED || bottom_point_lock == TERRAIN_STAIR)) {
                position.y += temp_move_speed * STAIRS_MOVE_MULTIPLIER;
            }

            // if bottom point is not stairs, leave it
            if (bottom_point_lock != TERRAIN_STAIR) {
                if (stairs_pos_center.x == -1 && (bottom_point_lock == TERRAIN_UNBLOCKED || bottom_point_lock == TERRAIN_WATER)) {
                    set_animation_type(ANIM_TYPE_JUMP);
                }
                if (bottom_point_lock != TERRAIN_UNBLOCKED && bottom_point_lock != TERRAIN_WATER) {
                    set_animation_type(ANIM_TYPE_STAND);
                }
            }
        // not in stairs, but over it
        } else {
            if (stairs_pos_center.x == -1 && bottom_point_lock == TERRAIN_STAIR) {
                // over stairs, enter it
                st_position stairs_pos_bottom = is_on_stairs(st_rectangle(position.x, position.y+frameSize.height, frameSize.width, frameSize.height/2));
                if (stairs_pos_bottom.x != -1) {
                    set_animation_type(ANIM_TYPE_STAIRS_SEMI);
                    position.y += temp_move_speed * STAIRS_MOVE_MULTIPLIER;
                    position.x = stairs_pos_bottom.x * TILESIZE - (frameSize.width-TILESIZE)/2;
                }
            }
        }

    }

	// is on stairs without moving
	if (moveCommands.down == 0 && moveCommands.up == 0 && state.animation_type() == ANIM_TYPE_STAIRS_MOVE) {
        _stairs_stopped_count++;
        if (_stairs_stopped_count > STAIR_ANIMATION_WAIT_FRAMES) {
            set_animation_type(ANIM_TYPE_STAIRS);
        }
    } else if ((moveCommands.down != 0 || moveCommands.up != 0) && _stairs_falling_timer < timer.getTimer()) {
        _stairs_stopped_count = 0;
        if (state.animation_type() == ANIM_TYPE_STAIRS) {
            set_animation_type(ANIM_TYPE_STAIRS_MOVE);
        }
    }
}

void character::charMove() {
    int mapLock = 0;
	bool moved = false;
    float temp_move_speed = move_speed;

    // store previous position
    store_previous_position();

    if (timer.is_paused() == true) {
        return;
    }


    bool did_hit_ground = hit_ground();

    if (_dashed_jump == true) {
        if (state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) {
            temp_move_speed = move_speed * 2;
            if (did_hit_ground == true) {
                _dashed_jump = false;
            }
        }
    }
    if (did_hit_ground == true) {
        _can_execute_airdash = true;
        _air_dashed = false;
    }



    if (gameControl.get_current_map_obj() == NULL) {
		return; // error - can't execute this action without an associated map
	}

    int water_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y+6)/TILESIZE));
    if (is_player() == true && water_lock == TERRAIN_WATER) {
        gameControl.get_current_map_obj()->add_bubble_animation(st_position(realPosition.x+frameSize.width/2, position.y+6));
    }

    int map_point_x = (position.x+frameSize.width/2)/TILESIZE;
    int map_point_y = (position.y+frameSize.height)/TILESIZE;
    int bottom_point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(map_point_x, map_point_y));

	if (state.frozen == true) {
		if (state.frozen_timer < timer.getTimer()) {
			state.frozen = false;
		} else { // if is player, check collision agains NPCs to leave freeze
			if (is_player()) {
                npc_touch_damage();
			}
		}
		return;
	}

	if (state.animation_type() == ANIM_TYPE_TELEPORT) {
		gravity(false);
		return;
	}

    if (state.animation_type() == ANIM_TYPE_HIT) {

        if (hit_moved_back_n < get_hit_push_back_n()) {

            if (state.direction() == ANIM_DIRECTION_LEFT) {
                moveCommands.left = 0;
                moveCommands.right = 1;
            } else {
                moveCommands.left = 1;
                moveCommands.right = 0;
            }
            moveCommands.up = 0;
            moveCommands.down = 0;
            moveCommands.jump = 0;
            if (did_hit_ground == true) {
                temp_move_speed = 0.5;
            } else {
                temp_move_speed = 0.8;
            }

        } else {
            set_animation_type(ANIM_TYPE_STAND);
			hit_moved_back_n = 0;
		}
	}

    if (move_horizontal_left(temp_move_speed, moved) == false) {
        return;
    }
    if (move_horizontal_right(temp_move_speed, moved) == false) {
        return;
    }

    move_ice_inertia(moved, bottom_point_lock);

    move_stairs_vertical(temp_move_speed, bottom_point_lock);

    check_reset_stand();

    if (is_player() == false) {
        character::attack(false, 0, false);
    }

    bool res_slide = slide(gameControl.get_current_map_obj()->getMapScrolling());

    bool resJump = false;
    resJump = jump(moveCommands.jump, gameControl.get_current_map_obj()->getMapScrolling());
    if (resJump == true || res_slide == true) {
        if (state.animation_type() == ANIM_TYPE_HIT) {
            gravity(false);
        }
    /// @TODO: removed a gravity from here in an ELSE. Hope it was not necessary
	}


    if (is_player_type && moved == false && resJump == false && res_slide == false) {
		//if (state.animation_type() != ANIM_TYPE_WALK) {
         if (is_in_stairs_frame() == false && state.animation_type() != ANIM_TYPE_STAND && state.animation_type() != ANIM_TYPE_JUMP && state.animation_type() != ANIM_TYPE_JUMP_ATTACK && state.animation_type() != ANIM_TYPE_TELEPORT && state.animation_type() != ANIM_TYPE_SHIELD && state.animation_type() != ANIM_TYPE_HIT && state.animation_type() != ANIM_TYPE_SLIDE && (is_on_attack_frame() == false || (is_on_attack_frame() == true && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
            set_animation_type(ANIM_TYPE_STAND);
		}
		if (state.animation_type() == ANIM_TYPE_HIT && timer.getTimer() > hit_duration/2+last_hit_time) { // finished hit time
            set_animation_type(ANIM_TYPE_STAND);
		}
	}

    if (_dropped_from_stairs == true) {
        if (timer.getTimer() > hit_duration+last_hit_time) {
            _dropped_from_stairs = false;
        } else if (hit_ground() == true) {
            _dropped_from_stairs = false;
        }
    }


    gameControl.get_current_map_obj()->reset_scrolled();
    _previous_position = position;

}

void character::store_previous_position()
{
    previous_position_list.push_back(position);
    if (previous_position_list.size() > PREVIOUS_FRAMES_MAX) {
        previous_position_list.erase(previous_position_list.begin());
    }
}

void character::clear_move_commands()
{
	moveCommands.up = 0;
	moveCommands.down = 0;
	moveCommands.left = 0;
	moveCommands.right = 0;
	moveCommands.attack = 0;
	moveCommands.jump = 0;
    moveCommands.start = 0;
}

void character::reset_jump()
{
    _obj_jump.finish();
}

void character::reset_gravity_speed()
{
    accel_speed_y = 0.25;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
// gravity of enemies/bosses: they only fall when they cannot fly (or fly but may fall while moving)
bool character::npc_gravity(bool boss_demo_mode, int gravity_max_speed)
{
    if (_ignore_gravity == true) {
        return false;
    }

    if (can_fly == false || can_fall_during_move == true || (_is_boss && SharedData::get_instance()->is_showing_boss_intro == true)) {
        bool is_moved = false;
        short int limit_speed = move_speed;
			if (boss_demo_mode == true) {
            limit_speed = gravity_max_speed;
			}
        if (limit_speed < 1) {
            limit_speed = 1;
        }

			for (int i=limit_speed; i>0; i--) {
            bool res_test_move = test_change_position(0, i);
            //if (i > 0) std::cout << "CHAR::GRAVITY[PLAYER] - res_test_move[" << res_test_move << "], yinc[" << i << "]" << std::endl;
            if ((boss_demo_mode == true && position.y <= TILESIZE*2) || res_test_move == true) {
                position.y += i;
					is_moved = true;
					break;
				}
        }
			return is_moved;
    }
    reset_gravity_speed();
    return false; // not moved because of IA type
}

// player falling: acceleration, map collision per pixel, water/quicksand, landing
bool character::player_fall(int gravity_max_speed)
{

    // tem que inicializar essa variável sempre que for false
    accel_speed_y = accel_speed_y + accel_speed_y*gravity_y;

    if (accel_speed_y < 0.25) {
        accel_speed_y = 0.25;
    } else if (accel_speed_y > gravity_max_speed) {
        accel_speed_y = gravity_max_speed;
    }

    int adjusted_speed = accel_speed_y;
    if (adjusted_speed < 1) {
        adjusted_speed = 1;
    }



		if (state.animation_type() == ANIM_TYPE_TELEPORT) {

        if (_teleport_minimal_y - position.y > TILESIZE) {
            adjusted_speed = gravity_max_speed;
        } else {
            adjusted_speed = gravity_max_speed/2;
        }
		}

    st_map_collision map_col;
    bool was_moved = false;
    for (int i=adjusted_speed; i>0; i--) {
        map_col = map_collision(0, i, gameControl.get_current_map_obj()->getMapScrolling());
        int mapLock = map_col.block;

			if (state.animation_type() == ANIM_TYPE_TELEPORT && position.y < _teleport_minimal_y-TILESIZE) {
				mapLock = BLOCK_UNBLOCKED;
        } else if (!is_player() && state.animation_type() == ANIM_TYPE_TELEPORT && position.y >= _teleport_minimal_y-TILESIZE) {
            _teleport_minimal_y = frameSize.height+TILESIZE*2; // RESET MIN_Y -> remove limit for next telepor
        } else if (position.y+frameSize.height >= RES_H) { // out of screen
				mapLock = BLOCK_UNBLOCKED;
			}

        if (_platform == NULL || (_platform != NULL && _platform->get_type() != OBJ_MOVING_PLATFORM_UPDOWN && _platform->get_type() != OBJ_MOVING_PLATFORM_UP_LOOP && _platform->get_type() != OBJ_MOVING_PLATFORM_DOWN)) {
            if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_STAIR_X || mapLock == BLOCK_STAIR_Y) {
                if (mapLock != BLOCK_WATER || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1)) {
                    position.y += i;
                } else {
                    position.y += i*WATER_SPEED_MULT;
                }
                if (state.animation_type() != ANIM_TYPE_JUMP && state.animation_type() != ANIM_TYPE_JUMP_ATTACK && state.animation_type() != ANIM_TYPE_TELEPORT && state.animation_type() != ANIM_TYPE_SLIDE && state.animation_type() != ANIM_TYPE_HIT && (state.animation_type() != ANIM_TYPE_JUMP_ATTACK || (state.animation_type() == ANIM_TYPE_JUMP_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                    set_animation_type(ANIM_TYPE_JUMP);
                }
                was_moved = true;

                if (state.animation_type() != ANIM_TYPE_TELEPORT) {
                    _is_falling = true;
                }
                break;
            } else if (map_col.terrain_type == TERRAIN_QUICKSAND) {
                //std::cout << "CHAR::GRAVITY - over QUICKSAND" << std::endl;
                position.y += QUICKSAND_GRAVITY;
            }
        }
			if (i == 1) {
            reset_gravity_speed();
			}
		}

		if (was_moved == false && (state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) && state.animation_type() != ANIM_TYPE_SLIDE) {
        set_animation_type(ANIM_TYPE_STAND);
			return true;
		} else if (was_moved == false && state.animation_type() == ANIM_TYPE_TELEPORT && position.y >= RES_H/3) {
        set_animation_type(ANIM_TYPE_STAND);
			return true;
		}
    if (was_moved == false && _is_falling == true) {
        _is_falling = false;
        if (is_player()) {
            set_animation_type(ANIM_TYPE_STAND);
            soundManager.play_sfx(SFX_PLAYER_JUMP);
        }
    }

    check_platform_move(map_col.terrain_type);

    return false;
}

// touching an enemy: touch damage (small or big, by comparing heights) and the enemy knows it hit the player
void character::npc_touch_damage()
{
    classnpc* npc_touch = gameControl.get_current_map_obj()->collision_player_npcs(this, 0, 0);
    if (npc_touch != NULL) {
        if (npc_touch->get_size().height > this->get_size().height) {
            damage(TOUCH_DAMAGE_SMALL, false);
        } else {
            damage(TOUCH_DAMAGE_BIG, false);
        }
        if (_was_hit == true) {
            npc_touch->hit_player();
        }
    }
}

bool character::gravity(bool boss_demo_mode=false)
{
    if (_progressive_appear_pos != 0) {
        reset_gravity_speed();
        return false;
    }

    if (!gameControl.get_current_map_obj()) {
        std::cout << "ERROR: can't execute gravity without a map" << std::endl;
        reset_gravity_speed();
        return false; // error - can't execute this action without an associated map
	}


    bool can_use_air_dash = false;
    if (is_player() == true) {
        can_use_air_dash = can_air_dash();
    }

    if (can_use_air_dash == true && state.animation_type() == ANIM_TYPE_SLIDE) {
        reset_gravity_speed();
        return false;
    }

    if ((_is_boss || _is_stage_boss) && get_anim_type() == ANIM_TYPE_INTRO) {
        return false;
    }

    if (is_player() && state.animation_type() == ANIM_TYPE_TELEPORT) {
        return false;
    }

    if (!is_player() && teleporting_out > 0) {
        return false;
    }

    if (is_player()  == false && (_number >= 0 && _number < (int)GameMediator::get_instance()->ai_list.size())) {
        if (game_data.final_boss_id == _number || (GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_DEAD].action ==  AI_ACTION_REPLACE_NPC && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_DEAD].extra_parameter == game_data.final_boss_id)) {
            _is_final_game_boss = true;
        }
    }

    int gravity_max_speed = GRAVITY_MAX_SPEED * SharedData::get_instance()->get_movement_multiplier();
    if (state.animation_type() == ANIM_TYPE_TELEPORT) {
        gravity_max_speed = GRAVITY_TELEPORT_MAX_SPEED * SharedData::get_instance()->get_movement_multiplier();
    } else if (state.animation_type() == ANIM_TYPE_HIT) {
        gravity_max_speed = 2 * SharedData::get_instance()->get_movement_multiplier();
    }

	// ------------- NPC gravity ------------------ //
	if (!is_player()) {
        return npc_gravity(boss_demo_mode, gravity_max_speed);
	}


	// ------------ PLAYER gravity --------------------- //
    if (is_player() && position.y > RES_H+TILESIZE) {
		hitPoints.current = 0;
		death();
        reset_gravity_speed();
        return false;
	}


	if (is_in_stairs_frame()) {
        //character* playerObj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y
        npc_touch_damage();
        reset_gravity_speed();
        return false;
	}

    if (_obj_jump.is_started() == false && can_fly == false && position.y < RES_H+TILESIZE+1 + frameSize.height) {
        return player_fall(gravity_max_speed);
    }
    return false;
}

void character::activate_super_jump()
{
    _super_jump = true;
}

void character::activate_force_jump()
{
	_force_jump = true;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
// pressing the dash button on the ground (or in the air once): starts the slide / air dash
void character::start_slide(bool did_hit_ground)
{
    if (state.animation_type() != ANIM_TYPE_SLIDE && _dash_button_released == true) {
        if (moveCommands.dash == 1) {
            if (did_hit_ground == true || (did_hit_ground == false && _can_execute_airdash == true && position.y >= 0)) { // no air dash above the top of the screen
                _can_execute_airdash = false;
                _air_dashed = (did_hit_ground == false);
                set_animation_type(ANIM_TYPE_SLIDE);
                state.slide_distance = 0;
                _dash_button_released = false;
                int adjust_x = -3;
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    adjust_x = frameSize.width+3;
                }
                previous_position_list.clear();
                gameControl.get_current_map_obj()->add_animation(ANIMATION_STATIC, &graphLib.dash_dust, position, st_position(adjust_x, frameSize.height-8), 160, 0, state.direction(), st_size(8, 8));
            }
		}
    }
}

// the slide cannot go on at the screen or map edges (and the distance is reset). Returns true when blocked
bool character::slide_blocked_at_edge()
{
    // check if trying to leave screen LEFT
    if (state.direction() == ANIM_DIRECTION_LEFT && position.x <= 0) {
        state.slide_distance = 0;
        return true;
    }

    // check if trying to leave screen RIGHT
    if (is_player() == true && (realPosition.x + frameSize.width/2) > RES_W) {
        state.slide_distance = 0;
        return true;
    }

    // end of map
    if (state.direction() == ANIM_DIRECTION_RIGHT && position.x + frameSize.width > MAP_W * TILESIZE) {
        state.slide_distance = 0;
        return true;
    }
    return false;
}

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

    if (state.animation_type() == ANIM_TYPE_TELEPORT || state.animation_type() == ANIM_TYPE_SHIELD) {
        return false;
    }

    if ((state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) && can_air_dash() == false) {
        return false;
    }


	if (is_in_stairs_frame()) {
		return false;
	}

	// no need to slide
    if (state.animation_type() != ANIM_TYPE_SLIDE && moveCommands.dash != 1) {
		return false;
    }

    if (position.x <= 0 && state.direction() == ANIM_DIRECTION_LEFT) {
        set_animation_type(ANIM_TYPE_JUMP);
        state.slide_distance = 0;
        return false;
    }


    bool did_hit_ground = hit_ground();
    int adjust = -1;
    if (slide_type == 1) { // if is slide-type, use greater adjust
        adjust = -TILESIZE;
    }
    //st_map_collision character::map_collision(const float incx, const short incy, st_float_position mapScrolling, int hitbox_anim_type)
    st_map_collision map_col = map_collision(0, adjust, gameControl.get_current_map_obj()->getMapScrolling(), ANIM_TYPE_SLIDE); // slide_adjust is used because of adjustments in slide collision
    int map_lock =  map_col.block;

    /*
    // player have double jump (without being armor) can't use slide in ground
    if (GameMediator::get_instance()->player_list_v3_1[_number].can_double_jump) {
        if (did_hit_ground == true) {
            return false;
        }
    }
    */


    int map_lock_above = gameControl.getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y)/TILESIZE));
    bool is_blocked_above = !(map_lock_above == TERRAIN_UNBLOCKED || map_lock_above == TERRAIN_WATER);

    // releasing down (or dash button) interrupts the slide
    if (moveCommands.dash != 1 && state.animation_type() == ANIM_TYPE_SLIDE && (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER) && !is_blocked_above) {
        if (did_hit_ground) {
            set_animation_type(ANIM_TYPE_STAND);
        } else {
            set_animation_type(ANIM_TYPE_JUMP);
        }
        return false;
    }


	if (state.slide_distance > TILESIZE*5 && (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER)) {
        if (did_hit_ground == true) {
            set_animation_type(ANIM_TYPE_STAND);
        } else {
            set_animation_type(ANIM_TYPE_JUMP);
        }
        state.slide_distance = 0;
        return false;
    }

    start_slide(did_hit_ground);

    if (state.animation_type() != ANIM_TYPE_SLIDE) {
        return false;
    }

    // if there is no ground, interrupts slide
    st_map_collision map_col_fall = map_collision(0, 1, gameControl.get_current_map_obj()->getMapScrolling());
    int fall_map_lock = map_col_fall.block;

    if (can_air_dash() == false && (fall_map_lock == BLOCK_UNBLOCKED || fall_map_lock == BLOCK_WATER)) {
        set_animation_type(ANIM_TYPE_JUMP);
		state.slide_distance = 0;
		return false;
	}

    if (slide_blocked_at_edge() == true) {
        return false;
    }

    float res_move_x = 0;
    int mapLockAfter = BLOCK_UNBLOCKED;
    _obj_jump.finish();

    // reduce progressively the jump-move value in oder to deal with collision
    float max_speed = move_speed * 2.5;
    for (float i=max_speed; i>0.0; i--) {

        int temp_i;
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            temp_i = -i;
        } else {
            temp_i = i;
        }
        st_map_collision map_col = map_collision(temp_i, 0, mapScrolling);;
        mapLockAfter = map_col.block;

        if (mapLockAfter == BLOCK_UNBLOCKED) {
            res_move_x = temp_i;
            break;
        } else if (mapLockAfter == BLOCK_WATER) {
            res_move_x = temp_i*0.8;
            break;
        }
    }

    if (res_move_x != 0 && (mapLockAfter == BLOCK_UNBLOCKED || mapLockAfter == BLOCK_WATER)) {
        position.x += res_move_x;
		state.slide_distance += abs((float)res_move_x);
    } else if (!is_blocked_above || slide_type != 1) {
        set_animation_type(ANIM_TYPE_JUMP);
        state.slide_distance = 0;
        return false;
    }

    return true;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
// jump button (or a forced jump, like the coil): leaves a ladder, or starts the jump / double jump / super jump.
// Returns true when jump() must end right away (the jump from a ladder is a fall)
bool character::start_jump(int jumpCommandStage, int water_lock)
{
    if (_force_jump == true || (jumpCommandStage == 1 && jump_button_released == true)) {

        if (is_in_stairs_frame()) {
            if (_obj_jump.is_started() == false) {
                set_animation_type(ANIM_TYPE_JUMP);
                _is_falling = true;
                _stairs_falling_timer = timer.getTimer() + STAIRS_GRAB_TIMEOUT; // avoid player entering stairs immediatlly after jumping from it
                return true;
            } else {
                _obj_jump.interrupt();
                if (_force_jump == true) {
                    _force_jump = false;
                }
            }
        } else {
            // after an air dash there is no new jump until landing (or grabbing a ladder)
            if (_air_dashed == true && hit_ground() == false) {
                jump_button_released = false;
            } else if (position.y < 0 && _obj_jump.is_started() == true) {
                // no double jump while above the top of the screen (sky)
                jump_button_released = false;
            } else if (_is_falling == false && (_obj_jump.is_started() == false || (character_max_jump_number > _obj_jump.get_max_jump_number()))) {
                if (_super_jump == true) {
                    _super_jump = false;
                    //std::cout << "### CHARACTER::SUPER-JUMP #1" << std::endl;
                    set_platform(NULL);
                    _obj_jump.start(true, water_lock);
                } else {
                    //std::cout << "### CHARACTER::REMOVE PLATFORM #2 - character_max_jump_number[" << character_max_jump_number << "], jump.count[" << _obj_jump.get_max_jump_number() << "]"<< std::endl;
                    set_platform(NULL);
                    if (_obj_jump.is_started() == false || character_max_jump_number > _obj_jump.get_max_jump_number()) { // fixed coil not jumping high (was starting a new jump)
                        //std::cout << "## CHARACTER::JUMP START #1" << std::endl;
                        _obj_jump.start(false, water_lock);
                    }
                }
                if (state.animation_type() == ANIM_TYPE_SLIDE && slide_type == 0) {
                    _dashed_jump = true;
                }
                set_animation_type(ANIM_TYPE_JUMP);
                jump_button_released = false;
            }
        }
    }
    return false;
}

bool character::jump(int jumpCommandStage, st_float_position mapScrolling)
{
    // @TODO - can only jump again once set foot on land
    if (jumpCommandStage == 0 && jump_button_released == false) {
        jump_button_released = true;
    }

    if (state.animation_type() == ANIM_TYPE_HIT) {
        return false;
    }

    // can't jump while on air dash
    if (state.animation_type() == ANIM_TYPE_SLIDE && hit_ground() == false) {
        return false;
    }

    // grabbing a ladder gives the jump back
    if (is_in_stairs_frame()) {
        _air_dashed = false;
    }

    if (timer.getTimer() < jump_lock_timer) { // some effect blocked jumping
        //std::cout << ">>>>>>>>>>>>>>>>> jump blocked[" << name << "]. Lock-Time: [" << jump_lock_timer << "], timer: {" << timer.getTimer() << "]" << std::endl;
        return false;
    }

    int water_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (position.y+6)/TILESIZE));

    if (start_jump(jumpCommandStage, water_lock) == true) {
        return false;
    }

    bool is_onquicksand = false;
    if (is_on_quicksand()) {
        is_onquicksand = true;
    }


    if (_obj_jump.is_started() == true) {
        float jump_speed = _obj_jump.get_speed();
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
        float abs_jump_speed = abs(jump_speed);
        for (float i=abs_jump_speed; i>0.0; i--) {
            int speed_y = 0;
            if (jump_speed > 0) {
                speed_y = i;
            } else {
                speed_y = i*-1;
            }
            st_map_collision map_col = map_collision(0, speed_y, mapScrolling);
            int map_lock = map_col.block;
            //std::cout << "CHAR::JUMP - speed[" << i << "], map_lock[" << map_lock << "]" << std::endl;

            if (abs(speed_y) > 0 && (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER || map_lock == BLOCK_QUICKSAND)) {
                position.y += speed_y;
                jump_last_moved = speed_y;
                jump_moved = true;
                break;
            }
        }
        //std::cout << "CHAR::JUMP - jump_speed[" << jump_speed << "], jump_last_moved[" << jump_last_moved << "], abs_jump_speed[" << abs_jump_speed << "], jump_moved[" << jump_moved << "], position.y[" << position.y << "]" << std::endl;
        if (jump_speed != 0 && jump_moved == false) {
            if (jump_speed < 0) {
                //std::cout << "CHAR::JUMP - INTERRUMP #1" << std::endl;
                _obj_jump.interrupt();
            } else if (hit_ground() == true) {
                //std::cout << "CHAR::JUMP - FINISH" << std::endl;
                _obj_jump.finish();
            }
            if (position.y < -frameSize.width) {
                //std::cout << "CHAR::JUMP - ADJUST" << std::endl;
                position.y += TILESIZE;
            }
            // landing on the coil during a jump sets super+force jump in this very step: keep it, or the big jump is lost
            if (_super_jump == false) {
                _force_jump = false;
            }
        }
        if (_obj_jump.is_started() == true && jump_speed < 0.0 && abs(jump_speed) < 1.0 && abs(jump_last_moved) < 1.0) {
//            std::cout << "CHAR::JUMP - INTERRUMP #1" << std::endl;
            _obj_jump.interrupt();
        }

        _obj_jump.execute(water_lock);
        if (_obj_jump.is_started() == false) {
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            if (_force_jump == true) {
                _force_jump = false;
            }
        } else {
            if (is_player() && position.y > RES_H+1) {
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

void character::check_reset_stand()
{
    if (!is_player()) { // NPCs do not need this
        return;
    }
    // is walking without moving, reset to stand
    if (moveCommands.left == 0 && moveCommands.right == 0) {
        if (state.animation_type() == ANIM_TYPE_WALK) {
            //std::cout << "CHAR - RESET STAND #1" << std::endl;
            set_animation_type(ANIM_TYPE_STAND);
        } else if (state.animation_type() == ANIM_TYPE_WALK_ATTACK) {
            set_animation_type(ANIM_TYPE_ATTACK);
        }
    }
    if ((state.animation_type() == ANIM_TYPE_ATTACK || state.animation_type() == ANIM_TYPE_WALK_ATTACK || state.animation_type() == ANIM_TYPE_JUMP_ATTACK || state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_DOWN || state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_UP) && timer.getTimer() > state.attack_timer+500) {
        switch (state.animation_type()) {
            case ANIM_TYPE_WALK_ATTACK:
                set_animation_type(ANIM_TYPE_WALK);
                //std::cout << "CHAR - RESET STAND #2" << std::endl;
                break;
            case ANIM_TYPE_JUMP_ATTACK:
                set_animation_type(ANIM_TYPE_JUMP);
                break;
            default:
                //std::cout << "CHAR - RESET STAND #3" << std::endl;
                set_animation_type(ANIM_TYPE_STAND);
                break;
        }

    }

}

// ********************************************************************************************** //
// set the object platform player is over, if any                                                 //
// ********************************************************************************************** //
void character::set_platform(object* obj)
{
    if (obj == NULL && _platform == NULL) {
        return;
    }
    _obj_jump.finish();
    if (obj != NULL) {
//        std::cout << "### CHARACTER[" << name << "].set_platform[" << obj->get_name() << "]" << std::endl;
		if (state.animation_type() == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_STAND);
        } else if (state.animation_type() == ANIM_TYPE_JUMP_ATTACK) {
            set_animation_type(ANIM_TYPE_ATTACK);
        }
        set_animation_type(ANIM_TYPE_STAND);
    }
	_platform = obj;
}

object* character::get_platform()
{
	return _platform;
}

bool character::is_jumping()
{
    return _obj_jump.is_started();
}

void character::interrupt_jump()
{
    _obj_jump.interrupt();
    _obj_jump.finish();
    set_animation_type(ANIM_TYPE_STAND);
}

void character::fall()
{
    if (name == "OCTOPUS TENTACLE") {
//        std::cout << "######## OCTOPUS TENTACLE::fall #1" << std::endl;
    }
    _obj_jump.finish();
    // already on the ground
    if (hit_ground() == true) {
        set_animation_type(ANIM_TYPE_STAND);
        return;
    }
    for (int i=0; i<RES_H; i++) {
		char_update_real_position();
		gravity(false);
		if (hit_ground() == true && state.animation_type() == ANIM_TYPE_STAND) {
            gameControl.get_current_map_obj()->show_map();
			show();
            gameControl.get_current_map_obj()->showAbove();
            draw_lib.update_screen();
			return;
		}
        gameControl.get_current_map_obj()->show_map();
		show();
        gameControl.get_current_map_obj()->showAbove();
        draw_lib.update_screen();
        timer.delay(10);
    }
}

// @TODO: find first ground from bottom, that have space for player (2 tiles above are free), check 2 tiles on the x-axis also
void character::fall_to_ground()
{
    if (name == "OCTOPUS TENTACLE") {
//        std::cout << "######## OCTOPUS TENTACLE::fall_to_ground #1" << std::endl;
    }
    _obj_jump.finish();
    if (hit_ground() == true) {
        return;
    }
    for (int i=0; i<RES_H; i++) {
        char_update_real_position();
        position.y++;
        if (position.y >= RES_H/2 && hit_ground() == true) {
            return;
        }
    }
}

void character::initialize_npc_position_to_ground()
{
    if (can_fly == true) {
        return;
    }
    // RES_H is a good enough limit
    for (int i=0; i<RES_H; i++) {
        char_update_real_position();
        gravity(false);
        if (hit_ground() == true) {
            break;
        }
    }
}

bool character::change_position(short xinc, short yinc)
{
    st_map_collision map_col = map_collision(xinc, yinc, gameControl.get_current_map_obj()->getMapScrolling());
    short int mapLock = map_col.block;

    int type = -1;
    if (_platform != NULL) {
        type = _platform->get_type();
    }
    bool is_on_fly_obj = (yinc > 0 && type == OBJ_ITEM_FLY);

    if (mapLock != BLOCK_UNBLOCKED && mapLock != BLOCK_WATER && is_on_fly_obj == false) {
		return false;
	}
    int py_before = position.y;
	position.x += xinc;
	position.y += yinc;
    //std::cout << "CHAR::change_position - TRUE, yinc[" << yinc << "], py_before[" << py_before << "], py_after[" <<position.y << "]" << std::endl;
    return true;
}

void character::change_position_x(short xinc)
{
    if (xinc == 0) { // nothing todo
        return;
    }
    for (int i=xinc; i>=0.1; i--) {
        if (state.animation_type() == ANIM_TYPE_HIT && hit_ground() == true) {
            hit_moved_back_n += xinc;
        }

        st_map_collision map_col = map_collision(i, 0, gameControl.get_current_map_obj()->getMapScrolling());
        int mapLock = map_col.block;
        //mapLock =  gameControl.getMapPointLock(st_position((position.x + frameSize.width + i)/TILESIZE, (position.y + frameSize.height/2)/TILESIZE));
        if (mapLock == BLOCK_UNBLOCKED || mapLock == BLOCK_WATER || mapLock == BLOCK_Y) {
            if (mapLock == TERRAIN_UNBLOCKED || (mapLock == BLOCK_WATER && abs((float)i*WATER_SPEED_MULT) < 1) || mapLock == BLOCK_Y) {
                position.x += i - gameControl.get_current_map_obj()->get_last_scrolled().x;
            } else {
                position.x += i*WATER_SPEED_MULT - gameControl.get_current_map_obj()->get_last_scrolled().x;
            }
            if (state.animation_type() != ANIM_TYPE_HIT) {
                set_direction(ANIM_DIRECTION_RIGHT);
            } else {
                gravity(false);
                return;
            }
            if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_WALK_ATTACK) {
                state.set_animation_timer(0);
            }
            if (state.animation_type() != ANIM_TYPE_WALK && state.animation_type() != ANIM_TYPE_JUMP && state.animation_type() != ANIM_TYPE_SLIDE && state.animation_type() != ANIM_TYPE_JUMP_ATTACK && state.animation_type() != ANIM_TYPE_HIT && (state.animation_type() != ANIM_TYPE_WALK_ATTACK || (state.animation_type() == ANIM_TYPE_WALK_ATTACK && state.attack_timer+ATTACK_DELAY < timer.getTimer()))) {
                set_animation_type(ANIM_TYPE_WALK);
            }
            position.x += xinc;
            break;
        }
    }
}

bool character::test_change_position(short xinc, short yinc)
{
    if (gameControl.get_current_map_obj() == NULL) {
        return false;
    }
    if (yinc < 0 && position.y < 0) {
        return false;
	}
    if (yinc > 0 && position.y > RES_H) {
        return true;
	}
    if (xinc < 0 && position.x <= 0) {
        return false;
    }

    if (xinc > 0 && (realPosition.x - frameSize.width) > RES_W) {
        return false;
    }

    if (is_ghost == false) {
        st_map_collision map_col = map_collision(xinc, yinc, gameControl.get_current_map_obj()->getMapScrolling());
        short int mapLock = map_col.block;

        if (mapLock != BLOCK_UNBLOCKED && mapLock != BLOCK_WATER) {
            if (mapLock == BLOCK_X && xinc != 0) {
                return false;
            } else if (mapLock == BLOCK_Y && yinc != 0) {
                return false;
            } else if (mapLock == BLOCK_XY) {
                return false;
            }
        }
    }

    // check wall-locks
    int map_x_point = (position.x+xinc);
    bool map_wall = gameControl.get_current_map_obj()->get_map_point_wall_lock(map_x_point);

    if (map_wall == true) {
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
    if (state.animation_type() == ANIM_TYPE_SLIDE) {
        if (hit_ground() == true) {
            set_animation_type(ANIM_TYPE_STAND);
        } else {
            set_animation_type(ANIM_TYPE_JUMP);
        }
    }
}
