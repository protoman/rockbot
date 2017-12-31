#include <cstdlib>
#include "projectilelib.h"
#include "timerlib.h"
#include "classmap.h"
#include "character/character.h"
#include "collision_detection.h"
#include <cmath>

#include "soundlib.h"
extern soundLib soundManager;

extern timerLib timer;
extern CURRENT_FILE_FORMAT::file_game game_data;

#include "game.h"
extern game gameControl;

#define QUAKE_TIMER 20 // time between changing direction of screen movement in quake

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern int freeze_weapon_id;
#define FREEZE_DURATION 3500
#define LIGHTING_FRAMES_N 6
#define BOMB_RAIN_DELAY 1600
#define BOMB_RAIN_N 4

#define LARGE_BEAM_DELAY 20

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
projectile::projectile(Uint8 id, Uint8 set_direction, st_position set_position, bool _owner_is_player) : _move_type(-1), is_reflected(false), status(0), _effect_timer(0), _effect_n(0), _points(1), _target_position(NULL), _weapon_id(0), _is_temporary(true)
{
    set_default_values();
	_id = id; // -1 is default projectile

    //std::cout << ">>>>>>>>>>>>> projectile.id[" << (int)id << "]" << std::endl;

    owner = NULL;


    std::cout << ">>>>> projectile.constrctor, id[" << (int)id << "], x[" << set_position.x << "], y[" << set_position.y << "]" << std::endl;

    position = set_position;
    direction = set_direction;
	_size = get_size();

	/// @TODO _ move this to game load
    _speed = PROJECTILE_DEFAULT_SPEED;
	if (_id == -1) {
		_size.width = 6;
		_size.height = 6;
    } else if (GameMediator::get_instance()->get_projectile(_id).speed > 0) {
        _speed = GameMediator::get_instance()->get_projectile(_id).speed;
    }

    _sin_x = 0;

    animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME; // used to control each frame duration

	position.y -= _size.height/2;
	_max_frames = get_surface()->width / _size.width;
	_move_type = get_trajectory();

    //std::cout << ">>> projectile::CONSTRUCTOR - x: " << position.x << ", y: " << position.y << ", _size.width: " << (int)_size.width << ", _size.height: " << (int)_size.height << std::endl;

    if (_move_type == TRAJECTORY_BOMB) {
        if (direction == ANIM_DIRECTION_LEFT) {
            position.x -= 13;
        } else {
            position.x += 30;
        }
    }

    //std::cout << ">>>>>>>>>>>> PROJECTILE::PROJECTILE::ID: " << (int)_id << ", trajectory: " << (int)_move_type << ", follow: " << TRAJECTORY_FOLLOW << std::endl;

    if (_move_type == TRAJECTORY_ARC) {
        position0.x = position.x;
        position0.y = position.y;
        _trajectory_parabola = trajectory_parabola(RES_W/3);
    } else if (_move_type == TRAJECTORY_ARC_SMALL) {
        position0.x = position.x;
        position0.y = position.y;
    } else if (_move_type == TRAJECTORY_ARC_TO_TARGET) {
        position0.x = position.x;
    } else if (_move_type == TRAJECTORY_SIN) {
        position0.x = position.x;
        position0.y = position.y;
    } else if (_move_type == TRAJECTORY_QUAKE) {
        _quake_info.timer = timer.getTimer() + QUAKE_TIMER;
        soundManager.play_repeated_sfx(SFX_PLAYER_CHARGED_SHOT, 2);
	} else if (_move_type == TRAJECTORY_FREEZE) {
		_quake_info.timer = timer.getTimer() + FREEZE_DURATION;
        if (_owner_is_player == true) {
            //std::cout << "[[[freeze_weapon_effect(SET #A)]]]" << std::endl;
            freeze_weapon_effect = FREEZE_EFFECT_NPC;
        } else {
            //std::cout << "[[[freeze_weapon_effect(SET #B)]]]" << std::endl;
            freeze_weapon_effect = FREEZE_EFFECT_PLAYER;
        }
        freeze_weapon_id = _id;
		graphLib.blink_screen(235, 235, 235);
        draw_lib.set_flash_enabled(true);
    } else if (_move_type == TRAJECTORY_PUSH_BACK) {
        _effect_timer = timer.getTimer() + 2000;
    } else if (_move_type == TRAJECTORY_LASER) {
        status = TRAJECTORY_LINEAR; // we use status as the real trajectory in laser
    } else if (_move_type == TRAJECTORY_CENTERED) {
        _points = 3;
    } else if (_move_type == TRAJECTORY_RING) {
        status = 0;
    } else if (_move_type == TRAJECTORY_LIGHTING) {
        status = 0;
        _effect_n = 0;
        position0.x = position.x;
        position0.y = position.y;
        int first_bottom_lock = gameControl.get_current_map_obj()->get_first_lock_on_bottom(position.x + get_size().width/2);
        position.y = 0;
        std::cout << "Y: " << position.y << std::endl;
        _size.height = first_bottom_lock*TILESIZE + TILESIZE;
    } else if (_move_type == TRAJECTORY_SPIRAL) {
        angle = 360;
        position0.x = position.x;
        position0.y = position.y;
        _speed_x = 0;
    } else if (_move_type == TRAJECTORY_BOUNCING) {
        position0.x = position.x;
        position0.y = position.y;
        _gravity = -0.2;
        _dist_y = 0;
    } else if (_move_type == TRAJECTORY_INVERSE_LINEAR) {
        int map_scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
        int abs_pos_x = position.x - map_scroll_x;
        std::cout << ">>>> abs_pos_x[" << abs_pos_x << "], pos.x[" << position.x << "], map.scroll.x[" << map_scroll_x << "]" << std::endl;

        if (direction == ANIM_DIRECTION_LEFT) {
            int scroll_diff_x = abs_pos_x - RES_W;
            direction = ANIM_DIRECTION_RIGHT;
            position.x -= abs_pos_x;

            std::cout << ">>>> new_pos.x[" << position.x << "]" << std::endl;
        } else {
            int scroll_diff_x = RES_W - abs_pos_x;
            direction = ANIM_DIRECTION_LEFT;
            position.x += scroll_diff_x;
        }
        _move_type = TRAJECTORY_LINEAR;
    } else if (_move_type == TRAJECTORY_BOMB_RAIN) {
        status_timer = timer.getTimer();
    } else if (_move_type == TRAJECTORY_LARGE_BEAM) {
        frame_w = _size.width/3;
        status = 0;
        status_timer = timer.getTimer() + LARGE_BEAM_DELAY;
        if (direction == ANIM_DIRECTION_LEFT) {
            position.x -= frame_w*2;
        }
    } else {
		position0.x = position.x;
		position0.y = position.y;
        //std::cout << ">> position0.x: " << position0.x << ", position.x: " << position.x << std::endl;
	}



    //_target_distance = 0;
    _effect_timer = timer.getTimer() + 3600;
    animation_pos = 0;
    diagonal_flag = false;
    angle = 0;
    radius = 0;
    _owner_direction = 0;
    is_reflected = false;
    _change_direction_counter = 0;
    _chain_width = 0;

    //std::cout << ">> Added projectyle, move_type: " << (int)_move_type << ", TRAJECTORY_TARGET_DIRECTION: " << TRAJECTORY_TARGET_DIRECTION << ", w: " << _size.width << ", h: " << _size.height << std::endl;

    // for size, use getsize
    // for speed, use get_speed
    // for damage, use get_damage
    // for trajectory, get_trajectory

    _speed_x = 8;
    _accel_x = 0.95;

    //std::cout << "_accel_x: " << _accel_x << std::endl;
}

void projectile::set_is_permanent()
{
    _is_temporary = false;
}

void projectile::set_default_values()
{
    _owner_position = NULL;
    is_finished = false;
    move_timer = 0;
    move_delay = 10;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
projectile::~projectile()
{
    /*
    if (_is_temporary == false && _move_type == TRAJECTORY_FREEZE) {
        //std::cout << "projectile::projectile - freeze projectile DESTRUCTOR" << std::endl;
        std::cout << "[[[freeze_weapon_effect(RESET #4)]]] - timer: " << _quake_info.timer << std::endl;
        //freeze_weapon_effect = FREEZE_EFFECT_NONE;
        //freeze_weapon_id = -1;
        draw_lib.set_flash_enabled(false);
	}
    */
}


st_size projectile::get_size() const
{
    if (_id == -1) {
        return st_size(6, 6);
    } else if (_move_type == TRAJECTORY_RING) {
        // depends on current frame
        Uint8 w = _size.width;
        Uint8 h = (_size.height/_max_frames) * animation_pos;
        return st_size(w, h);
    } else if (_move_type == TRAJECTORY_LARGE_BEAM) {
        return st_size(frame_w * (2+status), _size.height);
    }
    return GameMediator::get_instance()->get_projectile(_id).size;
}

void projectile::move_ahead(st_size &moved)
{
    if (direction == ANIM_DIRECTION_LEFT || direction == ANIM_DIRECTION_DOWN_LEFT || direction == ANIM_DIRECTION_UP_LEFT) {
        position.x -= get_speed();
        moved.width -= get_speed();
    } else {
        position.x += get_speed();
        moved.width += get_speed();
    }
}

void projectile::position_to_ground()
{
    // change y until the projectile reachs ground
    while ((position.y + get_surface()->height/2) < RES_H) {
        int lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(position.x/TILESIZE, (position.y + get_surface()->height/2)/TILESIZE)); //map->map_tiles.tiles[position.x/TILESIZE][position.y/TILESIZE].locked;
        if (lock != TERRAIN_UNBLOCKED && lock != TERRAIN_WATER) {
            return;
        }
        position.y++;
    }
}

void projectile::set_direction_from_xyinc(int xinc, int yinc)
{
    if (_change_direction_counter > 1) { // avoid too often changes in direction animation
        _change_direction_counter = 0;
    } else {
        return;
    }
    _change_direction_counter++;
    if (yinc > 0) {
        if (xinc > 0) {
            direction = ANIM_DIRECTION_DOWN_RIGHT;
        } else if (xinc < 0) {
            direction = ANIM_DIRECTION_DOWN_LEFT;
        } else {
            direction = ANIM_DIRECTION_DOWN;
        }
    } else if (yinc < 0) {
        if (xinc > 0) {
            direction = ANIM_DIRECTION_UP_RIGHT;
        } else if (xinc < 0) {
            direction = ANIM_DIRECTION_UP_LEFT;
        } else {
            direction = ANIM_DIRECTION_UP;
        }
    } else { // yinc equals zero
        if (xinc > 0) {
            direction = ANIM_DIRECTION_RIGHT;
        } else {
            direction = ANIM_DIRECTION_LEFT;
        }
    }
}

void projectile::set_direction_from_targetpos(int middle_tolerance)
{
    if (_target_position->y < position.y+_size.height/2-middle_tolerance) {        // target above
        if (_target_position->x+_size.width/2 > position.x) {
            direction = ANIM_DIRECTION_UP_RIGHT;
        } else if (_target_position->x+_size.width/2 < position.x) {
            direction = ANIM_DIRECTION_UP_LEFT;
        } else {
            direction = ANIM_DIRECTION_UP;
        }
    } else if (_target_position->y > position.y+_size.height/2+middle_tolerance) { // target under
        if (_target_position->x+_size.width/2 > position.x) {
            direction = ANIM_DIRECTION_DOWN_RIGHT;
        } else if (_target_position->x+_size.width/2 < position.x) {
            direction = ANIM_DIRECTION_DOWN_LEFT;
        } else {
            direction = ANIM_DIRECTION_DOWN;
        }
    } else {                                                    // target on same line
        if (_target_position->x+_size.width/2 > position.x) {
            direction = ANIM_DIRECTION_RIGHT;
        } else {
            direction = ANIM_DIRECTION_LEFT;
        }
    }
}



Uint8 projectile::get_speed() const
{
    return _speed;
}

Uint8 projectile::get_damage() const
{
    if (_id == -1) {
        return PROJECTILE_DEFAULT_DAMAGE;
    }
    if ((get_trajectory() == TRAJECTORY_BOMB || get_trajectory() == TRAJECTORY_FALL_BOMB) && _effect_n == 0) {
		return 0;
	}
    if (GameMediator::get_instance()->get_projectile(_id).damage <= 0) {
		return PROJECTILE_DEFAULT_DAMAGE;
	}
    return GameMediator::get_instance()->get_projectile(_id).damage;
}

Uint8 projectile::get_trajectory() const
{
    if (_id == -1) {
        return TRAJECTORY_LINEAR;
    }
    return GameMediator::get_instance()->get_projectile(_id).trajectory;
}

Uint8 projectile::get_vanishes_on_hit() const
{
    return GameMediator::get_instance()->get_projectile(_id).vanishes_on_hit;
}

void projectile::set_trajectory(short new_trajectory)
{
	_move_type = new_trajectory;
}


void projectile::set_target_position(st_float_position *pos)
{
	_target_position = pos;
	if (_target_position == NULL && _move_type == TRAJECTORY_FOLLOW) { // if a follow projectile could not find any target, act as zig-zag
        //std::cout << "TRAJECTORY_FOLLOW - could not find target" << std::endl;
		_move_type = TRAJECTORY_ZIGZAG;
    } else 	if (_target_position != NULL && _move_type == TRAJECTORY_TARGET_DIRECTION) { // change type to be the best one to hit player
        // three position (diagonal up, left/right or diagonal down)
        std::cout << ">> _target_position->y: " << _target_position->y << ", pos.y: " << position.y << std::endl;
		if (_target_position->y < position.y-TILESIZE*2) {
			_move_type = TRAJECTORY_DIAGONAL_UP;
		} else if (_target_position->y > position.y+TILESIZE*2) {
			_move_type = TRAJECTORY_DIAGONAL_DOWN;
        } else {
			_move_type = TRAJECTORY_LINEAR;
            _diagonal_speed.y = 0;
		}
        set_direction_from_targetpos(TILESIZE*2);


    } else 	if (_target_position != NULL && _move_type == TRAJECTORY_TARGET_EXACT) {
        _diagonal_speed.x = get_speed();
        int dist_x = _target_position->x - position.x;
        int dist_y = _target_position->y - position.y;
        _diagonal_speed.y = (_diagonal_speed.x * dist_y) / dist_x;
        if (dist_x < 0) {
            _diagonal_speed.y = _diagonal_speed.y * -1;
        }
        set_direction_from_targetpos(TILESIZE/2);


        graphLib.initSurface(st_size(_size.width, _size.height), &rotated_surface);
        int temp_direction = direction;
        direction  = ANIM_DIRECTION_LEFT;
        graphLib.copyArea(st_rectangle(0, 0, _size.width, _size.height), st_position(0, 0), get_surface(), &rotated_surface);
        direction = temp_direction;


        // calculate angle and set image
        angle = atan2(abs(dist_y), abs(dist_x));
        angle = (360*angle)/6.28;
        std::cout << ">>>>>>>>>>>>> ANGLE[" << angle << "], w[" << get_surface()->width << "]" << std::endl;
        //angle = 55;
        // TODO: generate an image from the region, not the whole picture
        // TARGET to the LEFT
        if (dist_x < 0) {
            if (dist_y < 0) {
                angle *= -1;
            }
            // if target is to the left, we need to "flip" image
        // TARGET to the RIGHT
        } else {
            angle += 180;
            if (dist_y > 0) {
                angle *= -1;
            }
        }

        std::cout << "### AJUDSTED-ANGLE[" << angle << "],dist_y[" << dist_y << "]" << std::endl;
        if (angle != 0.0) {
            graphLib.rotate_image(rotated_surface, angle);
        }


    } else if (_target_position != NULL && _move_type == TRAJECTORY_ARC_TO_TARGET) {
        _trajectory_parabola = trajectory_parabola(_target_position->x - position.x);
        position0.y = position.y;
    }
}

graphicsLib_gSurface *projectile::get_surface()
{
    // only use left/right, ignore other directions
    int temp_direction = 0;
    if (direction != ANIM_DIRECTION_LEFT) {
        temp_direction = 1;
    }

	if (_id == -1) {
        if (graphLib.projectile_surface[0].surface[temp_direction].get_surface() == NULL) {
            graphLib.show_debug_msg("projectile surface error #1");
            std::cout << "projectile surface error #1 - temp_direction[" << temp_direction << "]" << std::endl;
        }
        return &graphLib.projectile_surface[0].surface[temp_direction];
	} else {
        if (graphLib.projectile_surface[_id].surface[temp_direction].get_surface() == NULL) {
            graphLib.show_debug_msg("projectile surface error #2");
            std::cout << "projectile surface error #2 - temp_direction[" << temp_direction << "]" << std::endl;
        }
        return &graphLib.projectile_surface[_id].surface[temp_direction];
    }
}

void projectile::inc_status()
{
    status++;
}

st_rectangle projectile::get_area()
{
    return st_rectangle(position.x, position.y, _size.width, _size.height);
}

short projectile::get_max_shots()
{
    return GameMediator::get_instance()->get_projectile(_id).max_shots;
}

short projectile::get_id()
{
    return _id;
}

void projectile::play_sfx(bool called_from_npc)
{
    std::string projectile_sfx(GameMediator::get_instance()->get_projectile(_id).sfx_filename);


    if (projectile_sfx.length() > 0) {
        //Mix_Chunk* sfx = GameMediator::get_instance()->get_sfx(projectile_sfx);
        //soundManager.play_sfx_from_chunk(sfx, 1);
        soundManager.play_sfx_from_file(projectile_sfx, 1);
    } else if (called_from_npc == false) { // game enemies should not play default fire sound
        soundManager.play_sfx(SFX_PLAYER_SHOT);
    }
}

st_float_position projectile::get_position()
{
    return position;
}

st_size projectile::move() {
	st_size moved;

	//std::cout << "projectile::move - TRAJECTORY: " << _move_type << ", x: " << position.x << ", y: " << position.y << ", direction: " << direction << std::endl;

	if (move_timer >= timer.getTimer()) {
		//std::cout << "projectile::projectile - return #1" << std::endl;
		return st_size(0, 0);
	}
    move_timer = timer.getTimer()+ move_delay;




    if (_move_type == TRAJECTORY_LINEAR || _move_type == TRAJECTORY_DIAGONAL_UP || _move_type == TRAJECTORY_DIAGONAL_DOWN || _move_type == TRAJECTORY_RING) {
        move_ahead(moved);
        if (_move_type == TRAJECTORY_DIAGONAL_UP) {
            position.y -= get_speed();
            moved.height -= get_speed();
        } else if (_move_type == TRAJECTORY_DIAGONAL_DOWN) {
            position.y += get_speed();
            moved.height += get_speed();
        }
    } else if (_move_type == TRAJECTORY_TARGET_DIRECTION || _move_type == TRAJECTORY_TARGET_EXACT) {
        move_ahead(moved);
        position.y += _diagonal_speed.y;


    } else if (_move_type == TRAJECTORY_ARC) {
        if (position.y < _size.height || position.y > RES_H) {
            is_finished = true;
            return st_size(0, 0);
        }
        move_ahead(moved);
        position.y = position0.y - _trajectory_parabola.get_y_point(abs(position.x - position0.x));

    } else if (_move_type == TRAJECTORY_ARC_SMALL) {
        if (position.y < _size.height || position.y > RES_H) {
            is_finished = true;
            return st_size(0, 0);
        }

        if (_speed_x > 0) {
            if (direction == ANIM_DIRECTION_LEFT || direction == ANIM_DIRECTION_DOWN_LEFT || direction == ANIM_DIRECTION_UP_LEFT) {
                position.x -= _speed_x;
                moved.width -= _speed_x;
            } else {
                position.x += _speed_x;
                moved.width += _speed_x;
            }
            _speed_x -= _accel_x;
            //std::cout << "ARC::SMALL::speed_x: " << _speed_x << std::endl;
            if (_speed_x < 0) {
                _speed_x = 0;
            }
        }
        position.y += get_speed();

    } else if (_move_type == TRAJECTORY_ARC_TO_TARGET) {
        move_ahead(moved);
        position.y = position0.y - _trajectory_parabola.get_y_point(abs(position.x - position0.x));
	} else if (_move_type == TRAJECTORY_SIN) {
        _sin_x += 0.12;
        float sin_value = (TILESIZE*3)*sin(_sin_x);
        //std::cout << "_sin_x: " << _sin_x << ", sin_value: " << sin_value << std::endl;
        position.y = position0.y + sin_value;
        move_ahead(moved);

    } else if (_move_type == TRAJECTORY_BOMB) {
        if (_effect_n == 0 && (status > 0 || _effect_timer < timer.getTimer())) {
            transform_into_explosion();
        } else if (_effect_n == 1 && _effect_timer < timer.getTimer()) {
            std::cout << "BOMB - FINISH" << std::endl;
            is_finished = true;
        }
        //std::cout << "projectile::move - BOMB" << std::endl;
        // do nothing, it is a bomb, it just stays until explodes
	} else if (_move_type == TRAJECTORY_CHAIN) {
        if (_owner_position == NULL) {
            std::cout << "ERROR: owner positoon NOT set in TRAJECTORY_CHAIN projectile" << std::endl;
        } else {
            position.x = _owner_position->x + 5;
            position.y = _owner_position->y + 14;
            if (_owner_direction != NULL) {
                direction = *_owner_direction;
            }
        }

        if (status == 0) { // increasing
            //std::cout << "position0.x: " << position0.x << ", position.x: " << position.x << std::endl;
            _chain_width += get_speed();
            if (_chain_width > _size.width) {
				status = 1;
			}
		} else { // returning
            _chain_width -= get_speed();
            if (_chain_width <= 0) {
				is_finished = true;
			}
		}
	} else if (_move_type == TRAJECTORY_QUAKE) { // shake the screen
        //std::cout << "TRAJECTORY_QUAKE - quake.timer: " << _quake_info.timer << ", timer: " << timer.getTimer() << std::endl;
		if (_quake_info.timer < timer.getTimer()) {
			if (_quake_info.counter1 == 1) {
				_quake_info.counter1 = 2;
				graphLib.set_screen_adjust(st_position(-QUAKE_SCREEN_MOVE, 0));
			} else {
				_quake_info.counter1 = 1;
				graphLib.set_screen_adjust(st_position(QUAKE_SCREEN_MOVE, 0));
			}
			_quake_info.counter2++;
            if (_quake_info.counter2 > 50) {
				_quake_info.counter1 = 0;
				graphLib.set_screen_adjust(st_position(0, 0));
				is_finished = true;
			}
			_quake_info.timer = timer.getTimer() + QUAKE_TIMER;
		}
	} else if (_move_type == TRAJECTORY_FREEZE) {
        //std::cout << "projectile::projectile - freeze RUNNING, timer: " << timer.getTimer() << ", should stop at " << _quake_info.timer << std::endl;
		if (_quake_info.timer < timer.getTimer()) {
            //std::cout << "projectile::projectile - freeze STOPPED at[" << timer.getTimer() << "], should stop at [" << _quake_info.timer << "]" << std::endl;
            //std::cout << "[[[freeze_weapon_effect(RESET #5)]]]" << std::endl;
            freeze_weapon_effect = FREEZE_EFFECT_NONE;
            draw_lib.set_flash_enabled(false);
			is_finished = true;
		}
	} else if (_move_type == TRAJECTORY_CENTERED) {
		//std::cout << "TRAJECTORY_CENTERED CHECK - timer: " << timer.getTimer() << ", _quake_info.timer: " << _quake_info.timer << std::endl;

		if (_owner_position == NULL) {
            std::cout << "ERROR: owner positoon NOT set in centered projectile" << std::endl;
		} else {
            position.x = _owner_position->x - 15;
            position.y = _owner_position->y - 10;
			if (_owner_direction != NULL) {
				direction = *_owner_direction;
			}
            // after some time, change to linear
            if (status > 0 || _effect_timer < timer.getTimer()) {
				_move_type = TRAJECTORY_LINEAR;
			}
		}
	} else if (_move_type == TRAJECTORY_ZIGZAG) {
        move_ahead(moved);
		if (check_map_collision(st_position(moved.width, moved.height)) == true) {
			status++;
			if (status > 3) {
				is_finished = true;
			} else {
				direction = !direction;
			}
		}
	} else if (_move_type == TRAJECTORY_FOLLOW && _target_position != NULL) {
		int xinc = 0;
		int yinc = 0;
		if (_target_position->x > position.x) {
			xinc = get_speed();
		} else if (_target_position->x < position.x) {
			xinc = -get_speed();
		}
		if (_target_position->y > position.y) {
			yinc = get_speed();
		} else if (_target_position->y < position.y) {
			yinc = -get_speed();
		}
        //std::cout << "TRAJECTORY_FOLLOW - xinc: " << xinc << ", yinc: " << yinc << ", x: " << position.x << ", y: " << position.y << std::endl;
		position.x += xinc;
		moved.width += xinc;
		position.y += yinc;
		moved.height += yinc;


        set_direction_from_xyinc(xinc, yinc); // change frame type depending on xinc/yinc


	} else if (_move_type == TRAJECTORY_FOLLOW && _target_position == NULL) {
		_move_type = TRAJECTORY_ZIGZAG;
    } else 	if ((_move_type == TRAJECTORY_TARGET_DIRECTION || _move_type == TRAJECTORY_TARGET_EXACT) && _target_position == NULL) { // if do not have a target, act as linear
        move_ahead(moved);
    } else if (_move_type == TRAJECTORY_PROGRESSIVE) { // move a bit each time the animation is reset
        if (animation_pos == 0) {
            position_to_ground();
            move_ahead(moved);
        }
        // gravity
    } else if (_move_type == TRAJECTORY_FALL_BOMB) {
        if (_effect_n == 0) {
            position.y += get_speed();
            // check if hit ground
            int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(position.x/TILESIZE, position.y/TILESIZE));
            if (point_lock != TERRAIN_WATER && point_lock != TERRAIN_UNBLOCKED) { // hit ground, lets change to explosion
                //std::cout << "BOMB - TRANSFORM into explosion" << std::endl;
                /// morph into a bigger explosion
                _points = 5000;
                _effect_timer = timer.getTimer()+1600;

                _size.width = 56;
                _size.height = 56;
                position.x -= 28;
                position.y -= 48;
                _max_frames = get_surface()->width / _size.width;
                _effect_n++;
                soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 1);
            }
        } else if (_effect_n == 1 && _effect_timer < timer.getTimer()) {
            is_finished = true;
        }

    } else if (_move_type == TRAJECTORY_LASER) {
        move_ahead(moved);
        if (status == TRAJECTORY_DIAGONAL_UP) {
            position.y -= get_speed();
            moved.height -= get_speed();
        } else if (status == TRAJECTORY_DIAGONAL_DOWN) {
            position.y += get_speed();
            moved.height += get_speed();
        }
        // check for collisions to change direction
        bool collisionX = check_map_collision(st_position(moved.width, 0));
        bool collisionY = check_map_collision(st_position(moved.width, moved.height));

        std::cout << ">>>> collisionX: " << collisionX << ", collisionY: " << collisionY << ", moved.width: " << moved.width << ", moved.height: " << moved.height << std::endl;

        if (collisionX == true || collisionY == true) {
            if (_effect_n == 5) { // destroy if rebouncing for the fourth time
                std::cout << ">>> FINISH-LASER" << std::endl;
                is_finished = true;
            } else { // rebounce
                _effect_n++;
                if (status == TRAJECTORY_LINEAR) {
                    status = TRAJECTORY_DIAGONAL_UP;
                    direction = !direction;
                    std::cout << ">>> REFLECT #0" << std::endl;
                // only change trajectory if hit on top/down, otherwise just change direction
                } else if (status == TRAJECTORY_DIAGONAL_UP && collisionX == false) {
                    status = TRAJECTORY_DIAGONAL_DOWN;
                    std::cout << ">>> REFLECT #1" << std::endl;
                } else if (status == TRAJECTORY_DIAGONAL_DOWN && collisionX == false) {
                    std::cout << ">>> REFLECT #2" << std::endl;
                    status = TRAJECTORY_DIAGONAL_UP;
                } else {
                    std::cout << ">>> REFLECT #3" << std::endl;
                    direction = !direction;
                }
            }
        }


    } else if (_move_type == TRAJECTORY_PUSH_BACK) {
        // execution will be handled by move_projectiles() in player/npc classes, only control duration
        if (timer.getTimer() > _effect_timer) {
            is_finished = true;
        }

    } else if (_move_type == TRAJECTORY_LIGHTING) {
        if (_effect_n > LIGHTING_FRAMES_N) {
            play_sfx(false);
            if (direction == ANIM_DIRECTION_LEFT) {
                position.x -= RES_W/3;
            } else {
                position.x += RES_W/3;
            }
            _effect_n = 0;
            status = 0;
            if (status > 3) {
                finish();
            }
        }
    } else if (_move_type == TRAJECTORY_SPIRAL) {
        angle -= _speed;
        if (angle <= 0) {
            angle = 360;
        }
        _speed_x += _speed/3;
        if (_speed_x <= 0) { // prevent zero-speed
            _speed_x = 0.5;
        }

        //position.y += 0.5;
        float rad_angle = (angle * 3.1459)/180;
        position.x = _speed_x*cos(rad_angle) + position0.x;
        position.y = _speed_x*sin(rad_angle) + position0.y;


    } else if (_move_type == TRAJECTORY_BOUNCING) {
        // @TODO: bounce condition is hitting ground
        int moved_x = 0;
        if (direction == ANIM_DIRECTION_LEFT) {
            moved_x -= _speed;
        } else {
            moved_x += _speed;
        }
        position.x += moved_x;
        // check if hit ground

        if (check_map_collision(st_position(moved_x, 0)) == false) { // hit ground, bounce
            position.y -= _dist_y;
            if (_dist_y < TILESIZE-2) {
                _dist_y += _gravity;
            }
            //std::cout << "_dist_y[" << _dist_y << "]" << std::endl;
        } else {
            position.y -= abs(_dist_y);
            // first bounce must be stronger
            if (status == 0) {
                _dist_y *= -1.5;
                status++;
            } else {
                _dist_y *= -0.6;
            }

            if (fabs(_dist_y) < 0.01) {
                _dist_y = 0;
            }
        }
    } else if (_move_type == TRAJECTORY_DOUBLE_LINEAR || _move_type == TRAJECTORY_DOUBLE_DIAGONAL) {
        if (owner == NULL || is_finished == true) {
            is_finished = true;
            return st_size(0, 0);
        }
        // @TODO: calc the X position given the direction owner is facing and his projectile-origin
        st_position proj_pos_left = owner->get_attack_position(ANIM_DIRECTION_LEFT);
        st_position proj_pos_right = owner->get_attack_position(ANIM_DIRECTION_RIGHT);
        if (_move_type == TRAJECTORY_DOUBLE_LINEAR) {
            owner->add_projectile(_id, proj_pos_left, TRAJECTORY_LINEAR, ANIM_DIRECTION_LEFT);
            owner->add_projectile(_id, proj_pos_right, TRAJECTORY_LINEAR, ANIM_DIRECTION_RIGHT);
        } else {
            int trajectory = TRAJECTORY_DIAGONAL_UP;
            // @TODO: compare target position (create method get target in character class)
            if (owner->getPosition().y < RES_H/2) {
                trajectory = TRAJECTORY_DIAGONAL_DOWN;
            }
            owner->add_projectile(_id, proj_pos_left, trajectory, ANIM_DIRECTION_LEFT);
            owner->add_projectile(_id, proj_pos_right, trajectory, ANIM_DIRECTION_RIGHT);
        }
        is_finished = true;
    } else if (_move_type == TRAJECTORY_BOMB_RAIN) {
        //std::cout << "TRAJECTORY_BOMB_RAIN::EXECUTE, status[" << (int)status << "], timer[" << timer.getTimer() << "], move_timer[" << move_timer << "]" << std::endl;
        if (owner == NULL || is_finished == true) {
            std::cout << "TRAJECTORY_BOMB_RAIN::NO-OWNER-ERROR" << std::endl;
            is_finished = true;
            return st_size(0, 0);
        }
        if (status_timer < timer.getTimer()) {
            // make the projectile owner to add new one into its list
            st_position new_proj_pos;
            new_proj_pos.x = RES_W/BOMB_RAIN_N * status  + gameControl.get_current_map_obj()->getMapScrolling().x;
            if (direction == ANIM_DIRECTION_LEFT) {
                new_proj_pos.x = RES_W - (RES_W/BOMB_RAIN_N * status) + gameControl.get_current_map_obj()->getMapScrolling().x;
            }
            std::cout << "TRAJECTORY_BOMB_RAIN::ADD - new_proj_pos.x[" << new_proj_pos.x << "]" << std::endl;
            // adds same type to get properties and graphics, but chances trajectory for a different type
            owner->add_projectile(_id, new_proj_pos, TRAJECTORY_FALL_BOMB, direction);
            status_timer = timer.getTimer() + BOMB_RAIN_DELAY;
            status++;
            if (status > BOMB_RAIN_N) {
                std::cout << "TRAJECTORY_BOMB_RAIN::FINISHED" << std::endl;
                is_finished = true;
            }
        }


    } else if (_move_type == TRAJECTORY_LARGE_BEAM) {
        //std::cout << "PROJECTILE::move[TRAJECTORY_LARGE_BEAM] - timer[" << timer.getTimer() << "], status_timer[" << status_timer << "]" << std::endl;
        if (status_timer < timer.getTimer()) {
            status_timer = timer.getTimer() + LARGE_BEAM_DELAY;
            status++;
            if (direction == ANIM_DIRECTION_LEFT) {
                position.x -= frame_w;
            }
            if (status > RES_W/frame_w) {
                is_finished = true;
            }
        }
    } else {
        std::cout << "PROJECTILE::move - UNKNOWN TRAJECTORY #" << (int)_move_type << std::endl;
        is_finished = true;
	}

    realPosition.x = position.x - gameControl.get_current_map_obj()->getMapScrolling().x;
    realPosition.y = position.y - gameControl.get_current_map_obj()->getMapScrolling().y;

    //std::cout << "PROJECTILE::MOVE - y[" << position.y << "], map.scroll.y[" << gameControl.get_current_map_obj()->getMapScrolling().y << "]" << std::endl;

	// check out of screen
	if (_move_type != TRAJECTORY_FREEZE && _move_type != TRAJECTORY_QUAKE) { // special effect weapons can work out of screen
		if (realPosition.x > RES_W+TILESIZE*2 || realPosition.x < 0-TILESIZE*2 || realPosition.y > RES_H+TILESIZE*2 || realPosition.y < 0-+TILESIZE*2) {
			is_finished = true;
		}
	}
	return moved;
}

void projectile::draw() {
    if ((_move_type == TRAJECTORY_BOMB || _move_type == TRAJECTORY_FALL_BOMB) && _effect_n == 1) {
		graphLib.draw_explosion(realPosition);
		return;
	}

    if (_move_type == TRAJECTORY_QUAKE || _move_type == TRAJECTORY_FREEZE || _move_type == TRAJECTORY_PUSH_BACK) { /// QTODO: freeze could use some "sparkling" effect
		//std::cout << "projectile::draw - invisible type" << std::endl;
		return;
    } else if (_move_type == TRAJECTORY_BOMB_RAIN) {
        return;
    }

	if (animation_pos >= _max_frames) {
		//std::cout << "projectile::draw - RESET animation_pos" << std::endl;
        if (_move_type == TRAJECTORY_RING) { // ring will be kept at last two frames
            animation_pos = _max_frames-2;
        } else {
            animation_pos = 0;
        }
	}

    int anim_pos = animation_pos*_size.width;
    int show_width = _size.width;


    // @TODO: aqui está errado para direction=right, tem que ajustar para mostrar área à direita, não esquerda
    if (_move_type == TRAJECTORY_CHAIN) {
        show_width = _chain_width;
        if (show_width > _size.width) {
            show_width = _size.width;
        } else if (show_width < 0) {
            show_width = 0;
        }
        if (direction == ANIM_DIRECTION_RIGHT) {
            int show_x = _size.width - show_width;
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(show_x, 0, show_width, _size.height), st_position(realPosition.x + 15, realPosition.y));
        } else {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, 0, show_width, _size.height), st_position(realPosition.x - _chain_width, realPosition.y));
        }
        //std::cout << "CHAIN - x: " << anim_pos << ", show_width: " << show_width << std::endl;

    // lighting gets image from bottom to height
    } else if (_move_type == TRAJECTORY_LIGHTING) {
        int y_pos = get_surface()->height -_size.height;
        //std::cout << "LIGHTING::SHOW::y: " << y_pos << std::endl;
        graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, y_pos, show_width, _size.height), realPosition);

        if (animation_pos == _max_frames-1) {
            _effect_n++;
        }

    } else if (_move_type == TRAJECTORY_LARGE_BEAM) {
        // @TODO - add animation frames
        // back
        graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, 0, frame_w, _size.height), realPosition);
        // middle
        for (int i=0; i<status; i++) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos+frame_w, 0, frame_w, _size.height), st_position(realPosition.x + (frame_w + frame_w*i), realPosition.y));
        }
        // point
        graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos+frame_w*2, 0, frame_w, _size.height), st_position(realPosition.x + (frame_w + frame_w*status), realPosition.y));

    } else if (_move_type == TRAJECTORY_TARGET_EXACT) {
        //std::cout << "TRAJECTORY_TARGET_EXACT - w[" << rotated_surface.width << "], h[" << rotated_surface.height << "]" << std::endl;
        graphLib.showSurfaceAt(&rotated_surface, realPosition, false);
    } else {
        //printf(">> PROJECTILE::DRAW[%d] - x[%d], y[%d], direction[%d], show_width[%d], _size.height[%d], anim_pos[%d], img.w[%d], img.h[%d] <<\n", _id, realPosition.x, realPosition.y, direction, show_width, _size.height, anim_pos, get_surface()->width, get_surface()->height);
        if (direction == ANIM_DIRECTION_UP && get_surface()->height >= _size.height*2) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height, show_width, _size.height), realPosition);
        } else if (direction == ANIM_DIRECTION_DOWN && get_surface()->height >= _size.height*3) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height*2, show_width, _size.height), realPosition);
        } else if (direction == ANIM_DIRECTION_UP_LEFT && get_surface()->height >= _size.height*4) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height*3, show_width, _size.height), realPosition);
        } else if (direction == ANIM_DIRECTION_UP_RIGHT && get_surface()->height >= _size.height*5) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height*4, show_width, _size.height), realPosition);
        } else if (direction == ANIM_DIRECTION_DOWN_LEFT && get_surface()->height >= _size.height*6) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height*5, show_width, _size.height), realPosition);
        } else if (direction == ANIM_DIRECTION_DOWN_RIGHT && get_surface()->height >= _size.height*7) {
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, _size.height*6, show_width, _size.height), realPosition);
        } else { // right is the default frame
            graphLib.showSurfaceRegionAt(get_surface(), st_rectangle(anim_pos, 0, show_width, _size.height), realPosition);
        }
    }

	//std::cout << "projectile::draw - animation_timer: " << animation_timer << ", timer: " << timer.getTimer() << std::endl;
    if (animation_timer < timer.getTimer()) {
        if (_move_type != TRAJECTORY_LASER) {
            animation_pos++;
        } else {
            if (status == TRAJECTORY_DIAGONAL_DOWN) {
                animation_pos = 1;
            } else if (status == TRAJECTORY_DIAGONAL_UP) {
                animation_pos = 2;
            } else {
                animation_pos = 0;
            }
        }
		//std::cout << "projectile::draw - inc anim_pos to " << animation_pos << std::endl;
        if (_move_type == TRAJECTORY_CENTERED) {
            animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME/2;
        } else if (_move_type == TRAJECTORY_LIGHTING) {
            animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME/3;
        } else if (_move_type == TRAJECTORY_SPIRAL) {
            animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME/4;
        } else {
            animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME;
        }
	}
}

// TODO: width/height must come from editor instead of using graphLib.projectile_surface
bool projectile::check_collision(st_rectangle enemy_pos, st_position pos_inc) const
{
    if (_move_type == TRAJECTORY_QUAKE || _move_type == TRAJECTORY_FREEZE || _move_type == TRAJECTORY_PUSH_BACK) {
        //std::cout << ">>>> PROJECTILE::check_collision - LEAVE #1" << std::endl;
        return false;
    }

    //std::cout << ">>>> PROJECTILE::check_collision - #1 enemy_rect[" << enemy_pos.x << "," << enemy_pos.y << "," << enemy_pos.w << "," << enemy_pos.h << "]" << std::endl;
    collision_detection rect_collision_obj;

    int px = position.x;
    int py = position.y;
    int pw = _size.width;
    int ph = _size.height;

    if (_move_type == TRAJECTORY_CHAIN) {
        if (direction == ANIM_DIRECTION_LEFT) {
            px -= _chain_width;
            pw = pos_inc.x;
        } else {
            pw = pos_inc.x + _chain_width;
        }
    } else if (_move_type == TRAJECTORY_LARGE_BEAM) {
        pw = frame_w * (2+status);
    }

    st_rectangle p_rect(enemy_pos.x, enemy_pos.y, enemy_pos.w, enemy_pos.h);


    if (_move_type == TRAJECTORY_RING) {
        int ring_h = 4;
        st_rectangle projectile_rect1(px, py, pw, ring_h);
        bool res1 = rect_collision_obj.rect_overlap(projectile_rect1, p_rect);
        st_rectangle projectile_rect2(px, py+ph-ring_h, pw, ring_h);
        bool res2 = rect_collision_obj.rect_overlap(projectile_rect2, p_rect);

        if (res1 || res2) {
            return true;
        } else {
            return false;
        }
    }

    st_rectangle projectile_rect(px, py, pw, ph);
    bool res = rect_collision_obj.rect_overlap(projectile_rect, p_rect);

    //std::cout << ">>>> PROJECTILE::check_collision - #2, res[" << res << "], proj_rect[" << projectile_rect.x << "," << projectile_rect.y << "," << projectile_rect.w << "," << projectile_rect.h << "], enemy_rect[" << p_rect.x << "," << p_rect.y << "," << p_rect.w << "," << p_rect.h << "]" << std::endl;
    return res;
}

bool projectile::check_map_collision(st_position pos_inc) const
{
	int p_y[3];
	p_y[0] = position.y + pos_inc.y;
	p_y[1] = position.y + get_size().height/2 + pos_inc.y;
	p_y[2] = position.y + get_size().height + pos_inc.y;
	int p_x;
	if (direction == ANIM_DIRECTION_LEFT) {
		p_x = position.x + pos_inc.x;
	} else {
		p_x = position.x + get_size().width + pos_inc.x;
	}
    for (int i=0; i<3; i++) {
        int lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(p_x/TILESIZE, p_y[i]/TILESIZE));// map->map_tiles.tiles[p_x/TILESIZE][p_y[i]/TILESIZE].locked;
        //std::cout << ">> projectile::check_map_collision - point (" << p_x << ", " << p_y[i] << ") lock: " << lock << std::endl;
        if (lock != TERRAIN_UNBLOCKED && lock != TERRAIN_WATER) {
			return true;
		}
	}
	return false;
}


Uint8 projectile::get_direction() const
{
	return direction;
}

void projectile::reflect()
{
    // if it is a bomb, don't reflect at all
    if (get_trajectory() == TRAJECTORY_BOMB || get_trajectory() == TRAJECTORY_FALL_BOMB || get_trajectory() == TRAJECTORY_LIGHTING) {
        return;
    } else if (get_trajectory() == TRAJECTORY_LARGE_BEAM) {
        return;
    } else if (get_trajectory() == TRAJECTORY_CHAIN) {
        soundManager.play_sfx(SFX_SHOT_REFLECTED);
        is_finished = true;
        return;
    }

	if (direction == ANIM_DIRECTION_LEFT) {
		direction = ANIM_DIRECTION_RIGHT;
	} else {
		direction = ANIM_DIRECTION_LEFT;
	}
    if (((int)position.y)%2) {
		_move_type = TRAJECTORY_DIAGONAL_UP;
	} else {
		_move_type = TRAJECTORY_DIAGONAL_DOWN;
	}
	is_reflected = true;
	soundManager.play_sfx(SFX_SHOT_REFLECTED);
}

Uint8 projectile::get_move_type() const
{
    if (_id == -1) {
        return TRAJECTORY_LINEAR;
    }
    return GameMediator::get_instance()->get_projectile(_id).trajectory;
}

bool projectile::is_explosive()
{
    if (_id == -1) {
        return false;
    }
    return GameMediator::get_instance()->get_projectile(_id).is_explosive;
}

void projectile::set_y(int sety)
{
	position.y = sety;
	position0.y = sety;
    position1.y = sety;
}

void projectile::consume_projectile()
{
	// non-consumable types and conditions
    int my_traj = get_trajectory();
    if (my_traj == TRAJECTORY_CHAIN || my_traj == TRAJECTORY_FREEZE) {
		return;
    } else if ((get_trajectory() == TRAJECTORY_BOMB || get_trajectory() == TRAJECTORY_FALL_BOMB) && _effect_n == 0) {
		return;
	}
	_points--;
	//std::cout << "projectile::consume_projectile - _points: " << _points << std::endl;
	if (_points <= 0) {
		is_finished = true;
    }
}

void projectile::transform_into_explosion()
{
    // if not a bomb, transform into one
    _move_type = TRAJECTORY_BOMB;
    std::cout << "BOMB - TRANSFORM into explosion" << std::endl;
    /// morph into a bigger explosion
    _points = 5000;
    _effect_timer = timer.getTimer()+3600;

    _size.width = 56;
    _size.height = 56;
    position.x -= 28;
    position.y -= 28;
    _max_frames = get_surface()->width / _size.width;
    _effect_n++;
    soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 3);
}

void projectile::finish()
{
    // @TODO - quake
    if (_is_temporary == false && _move_type == TRAJECTORY_FREEZE) {
        //std::cout << "[[[freeze_weapon_effect(RESET #4)]]] - timer: " << _quake_info.timer << std::endl;
        freeze_weapon_effect = FREEZE_EFFECT_NONE;
        freeze_weapon_id = -1;
        draw_lib.set_flash_enabled(false);
    }
}

void projectile::set_weapon_id(short wpn_id)
{
	_weapon_id = wpn_id;
}

short projectile::get_weapon_id() const
{
    return _weapon_id;
}

short projectile::get_effect_n() const
{
    return _effect_n;
}

void projectile::set_owner_position(st_float_position *owner_position)
{
	_owner_position = owner_position;
}

void projectile::set_owner_direction(Uint8 *owner_direction)
{
    _owner_direction = owner_direction;
}

void projectile::set_owner(character *owner_ptr)
{
    owner = owner_ptr;
}
