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


bool character::hit_ground() // indicates if character is standing above ground
{
    // if position did not changed since last execution, return previous hit_ground value
    /*
    if (position == _previous_position) {
        return _hit_ground;
    }
    */
    st_rectangle hitbox = get_hitbox();
    std::vector<short> map_tile_x;
    map_tile_x.push_back((hitbox.x + hitbox.w/2)/TILESIZE); // center
    map_tile_x.push_back((hitbox.x)/TILESIZE); // left
    map_tile_x.push_back((hitbox.x + hitbox.w)/TILESIZE); // right
    short map_tile_y1 = (hitbox.y + hitbox.h)/TILESIZE;
    short map_tile_y2 = (hitbox.y + hitbox.h/2)/TILESIZE;

    for (unsigned int i=0; i<map_tile_x.size(); i++) {
        int pointLock1 = gameControl.getMapPointLock(st_position(map_tile_x.at(i), map_tile_y1));
        _hit_ground = false;
        if (pointLock1 != TERRAIN_UNBLOCKED && pointLock1 != TERRAIN_WATER && pointLock1 != TERRAIN_STAIR) {
            _hit_ground = true;
            break;
        } else if (!is_player() && pointLock1 == TERRAIN_STAIR) {
            _hit_ground = true;
            break;
        } else {
            int pointLock2 = gameControl.getMapPointLock(st_position(map_tile_x.at(i), map_tile_y2));
            // a ladder tile at the body (or feet) and free air at the other point is not ground for the player:
            // it made falling past a ladder count as landed (jump after an air dash, drop-from-ladder flag cleared too early)
            bool stair_vs_free = is_player() && (pointLock1 == TERRAIN_STAIR || pointLock2 == TERRAIN_STAIR) &&
                                 (pointLock2 == TERRAIN_UNBLOCKED || pointLock2 == TERRAIN_WATER || pointLock2 == TERRAIN_STAIR);
            if (pointLock1 != pointLock2 && stair_vs_free == false) {
                _hit_ground = true;
                break;
            }

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

void character::check_map_collision_point(int &map_block, int &new_map_lock, int &old_map_lock, int mode_xy) // mode_xy 0 is x, 1 is y
{
    if (map_block == BLOCK_UNBLOCKED && new_map_lock == TERRAIN_WATER) {
        map_block = BLOCK_WATER;
    }
    if (map_block == BLOCK_UNBLOCKED && new_map_lock == TERRAIN_QUICKSAND) {
        map_block = BLOCK_QUICKSAND;
    }

    bool must_block = false;

    if (old_map_lock != new_map_lock) {
        if (is_player() == false && new_map_lock == TERRAIN_UNBLOCKED && old_map_lock == TERRAIN_WATER) { // NPCs must not leave water
            must_block = true;
        } else if (is_player() == false && _is_boss == false && old_map_lock == TERRAIN_UNBLOCKED && new_map_lock == TERRAIN_WATER) { // non-boss NPCs must not enter water
            must_block = true;
        } else if (is_player() == false && new_map_lock == TERRAIN_SCROLL_LOCK) {
            must_block = true;
        } else if (is_player() == false && new_map_lock == TERRAIN_DOOR) {
            must_block = true;
        } else if (new_map_lock == TERRAIN_EASYMODEBLOCK) {
            if (game_save.difficulty == DIFFICULTY_EASY) {
                must_block = true;
            }
        } else if (new_map_lock == TERRAIN_HARDMODEBLOCK) {
            if (game_save.difficulty == DIFFICULTY_HARD) {
                if (mode_xy == 1 ||  is_player()) {
                    damage_spikes(true);
                }
                must_block = true;
            }
        } else if (new_map_lock != TERRAIN_UNBLOCKED && new_map_lock != TERRAIN_WATER && new_map_lock != TERRAIN_SCROLL_LOCK && new_map_lock != TERRAIN_CHECKPOINT && new_map_lock != TERRAIN_STAIR && new_map_lock != TERRAIN_HARDMODEBLOCK && new_map_lock != TERRAIN_EASYMODEBLOCK) {
            must_block = true;
        }

    } else {
        if (new_map_lock == TERRAIN_EASYMODEBLOCK && game_save.difficulty == DIFFICULTY_EASY) {
            must_block = true;
        } else if (new_map_lock == TERRAIN_HARDMODEBLOCK && game_save.difficulty == DIFFICULTY_HARD) {
            if (mode_xy == 1 ||  is_player()) {
                damage_spikes(true);
            }
            must_block = true;
        } else if (map_block == BLOCK_UNBLOCKED && (new_map_lock != BLOCK_UNBLOCKED && new_map_lock != TERRAIN_STAIR && new_map_lock != TERRAIN_WATER && new_map_lock != TERRAIN_EASYMODEBLOCK && new_map_lock != TERRAIN_HARDMODEBLOCK)) {
            must_block = true;
        } else if (map_block == BLOCK_WATER && new_map_lock == BLOCK_X) {
            must_block = true;
        } else if (map_block == BLOCK_QUICKSAND && new_map_lock == BLOCK_X) {
            must_block = true;
        }
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

bool character::process_special_map_points(int map_lock, int incx, int incy, st_position map_pos)
{
    UNUSED(incy);
    int direction = ANIM_DIRECTION_LEFT;
    if (incx > 0) {
        direction = ANIM_DIRECTION_RIGHT;
    }
    if (incx != 0 && map_lock == TERRAIN_SCROLL_LOCK) {
        //gameControl.horizontal_screen_move(direction, false, map_pos.x, map_pos.y);
        gameControl.horizontal_screen_move(state.direction(), false, map_pos.x);
		return true;
	}
    if (state.animation_type() != ANIM_TYPE_TELEPORT && (map_lock == TERRAIN_SPIKE || (map_lock == TERRAIN_HARDMODEBLOCK && game_save.difficulty == 2))) {
        damage_spikes(true);
        return true;
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
            if (is_player() && state.direction() == ANIM_DIRECTION_RIGHT) { // add a few pixels because of graphic when turned right
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
        } else {
            return;
        }
        if (can_move) {
            position.x += move;
            _moving_platform_timer = timer.getTimer()+MOVING_GROUND;
        }
    }
}

st_map_collision character::map_collision(const float incx, const short incy, st_float_position mapScrolling, int hitbox_anim_type)
{
    UNUSED(mapScrolling);

    int py_adjust = 0;
    if (is_player() == true) {
        py_adjust = 8;
    }
    int terrain_type = TERRAIN_UNBLOCKED;

    /// @TODO: move to char hitbox
    if (state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) {
        py_adjust = 1;
    }

    int map_block = BLOCK_UNBLOCKED;

    if (gameControl.get_current_map_obj() == NULL) {
        return st_map_collision(BLOCK_XY, TERRAIN_SOLID);
    }

    gameControl.get_current_map_obj()->collision_char_object(this, incx, incy);
    object_collision res_collision_object = gameControl.get_current_map_obj()->get_obj_collision();

    bool is_on_moving_platform = false;
    if (_platform != NULL && _platform == res_collision_object._object && (_platform->get_type() == OBJ_MOVING_PLATFORM_UP_LOOP || _platform->get_type() == OBJ_MOVING_PLATFORM_DOWN)) {
        is_on_moving_platform = true;
    }

    //if (is_player()) std::cout << "CHAR::map_collision - is_on_moving_platform[" << is_on_moving_platform << "], py[" << position.y << "], py+h[" << (get_hitbox().y + get_hitbox().h) << "]" << std::endl;

    if (is_player() == true && res_collision_object._block != 0 && is_on_moving_platform == false) {
        // deal with teleporter object that have special block-area and effect (9)teleporting)
        if (state.animation_type() != ANIM_TYPE_TELEPORT && res_collision_object._object != NULL) {

            if (res_collision_object._object->get_type() == OBJ_BOSS_TELEPORTER || (res_collision_object._object->get_type() == OBJ_FINAL_BOSS_TELEPORTER && res_collision_object._object->is_started() == true)) {
                if (is_on_teleporter_capsulse(res_collision_object._object) == true) {
                    set_direction(ANIM_DIRECTION_RIGHT);
                    gameControl.object_teleport_boss(res_collision_object._object->get_boss_teleporter_dest(), res_collision_object._object->get_boss_teleport_map_dest(), res_collision_object._object->get_obj_map_id(), true);
                }
            } else if (res_collision_object._object->get_type() == OBJ_STAGE_BOSS_TELEPORTER) {
                if (is_on_teleporter_capsulse(res_collision_object._object) == true) {
                    set_direction(ANIM_DIRECTION_RIGHT);
                    gameControl.object_teleport_boss(res_collision_object._object->get_boss_teleporter_dest(), res_collision_object._object->get_boss_teleport_map_dest(), res_collision_object._object->get_obj_map_id(), false);
                }
            // platform teleporter is just a base where player can step in to teleport
            } else if (res_collision_object._object->get_type() == OBJ_PLATFORM_TELEPORTER && is_on_teleport_platform(res_collision_object._object) == true) {
                set_direction(ANIM_DIRECTION_RIGHT);
                soundManager.play_sfx(SFX_TELEPORT);
                gameControl.object_teleport_boss(res_collision_object._object->get_boss_teleporter_dest(), res_collision_object._object->get_boss_teleport_map_dest(), res_collision_object._object->get_obj_map_id(), false);
            // ignore block
            } else if (res_collision_object._object->get_type() == OBJ_FINAL_BOSS_TELEPORTER && res_collision_object._object->is_started() == false) {
                // acho que era para ver se todos os inimigos estão mortos, mas não completei o código, então vamos fazer como um boss-teleproter
                set_direction(ANIM_DIRECTION_RIGHT);
                gameControl.object_teleport_boss(res_collision_object._object->get_boss_teleporter_dest(), res_collision_object._object->get_boss_teleport_map_dest(), res_collision_object._object->get_obj_map_id(), false);
            } else if (!get_item(res_collision_object)) {
                map_block = res_collision_object._block;

                if (map_block == BLOCK_Y || map_block == BLOCK_XY) {
                    _can_execute_airdash = true;
                    _air_dashed = false;
                }
                // INSIDE PLATFORM OBJECT, MUST DIE
                if (map_block == BLOCK_INSIDE_OBJ) {
                    damage(999, true);
                    return st_map_collision(BLOCK_UNBLOCKED, TERRAIN_SOLID);
                }
            }
        }
    } else if (is_player() == false && res_collision_object._block != 0) {
        map_block = res_collision_object._block;
        if (map_block == BLOCK_Y || map_block == BLOCK_XY) {
            _can_execute_airdash = true;
            _air_dashed = false;
        }
    }


	if (is_player()) {
        if (have_shoryuken() == true && state.animation_type() == ANIM_TYPE_SPECIAL_ATTACK) {
            gameControl.get_current_map_obj()->collision_player_special_attack(this, incx, incy, 9, py_adjust);
        } else {
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
	}



    // no need to test map collision if object collision is already X+Y
    if (map_block == BLOCK_XY && incx != 0) {
        return st_map_collision(BLOCK_XY, TERRAIN_SOLID);
    }


    if (incx == 0 && incy == 0) {
        return st_map_collision(BLOCK_UNBLOCKED, TERRAIN_UNBLOCKED);
    }

    if (_always_move_ahead == false &&
        ((incx < 0 && (position.x + incx < 0)) ||
        (incx > 0 && (position.x + incx > MAP_W * TILESIZE)))) {
        if (map_block == BLOCK_UNBLOCKED) {
            map_block = BLOCK_X;
        } else {
            map_block = BLOCK_XY;
        }
	}
    if (incy < 0 && position.y+incy+frameSize.height < 0) {
		if (map_block == BLOCK_UNBLOCKED) {
			map_block = BLOCK_Y;
		} else {
			map_block = BLOCK_XY;
		}
    }

    // if we are out of map, return always true
    if (_always_move_ahead == true) {
        if ((incx < 0 && (position.x+incx < 0)) || (incx > 0 && position.x+incx > MAP_W*TILESIZE)) {
            return st_map_collision(BLOCK_UNBLOCKED, TERRAIN_UNBLOCKED);
        }
    }

	/// @TODO - use collision rect for the current frame. Until there, use 3 points check
	int py_top, py_middle, py_bottom;
    int px_left, px_center, px_right;
    int old_px_left, old_px_center, old_px_right;

    st_rectangle rect_hitbox = get_hitbox(hitbox_anim_type);

    py_top = rect_hitbox.y + incy + py_adjust;

    py_middle = rect_hitbox.y + incy + rect_hitbox.h/2;
    py_bottom = rect_hitbox.y + incy + rect_hitbox.h - 2;

    px_center = rect_hitbox.x + incx + rect_hitbox.w/2;
    px_left = rect_hitbox.x + incx;
    px_right = rect_hitbox.x + incx + rect_hitbox.w;

    old_px_left = rect_hitbox.x;
    old_px_right = rect_hitbox.x + rect_hitbox.w;

    if (incx == 0 && incy != 0) {
        px_right--;
    }

    st_position map_point;
    st_position old_map_point;
    map_point.x = px_left/TILESIZE;
    old_map_point.x = old_px_left/TILESIZE;
    int new_map_lock = TERRAIN_UNBLOCKED;
    int old_map_lock = TERRAIN_UNBLOCKED;
	if (incx > 0) {
        map_point.x = px_right/TILESIZE;
        old_map_point.x = old_px_right/TILESIZE;
	}

    /// @TODO - use a array-of-array for points in order to having a cleaner code

    int map_x_points[3];
    if (incx == 0 && incy != 0) {
        px_left++;
        px_right--;
    }
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
            if (is_player() && (state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) && i == 0) {
                map_point.y = (py_top+1)/TILESIZE;
            } else {
                map_point.y = map_y_points[i];
            }
            old_map_point.y = map_point.y;
            old_map_lock = gameControl.getMapPointLock(old_map_point);
            new_map_lock = gameControl.getMapPointLock(map_point);
            check_map_collision_point(map_block, new_map_lock, old_map_lock, 0);

            if (is_player() && process_special_map_points(new_map_lock, incx, incy, map_point) == true) {
                return st_map_collision(map_block, new_map_lock);
            }
        }
        //if (is_player() == false) std::cout << "CHAR::map_collision - px_left[" << px_left << "],px_right[" << px_right << "], map_block[" << (int)map_block << "]" << std::endl;
    }

	// TEST Y POINTS
	if (incy < 0) {
        map_point.y = py_top/TILESIZE;
	} else if (incy > 0) {
		map_point.y = py_bottom/TILESIZE;
	}
    old_map_point.y = py_middle/TILESIZE;

    if (incy != 0) {
        for (int i=0; i<3; i++) {
            map_point.x = map_x_points[i];
            old_map_point.x = map_x_points[i];

            old_map_lock = gameControl.getMapPointLock(old_map_point);
			new_map_lock = gameControl.getMapPointLock(map_point);


            check_map_collision_point(map_block, new_map_lock, old_map_lock, 1);

            if (new_map_lock != TERRAIN_UNBLOCKED) {
                terrain_type = new_map_lock;
            }

            if (is_player() && process_special_map_points(new_map_lock, incx, incy, map_point) == true) {
                return st_map_collision(map_block, new_map_lock);
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

		if (incy != 0) {
			if (point_top == TERRAIN_UNBLOCKED && point_middle == TERRAIN_UNBLOCKED && point_bottom == TERRAIN_WATER && _water_splash == false) {
				if (incy < 0) {
					soundManager.play_sfx(SFX_WATER_LEAVE);
				} else {
					soundManager.play_sfx(SFX_WATER_ENTER); // both branches played the "leave" sound
				}
				_water_splash = true;
                gameControl.get_current_map_obj()->add_animation(ANIMATION_STATIC, &graphLib.water_splash, st_float_position(position.x, (map_y_points[2]-1)*TILESIZE-TILESIZE/3), st_position(0, -6), 100, 0, ANIM_DIRECTION_LEFT, st_size(32, 23));
			} else if (point_top == point_bottom && point_top == point_middle && _water_splash == true) {
				_water_splash = false;
			}
        }
	}

    return st_map_collision(map_block, terrain_type);

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
        if (px > limit_min && px < limit_max) {
            return true;
        }
    }
    return false;
}

bool character::is_on_teleport_platform(object *object)
{
    // check if player is above platform
    int obj_y = object->get_position().y;
    int py = position.y + frameSize.height;
    if (py-obj_y <= 2) {
        // só teleporta quando estiver no centro (1 TILE), caso contrário, ignora block
        double abs_value = TILESIZE/2 - object->get_size().width;
        int obj_center_diff = abs(abs_value)/2;
        int limit_min = object->get_position().x + obj_center_diff;
        int limit_max = object->get_position().x + object->get_size().width - obj_center_diff;
        int px = position.x + frameSize.width/2;
        if (px > limit_min && px < limit_max) {
            return true;
        }
    }
    return false;
}

// ********************************************************************************************** //
// Returns true of character is over a staircase                                                  //
// ********************************************************************************************** //
st_position character::is_on_stairs(st_rectangle pos)
{
    if (_dropped_from_stairs == true) { // was dropped from stairs, can't grab again until invencibility time ends
        return st_position(-1, -1);;
    }
    int map_tile_x, map_tile_y;

    int diff_w = pos.w/3;
	map_tile_x = (pos.x+diff_w)/TILESIZE;
	map_tile_y = (pos.y)/TILESIZE;

    if (gameControl.get_current_map_obj()->getMapPointLock(st_position(map_tile_x, map_tile_y)) == TERRAIN_STAIR) {
		return st_position(map_tile_x, map_tile_y);
    }

    map_tile_x = (pos.x+pos.w-diff_w)/TILESIZE;
    if (gameControl.get_current_map_obj()->getMapPointLock(st_position(map_tile_x, map_tile_y)) == TERRAIN_STAIR) {
        return st_position(map_tile_x, map_tile_y);
    }

	return st_position(-1, -1);
}

bool character::is_on_quicksand()
{
    int px = (get_hitbox().x + get_hitbox().w/2)/TILESIZE;
    int py = (get_hitbox().y + get_hitbox().h - QUICKSAND_JUMP_LIMIT)/TILESIZE;
    Uint8 terrain_type = gameControl.get_current_map_obj()->getMapPointLock(st_position(px, py));
    bool result = (terrain_type == TERRAIN_QUICKSAND);
    //std::cout << "CHAR::is_on_quicksand - [" << result << "], terrain[" << (int)terrain_type << "]" << std::endl;
    return result;

}
