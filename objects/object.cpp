#include <cstdlib>

#include "object.h"
#include "../file/format.h"
#include "../classmap.h"
#include "../file/format/st_common.h"

extern graphicsLib graphLib;
extern std::string FILEPATH;

#include "../timerlib.h"
extern timerLib timer;

#include "../graphicslib.h"
extern graphicsLib graphLib;

#include "../soundlib.h"
extern soundLib soundManager;

extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern FREEZE_EFFECT_TYPES freeze_weapon_effect;

#define GRAVITY_SPEED 4
#define FRAMETIMER_RAY 30
#define RAYFRAME_DELAY 70
#define DELAY_RAY 1500
#define FRAMETIMER_DEATHRAY 30

#include "../game.h"
extern game gameControl;

#define DISAPPEARNING_VISIBLE_TIME 1500
#define DISAPPEARNING_HIDDEN_TIME 2000
#define DAMAGING_PLATFORM_TIME 100
#define INITIAL_ACTIVATION_DELAY 220

#define CRUSHER_X_TOLERANCE 3
#define CRUSHER_Y_TOLERANCE 4
#define CRUSHER_DELAY 800

// constructor for game_object


// constructor for map_object
object::object(short set_id, classMap *set_map, st_position map_pos, st_position teleporter_dest, short map_dest) : _finished(false), _state(0), _duration(0), _timer_limit(0), _started(false), _animation_finished(false), _animation_reversed(false), _hidden(false)
{
	map = set_map;
	_id = set_id;
    name = std::string(GameMediator::get_instance()->object_list.at(_id).name);
    graphic_filename = GameMediator::get_instance()->object_list.at(_id).graphic_filename;
    type = GameMediator::get_instance()->object_list.at(_id).type;
    obj_timer = GameMediator::get_instance()->object_list.at(_id).timer;
    speed = GameMediator::get_instance()->object_list.at(_id).speed * SharedData::get_instance()->get_movement_multiplier();
    limit = GameMediator::get_instance()->object_list.at(_id).limit;
	direction = 0;
	distance = 0;
    framesize_w = GameMediator::get_instance()->object_list.at(_id).size.width;
    framesize_h = GameMediator::get_instance()->object_list.at(_id).size.height;
	frame = 0;
	start_point.x = map_pos.x*TILESIZE;
	start_point.y = map_pos.y*TILESIZE;
    position.x = map_pos.x*TILESIZE;
    position.y = map_pos.y*TILESIZE;

    _boss_teleporter_dest.x = teleporter_dest.x;
    _boss_teleporter_dest.y = teleporter_dest.y;
    _boss_teleporter_map_dest = map_dest;
    _obj_map_id = -1;

	if (limit == 0 && (type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_MOVING_PLATFORM_UPDOWN)) {
		limit = 48;
    } else if (type == OBJ_DISAPPEARING_BLOCK) {
        _hidden = true;
        _started = true;
    }

    _command_up = false;
	_command_down = false;
    _start_timer = 0;
    _frame_duration = GameMediator::get_instance()->object_list.at(_id).frame_duration;
    _obj_frame_timer = timer.getTimer()+_frame_duration;
    frame = 0;
    _timer_limit = 0;
    _must_play_appearing_sfx = false;
    _teleport_state = 0;
    _ray_state = 0;
    _collision_mode = COLlISION_MODE_XY;
    reset_timer();
    // control animation for objects that expand/contract
    _expanding = true;
    _size = 0;
    is_dropped = false;
    show_teleport = false;
    teleport_max_timer = 0;
    add_graphic();
}

object::~object()
{
}

void object::reset()
{
    _finished = false;
    _state = 0;
    _size = 0;
    _started = false;
    _animation_finished = false;
    _animation_reversed = false;
    _hidden = false;
    if (type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_MOVING_PLATFORM_UPDOWN || type == OBJ_TRACK_PLATFORM) {
        direction = 0;
    }
    distance = 0;
    position.x = start_point.x;
    position.y = start_point.y;
    if (limit == 0 && (type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_MOVING_PLATFORM_UPDOWN)) {
        limit = 48;
    } else if (type == OBJ_DISAPPEARING_BLOCK) {
        _hidden = true;
    }
    _command_up = false;
    _command_down = false;
    frame = 0;
    //show_teleport = false;
    reset_timer();

    // execute gravity until out of screen or stops moving
    int pos_y_before = position.y;
    int n = 0;
    while (true) {
        gravity();
        if (pos_y_before == position.y || position.y > RES_H+TILESIZE || n > RES_H+TILESIZE) {
            break;
        }
        pos_y_before = position.y;
        n++;
    }
}

void object::reset_timer()
{
    if (_duration != 0) {
        _timer_limit = timer.getTimer() + _duration;
    } else if (type == OBJ_DISAPPEARING_BLOCK) {
        _timer_limit = 0;
        _obj_frame_timer = timer.getTimer() + _frame_duration;
        _hidden = true;
    } else if (type == OBJ_DAMAGING_PLATFORM) {
        _timer_limit = timer.getTimer() + DAMAGING_PLATFORM_TIME;
        _obj_frame_timer = timer.getTimer() + _frame_duration;
    } else if (type == OBJ_RAY_VERTICAL || type == OBJ_RAY_HORIZONTAL) {
        _timer_limit = 1000;
        _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
        _ray_state = 0;
        _state = 0;
    } else if (type == OBJ_TRACK_PLATFORM) {
        _timer_limit = 100;
        direction = ANIM_DIRECTION_LEFT;
    } else if (type == OBJ_DEATHRAY_HORIZONTAL || type == OBJ_DEATHRAY_VERTICAL) {
        _timer_limit = timer.getTimer() + obj_timer;
        _ray_state = 0;
        _state = 0;
    } else {
        _obj_frame_timer = timer.getTimer() + _frame_duration;
        _timer_limit = timer.getTimer() + obj_timer;
    }

}

void object::reset_obj_anim_timer()
{
    if (type == OBJ_DISAPPEARING_BLOCK) {
        _obj_frame_timer = timer.getTimer() + _frame_duration;
    } else if (type == OBJ_DAMAGING_PLATFORM) {
        _obj_frame_timer = timer.getTimer() + _frame_duration;
    } else if (type == OBJ_RAY_VERTICAL || type == OBJ_RAY_HORIZONTAL) {
        _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
    } else {
        _obj_frame_timer = timer.getTimer() + _frame_duration;
    }
}


// ********************************************************************************************** //
// adds an entry into object_graphics_list map, if needed                                      //
// ********************************************************************************************** //
void object::add_graphic()
{
    draw_lib.get_object_graphic(_id);
    max_frames = ((draw_lib.get_object_graphic(_id))->width/framesize_w)-1;
}

void object::remove_graphic()
{
}

void object::gravity()
{
    if (is_on_screen() == false) {
        return;
    }
	// non-falling object types
    if (type == OBJ_MOVING_PLATFORM_UPDOWN || type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_DISAPPEARING_BLOCK || type == OBJ_FALL_PLATFORM || type == OBJ_ITEM_FLY || type == OBJ_FLY_PLATFORM || type == OBJ_ACTIVE_DISAPPEARING_BLOCK|| type == OBJ_RAY_HORIZONTAL || type == OBJ_RAY_VERTICAL || type == OBJ_TRACK_PLATFORM || type == OBJ_DEATHRAY_VERTICAL || type == OBJ_DEATHRAY_HORIZONTAL || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM || type == OBJ_CRUSHER) {
		return;
	}
    for (int i=(int)(GRAVITY_SPEED*SharedData::get_instance()->get_movement_multiplier()); i>0; i--) {
        bool can_fall = test_change_position(0, i);
        if (can_fall == true) {
			position.y += i;
			check_player_move(0, i);
			break;
		}
	}
	if (position.y > RES_H) {
		_finished = true;
	}
}

bool object::test_change_position(short xinc, short yinc)
{
	if (map == NULL) {
		return false;
	}
	if (xinc == 0 && yinc == 0) {
		return true;
	}

    // TODO - OBJ_CRUSHER ignore walls until it leaves inside it. It accelerated until hit a ghround after leaving the wall

    if (yinc > 0 && position.y > RES_H) { // check if item felt out of screen
        if (position.y < RES_H+TILESIZE*2) {
            if (type != OBJ_FALL_PLATFORM && type != OBJ_MOVING_PLATFORM_UPDOWN) {
                _finished = true;
                return false;
            } else {
                return true; // fall platform can move out of screen
            }
        } else {
            return true; // too much out of the screen, can't move more
        }
    }

    if (is_consumable() == false) {
        // collision against player when player is not using a platform
        int blocked = map->collision_rect_player_obj(gameControl.get_player()->get_hitbox(), this, 0, 0, xinc, yinc);
        /// @TODO - consumable items should not stop if blocked by player
        if (gameControl.get_player_platform() != this && blocked != 0 && is_teleporting() == false && !(type == OBJ_ITEM_JUMP && yinc > 0) && is_consumable() == false && type != OBJ_CHECKPOINT) {
            return false;
        }
    }

    if (position.y+yinc+framesize_h-2 > RES_H) { // falling out of the screen
        return true;
    }

	short p1 = map->getMapPointLock(st_position((position.x+2+xinc)/TILESIZE, (position.y+yinc+framesize_h-2)/TILESIZE));
	short p2 = map->getMapPointLock(st_position((position.x+framesize_w-2+xinc)/TILESIZE, (position.y+yinc+framesize_h-2)/TILESIZE));

    if (type != OBJ_FALL_PLATFORM && type != OBJ_MOVING_PLATFORM_UPDOWN) {
        if (p1 == TERRAIN_SPIKE && p2 == TERRAIN_SPIKE) {
            return false;
        }
    }

    if (((p1 == TERRAIN_HARDMODEBLOCK && game_save.difficulty != DIFFICULTY_HARD) || (p1 == TERRAIN_EASYMODEBLOCK && game_save.difficulty != DIFFICULTY_EASY) || p1 == TERRAIN_UNBLOCKED ||  p1 == TERRAIN_WATER || p1 == TERRAIN_SPIKE)
            && ((p2 == TERRAIN_HARDMODEBLOCK && game_save.difficulty != DIFFICULTY_HARD) || (p2 == TERRAIN_EASYMODEBLOCK && game_save.difficulty != DIFFICULTY_EASY) || p2 == TERRAIN_UNBLOCKED ||  p2 == TERRAIN_WATER || p2 == TERRAIN_SPIKE)) {
        return true;
	}



	return false;
}

void object::check_player_move(int xinc, int yinc)
{
	if (xinc == 0 && yinc == 0) {
		return;
	}
	if (yinc < 0 && position.y+framesize_h < 0) {
		return;
	}
	if (yinc > 0 && position.y > RES_H) {
		return;
    }
	if (gameControl.get_player_platform() == this) {
        gameControl.change_player_position(xinc, yinc);
        if (!item_jet_started) {
            item_jet_started = true;
        }
    }
}

void object::set_collision_mode(collision_modes collision_mode)
{
    _collision_mode = collision_mode;
}

collision_modes object::get_collision_mode() const
{
    return _collision_mode;
}

void object::reset_timers()
{
    frame = 0;
    reset_timer();
}

bool object::is_consumable()
{
    if (type == OBJ_ENERGY_PILL_BIG || type == OBJ_ENERGY_PILL_SMALL || type == OBJ_ENERGY_TANK || type == OBJ_WEAPON_PILL_BIG || type == OBJ_WEAPON_PILL_SMALL || type == OBJ_WEAPON_TANK) {
        return true;
    }
    return false;
}

void object::enable_teleport_animation()
{
    show_teleport = true;
    if (type == OBJ_ITEM_FLY) {
        draw_lib.set_teleport_small_colors(st_color(255, 155, 59), st_color(235, 235, 235));
    } else if (type == OBJ_ITEM_JUMP) {
        draw_lib.set_teleport_small_colors(st_color(0, 151, 0), st_color(227, 255, 163));
    } else {
        draw_lib.set_teleport_small_colors(st_color(112, 110, 110), st_color(235, 235, 235));
    }
    teleport_max_timer = timer.getTimer()+TELEPORT_TIME;
    _teleport_state = e_object_teleport_state_initial;
}

void object::set_precise_position(st_position pos, int direction)
{
    position = pos;
    if (direction == ANIM_DIRECTION_LEFT) {
        position.x -= framesize_w;
    }
    start_point.x = position.x;
    start_point.y = position.y;

}

void object::set_position(st_position pos)
{
    position.x = pos.x - framesize_w/2;
    position.y = pos.y - framesize_h/2;
}





// ********************************************************************************************** //
// adds an entry into object_graphics_list map, if needed                                      //
// ********************************************************************************************** //
void object::show(int adjust_y, int adjust_x)
{
	st_rectangle graphic_origin;
	st_position graphic_destiny;

	if (map == NULL) {
		return;
	}

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }

    if (draw_lib.get_object_graphic(_id) == NULL) {
		return;
	}

    if (_hidden == true && type != OBJ_ACTIVE_OPENING_SLIM_PLATFORM) {
		return;
	}

    if (freeze_weapon_effect == FREEZE_EFFECT_NPC) {
        _ray_state = 0;
        _state = 0;
    }


    // ray have a different way to show itself
    if (type == OBJ_RAY_VERTICAL) {
        show_vertical_ray(adjust_x, adjust_y);
        return;
    } else if (type == OBJ_RAY_HORIZONTAL) {
        show_horizontal_ray(adjust_x, adjust_y);
        return;
    } else if (type == OBJ_TRACK_PLATFORM) {
        show_track_platform(adjust_x, adjust_y);
        return;
    } else if (type == OBJ_DEATHRAY_VERTICAL) {
        show_deathray_vertical(adjust_x, adjust_y);
        return;
    } else if (type == OBJ_DEATHRAY_HORIZONTAL) {
        show_deathray_horizontal(adjust_x, adjust_y);
        return;
    //} else if (type == OBJ_BOSS_DOOR) {
        //show_boss_door(adjust_x, adjust_y);
        //return;
    } else if (type == OBJ_CRUSHER) {
        show_crusher();
        return;
    }

    if (show_teleport) {
        if (_teleport_state == e_object_teleport_state_initial) {
            _teleport_state = e_object_teleport_state_teleport_in;
        }
        if (_teleport_state == e_object_teleport_state_teleport_in || _teleport_state == e_object_teleport_state_teleport_out) {
            if (timer.getTimer() < teleport_max_timer) {
                draw_lib.show_teleport_small(position.x-scroll_x, position.y);
                return;
            } else {
                if (_teleport_state == e_object_teleport_state_teleport_in) {
                    _teleport_state = e_object_teleport_state_waiting;
                } else {
                    _finished = true;
                }
            }
        }
    }

	// checks if the Object is near the screen to show it
    if (position.x+TILESIZE >= abs(scroll_x) && position.x-TILESIZE <= abs(scroll_x)+RES_W) {

        // animation
        if ((GameMediator::get_instance()->object_list.at(_id).animation_auto_start == true || (GameMediator::get_instance()->object_list.at(_id).animation_auto_start == false && _started == true)) && framesize_w * 2 <= (draw_lib.get_object_graphic(_id)->width))  { // have at least two frames
			graphic_origin.x = frame * framesize_w;
            if (_obj_frame_timer < timer.getTimer()) {
                if (_animation_finished == false) {
					if (_animation_reversed == false) {
						frame++;
                    } else {
						frame--;
					}
                    _obj_frame_timer = timer.getTimer() + _frame_duration;
                }
			}

			if (frame <= 0) {
				if (_animation_reversed == true) {
					_animation_finished = false;
					_animation_reversed = false;
                    _started = false;
                    // force stop if active block
                    if (type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM) {
                        _hidden = false;
                        _obj_frame_timer = timer.getTimer() + _frame_duration;
                        _animation_finished = false;
                    }
				}
				frame = 0;
			}

			if	(_animation_reversed == false && frame > max_frames) {
                if (GameMediator::get_instance()->object_list.at(_id).animation_loop == false) { // if animation loop is set to false, set this to show always the last frame
                    if (GameMediator::get_instance()->object_list.at(_id).animation_reverse == false) { // don't need to reverse animation, finish it
						_animation_finished = true;
                        frame = draw_lib.get_object_graphic(_id)->width/framesize_w-1;
					} else {
						_animation_reversed = true; // start animation reverse process
						frame--;
					}
				} else {
					frame = 0;
				}
			}
		} else {
			graphic_origin.x = 0;
            frame = 0;
		}

		// direction
        if (framesize_h*2 <= draw_lib.get_object_graphic(_id)->height)  {
			if (direction != ANIM_DIRECTION_RIGHT) {
				graphic_origin.y = 0;
			} else {
				graphic_origin.y = framesize_h;
			}
		} else {
			graphic_origin.y = 0;
		}

		graphic_origin.w = framesize_w;
		graphic_origin.h = framesize_h;

		// parte que vai ser colada
        graphic_destiny.x = position.x - scroll_x;
        graphic_destiny.y = adjust_y + position.y;


        if (draw_lib.get_object_graphic(_id) != NULL) { // there is no graphic with this key yet, add it
            graphLib.copyArea(st_rectangle(graphic_origin.x, graphic_origin.y, graphic_origin.w, graphic_origin.h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            // disappearning block has a shadow under it
            if (type == OBJ_DISAPPEARING_BLOCK) {
                graphLib.clear_area(graphic_destiny.x, graphic_destiny.y+framesize_h, framesize_w, 6, 0, 0, 0);
                if (_must_play_appearing_sfx == true) {
                    _must_play_appearing_sfx = false;
                    soundManager.play_sfx(SFX_DISAPPEARING_BLOCK);
                }
            }
		}

	// remove item that is out of vision
    } else if (_teleport_state == e_object_teleport_state_waiting && (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP)) {
		_finished = true;
    }
}


void object::show_deathray_vertical(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;


    if (draw_lib.get_object_graphic(_id) != NULL) {
        if (_obj_frame_timer < timer.getTimer()) {
            _obj_frame_timer = timer.getTimer() + _frame_duration;
            _ray_state = !_ray_state;
        }

        // draw base
        graphLib.copyArea(st_rectangle(framesize_w*_ray_state, 0, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
        if (_state == 1) {
            // point
            graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*3, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
        } else if (_state == 2) {
            // point
            graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*3, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE*2), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            // body
            graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*2, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
        } else if (_state > 2) {
            if (_expanding == true) {
                // point
                graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*3, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE*_state), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                // body
                for (int i=1; i<_state; i++) {
                    graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*2, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE*i), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                }
            } else {
                int start_x = _state - MAP_H;
                // body
                for (int i=start_x+1; i<_state; i++) {
                    graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE*2, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE*i), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                }
                // tail
                graphLib.copyArea(st_rectangle(framesize_w*_ray_state, TILESIZE, framesize_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y+TILESIZE*start_x), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            }

        }
    }
}

void object::show_deathray_horizontal(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;


    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;

    if (draw_lib.get_object_graphic(_id) != NULL) {
        if (_obj_frame_timer < timer.getTimer()) {
            _obj_frame_timer = timer.getTimer() + _frame_duration;
            _ray_state = !_ray_state;
        }


        if (direction == ANIM_DIRECTION_LEFT) {
            // draw base
            graphLib.copyArea(st_rectangle(TILESIZE*3+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            if (_size == 1) {
                // point
                graphLib.copyArea(st_rectangle(framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x-TILESIZE*2, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            } else if (_size == 2) {
                // point
                graphLib.copyArea(st_rectangle(framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x-TILESIZE*2, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                // body
                graphLib.copyArea(st_rectangle(TILESIZE+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x-TILESIZE, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            } else if (_size > 2) {
                // point
                graphLib.copyArea(st_rectangle(framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x-TILESIZE*_size, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                // body
                for (int i=1; i<_size; i++) {
                    graphLib.copyArea(st_rectangle(TILESIZE+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x-TILESIZE*i, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                }
            }
        } else {
            // draw base
            graphLib.copyArea(st_rectangle(framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            if (_size == 1) {
                // point
                graphLib.copyArea(st_rectangle(TILESIZE*3+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x+TILESIZE, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            } else if (_size == 2) {
                // point
                graphLib.copyArea(st_rectangle(TILESIZE*3+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x+TILESIZE*2, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                // body
                graphLib.copyArea(st_rectangle(TILESIZE*2+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x+TILESIZE, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            } else if (_size > 2) {
                // point
                graphLib.copyArea(st_rectangle(TILESIZE*3+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x+TILESIZE*_size, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                // body
                for (int i=1; i<_size; i++) {
                    graphLib.copyArea(st_rectangle(TILESIZE*2+framesize_w*_ray_state, framesize_h*direction, TILESIZE, framesize_h), st_position(graphic_destiny.x+TILESIZE*i, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
                }
            }
        }
    }
}

void object::show_boss_door(int adjust_x, int adjust_y)
{
    if (_state == e_OBJECT_BOSS_DOOR_STATE_OPENED) {
        return;
    }
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;

    int graph_h = framesize_h;

    if (draw_lib.get_object_graphic(_id) != NULL) {
        graphLib.copyArea(st_rectangle(0, 0, framesize_w, graph_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    }

}

void object::show_crusher()
{
    float scroll_x = (float)map->getMapScrolling().x;
    float scroll_y = (float)map->getMapScrolling().y;
    st_position graphic_destiny = st_position(position.x - scroll_x, position.y + scroll_y);
    // first frame is the lower part
    graphLib.copyArea(st_rectangle(0, 0, framesize_w, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    // second frame repeats to fill the area above
    int repeat_n = (position.y - start_point.y)/framesize_h;
    //std::cout << "OBJ::show_crusher - repeat_n[" << repeat_n << "], start_point.y[" << start_point.y << "], position.y[" << position.y << "]" << std::endl;
    for (int i=0; i<repeat_n; i++) {
        graphLib.copyArea(st_rectangle(framesize_w, 0, framesize_w, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y-(framesize_h*(i+1))), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    }
    // draw the remaining, if needed
    int remaining = position.y - framesize_h*repeat_n - start_point.y;
    graphLib.copyArea(st_rectangle(framesize_w, 0, framesize_w, remaining), st_position(start_point.x-scroll_x, start_point.y-scroll_y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
}

bool object::is_platform()
{
    if (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP || type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_FALL_PLATFORM || type == OBJ_FLY_PLATFORM || type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_MOVING_PLATFORM_UPDOWN || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM) {
        return true;
    }
    return false;
}

void object::show_vertical_ray(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;

    if (draw_lib.get_object_graphic(_id) != NULL) {
        if (_obj_frame_timer < timer.getTimer()) {
            _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
            _ray_state = !_ray_state;
        }

        int dest_y = draw_lib.get_object_graphic(_id)->height-(TILESIZE*(_state+1));

        graphLib.copyArea(st_rectangle(TILESIZE*_ray_state, dest_y, TILESIZE, TILESIZE*(_state+1)), st_position(graphic_destiny.x, graphic_destiny.y-TILESIZE*(_state)), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    }
}

void object::show_horizontal_ray(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;

    if (draw_lib.get_object_graphic(_id) != NULL) {
        if (_obj_frame_timer < timer.getTimer()) {
            _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
            _ray_state = !_ray_state;
        }

        if (direction == ANIM_DIRECTION_LEFT) {
            int graphic_x = framesize_w - TILESIZE*(_state+1) + framesize_w*_ray_state;
            int graphic_w = TILESIZE*(_state+1);
            int dest_x = graphic_destiny.x - graphic_w + TILESIZE;
            graphLib.copyArea(st_rectangle(graphic_x, 0, graphic_w, TILESIZE), st_position(dest_x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
        } else {
            int graphic_x = framesize_w*_ray_state;
            int graphic_w = TILESIZE*(_state+1);
            graphLib.copyArea(st_rectangle(graphic_x, framesize_h, graphic_w, TILESIZE), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
        }
    }
}

void object::show_track_platform(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + adjust_y;
    if (draw_lib.get_object_graphic(_id) != NULL) {
        graphLib.copyArea(st_rectangle(_state*framesize_w, 0, framesize_w, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    }
}


void object::move(bool paused)
{
    if (paused == true && (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP)) {
        return;
    }

    if (freeze_weapon_effect == FREEZE_EFFECT_NPC) {
        return;
    }

    if (_duration > 0 && timer.getTimer() > _timer_limit && !(type == OBJ_ITEM_FLY && _started == true)) { // eagle-jet, when active, can't teleport out because of timer
        if (show_teleport == true && _teleport_state != e_object_teleport_state_teleport_out) {
            _teleport_state = e_object_teleport_state_teleport_out;
            teleport_max_timer = timer.getTimer() + TELEPORT_TIME;
        } else if (show_teleport == false) {
            _finished = true;
        }
        return;
    }
	if (type == OBJ_MOVING_PLATFORM_LEFTRIGHT) {
		if (distance > limit) {
			invert_direction_x();
			distance = 0;
		}
		int xinc=0;
		if (direction == ANIM_DIRECTION_LEFT) {
            xinc = -speed;
		} else {
            xinc = speed;
		}
		bool can_move = test_change_position(xinc, 0);
		if (can_move) {
			position.x += xinc;
			check_player_move(xinc, 0);
			distance += abs((float)xinc);
		} else {
			invert_direction_x();
            // calc the distance based on how much was missing for the limit
            distance = limit - distance;
		}
	} else if (type == OBJ_MOVING_PLATFORM_UPDOWN) {
		if (distance > limit) {
			invert_direction_y();
			distance = 0;
		}
        int yinc = 0;
		if (direction == ANIM_DIRECTION_UP) {
            yinc = -speed;
		} else {
            yinc = speed;
		}
		bool can_move = test_change_position(0, yinc);
		if (can_move) {
			position.y += yinc;
			check_player_move(0, yinc);
			distance += abs((float)yinc);
		} else {
            distance = limit - distance + yinc;
            invert_direction_y();
        }
	} else if (type == OBJ_FALL_PLATFORM) {
        if (timer.getTimer() < _start_timer) { // initial delay
            return;
        }
		if (_state == OBJ_STATE_MOVE) { // falling state
            int yinc = speed;
			bool can_move = test_change_position(0, yinc);
			if (can_move) {
				position.y += yinc;
				check_player_move(0, yinc);
				distance += abs((float)yinc);
			} else {
				_state = OBJ_STATE_RETURN;
			}
		} else if (_state == OBJ_STATE_RETURN) { // returning state
            if (gameControl.get_player_platform() != this) { // do not return if player is on it
                if (distance == 0) { // added because of initial delay
                    stop();
                } else {
                    int yinc = -((speed)/2);
                    if (yinc == 0) {
                        yinc = -1;
                    }
                    position.y += yinc;
                    check_player_move(0, yinc);
                    distance -= abs((float)yinc);
                    if (distance == 0) {
                        stop();
                    }
                }
            }
		}
		// check out of screen
		if (_state == 1 && position.y > RES_H+TILESIZE*3) {
			_state = OBJ_STATE_RETURN;
		}
	} else if (type == OBJ_FLY_PLATFORM) {
        if (timer.getTimer() < _start_timer) { // initial delay
            return;
        }
        if (_state == OBJ_STATE_MOVE) { // flying state
            int yinc = -speed;
            bool can_move = test_change_position(0, yinc);
            if (can_move) {
                check_player_move(0, yinc);
                position.y += yinc;
                distance += abs((float)yinc);
            } else {
                _state = OBJ_STATE_RETURN;
            }
            // check out of screen
            if (_state == 1 && position.y < -(framesize_h+TILESIZE)) {
                _state = OBJ_STATE_RETURN;
            }
        } else if (_state == OBJ_STATE_RETURN) { // returning state
            if (distance == 0) { // added because of initial delay
                stop();
            } else {
                int yinc = (speed)/2;
                if (yinc == 0) {
                    yinc = -1;
                }
                position.y += yinc;
                check_player_move(0, yinc);
                distance -= abs((float)yinc);
                if (distance == 0) {
                    stop();
                }
            }
        }
	} else if (type == OBJ_ITEM_FLY && _started == true) {
        if (speed < 1) {
            speed = 3 * SharedData::get_instance()->get_movement_multiplier();
		}
        int xinc = speed;
		int yinc = 0;
		if (direction == ANIM_DIRECTION_LEFT) {
            xinc = -speed;
		}
		if (_command_up == true) {
            yinc = -speed;
		}
		if (_command_down == true) {
            yinc = speed;
        }
        check_player_move(xinc, yinc); // @TODO - player can move up/down
        if (item_jet_started == true) {
            position.x += xinc;
            position.y += yinc;
            distance += abs((float)xinc);
            _command_up = false;
            _command_down = false;
        }
    } else if (type == OBJ_DISAPPEARING_BLOCK) { // 1500 is visible time and 2000 is hidden time. initial delay is defined by obj_timer
        /// @TODO - this must be "static" for all objects of this type
        if (_timer_limit < timer.getTimer()) {
            if (_timer_limit == 0) {
                _timer_limit = timer.getTimer() + obj_timer;
            } else {
                int timer_add = DISAPPEARNING_HIDDEN_TIME;
                if (_hidden == false) {
                    timer_add = DISAPPEARNING_VISIBLE_TIME;
                }
                _timer_limit = timer.getTimer() + timer_add;
                _hidden = !_hidden;
                if (_hidden == false && is_on_screen() == true) {
                    reset_animation();
                    _must_play_appearing_sfx = true;
                }
            }
        }
    } else if (type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM) {
        if (_started == true && _timer_limit < timer.getTimer()) {
            if (_hidden == false) { // turn hidden
                _hidden = true;
                _timer_limit = timer.getTimer() + DISAPPEARNING_HIDDEN_TIME;
            } else { // return to visible state
                _started = false;
                _hidden = false;
                _obj_frame_timer = timer.getTimer() + _frame_duration;
                _animation_finished = false;
            }
        }
    } else if (type == OBJ_DAMAGING_PLATFORM) {
        if (_started == true && _timer_limit < timer.getTimer()) {
            if (_state == 0) {
                // check is player is above it
                int blocked = map->collision_rect_player_obj(gameControl.get_player()->get_hitbox(), this, 0, 2, 0, 0);

                /// @TODO - consumable items should not stop if blocked by player
                if (blocked != 0) {
                    gameControl.get_player()->damage(TOUCH_DAMAGE_SMALL, false);
                }
                _state++;
                _timer_limit = timer.getTimer() + DAMAGING_PLATFORM_TIME;
            } else {
                _state = 0;
                _started = false;
                reset_animation();
            }
        }
    } else if (type == OBJ_RAY_VERTICAL || type == OBJ_RAY_HORIZONTAL) {
        if (_timer_limit < timer.getTimer()) {
            if (_expanding == true) {
                _state++;
                _timer_limit = timer.getTimer() + FRAMETIMER_RAY;
                if (draw_lib.get_object_graphic(_id) != NULL) {
                    int limit = framesize_w/TILESIZE - 1;
                    if (type == OBJ_RAY_VERTICAL) {
                        limit = framesize_h/TILESIZE - 1;
                    }
                    if (_state >= limit) {
                        _expanding = false;
                        _timer_limit = timer.getTimer() + DELAY_RAY;
                    }
                } else {
                    _state = 0;
                    _timer_limit = timer.getTimer() + DELAY_RAY;
                }
            } else {
                _state--;
                _timer_limit = timer.getTimer() + FRAMETIMER_RAY;
                if (_state < 0) {
                    _state = 0;
                    _timer_limit = timer.getTimer() + DELAY_RAY;
                    _expanding = true;
                }
            }
        }
    } else if (is_on_screen() == false && (type == OBJ_DEATHRAY_VERTICAL || type == OBJ_DEATHRAY_HORIZONTAL)) { // keep timers correct, soo when it enters screen, it will have those fine
        _timer_limit = timer.getTimer() + obj_timer;
    } else if (type == OBJ_DEATHRAY_VERTICAL) {
        if (_timer_limit < timer.getTimer()) {
            if (_state == 0 && is_on_visible_screen()) {
                soundManager.play_sfx(SFX_BEAM);
            }
            _state++;
            if (_state > MAP_H*2) {
                _state = 0;
                _timer_limit = timer.getTimer() + DELAY_RAY;
            } else {
                if (_state > MAP_H) {
                    _expanding = false;
                } else {
                    _expanding = true;
                }
                _timer_limit = timer.getTimer() + FRAMETIMER_DEATHRAY;
            }
        }
    } else if (type == OBJ_DEATHRAY_HORIZONTAL) {
        if (_timer_limit < timer.getTimer()) {
            if (_state == 0 && is_on_visible_screen() == true) {
                soundManager.play_sfx(SFX_BEAM);
            }

            _state++;
            int objx = 0;
            if (direction == ANIM_DIRECTION_LEFT) {
                objx = (position.x - (_size+1) * TILESIZE) / TILESIZE;
            } else {
                objx = (position.x + (_size+1) * TILESIZE) /  TILESIZE;
            }
            int objy = (position.y+framesize_h/2)/TILESIZE;
            int map_lock = map->getMapPointLock(st_position(objx, objy));
            if (map_lock == TERRAIN_UNBLOCKED || map_lock == TERRAIN_WATER) {
                _size = _state;
            }

            if (_state > (RES_W/TILESIZE)*2) {
                _state = 0;
                _size = 0;
                _timer_limit = timer.getTimer() + DELAY_RAY;
            } else {
                if (_state > (RES_W/TILESIZE)) {
                    _expanding = false;
                } else {
                    int xinc = TILESIZE;
                    if (direction == ANIM_DIRECTION_LEFT) {
                        xinc = -TILESIZE;
                    }
                    bool can_move = test_change_position(xinc, 0);
                    if (can_move == true) {
                        _expanding = true;
                    } else {
                        _expanding = false;
                    }
                }
                _timer_limit = timer.getTimer() + FRAMETIMER_DEATHRAY;
            }
        }
    } else if (type == OBJ_TRACK_PLATFORM) {
        st_position_int8 p1;
        st_position_int8 p2;
        st_position_int8 p3;
        int xinc = 0;

        p1 = map->get_map_point_tile1(st_position((start_point.x+1)/TILESIZE, (start_point.y+1)/TILESIZE));
        if (direction == ANIM_DIRECTION_LEFT) {
            position.x -= speed;
            xinc = -speed;
            p2 = map->get_map_point_tile1(st_position((position.x)/TILESIZE, (position.y+1)/TILESIZE));
            p3 = map->get_map_point_tile1(st_position(((position.x)/TILESIZE)-1, (position.y+1)/TILESIZE));
        } else {
            position.x += speed;
            xinc = speed;
            p2 = map->get_map_point_tile1(st_position((position.x+TILESIZE/3)/TILESIZE, (position.y+1)/TILESIZE));
            p3 = map->get_map_point_tile1(st_position(((position.x+TILESIZE/3)/TILESIZE)+1, (position.y+1)/TILESIZE));
        }
        check_player_move(xinc, 0);
        // next two points have a different tile than origin point -invert direction
        if (p1 != p2 && p1 != p3) {
            direction = !direction;
            _state = 0;
        } else if (p1 != p2 && p1 == p3) {
            _state = 1;
        } else {
            _state = 0;
        }
    } else if (type == OBJ_BOSS_DOOR) {
        if (_state == e_OBJECT_BOSS_DOOR_STATE_OPENING) {
            _ray_state++;
            if (_ray_state >= framesize_h) {
                _state = e_OBJECT_BOSS_DOOR_STATE_OPENED;
            }
        } else if (_state == e_OBJECT_BOSS_DOOR_STATE_OPENED) {
            gameControl.horizontal_screen_move(ANIM_DIRECTION_RIGHT, true, position.x/TILESIZE);
            _state = e_OBJECT_BOSS_DOOR_STATE_NONE;
            gameControl.show_door_animation();
        } else if (_state == e_OBJECT_BOSS_DOOR_STATE_CLOSING) {
            _ray_state--;
            if (_ray_state <= 0) {
                _state = e_OBJECT_BOSS_DOOR_STATE_NONE;
            }
        }
    } else if (type == OBJ_CRUSHER) {
        move_crusher();
    }
}

void object::move_crusher()
{
    if (_state == e_OBJECT_CRUSHER_STATE_INIT) {
        position.y += crusher_speed;
        crusher_speed += 0.1;
        short p1 = map->getMapPointLock(st_position((position.x+(framesize_w/2))/TILESIZE, (position.y+framesize_h)/TILESIZE));
        if (p1 == TERRAIN_UNBLOCKED) {
            _state = e_OBJECT_CRUSHER_STATE_FOUND_FREE_WALL;
        }
    } else if (_state == e_OBJECT_CRUSHER_STATE_FOUND_FREE_WALL) {
        position.y += crusher_speed;
        crusher_speed += 0.1;
        short p1 = map->getMapPointLock(st_position((position.x+(framesize_w/2))/TILESIZE, (position.y+framesize_h)/TILESIZE));
        if (p1 != TERRAIN_UNBLOCKED) {
            _state = e_OBJECT_CRUSHER_STATE_FOUND_HIT_GROUND;
            obj_timer = timer.getTimer() + CRUSHER_DELAY;
        }
    } else if (_state == e_OBJECT_CRUSHER_STATE_FOUND_HIT_GROUND) {
        crusher_speed = 0.0;
        if (timer.getTimer() > obj_timer) {
            _state = e_OBJECT_CRUSHER_STATE_RETURNING;
        }
    } else if (_state == e_OBJECT_CRUSHER_STATE_RETURNING) {
        position.y -= speed;
        if (position.y <= start_point.y) {
            position.y = start_point.y;
            _state = e_OBJECT_CRUSHER_STATE_INIT;
            // TODO: add timer
        }
    }

}

bool object::check_player_crushed()
{
    if (_state != e_OBJECT_CRUSHER_STATE_INIT && _state != e_OBJECT_CRUSHER_STATE_FOUND_FREE_WALL) {
        return false;
    }
    st_rectangle p_rect = gameControl.get_player()->get_hitbox();
    std::cout << "check_player_crushed - p_rect.x[" << p_rect.x << "], position.x[" << position.x << "]" << std::endl;
    if (p_rect.x >= position.x+CRUSHER_X_TOLERANCE && p_rect.x <= position.x+framesize_w-CRUSHER_X_TOLERANCE && p_rect.y <= position.y+framesize_h-CRUSHER_Y_TOLERANCE) {
        return true;
    }
    if (p_rect.x+p_rect.w >= position.x+CRUSHER_X_TOLERANCE && p_rect.x+p_rect.w <= position.x+framesize_w-CRUSHER_X_TOLERANCE && p_rect.y <= position.y+framesize_h-CRUSHER_Y_TOLERANCE) {
        return true;
    }
    return false;
}

void object::reset_animation()
{
    frame = 0;
    _animation_finished = false;
    _obj_frame_timer = timer.getTimer() + _frame_duration;
}

void object::stop()
{
    _state = OBJ_STATE_STAND;
    _started = false;
    reset_animation();
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void object::execute(bool paused)
{
    move(paused);
    gravity();
	/// @TODO: finish items
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_position object::get_position() const
{
    if (type == OBJ_RAY_VERTICAL) {
        int y = position.y - _state*TILESIZE;
        return st_position(position.x, y);
    } else if (type == OBJ_RAY_HORIZONTAL) {
        int x = 0;
        if (direction == ANIM_DIRECTION_LEFT) {
            x = position.x - _state*TILESIZE;
        } else {
            x = position.x + TILESIZE;
        }
        int y = position.y + 4;
        return st_position(x, y);
    } else if (type == OBJ_DEATHRAY_VERTICAL) {
        if (_expanding == false) {
            return st_position(position.x+TILESIZE/2, (_state - MAP_H)); // TILESIZE/2 is because of lava graphic not taking the whole width
        } else {
            return st_position(position.x+TILESIZE/2, position.y); // TILESIZE/2 is because of lava graphic not taking the whole width
        }
    } else if (type == OBJ_DEATHRAY_HORIZONTAL) {
        if (direction == ANIM_DIRECTION_LEFT) {
            st_position res = st_position(position.x-(_size*TILESIZE) + TILESIZE, position.y+TILESIZE/2);
            return res; // TILESIZE/2 is because of lava graphic not taking the whole width
        } else {
            return st_position(position.x+TILESIZE, position.y+TILESIZE/2); // TILESIZE/2 is because of lava graphic not taking the whole width
        }
    }
    return position;
}

st_position object::get_start_position() const
{
    return start_point;
}

st_rectangle object::get_area()
{
    st_size size = get_size();
    return st_rectangle(position.x, position.y, size.width, size.height);
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_size object::get_size()
{
    if (type == OBJ_RAY_VERTICAL) {
        int h = _state*TILESIZE;
        return st_size(framesize_w, h);
    } else if (type == OBJ_RAY_HORIZONTAL) {
        int w = _state*TILESIZE;
        return st_size(w, framesize_h-4);
    } else if (type == OBJ_DEATHRAY_VERTICAL) {
        if (_expanding == true) {
            return st_size(framesize_w-TILESIZE, TILESIZE*_state); // -TILESIZE in width is because of lava graphic not taking the whole width
        } else {
            return st_size(framesize_w-TILESIZE, MAP_H); // -TILESIZE in width is because of lava graphic not taking the whole width
        }
    } else if (type == OBJ_DEATHRAY_HORIZONTAL) {
        if (direction == ANIM_DIRECTION_LEFT) {
            return st_size(TILESIZE*_size, framesize_h-TILESIZE); // -TILESIZE in width is because of lava graphic not taking the whole width
        } else {
            return st_size((_size*TILESIZE), framesize_h-TILESIZE); // -TILESIZE in width is because of lava graphic not taking the whole width
        }
    }
	return st_size(framesize_w, framesize_h);
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 object::get_type() const
{
    return type;
}

Uint8 object::get_id() const
{
    return _id;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 object::get_direction() const
{
	return direction;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void object::set_direction(int new_dir)
{
	direction = new_dir;
}

int object::get_distance() const
{
	return distance;
}



void object::set_distance(int new_dist)
{
	distance = new_dist;
}


void object::set_timer(int new_timer)
{
	obj_timer = new_timer;
}

unsigned int object::get_timer() const
{
	return obj_timer;
}

bool object::finished() const
{
	return _finished;
}

void object::set_finished(bool is_finished)
{
	_finished = is_finished;
}

void object::invert_direction_x()
{
	if (direction == ANIM_DIRECTION_LEFT) {
		direction = ANIM_DIRECTION_RIGHT;
	} else {
		direction = ANIM_DIRECTION_LEFT;
	}
}

void object::invert_direction_y() {
	if (direction == ANIM_DIRECTION_UP) {
		direction = ANIM_DIRECTION_DOWN;
	} else {
		direction = ANIM_DIRECTION_UP;
	}
}

void object::set_state(short obj_state)
{
	_state = obj_state;
}

short object::get_state() const
{
	return _state;
}

void object::set_duration(int duration)
{
	_duration = duration;
	_timer_limit = timer.getTimer() + duration;
}

void object::start()
{
	_started = true;
    _start_timer = timer.getTimer() + INITIAL_ACTIVATION_DELAY;
    _obj_frame_timer = timer.getTimer() + _frame_duration;
    if (type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM || type == OBJ_BOSS_DOOR) {
        if (type == OBJ_BOSS_DOOR) {
            _state = e_OBJECT_BOSS_DOOR_STATE_OPENED;
            _teleport_state = 0;
            gameControl.show_door_animation();
        }
        _timer_limit = timer.getTimer() + obj_timer;
    }
}

void object::command_up()
{
	_command_up = true;
}

void object::command_down()
{
	_command_down = true;
}

std::string object::get_name() const
{
    return name;
}

bool object::is_hidden() const
{
    return _hidden;
}

bool object::is_started() const
{
    return _started;
}

bool object::is_on_screen()
{
    st_float_position scroll = map->getMapScrolling();
    // entre scroll.x-RES_W/2 e scroll.x+RES_W+RES_W/2

    if (abs((float)position.x) > scroll.x-RES_W/2 && abs((float)position.x) < scroll.x+RES_W*1.5) {
        return true;
    }
    return false;
}

bool object::is_on_visible_screen()
{
    st_float_position scroll = map->getMapScrolling();

    if (abs((float)position.x) > scroll.x && abs((float)position.x) < scroll.x+RES_W) {
        return true;
    }
    return false;
}

st_position object::get_boss_teleporter_dest()
{
    return _boss_teleporter_dest;
}

Uint8 object::get_boss_teleport_map_dest()
{
    return _boss_teleporter_map_dest;
}

Uint8 object::get_obj_map_id()
{
    return _obj_map_id;
}

void object::set_obj_map_id(Uint8 id)
{
    _obj_map_id = id;
}

bool object::is_teleporting()
{
    if (_teleport_state == e_object_teleport_state_teleport_in || _teleport_state == e_object_teleport_state_teleport_out) {
        return true;
    }
    return false;
}

void object::set_is_dropped(bool dropped)
{
    is_dropped = dropped;
}

bool object::get_is_dropped()
{
    return is_dropped;
}

