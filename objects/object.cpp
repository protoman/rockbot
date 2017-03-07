#include <cstdlib>
#include "object.h"
#include "file/format.h"
#include "classmap.h"
#include "file/format/st_common.h"

extern graphicsLib graphLib;
extern std::string FILEPATH;

#include "timerlib.h"
extern timerLib timer;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "soundlib.h"
extern soundLib soundManager;

extern CURRENT_FILE_FORMAT::st_save game_save;


extern CURRENT_FILE_FORMAT::file_game game_data;

#define GRAVITY_SPEED 4
#define FRAMETIMER_RAY 30
#define RAYFRAME_DELAY 70
#define DELAY_RAY 1500
#define FRAMETIMER_DEATHRAY 30

#include "game.h"
extern game gameControl;

#define DISAPPEARNING_VISIBLE_TIME 1500
#define DISAPPEARNING_HIDDEN_TIME 2000
#define INITIAL_ACTIVATION_DELAY 220

// constructor for game_object


// constructor for map_object
object::object(Uint8 set_id, classMap *set_map, st_position map_pos, st_position teleporter_dest, Uint8 map_dest) : _finished(false), _state(0), _duration(0), _timer_limit(0), _started(false), _animation_finished(false), _animation_reversed(false), _hidden(false)
{
	map = set_map;
	_id = set_id;
    name = std::string(GameMediator::get_instance()->object_list.at(_id).name);
    graphic_filename = GameMediator::get_instance()->object_list.at(_id).graphic_filename;
    type = GameMediator::get_instance()->object_list.at(_id).type;
    obj_timer = GameMediator::get_instance()->object_list.at(_id).timer;
    speed = GameMediator::get_instance()->object_list.at(_id).speed;
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
    _must_teleport_in = false;
    _teleport_state = 0;
    _ray_state = 0;
    _collision_mode = COLlISION_MODE_XY;
    if (type == OBJ_RAY_VERTICAL || type == OBJ_RAY_HORIZONTAL) {
        _timer_limit = 1000;
        direction = 0;
        _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
    } else if (type == OBJ_TRACK_PLATFORM) {
        _timer_limit = 100;
        direction = ANIM_DIRECTION_LEFT;
    }
    if (type == OBJ_DEATHRAY_HORIZONTAL || type == OBJ_DEATHRAY_VERTICAL) {
        _timer_limit = timer.getTimer() + obj_timer;
        //std::cout << "OBJ[" << name << "] INIT, timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
    }
    // control animation for objects that expand/contract
    _expanding = true;
    _size = 0;
    add_graphic();
}

object::~object()
{
    //std::cout << "object[" << name << "] destructor" << std::endl;
}

void object::reset()
{
    _finished = false;
    _state = 0;
    _size = 0;
    _duration = 0;
    _timer_limit = 0;
    //std::cout << "OBJECT - set_start_false[RESET]" << std::endl;
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
    if (type == OBJ_DEATHRAY_HORIZONTAL || type == OBJ_DEATHRAY_VERTICAL) {
        _timer_limit = timer.getTimer() + obj_timer;
        //std::cout << "OBJ[" << name << "] RESET, timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
    }
    _obj_frame_timer = timer.getTimer() + _frame_duration;
    frame = 0;
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
///@TODO

}

void object::gravity()
{
    if (is_on_screen() == false) {
        return;
    }
	// non-falling object types
    if (type == OBJ_MOVING_PLATFORM_UPDOWN || type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_DISAPPEARING_BLOCK || type == OBJ_FALL_PLATFORM || type == OBJ_ITEM_FLY || type == OBJ_FLY_PLATFORM || type == OBJ_ACTIVE_DISAPPEARING_BLOCK|| type == OBJ_RAY_HORIZONTAL || type == OBJ_RAY_VERTICAL || type == OBJ_TRACK_PLATFORM || type == OBJ_DEATHRAY_VERTICAL || type == OBJ_DEATHRAY_HORIZONTAL || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM) {
		return;
	}
    for (int i=GRAVITY_SPEED * gameControl.get_fps_speed_multiplier(); i>0; i--) {
        bool can_fall = test_change_position(0, i);
        //std::cout << "OBJECT::gravity[" << name << "], i[" << i << "], y[" << position.y << "], can_fall[" << can_fall << "]" << std::endl;
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

    if (yinc > 0 && position.y > RES_H) { // check if item felt out of screen
        //std::cout << "OBJ::test_change_position - out of screen(down)" << std::endl;
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
        //if (blocked != 0) std::cout << "obj.blocked: " << blocked << std::endl;
        /// @TODO - consumable items should not stop if blocked by player
        ///
        if (gameControl.get_player_platform() != this && blocked != 0 && is_teleporting() == false && !(type == OBJ_ITEM_JUMP && yinc > 0) && is_consumable() == false) {
            std::cout << "OBJ::test_change_position - can't move, BLOCKED by player" << std::endl;
            return false;
        }
    }

    if (position.y+yinc+framesize_h-2 > RES_H) { // falling out of the screen
        return true;
    }

	short p1 = map->getMapPointLock(st_position((position.x+2+xinc)/TILESIZE, (position.y+yinc+framesize_h-2)/TILESIZE));
	short p2 = map->getMapPointLock(st_position((position.x+framesize_w-2+xinc)/TILESIZE, (position.y+yinc+framesize_h-2)/TILESIZE));
    //std::cout << "object::test_change_position[" << name << "] - p1: " << p1 << ", p2: " << p2 << std::endl;

    if (p1 == TERRAIN_SPIKE && p2 == TERRAIN_SPIKE) {
        return false;
    }

    if ((p1 == TERRAIN_UNBLOCKED ||  p1 == TERRAIN_WATER || p1 == TERRAIN_SPIKE) && (p2 == TERRAIN_UNBLOCKED ||  p2 == TERRAIN_WATER || p2 == TERRAIN_SPIKE)) {
        return true;
	}
	return false;
}

void object::check_player_move(int xinc, int yinc) const
{
	//std::cout << "object::check_player_move::START - p.platform: " << gameControl.get_player_platform() << ", this: " << this << std::endl;
	if (xinc == 0 && yinc == 0) {
        //std::cout << "object::check_player_move::LEAVE" << std::endl;
		return;
	}
	if (yinc < 0 && position.y+framesize_h < 0) {
		return;
	}
	if (yinc > 0 && position.y > RES_H) {
		return;
    }
	if (gameControl.get_player_platform() == this) {
        //std::cout << "************* object::check_player_move - MOVE xinc: " << xinc << ", yinc: " << yinc << " **************" << std::endl;
        gameControl.change_player_position(xinc, yinc);
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
    _obj_frame_timer = 0;
    _timer_limit = 0;
    frame = 0;
}

void object::use_teleport_in_out()
{
    _must_teleport_in = true;
}

bool object::is_consumable()
{
    if (type == OBJ_ENERGY_PILL_BIG || type == OBJ_ENERGY_PILL_SMALL || type == OBJ_ENERGY_TANK || type == OBJ_LIFE || type == OBJ_WEAPON_PILL_BIG || type == OBJ_WEAPON_PILL_SMALL || type == OBJ_WEAPON_TANK) {
        return true;
    }
    return false;
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
        std::cout << "### OBJECT::SHOW - can't find graphic' ###" << std::endl;
		return;
	}

    if (_hidden == true && type != OBJ_ACTIVE_OPENING_SLIM_PLATFORM) {
        //std::cout << "### OBJECT::SHOW::HIDDEN ###" << std::endl;
		return;
	}

    if (_must_teleport_in) {
        if (_teleport_state == e_object_teleport_state_teleport_in || _teleport_state == e_object_teleport_state_teleport_out) {
            draw_lib.show_teleport_small(position.x - scroll_x, position.y);
            return;
        }
    }

    /*
    if (type == OBJ_ITEM_JUMP) {
        std::cout << "OBJ::SHOW, y[" << position.y << "]" << std::endl;
    }
    */

    //std::cout << "LOOP: obj[" << name << "] position.x: " << position.x << ", scroll_x: " << scroll_x << ", dest.x: " << graphic_destiny.x << ", dest.y: " << graphic_destiny.y << std::endl;

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
    }

	// checks if the Object is near the screen to show it
    if (position.x+TILESIZE >= abs(scroll_x) && position.x-TILESIZE <= abs(scroll_x)+RES_W) {

        // animation
        if ((GameMediator::get_instance()->object_list.at(_id).animation_auto_start == true || (GameMediator::get_instance()->object_list.at(_id).animation_auto_start == false && _started == true)) && framesize_w * 2 <= (draw_lib.get_object_graphic(_id)->width))  { // have at least two frames

            //std::cout << "OBJECT::SHOW::ANIM[#1], _obj_frame_timer[" << _obj_frame_timer << "], timer[" << timer.getTimer() << "]" << std::endl;

			graphic_origin.x = frame * framesize_w;
            if (_obj_frame_timer < timer.getTimer()) {

                //std::cout << "OBJECT::SHOW - timer passed, _animation_finished[" << _animation_finished << "]" << std::endl;

                if (_animation_finished == false) {
					if (_animation_reversed == false) {
						frame++;
                        //std::cout << "obj[" << name << "].show.frame++[" << (int)frame << "]" << std::endl;
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
                    //std::cout << "OBJECT - set_start_false[FRAME]" << std::endl;
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
            //std::cout << "obj[" << name << "].show.frame[" << (int)frame << "]" << std::endl;

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
            //std::cout << "obj[" << name << "].show - using frame zero, because!" << std::endl;
			graphic_origin.x = 0;
            frame = 0;
		}

        if (type == OBJ_FINAL_BOSS_TELEPORTER && _started == false) {
            frame = 0;
        }



		//std::cout << "object::show - frame_n: " << frame << ", _animation_reversed: " << _animation_reversed << ", max_frames: " << max_frames << std::endl;
		// direction
        if (framesize_h*2 <= draw_lib.get_object_graphic(_id)->height)  {
			//std::cout << ">>>> object height is enought for direction-right";
			if (direction != ANIM_DIRECTION_RIGHT) {
				graphic_origin.y = 0;
			} else {
				graphic_origin.y = framesize_h;
			}
		} else {
            //std::cout << ">>>> NOT object image-height(" << (objects_sprite_list.find(name)->second).height << ") is NOT enought for direction-right. framesize_h: " << framesize_h << std::endl;
			graphic_origin.y = 0;
		}

		graphic_origin.w = framesize_w;
		graphic_origin.h = framesize_h;

		// parte que vai ser colada
        graphic_destiny.x = position.x - scroll_x;
		graphic_destiny.y = position.y + map->getMapScrolling().y;

        //std::cout << "obj[" << name << "] position.x: " << position.x << ", scroll_x: " << scroll_x << ", dest.x: " << graphic_destiny.x << ", dest.y: " << graphic_destiny.y << std::endl;
        if (type == OBJ_LIFE) {
            int init_x = graphic_origin.w*game_save.selected_player;
            // avoid drawing something we don't have
            if (draw_lib.get_object_graphic(_id)->width < init_x + framesize_w) {
                init_x = 0;
            }
            graphLib.copyArea(st_rectangle(init_x, 0, graphic_origin.w, graphic_origin.h), st_position(graphic_destiny.x, graphic_destiny.y+adjust_y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            return;
        }


        if (draw_lib.get_object_graphic(_id) != NULL) { // there is no graphic with this key yet, add it
            graphLib.copyArea(st_rectangle(graphic_origin.x, graphic_origin.y, graphic_origin.w, graphic_origin.h), st_position(graphic_destiny.x, graphic_destiny.y+adjust_y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
            // disappearning block has a shadow under it
            if (type == OBJ_DISAPPEARING_BLOCK) {
                graphLib.clear_area(graphic_destiny.x, graphic_destiny.y+framesize_h+adjust_y, framesize_w, 6, 0, 0, 0);
                if (_must_play_appearing_sfx == true) {
                    _must_play_appearing_sfx = false;
                    soundManager.play_sfx(SFX_DISAPPEARING_BLOCK);
                }
            }
		}

	// remove item that is out of vision
    } else if (_teleport_state == 2 && (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP)) {
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
    graphic_destiny.y = position.y + map->getMapScrolling().y;


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
    graphic_destiny.y = position.y + map->getMapScrolling().y;

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

bool object::is_platform()
{
    if (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP || type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_FALL_PLATFORM || type == OBJ_FLY_PLATFORM || type == OBJ_MOVING_PLATFORM_LEFTRIGHT || type == OBJ_MOVING_PLATFORM_UPDOWN || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM) {
        return true;
    }

}

void object::show_vertical_ray(int adjust_x, int adjust_y)
{
    st_position graphic_destiny;

    float scroll_x = (float)map->getMapScrolling().x;
    if (adjust_x != 0) {
        scroll_x = adjust_x;
    }
    graphic_destiny.x = position.x - scroll_x;
    graphic_destiny.y = position.y + map->getMapScrolling().y + +adjust_y;

    if (draw_lib.get_object_graphic(_id) != NULL) {
        if (_obj_frame_timer < timer.getTimer()) {
            _obj_frame_timer = timer.getTimer() + RAYFRAME_DELAY;
            _ray_state = !_ray_state;
        }
        int dest_y = draw_lib.get_object_graphic(_id)->height-(TILESIZE*(_state+1));
        //std::cout << "OBJECT::show_vertical_ray - dest_y: " << dest_y << ", _state: " << _state << std::endl;
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
    graphic_destiny.y = position.y + map->getMapScrolling().y + +adjust_y;

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
    graphic_destiny.y = position.y + map->getMapScrolling().y + +adjust_y;
    if (draw_lib.get_object_graphic(_id) != NULL) {
        //std::cout << "show_track_platform - _state: " << _state << std::endl;
        graphLib.copyArea(st_rectangle(_state*framesize_w, 0, framesize_w, framesize_h), st_position(graphic_destiny.x, graphic_destiny.y), draw_lib.get_object_graphic(_id), &graphLib.gameScreen);
    }
}


void object::move(bool paused)
{
    if (paused == true && (type == OBJ_ITEM_FLY || type == OBJ_ITEM_JUMP)) {
        return;
    }
    // check teleport
    if (_must_teleport_in == true) {
        //std::cout << "OBJECT::TELEPORT #1 - _teleport_state: " << (int)_teleport_state << std::endl;
        // init teleport IN
        if (_teleport_state == e_object_teleport_state_initial) {
            if (type == OBJ_ITEM_FLY) {
                draw_lib.set_teleport_small_colors(st_color(219, 43, 0), st_color(235, 235, 235));
            } else if (type == OBJ_ITEM_JUMP) {
                draw_lib.set_teleport_small_colors(st_color(235, 235, 235), st_color(219, 43, 0));
            } else {
                draw_lib.set_teleport_small_colors(st_color(112, 110, 110), st_color(235, 235, 235));
            }
            position.y = -framesize_h+2;
            //std::cout << "OBJECT::TELEPORT #2 - pos.y: " << position.y << std::endl;
            _teleport_state = e_object_teleport_state_teleport_in;
            return;
        // teleporting IN
        } else if (_teleport_state == e_object_teleport_state_teleport_in) {
            int yinc = GRAVITY_SPEED*3 * gameControl.get_fps_speed_multiplier();
            std::cout << "OBJECT::TELEPORT #3 - pos.y: " << position.y << ", start_point.y: " << start_point.y << std::endl;
            if (position.y+yinc > start_point.y) {
                position.y = start_point.y;
            } else {
                position.y += yinc;
            }
            if (position.y == start_point.y) {
                // check if not teleported inside terrain, if so, teleport out
                int map_lock = map->getMapPointLock(st_position((position.x+framesize_w/2)/TILESIZE, (position.y+framesize_h/2)/TILESIZE));
                if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
                    std::cout << "OBJECT::TELEPORT #4 - finish(map-lock), y[" << position.y << "], map_lock[" << map_lock << "]" << std::endl;
                    _teleport_state = e_object_teleport_state_teleport_out;
                } else {
                    std::cout << "OBJECT::TELEPORT #4 - finish(reached-start-point), y[" << position.y << "], map_lock[" << map_lock << "]" << std::endl;
                    _teleport_state = e_object_teleport_state_waiting;
                }
            }
            return; // will stay in 2 until finished, when will be set to 3
        // teleporting out
        } else if (_teleport_state == e_object_teleport_state_teleport_out) {
            //std::cout << "OBJECT::move - teleport OUT" << std::endl;
            int yinc = GRAVITY_SPEED*3 * gameControl.get_fps_speed_multiplier();
            position.y -= yinc;
            if (position.y + framesize_h < 0) {
                _finished = true;
            }
        }
    }


    //std::cout << "name: " << name << ", _duration: " << _duration << ", time-limit: " << _timer_limit << ", timer: " << timer.getTimer() << std::endl;
    if (_duration > 0 && timer.getTimer() > _timer_limit && !(type == OBJ_ITEM_FLY && _started == true)) { // eagle-jet, when active, can't teleport out because of timer
        if (_must_teleport_in == true) {
            //std::cout << "OBJECT::move - is finished, but must teleport out first" << std::endl;
            _teleport_state = e_object_teleport_state_teleport_out;
        } else {
            _finished = true;
        }
		return;
	}
	//std::cout << "object::move::START" << std::endl;
	if (type == OBJ_MOVING_PLATFORM_LEFTRIGHT) {
		if (distance > limit) {
			invert_direction_x();
			distance = 0;
		}
		int xinc=0;
		if (direction == ANIM_DIRECTION_LEFT) {
            xinc = -speed * gameControl.get_fps_speed_multiplier();
		} else {
            xinc = speed * gameControl.get_fps_speed_multiplier();
		}
		bool can_move = test_change_position(xinc, 0);
		//std::cout << "object::move::OBJ_MOVING_PLATFORM_LEFTRIGHT - xinc: " << xinc << ", can_move: " << can_move << ", distance: " << distance << ", limit: " << limit << std::endl;
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
			//std::cout << "OBJ_MOVING_PLATFORM_UPDOWN - limit reached - distance: " << distance << ", limit: " << limit << ", invert" << std::endl;
			invert_direction_y();
			distance = 0;
		}
        int yinc = 0;
		if (direction == ANIM_DIRECTION_UP) {
            yinc = -speed * gameControl.get_fps_speed_multiplier();
		} else {
            yinc = speed * gameControl.get_fps_speed_multiplier();
		}
		bool can_move = test_change_position(0, yinc);
		//std::cout << "object::move::OBJ_MOVING_PLATFORM_LEFTRIGHT - xinc: " << xinc << ", can_move: " << can_move << ", distance: " << distance << ", limit: " << limit << std::endl;
		if (can_move) {
			position.y += yinc;
			check_player_move(0, yinc);
			distance += abs((float)yinc);
		} else {
            std::cout << "object is blocked, yinc: " << yinc << ", current-dist: " << distance << ", limit: " << limit << ", new-dist: " << (limit-distance) << std::endl;
            distance = limit - distance + yinc;
            invert_direction_y();
        }
	} else if (type == OBJ_FALL_PLATFORM) {
        if (timer.getTimer() < _start_timer) { // initial delay
            return;
        }
		if (_state == OBJ_STATE_MOVE) { // falling state
            int yinc = speed * gameControl.get_fps_speed_multiplier();
			bool can_move = test_change_position(0, yinc);
			if (can_move) {
                //std::cout << "OBJ_FALL_PLATFORM - yinc: " << yinc << ", pos.y: " << position.y << std::endl;
				position.y += yinc;
				check_player_move(0, yinc);
				distance += abs((float)yinc);
			} else {
                //std::cout << "OBJ_FALL_PLATFORM - RETURN " << std::endl;
				_state = OBJ_STATE_RETURN;
			}
		} else if (_state == OBJ_STATE_RETURN) { // returning state
            if (gameControl.get_player_platform() != this) { // do not return if player is on it
                if (distance == 0) { // added because of initial delay
                    stop();
                } else {
                    int yinc = -((speed * gameControl.get_fps_speed_multiplier())/2);
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
            int yinc = -speed * gameControl.get_fps_speed_multiplier();
            bool can_move = test_change_position(0, yinc);
            if (can_move) {
                check_player_move(0, yinc);
                position.y += yinc;
                distance += abs((float)yinc);
            } else {
                _state = OBJ_STATE_RETURN;
            }
            // check out of screen
            //std::cout << "FLY PLATFORM, position.y: " << position.y << ", framesize_h+TILESIZE: " << (framesize_h+TILESIZE) << std::endl;
            if (_state == 1 && position.y < -(framesize_h+TILESIZE)) {
                //std::cout << "FLY PLATFORM -> RETURN" << std::endl;
                _state = OBJ_STATE_RETURN;
            }
        } else if (_state == OBJ_STATE_RETURN) { // returning state
            if (distance == 0) { // added because of initial delay
                //std::cout << "FLY_PLATFORM::RETURN::STOP#1" << std::endl;
                stop();
            } else {
                int yinc = (speed * gameControl.get_fps_speed_multiplier())/2;
                if (yinc == 0) {
                    yinc = -1;
                }
                position.y += yinc;
                //std::cout << "FLY_PLATFORM::RETURN::MOVE, yinc: " << yinc << ", pos.y: " << position.y << std::endl;
                check_player_move(0, yinc);
                distance -= abs((float)yinc);
                if (distance == 0) {
                    //std::cout << "FLY_PLATFORM::RETURN::STOP#2" << std::endl;
                    stop();
                }
            }
        }
	} else if (type == OBJ_ITEM_FLY && _started == true) {
        if (speed < 1) {
            speed = 3;
		}
        int xinc = speed * gameControl.get_fps_speed_multiplier();
		int yinc = 0;
		if (direction == ANIM_DIRECTION_LEFT) {
            xinc = -speed * gameControl.get_fps_speed_multiplier();
		}
		if (_command_up == true) {
            yinc = -speed * gameControl.get_fps_speed_multiplier();
		}
		if (_command_down == true) {
            yinc = speed * gameControl.get_fps_speed_multiplier();
		}
		position.x += xinc;
		position.y += yinc;
        //std::cout << "OBJ FLY STARTED - move - xinc: " << xinc << ", position.x: " << position.x << std::endl;
		check_player_move(xinc, yinc); // @TODO - player can move up/down
		distance += abs((float)xinc);
		_command_up = false;
		_command_down = false;
    } else if (type == OBJ_DISAPPEARING_BLOCK) { // 1500 is visible time and 2000 is hidden time. initial delay is defined by obj_timer
        /// @TODO - this must be "static" for all objects of this type
        if (_timer_limit < timer.getTimer()) {
            if (_timer_limit == 0) {
                //std::cout << "OBJ::move() - init timer[" << _id << "]: " << obj_timer << std::endl;
                _timer_limit = timer.getTimer() + obj_timer;
            } else {
                //std::cout << "OBJ_DISAPPEARING_BLOCK - obj_timer: " << obj_timer << ", limit: " << limit << std::endl;
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
        //std::cout << "OBJ_ACTIVE_DISAPPEARING_BLOCK - MOVE - _started: " << _started << ", _timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
        if (_started == true && _timer_limit < timer.getTimer()) {
            if (_hidden == false) { // turn hidden
                std::cout << "OBJ_ACTIVE_DISAPPEARING_BLOCK - HIDE" << std::endl;
                _hidden = true;
                _timer_limit = timer.getTimer() + DISAPPEARNING_HIDDEN_TIME;
            } else { // return to visible state
                std::cout << "OBJ_ACTIVE_DISAPPEARING_BLOCK - SHOW" << std::endl;
                _started = false;
                _hidden = false;
                _obj_frame_timer = timer.getTimer() + _frame_duration;
                _animation_finished = false;
            }
        }
    } else if (type == OBJ_DAMAGING_PLATFORM) {
        //std::cout << "OBJ_ACTIVE_DISAPPEARING_BLOCK - MOVE - _started: " << _started << ", _timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
        if (_started == true && _timer_limit < timer.getTimer()) {
            if (_state == 0) {
                // check is player is above it
                int blocked = map->collision_rect_player_obj(gameControl.get_player()->get_hitbox(), this, 0, 2, 0, 0);

                std::cout << "OBJ_DAMAGING_PLATFORM - DAMAGE-CHECK, blocked[" << blocked << "]" << std::endl;

                /// @TODO - consumable items should not stop if blocked by player
                if (blocked != 0) {
                    //std::cout << "OBJ::test_change_position - can't move, BLOCKED by player" << std::endl;
                    gameControl.get_player()->damage(TOUCH_DAMAGE_SMALL, false);
                }
                _state++;
                _timer_limit = timer.getTimer() + 100;
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
                        //if (type == OBJ_RAY_HORIZONTAL) std::cout << "_state: " << _state << ", LIMIT: " << limit << ", framesize_h: " << framesize_h << std::endl;
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
        //std::cout << "OBJ[" << name << "] out of screen, timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
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
                std::cout << "OBJ[" << name << "] EXECUTING, timer_limit: " << _timer_limit << ", timer.now: " << timer.getTimer() << std::endl;
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
            //std::cout << "map_lock: " << map_lock << ", _state: " << (int)_state << ", objx: " << objx << std::endl;
            if (map_lock == TERRAIN_UNBLOCKED || map_lock == TERRAIN_WATER) {
                _size = _state;
            //} else {
                //std::cout << "<<<<<<<<<<<<<<<<<<< LOCK >>>>>>>>>>>>>>>>>>>>>>" << std::endl;
            }

            if (_state > (RES_W/TILESIZE)*2) {
                std::cout << "RESET OBJ_DEATHRAY_HORIZONTAL" << std::endl;
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
            position.x -= speed * gameControl.get_fps_speed_multiplier();
            xinc = -speed * gameControl.get_fps_speed_multiplier();
            p2 = map->get_map_point_tile1(st_position((position.x)/TILESIZE, (position.y+1)/TILESIZE));
            p3 = map->get_map_point_tile1(st_position(((position.x)/TILESIZE)-1, (position.y+1)/TILESIZE));
        } else {
            position.x += speed * gameControl.get_fps_speed_multiplier();
            xinc = speed * gameControl.get_fps_speed_multiplier();
            p2 = map->get_map_point_tile1(st_position((position.x+TILESIZE/3)/TILESIZE, (position.y+1)/TILESIZE));
            p3 = map->get_map_point_tile1(st_position(((position.x+TILESIZE/3)/TILESIZE)+1, (position.y+1)/TILESIZE));
        }
        check_player_move(xinc, 0);
        //std::cout << "MOVE::OBJ_TRACK_PLATFORM - p1: " << p1 << ", p2: " << p2 << ", p3: " << p3 << std::endl;
        // next two points have a different tile than origin point -invert direction
        if (p1 != p2 && p1 != p3) {
            direction = !direction;
            _state = 0;
        } else if (p1 != p2 && p1 == p3) {
            _state = 1;
        } else {
            _state = 0;
        }
    }
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
    //std::cout << "OBJECT - set_start_false[STOP]" << std::endl;
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
            //std::cout << "_state: " << _state << ", position.x: " << position.x << ", res.x: " << res.x << std::endl;
            return res; // TILESIZE/2 is because of lava graphic not taking the whole width
        } else {
            //std::cout << "start_x"
            return st_position(position.x+TILESIZE, position.y+TILESIZE/2); // TILESIZE/2 is because of lava graphic not taking the whole width
        }
    }
    return position;
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
		//std::cout << "invert left->right" << std::endl;
		direction = ANIM_DIRECTION_RIGHT;
	} else {
		//std::cout << "invert right->left" << std::endl;
		direction = ANIM_DIRECTION_LEFT;
	}
}

void object::invert_direction_y() {
	if (direction == ANIM_DIRECTION_UP) {
		//std::cout << "invert up->down" << std::endl;
		direction = ANIM_DIRECTION_DOWN;
	} else {
		//std::cout << "invert down->up" << std::endl;
		direction = ANIM_DIRECTION_UP;
	}
}

void object::set_state(short obj_state)
{
	_state = obj_state;
	//std::cout << "********* object::set_state::state: " << _state << std::endl;
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
    //std::cout << "OBJECT::start - _start_timer: " << _start_timer << std::endl;
    if (type == OBJ_ACTIVE_DISAPPEARING_BLOCK || type == OBJ_ACTIVE_OPENING_SLIM_PLATFORM || type == OBJ_DAMAGING_PLATFORM) {
        std::cout << "OBJ_ACTIVE_OBJECT - STARTED - obj_timer: " << obj_timer << std::endl;
        _timer_limit = timer.getTimer() + obj_timer;
    }
}

void object::command_up()
{
	_command_up = true;
    //std::cout << ">>>>> SET COMMAND UP" << std::endl;
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
    //std::cout << "object::is_hidden - _hidden: " << _hidden << std::endl;
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

    //if (realPosition.x < -TILESIZE*2 || realPosition.x > (RES_W+TILESIZE*2)) {
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

