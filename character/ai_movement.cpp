// artificial intelligence: movement actions (walk, fly, jump, dash, teleport, circle...) and the movement helpers (split from artificial_inteligence.cpp, no change of behaviour)
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "logger.h"
#include "artificial_inteligence.h"
#include "classplayer.h"
#include "../classmap.h"
#include "../game_mediator.h"
#include "../aux_tools/exception_manager.h"

#include "../game.h"
extern game gameControl;


#include "../soundlib.h"
extern soundLib soundManager;


extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern FREEZE_EFFECT_TYPES freeze_weapon_effect;

#define JUMP_ROOF_MIN_SPEED 3
#define MAX_NPC_SPAWN 3

#define SHOOTER_DELAY 1000

#include "shareddata.h"



/// @TODO - jump if needed
void artificial_inteligence::execute_ai_step_walk()
{
    can_fall_during_move = false;
    short move_type = _parameter;
    if (move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_AHEAD_FALLING) {
        can_fall_during_move = true;
    }
    _dest_point.y = position.y; // is not flying, keep the position


    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        // if not on ground and can't fly, leave

        // aqui, está dizendo que não chegou no chão porque está em cima de escada

        if (hit_ground() == false && can_fly == false) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            set_animation_type(ANIM_TYPE_STAND);
            return;
        }

        if (move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_TURN || move_type == AI_ACTION_WALK_OPTION_TURN_TO_PLAYER) {
            if (state.animation_type() != ANIM_TYPE_TURN) {
                set_animation_type(ANIM_TYPE_TURN);
            }
        } else {
            if (move_type == AI_ACTION_WALK_OPTION_TO_PLAYER) {
                struct_player_dist dist_players = dist_npc_players();
                _dest_point = dist_players.pObj->getPosition();
                if (dist_players.pObj->getPosition().x > (position.x  + frameSize.width/2)) {
                    set_direction(ANIM_DIRECTION_RIGHT);
                } else {
                    set_direction(ANIM_DIRECTION_LEFT);
                }
            } else if (move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_AHEAD || move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_AHEAD_FALLING) {
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    _dest_point.x = position.x - frameSize.width/2 - walk_range;
                } else {
                    _dest_point.x = position.x + frameSize.width/2 + walk_range;
                }
            } else if (move_type == AI_ACTION_WALK_OPTION_TO_RANDOM_DIRECTION) {
                set_direction(rand() % 2);
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    _dest_point.x = position.x - frameSize.width/2 - walk_range;
                } else {
                    _dest_point.x = position.x + frameSize.width/2 + walk_range;
                }
            } else if (move_type == AI_ACTION_WALK_OPTION_TO_OPPOSITE_DIRECTION) {
                if (realPosition.x > RES_W/2) {
                    set_direction(ANIM_DIRECTION_LEFT);
                    _dest_point.x = position.x - walk_range;
                } else {
                    set_direction(ANIM_DIRECTION_RIGHT);
                    _dest_point.x = position.x + walk_range;
                }
                _origin_point.x = position.x;
            }
            set_animation_type(ANIM_TYPE_WALK);
        }
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else {
        if (move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_TURN) {
            if (state.on_last_frame() == true) { // finished turn animation
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    set_direction(ANIM_DIRECTION_RIGHT); /// @TODO - turn if not facing the given direction
                } else {
                    set_direction(ANIM_DIRECTION_LEFT);
                }
                set_animation_type(ANIM_TYPE_WALK);
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        } else if (move_type == AI_ACTION_WALK_OPTION_TURN_TO_PLAYER) {
            struct_player_dist dist_players = dist_npc_players();
            if (dist_players.pObj->getPosition().x > position.x) {
                set_direction(ANIM_DIRECTION_RIGHT);
            } else {
                set_direction(ANIM_DIRECTION_LEFT);
            }
            set_animation_type(ANIM_TYPE_WALK);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        } else {
            bool moved = move_to_point(_dest_point, move_speed, 0, is_ghost, false);
            if (moved == true) {
                set_animation_type(ANIM_TYPE_STAND);
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        }
    }
    last_execute_time = timer.getTimer() + 20;
}

void artificial_inteligence::execute_ai_step_fly()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        fly_start();
    } else {
        fly_update();
    }
}

// sets the destination, direction and variables of the chosen fly option
void artificial_inteligence::fly_start()
{
    must_show_dash_effect = false;
    previous_position_list.clear();
    if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER || _parameter == AI_ACTION_FLY_OPTION_DASH_TO_PLAYER) {
        struct_player_dist dist_players = dist_npc_players();
        _dest_point = dist_players.pObj->getPosition();
        if (_parameter == AI_ACTION_FLY_OPTION_DASH_TO_PLAYER) {
            must_show_dash_effect = true;
        }
        if (dist_players.pObj->getPosition().x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_RANDOM_POINT) {
        int rand_x = create_rand_x_point(walk_range);
        int rand_Y = create_rand_y_point(walk_range);
        _dest_point = st_position(rand_x, rand_Y);
    } else if (_parameter == AI_ACTION_FLY_OPTION_RANDOM_X) {
        int rand_x = create_rand_x_point(walk_range);
        _dest_point = st_position(rand_x, position.y);
    } else if (_parameter == AI_ACTION_FLY_OPTION_RANDOM_Y) {
        int rand_y = create_rand_y_point(walk_range);
        _dest_point = st_position(position.x, rand_y);
    } else if (_parameter == AI_ACTION_FLY_OPTION_HORIZONTAL_AHEAD) {
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _dest_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_UP) {
        _dest_point.y = position.y - walk_range;
    } else if (_parameter == AI_ACTION_FLY_OPTION_DOWN) {
        _dest_point.y = position.y + walk_range;
    } else if (_parameter == AI_ACTION_FLY_OPTION_FALL) {
        _dest_point.y = RES_H + frameSize.height + 2;
    } else if (_parameter == AI_ACTION_FLY_OPTION_DRILL_DOWN) { // DRILL_DOWN is similar to move-down, but NPC is ghostly (can walk walls) and when on walls, move 1/4 of speed.Also, it randomizes its x point when restarted
        _dest_point.y = RES_H + frameSize.height + TILESIZE;
        _ghost_move_speed_reducer = 4;
        if (_saved_walk_range == -1) { // restored when the drill ends, so it does not change the range of the next actions
            _saved_walk_range = walk_range;
        }
        walk_range = RES_H + TILESIZE*4;
    } else if (_parameter == AI_ACTION_FLY_OPTION_VERTICAL_CENTER) {
        _dest_point.y = RES_H/2 - frameSize.height/2;
    } else if (_parameter == AI_ACTION_FLY_OPTION_PLAYER_DIRECTION) {
        struct_player_dist dist_players = dist_npc_players();
        if (dist_players.pObj->getPosition().x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
            _dest_point.x = position.x - walk_range;
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
            _dest_point.x = position.x + walk_range;
        }
        _dest_point.y = position.y;
    } else if (_parameter == AI_ACTION_FLY_OPTION_OPOSITE_WALL) {
        if (realPosition.x > RES_W/2) {
            set_direction(ANIM_DIRECTION_LEFT);
            _dest_point.x = position.x - walk_range;
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
            _dest_point.x = position.x + walk_range;
        }
        _dest_point.y = position.y;
        _origin_point.x = position.x;
    } else if (_parameter == AI_ACTION_FLY_OPTION_OPOSITE_SHOOT_1) {
        if (realPosition.x + frameSize.width/2 > RES_W/2) {
            set_direction(ANIM_DIRECTION_LEFT);
            _dest_point.x = position.x - walk_range;
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
            _dest_point.x = position.x + walk_range;
        }
        _dest_point.y = position.y;
        _origin_point.x = position.x;
        _counter = 0;
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER_X) {
        struct_player_dist dist_players = dist_npc_players();
        if (dist_players.pObj->getPosition().x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
        }
        _dest_point.x = dist_players.pObj->getPosition().x;
        _dest_point.y = position.y;
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER_Y) {
        struct_player_dist dist_players = dist_npc_players();
        _dest_point.x = position.x;
        _dest_point.y = dist_players.pObj->getPosition().y;
    } else if (_parameter == AI_ACTION_FLY_OPTION_ZIGZAG_AHEAD || _parameter == AI_ACTION_FLY_OPTION_SIN_AHEAD) {
        _sin_x = 0;
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _dest_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
        }
        _ai_state.initial_position.x = 0;
        _ai_state.initial_position.y = position.y;
        _dest_point.y = position.y;
        //std::cout << "### AI_ACTION_FLY_OPTION_ZIGZAG_AHEAD.INIT - direction[" << (int)state.direction() << "], pos.x[" << position.x << "], dest.x[" << _dest_point.x << "], dest.y[" << _dest_point.y << "]" << std::endl;
    } else if (_parameter == AI_ACTION_FLY_OPTION_DOWN_RANDOM_DIAGONAL) {
        _ai_state.initial_position.x = position.x;
        _ai_state.initial_position.y = position.y;
        _angle = 30 + rand() % 20;
    } else if (_parameter == AI_ACTION_FLY_OPTION_DIAGONAL_UP || _parameter == AI_ACTION_FLY_OPTION_DIAGONAL_DOWN) {
        _ai_state.initial_position.x = position.x;
        _ai_state.initial_position.y = position.y;
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _dest_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
        }
        if (_parameter == AI_ACTION_FLY_OPTION_DIAGONAL_UP) {
            _dest_point.y = position.y - walk_range;
        } else {
            _dest_point.y = position.y + walk_range;
        }
    }

    // the move code does not turn the NPC (that part of check_can_move_to_point never runs), so face the target
    if (_parameter == AI_ACTION_FLY_OPTION_TO_RANDOM_POINT || _parameter == AI_ACTION_FLY_OPTION_RANDOM_X) {
        if (_dest_point.x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
        } else if (_dest_point.x > position.x) {
            set_direction(ANIM_DIRECTION_RIGHT);
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_SAVED_POINT) {
        if (_saved_point.x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
        } else if (_saved_point.x > position.x) {
            set_direction(ANIM_DIRECTION_RIGHT);
        }
    }

    set_animation_type(ANIM_TYPE_WALK_AIR);
    _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
}

// one step of the chosen fly option
void artificial_inteligence::fly_update()
{
    //std::cout << "AI::execute_ai_step_fly::EXECUTE, param[" << _parameter << "]" << std::endl;
    if (state.animation_type() == ANIM_TYPE_TURN && have_frame_graphic(state.direction(), state.animation_type(), (state.animation_state()+1)) == false) {
        set_animation_type(ANIM_TYPE_STAND);
        turn_around();
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }

    if (_parameter == AI_ACTION_FLY_OPTION_HORIZONTAL_AHEAD) {
        if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_HORIZONTAL_TURN) {
        if (state.animation_type() != ANIM_TYPE_TURN) {
            if (have_frame_graphic(state.direction(), ANIM_TYPE_TURN, 0) == false) {
                turn_around();
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            } else {
                set_animation_type(ANIM_TYPE_TURN);
            }
            state.set_animation_state(0);
            distance.width = 0;
            state.set_animation_timer(timer.getTimer() + 200);
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_UP) {
        if (move_to_point(_dest_point, 0, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_DOWN) {
        if (move_to_point(_dest_point, 0, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_DRILL_DOWN) {
        if (position.y >= RES_H+TILESIZE) {
            position.y = -TILESIZE*2;
            randomize_x_point(TILESIZE*3);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            if (_saved_walk_range != -1) {
                walk_range = _saved_walk_range;
                _saved_walk_range = -1;
            }
        } else {
            can_move_struct check_move_res = check_can_move_to_point(_dest_point, 0, move_speed, false, false);
            float adjusted_move_speed = move_speed;
            if (!check_move_res.can_move_y) {
                adjusted_move_speed = move_speed / _ghost_move_speed_reducer;
            }
            if (adjusted_move_speed <= 0.0) {
                adjusted_move_speed = 1.0;
            }
            move_to_point(_dest_point, 0, adjusted_move_speed, true, false);
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_FALL) {
        if (move_to_point(_dest_point, 0, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER || _parameter == AI_ACTION_FLY_OPTION_TO_RANDOM_POINT || _parameter == AI_ACTION_FLY_OPTION_RANDOM_X || _parameter == AI_ACTION_FLY_OPTION_RANDOM_Y) {
        if (move_to_point(_dest_point, move_speed, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_DASH_TO_PLAYER) {
        if (move_to_point(_dest_point, move_speed*2, move_speed*2, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_SAVED_POINT) {
        //std::cout << "AI_ACTION_FLY_OPTION_TO_SAVED_POINT - point[" << _saved_point.x << "][" << _saved_point.y << "]" << std::endl;
        if (move_to_point(_saved_point, move_speed, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_VERTICAL_CENTER) {
        if (move_to_point(_dest_point, 0, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_PLAYER_DIRECTION) {
        if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }

    } else if (_parameter == AI_ACTION_FLY_OPTION_OPOSITE_WALL) {
        if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
            // invert direction so the character won't be facing the wall
            invert_left_right_direction();
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_OPOSITE_SHOOT_1) {
        int dist = abs(position.x - _origin_point.x);
        if (dist > 120 * _counter) {
        fly_shoot_while_moving();
        }
        if (move_to_point(_dest_point, move_speed, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER_X) {

        struct_player_dist dist_players = dist_npc_players();

        _dest_point.x = dist_players.pObj->getPosition().x;
        if (dist_players.pObj->getPosition().x < position.x) {
            set_direction(ANIM_DIRECTION_LEFT);
        } else {
            set_direction(ANIM_DIRECTION_RIGHT);
        }

        if (abs(dist_players.pObj->getPosition().x - position.x) < TILESIZE/2) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        } else if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER_Y) {
        if (move_to_point(_dest_point, 0, move_speed, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_ZIGZAG_AHEAD) {
        fly_zigzag_update();
    } else if (_parameter == AI_ACTION_FLY_OPTION_SIN_AHEAD) {
        fly_sin_update();
    } else if (_parameter == AI_ACTION_FLY_OPTION_DOWN_RANDOM_DIAGONAL) {
        position.y += move_speed;
        int radius = position.y - _ai_state.initial_position.y;
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            position.x = _ai_state.initial_position.x - (radius * sin(_angle));
        } else {
            position.x = _ai_state.initial_position.x + (radius * sin(_angle));
        }
        //std::cout << "AI_ACTION_FLY_OPTION_DOWN_RANDOM_DIAGONAL - angle[" << _angle << "], x[" << position.x << "]" << std::endl;
        if (position.y > RES_H) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            position.x = _ai_state.initial_position.x;
        }
        //var x = radius * Math.sin(Math.PI * 2 * angle / 360);
    } else if (_parameter == AI_ACTION_FLY_OPTION_UP_UNTIL_GROUND) {
        st_position current_map_point((position.x + frameSize.width/2)/TILESIZE, (position.y + frameSize.height)/TILESIZE);
        int lock = gameControl.get_current_map_obj()->getMapPointLock(current_map_point);
        if (lock == TERRAIN_UNBLOCKED || lock == TERRAIN_WATER) {
            // small adjust to get perfect y
            for (int i=move_speed-1; i>0; i--) {
                st_position current_map_point((position.x + frameSize.width/2)/TILESIZE, (position.y + i + frameSize.height)/TILESIZE);
                int lock = gameControl.get_current_map_obj()->getMapPointLock(current_map_point);
                if (lock != TERRAIN_UNBLOCKED && lock != TERRAIN_WATER) {
                    position.y -= i-1;
                    break;
                }
            }
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        } else {
            position.y -= move_speed;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_DOWN_INTO_GROUND) {
        if (position.y + frameSize.height + move_speed < RES_H-TILESIZE/2) {
            position.y += move_speed;
        } else {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    } else if (_parameter == AI_ACTION_FLY_OPTION_DIAGONAL_UP || _parameter == AI_ACTION_FLY_OPTION_DIAGONAL_DOWN) {
        /*
        int move_speed_y = move_speed;
        int move_speed_x = move_speed;
        if (_parameter == AI_ACTION_FLY_OPTION_DIAGONAL_UP) {
            move_speed_y = -move_speed;
        }
        if (state.direction() == ANIM_DIRECTION_DOWN_LEFT) {
            int move_speed_x = -move_speed;
        }
        */
        can_move_struct check_move_res = check_can_move_to_point(_dest_point, move_speed, move_speed, false, false);
        if (check_move_res.can_move_x == false || check_move_res.can_move_y == false) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
        move_to_point(_dest_point, move_speed, move_speed, is_ghost, false);
    }
}

// OPOSITE_SHOOT_1: a shot every 120 pixels moved
void artificial_inteligence::fly_shoot_while_moving()
{
    int dist = abs(position.x - _origin_point.x);
    if (dist > 120 * _counter) {
        _counter++;

        int proj_direction = state.direction();
        projectile_list.push_back(projectile(_parameter, proj_direction, get_attack_position(), is_player(), _number));
        projectile &temp_proj = projectile_list.back();
        temp_proj.play_sfx(true);
        temp_proj.set_owner(this);

        if (GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_CENTERED) {
            temp_proj.set_owner_direction(state.direction_ref());
            temp_proj.set_owner_position(&position);
        }

        if (GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_TARGET_DIRECTION || GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_TARGET_EXACT || GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_ARC_TO_TARGET || GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_FOLLOW) {
            if (!is_player() && gameControl.get_current_map_obj()->_player_ref != NULL) {
                character* p_player = gameControl.get_current_map_obj()->_player_ref;
                temp_proj.set_target_position(p_player->get_position_ref());
            }
        }

        _did_shot = true;
    }
}

// ZIGZAG_AHEAD: moves forward going up and down, turning around at walls
void artificial_inteligence::fly_zigzag_update()
{
    if (position.x != _dest_point.x) {
        if (_ai_state.initial_position.y == position.y) {
            _dest_point.y = position.y - TILESIZE;
        } else if (position.y <= _ai_state.initial_position.y - TILESIZE) {
            _dest_point.y = position.y + TILESIZE;
        } else if (hit_ground() == true) {
            _dest_point.y = position.y - TILESIZE;
        }
        //std::cout << "### AI::AI_ACTION_FLY_OPTION_ZIGZAG_AHEAD - pos.y[" << position.y << "], dest.y[" << _dest_point.y << "]" << std::endl;
    }

    int pos_x_before = position.x;

    bool res_moved = move_to_point(_dest_point, move_speed, 2, is_ghost, false);
    bool reached_point = position.x == _dest_point.x && position.y == _dest_point.y;

    if ((move_blocked_x_axis == true && move_blocked_y_axis == false && (res_moved == true || (move_speed != 0 && pos_x_before == position.x))) || reached_point == true) {
        turn_around();
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _dest_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
        }
    }
}

// SIN_AHEAD: moves forward following a sine wave
void artificial_inteligence::fly_sin_update()
{

    float dist_y = abs(position.y - _dest_point.y);
    _sin_x += 0.12;
    float sin_value = (TILESIZE*3)*sin(_sin_x);

    if (position.x == _dest_point.x && dist_y < 0.2) {
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    } else {
        if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
            if (dist_y > 0.2) {

                double move_adjust_y = 0.1;
                if (dist_y > sin_value) {
                    move_adjust_y = sin_value;
                } else {
                    if (dist_y > 10) {
                        move_adjust_y = 5;
                    } else if (dist_y > 3) {
                        move_adjust_y = 1;
                    }
                }

                position.y = _ai_state.initial_position.y + move_adjust_y;
            } else {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        } else {
            // reached x, use small y-movement to reach point
            position.y = _ai_state.initial_position.y + sin_value;
        }
    }
}

void artificial_inteligence::execute_ai_step_dash()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(ANIM_TYPE_SLIDE);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        if (_parameter == AI_ACTION_DASH_OPTION_AHEAD) {


            // check if can't move to the current direction and change to other, if needed
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                can_move_struct can_move_left = check_can_move_to_point(st_float_position(position.x-move_speed*4, position.y), -move_speed*2, 0, is_ghost, false);
                if (can_move_left.result != CAN_MOVE_SUCESS || can_move_left.xinc == 0 || can_move_left.can_move_x == false) {
                    set_direction(ANIM_DIRECTION_RIGHT);
                }
            } else if (state.direction() == ANIM_DIRECTION_RIGHT) {
                can_move_struct can_move_right = check_can_move_to_point(st_float_position(position.x+move_speed*4, position.y), move_speed*2, 0, is_ghost, false);
                if (can_move_right.result != CAN_MOVE_SUCESS || can_move_right.xinc == 0 || can_move_right.can_move_x == false) {
                    set_direction(ANIM_DIRECTION_LEFT);
                }
            }

            if (state.direction() == ANIM_DIRECTION_LEFT) {
                _dest_point.x = position.x + frameSize.width/2 - walk_range;
            } else {
                _dest_point.x = position.x + frameSize.width/2 + walk_range;
            }
        } else if (_parameter == AI_ACTION_DASH_OPTION_LEFT) {
            _dest_point.x = position.x + frameSize.width/2 - walk_range;
            set_direction(ANIM_DIRECTION_LEFT);
        } else if (_parameter == AI_ACTION_DASH_OPTION_RIGHT) {
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
            set_direction(ANIM_DIRECTION_RIGHT);
        } else if (_parameter == AI_ACTION_DASH_OPTION_TO_PLAYER) {
            struct_player_dist dist_players = dist_npc_players();
            if (dist_players.pObj->getPosition().x > (position.x  + frameSize.width/2)) {
                set_direction(ANIM_DIRECTION_RIGHT);
                _dest_point.x = position.x + frameSize.width/2 + walk_range;
            } else {
                set_direction(ANIM_DIRECTION_LEFT);
                _dest_point.x = position.x + frameSize.width/2 - walk_range;
            }
            _dest_point.y = position.y;
        } else if (_parameter == AI_ACTION_DASH_OPTION_TO_NEAR_PLAYER) {
            st_rectangle player_hitbox = gameControl.get_current_map_obj()->_player_ref->get_hitbox();
            struct_player_dist dist_players = dist_npc_players();
            if (dist_players.pObj->getPosition().x > (position.x  + frameSize.width/2)) {
                set_direction(ANIM_DIRECTION_RIGHT);
                _dest_point.x = player_hitbox.x - (player_hitbox.w*2 + 10);
            } else {
                set_direction(ANIM_DIRECTION_LEFT);
                _dest_point.x = player_hitbox.x + player_hitbox.w - 10;
            }
            int scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
            _dest_point.y = position.y;
        } else if (_parameter == AI_ACTION_DASH_OPTION_OPPOSITE_DIRECTION) {
            if (realPosition.x > RES_W/2) {
                set_direction(ANIM_DIRECTION_LEFT);
                _dest_point.x = position.x - walk_range;
            } else {
                set_direction(ANIM_DIRECTION_RIGHT);
                _dest_point.x = position.x + walk_range;
            }
            _origin_point.x = position.x;
        } else {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED; // unknown mode
            set_animation_type(ANIM_TYPE_STAND);
        }
    } else {
        if (move_to_point(_dest_point, move_speed*2, 0, is_ghost, false) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            set_animation_type(ANIM_TYPE_STAND);
        }
        if (abs(_dest_point.x - position.x) < TILESIZE) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            set_animation_type(ANIM_TYPE_STAND);
        }
    }
    last_execute_time = timer.getTimer() + 20;
}

void artificial_inteligence::execute_ai_wall_walk()
{
    // INITIALIZATION
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        must_show_dash_effect = false;
        previous_position_list.clear();
        bool must_turn = false;
        if (_parameter == AI_ACTION_WALL_WALK_OPTION_LEFT) {
            _dest_point.x = position.x + frameSize.width/2 - walk_range;
            _dest_point.y = position.y;
            if (state.direction() != ANIM_DIRECTION_LEFT) {
                must_turn = true;
            }
        } else if (_parameter == AI_ACTION_WALL_WALK_OPTION_RIGHT) {
            if (state.direction() != ANIM_DIRECTION_RIGHT) {
                must_turn = true;
            }
            _dest_point.x = position.x + frameSize.width/2 + walk_range;
            _dest_point.y = position.y;
        } else if (_parameter == AI_ACTION_WALL_WALK_OPTION_UP) {
            _dest_point.y = position.y + frameSize.height/2 - walk_range;
            _dest_point.x = position.x;
        } else if (_parameter == AI_ACTION_WALL_WALK_OPTION_DOWN) {
            _dest_point.y = position.y + frameSize.height/2 + walk_range;
            _dest_point.x = position.x;
        }


        if (must_turn) {
            if (state.animation_type() != ANIM_TYPE_TURN && have_frame_graphic(state.direction(), ANIM_TYPE_TURN, (state.animation_state()+1)) == true) {
                set_animation_type(ANIM_TYPE_TURN);
            } else {
                turn_around();
            }
        }

        set_animation_type(ANIM_TYPE_WALK_AIR);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;

    // EXECUTION
    } else {
        if (state.animation_type() == ANIM_TYPE_TURN && have_frame_graphic(state.direction(), state.animation_type(), (state.animation_state()+1)) == false) {
            set_animation_type(ANIM_TYPE_STAND);
            turn_around();
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
        }

        int move_speed_x = 0;
        int move_speed_y = 0;
        if (_parameter == AI_ACTION_WALL_WALK_OPTION_LEFT || _parameter == AI_ACTION_WALL_WALK_OPTION_RIGHT) {
            move_speed_x = move_speed;
        } else if (_parameter == AI_ACTION_WALL_WALK_OPTION_UP || _parameter == AI_ACTION_WALL_WALK_OPTION_DOWN) {
            move_speed_y = move_speed;
        }


        // all actions are just move to point-with-wall
        if (move_to_point(_dest_point, move_speed_x, move_speed_y, is_ghost, true) == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

// returns false if can move and true if blocked
bool artificial_inteligence::move_to_point(st_float_position dest_point, float speed_x, float speed_y, bool can_pass_walls, bool must_walk_along_wall)
{
    move_blocked_x_axis = false;
    move_blocked_y_axis = false;
    can_move_struct move_inc = check_can_move_to_point(dest_point, speed_x, speed_y, can_pass_walls, must_walk_along_wall);

    // decrease move-speed until can move (only for horizontal move, for now)
    if (move_inc.can_move_x == false && speed_x != 0 && abs(speed_x) > 1) {
        int abs_speed_x = abs(speed_x);
        int multiplier = 1;
        if (speed_x < 0) {
            multiplier = -1;
        }
        for (int i=abs_speed_x; i>=1; i--) {
            move_inc = check_can_move_to_point(dest_point, i*multiplier, speed_y, can_pass_walls, must_walk_along_wall);
            if (move_inc.can_move_x == true) {
                break;
            }
        }
    }

    // same for vertical moves: without it a fast NPC stops up to speed-1 pixels away from the floor/ceiling
    if (can_pass_walls == false && move_inc.can_move_y == false && speed_y != 0 && abs(speed_y) > 1) {
        int abs_speed_y = abs(speed_y);
        int multiplier_y = 1;
        if (speed_y < 0) {
            multiplier_y = -1;
        }
        for (int i=abs_speed_y-1; i>=1; i--) {
            can_move_struct try_inc = check_can_move_to_point(dest_point, speed_x, i*multiplier_y, can_pass_walls, must_walk_along_wall);
            if (try_inc.can_move_y == true) {
                move_inc = try_inc;
                break;
            }
        }
    }

    move_blocked_x_axis = !move_inc.can_move_x;
    move_blocked_y_axis = !move_inc.can_move_y;
    //std::cout << "AI::move_to_point[" << name << "] - pos.x[" << position.x << "], dest.x[" << dest_point.x << "], move_inc.result[" << (int)move_inc.result << "], move_inc.can_move_x[" << move_inc.can_move_x << "], move_inc.can_move_y[" << move_inc.can_move_y << "]" << std::endl;

    if (move_inc.result == CAN_MOVE_LEAVE_TRUE) {
        return true;
    } else if (move_inc.result == CAN_MOVE_LEAVE_FALSE) {
        return false;
    }

    if (can_pass_walls) {
        position.x += move_inc.xinc;
        position.y += move_inc.yinc;
        return false;
    } else if ((move_inc.can_move_x == false && move_inc.can_move_y == false) || (move_inc.can_move_x == false && move_inc.yinc == 0) || (move_inc.can_move_y == false && move_inc.xinc == 0)) {
        return true;
    } else {
        if (move_inc.can_move_x == true) {
            position.x += move_inc.xinc;
        }
        if (move_inc.can_move_y == true) {
            position.y += move_inc.yinc;
        }
    }

    return false;
}

can_move_struct artificial_inteligence::check_can_move_to_point(st_float_position dest_point, float speed_x, float speed_y, bool can_pass_walls, bool must_walk_along_wall)
{
    float xinc = 0;
    float yinc = 0;

    // invert direction if needed
    if (xinc != 0 && position.x > dest_point.x && state.direction() != ANIM_DIRECTION_LEFT) {
        set_direction(ANIM_DIRECTION_LEFT);
    }
    if (xinc != 0 && position.x < dest_point.x && state.direction() != ANIM_DIRECTION_RIGHT) {
        set_direction(ANIM_DIRECTION_RIGHT);
    }

    if (abs(dest_point.x - position.x) < speed_x) {
        speed_x = abs(dest_point.x - position.x);
    }
    if (abs(dest_point.y - position.y) < speed_y) {
        speed_y = abs(dest_point.y - position.y);
    }

    float block_speed_x = speed_x;
    float block_speed_y = speed_y;

    if (can_pass_walls == true) {
        must_walk_along_wall = false; // can't have both flags enabled
        if (_ghost_move_speed_reducer > 0 && speed_x != 0) {
            block_speed_x = speed_x/_ghost_move_speed_reducer;
            if ((int)block_speed_x == 0) {
                if (speed_x > 0) {
                    block_speed_x = 1;
                } else {
                    block_speed_x = -1;
                }
            }
        }
        if (_ghost_move_speed_reducer > 0 && speed_y != 0) {
            block_speed_y = (int)(speed_y/_ghost_move_speed_reducer);

            if (block_speed_y == 0) {
                if (speed_y > 0) {
                    block_speed_y = 1;
                } else {
                    block_speed_y = -1;
                }
            }
        }
    }

    if (_ghost_move_speed_reducer > 0) {

        bool test_x_move = test_change_position(block_speed_x, 0);
        bool test_y_move = test_change_position(0, block_speed_y);

        if (!test_x_move) {
            speed_x = block_speed_x;
        }
        if (!test_y_move) {
            speed_y = block_speed_y;
        }
    }


    if (dest_point.x > position.x) {
        xinc = speed_x;
    } else if (dest_point.x < position.x) {
        xinc = -speed_x;
    }

    if (dest_point.y > position.y) {
        yinc = speed_y;
    } else if (dest_point.y < position.y) {
        yinc = -speed_y;
    }

    // checking
    bool can_move_x = true;
    bool can_move_y = true;
    can_move_x = test_change_position(xinc, 0);
    can_move_y = test_change_position(0, yinc);

    //std::cout << "AI::check_can_move_to_point - x[" << position.x << "], xinc[" << xinc << "], can_move_x[" << can_move_x << "]" << std::endl;

    if (must_walk_along_wall == true) {
        if (check_moving_along_wall(xinc, yinc) == false) {
            return can_move_struct(0, 0, false, false, CAN_MOVE_LEAVE_TRUE);
        }
    }

    if (xinc == 0 && yinc == 0) {
        return can_move_struct(0, 0, false, false, CAN_MOVE_LEAVE_TRUE);
    }

    // if is walking check for holes on the path
    int type = get_ai_type();

    // check that we are not in a infinite loop of walking, because if we do, we don't stop moving
    if (_always_move_ahead == false) {
        if ((type == AI_ACTION_WALK || type == AI_ACTION_DASH) && can_fly == false && can_move_x == true && xinc != 0 && yinc == 0) {
            st_position map_point(position.x/TILESIZE, (position.y + frameSize.height + 3)/TILESIZE);
            if (state.direction() == ANIM_DIRECTION_RIGHT) {
                map_point.x = (position.x + frameSize.width)/TILESIZE;
            }
            st_position current_map_point((position.x + frameSize.width/2)/TILESIZE, (position.y + frameSize.height + 3)/TILESIZE);
            int map_lock = gameControl.get_current_map_obj()->getMapPointLock(map_point);
            int current_map_lock = gameControl.get_current_map_obj()->getMapPointLock(current_map_point);

            if ((can_fall_during_move == false || can_fall_during_move == true && map_lock == TERRAIN_SPIKE) && map_lock != current_map_lock) {
                if (map_lock == TERRAIN_UNBLOCKED || map_lock == TERRAIN_WATER || (map_lock == TERRAIN_EASYMODEBLOCK && game_save.difficulty != DIFFICULTY_EASY) || (map_lock == TERRAIN_HARDMODEBLOCK && game_save.difficulty != DIFFICULTY_HARD)) {
                    //std::cout << "AI::check_can_move_to_point - LEAVE #3 - can_fall_during_move[" << can_fall_during_move << "]" << std::endl;
                    return can_move_struct(0, 0, false, false, CAN_MOVE_LEAVE_TRUE);
                }
            }
        }
    // always ahead will try to jump over obstables checking them a whole TILE ahead of time
    } else if (xinc != 0) {
        int xinc2 = xinc + TILESIZE;
        if (xinc < 0) {
            xinc2 = xinc - TILESIZE;
        }

        st_position map_point_ahead((position.x+xinc2)/TILESIZE, (position.y + frameSize.height-2)/TILESIZE);
        int map_lock_ahead = gameControl.get_current_map_obj()->getMapPointLock(map_point_ahead);
        st_position map_point_top((position.x+xinc2)/TILESIZE, (position.y - TILESIZE + frameSize.height-2)/TILESIZE);
        int map_lock_top = gameControl.get_current_map_obj()->getMapPointLock(map_point_top);



        if (hit_ground() == true && speed_y == 0 && speed_x != 0) { // check if is trying to move on X axis only
            if ((map_lock_ahead != TERRAIN_WATER && map_lock_ahead != TERRAIN_UNBLOCKED) && (map_lock_top == TERRAIN_WATER || map_lock_top == TERRAIN_UNBLOCKED)) { // check that the terrain over the block is free
                /// @TODO - implement have a way to change AI type only for a short period then return to the current one
                _current_ai_type = AI_ACTION_JUMP;
                _parameter = AI_ACTION_JUMP_OPTION_ONCE;
                _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
                _ai_state.main_status = 0;
                return can_move_struct(0, 0, false, false, CAN_MOVE_LEAVE_FALSE);
            }
        }
    }

    //if (name == "BOUNCING BALL") std::cout << "AI::check_can_move_to_point - END, xinc[" << xinc << "], can_move_x[" << can_move_x << "], can_move_y[" << can_move_y << "]" << std::endl;
    return can_move_struct(xinc, yinc, can_move_x, can_move_y, CAN_MOVE_SUCESS);
}

bool artificial_inteligence::check_moving_along_wall(int xinc, int yinc)
{
    if (xinc != 0) {
        st_position check_point_top;
        st_position check_point_bottom;
        if (xinc > 0) {
            check_point_top = st_position((position.x+frameSize.width+xinc)/TILESIZE, (position.y-4)/TILESIZE);
            check_point_bottom = st_position((position.x+frameSize.width+xinc)/TILESIZE, (position.y+frameSize.height+4)/TILESIZE);
        } else {
            check_point_top = st_position((position.x+xinc)/TILESIZE, (position.y-4)/TILESIZE);
            check_point_bottom = st_position((position.x+xinc)/TILESIZE, (position.y+frameSize.height+4)/TILESIZE);
        }
        int point_lock_top = gameControl.get_current_map_obj()->getMapPointLock(check_point_top);
        int point_lock_bottom = gameControl.get_current_map_obj()->getMapPointLock(check_point_bottom);

        if (point_lock_top == TERRAIN_UNBLOCKED && point_lock_bottom == TERRAIN_UNBLOCKED) {
            return false;
        }
    } else if (yinc != 0) {
        st_position check_point_left;
        st_position check_point_right;
        if (yinc > 0) {
            check_point_left = st_position((position.x-4)/TILESIZE, (position.y+frameSize.height+yinc)/TILESIZE);
            check_point_right = st_position((position.x+4+frameSize.width)/TILESIZE, (position.y+frameSize.height+yinc)/TILESIZE);
        } else {
            check_point_left = st_position((position.x-4)/TILESIZE, position.y/TILESIZE);
            check_point_right = st_position((position.x+4+frameSize.width)/TILESIZE, (position.y+yinc)/TILESIZE);
        }
        int point_lock_left = gameControl.get_current_map_obj()->getMapPointLock(check_point_left);
        int point_lock_right = gameControl.get_current_map_obj()->getMapPointLock(check_point_right);
        if (point_lock_left == TERRAIN_UNBLOCKED && point_lock_right == TERRAIN_UNBLOCKED) {
            return false;
        }
    }
    return true;
}

void artificial_inteligence::randomize_x_point(int max_adjust)
{
    if (max_adjust < 1) {
        return;
    }
    int rand_x = rand() % max_adjust*2;
    position.x = position.x + rand_x - max_adjust;
}

int artificial_inteligence::create_rand_x_point(int max_range)
{
    if (max_range < 1) {
        return position.x; // walk_range 0 would divide by zero
    }
    int rand_x = position.x;
    int scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
    int loop_count = 0;
    while (loop_count < 100) {
        rand_x = position.x + rand() % max_range*2 - max_range;
        if (rand_x < scroll_x+TILESIZE || rand_x+frameSize.width > scroll_x+RES_W-TILESIZE) {
            loop_count++;
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((rand_x+frameSize.width/2)/TILESIZE, (position.y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            return rand_x;
        }
        loop_count++;
    }
    return position.x;
}

int artificial_inteligence::create_rand_y_point(int max_range)
{
    if (max_range < 1) {
        return position.y;
    }
    int rand_y = position.y;
    int loop_count = 0;
    while (loop_count < 100) {
        rand_y = position.y + rand() % max_range*2 - max_range;
        if (rand_y < 0 || rand_y+frameSize.height > RES_H-TILESIZE) {
            loop_count++;
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (rand_y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            return rand_y;
        }
        loop_count++;
    }
    return position.y;
}

st_position artificial_inteligence::create_rand_point(int max_range)
{
    if (max_range < 1) {
        return st_position(position.x, position.y);
    }
    int rand_x = position.x;
    int rand_y = position.y;
    int scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
    int loop_count = 0;
    while (loop_count < 100) {
        rand_x = position.x + rand() % max_range*2 - max_range;
        rand_y = position.y + rand() % max_range*2 - max_range;
        // TODO: check if new position X is out of visible screen
        if (rand_x < scroll_x+TILESIZE || rand_x+frameSize.width > scroll_x+RES_W-TILESIZE) {
            loop_count++;
            continue;
        }

        // check if new position Y is out of visible screen
        if (rand_y < 0 || rand_y+frameSize.height > RES_H-TILESIZE) {
            loop_count++;
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((rand_x+frameSize.width/2)/TILESIZE, (rand_y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            return st_position(rand_x, rand_y);
        }
        loop_count++;
    }
    return st_position(position.x, position.y);
}

void artificial_inteligence::execute_ai_circle_player()
{
    // define point near player to move to initially
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _origin_point.x = position.x;
        _origin_point.y = position.y;
        // avoid erros if user forgot to set those
        is_ghost = true;
        can_fly = true;
        struct_player_dist dist_players = dist_npc_players();

        _target_point.x = dist_players.pObj->getPosition().x +  dist_players.pObj->get_size().width / 2;
        _target_point.y = dist_players.pObj->getPosition().y + dist_players.pObj->get_size().height / 2;

        if (dist_players.pObj->getPosition().x > position.x) {
            set_direction(ANIM_DIRECTION_RIGHT);
            _dest_point.x = dist_players.pObj->getPosition().x - (_parameter * TILESIZE) - (frameSize.width *2);
        } else {
            set_direction(ANIM_DIRECTION_LEFT);
            _dest_point.x = dist_players.pObj->getPosition().x + (_parameter * TILESIZE) + (frameSize.width * 2);
        }

        _execution_state = 0;
        _dest_point.y = _target_point.y;
        // @TODO - use TURN frames to leave stand

        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    // executing
    } else {
        if (_execution_state == 0) {
            // moving from stand point to near-player
            if (move_to_point(_dest_point, move_speed, move_speed, true, false) == true) {
                _execution_state = 1;
                _counter = 0;
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    _angle = 0;
                } else {
                    _angle = 3.14;
                }
                radius = _parameter * TILESIZE + frameSize.width;
            }
        // circle player (3 and half laps)
        } else if (_execution_state == 1) {
            float x = _target_point.x + radius * cos(_angle) - (TILESIZE*1.5);
            float y = _target_point.y + radius * sin(_angle);

            //move_to_point(st_float_position(x, y), move_speed, move_speed, is_ghost);
            position.x = x;
            position.y = y;
            _angle += 0.1;

            if (state.direction() == ANIM_DIRECTION_RIGHT) {
                if (_angle >= 2 * 3.14) {
                    _counter++;
                    _angle = 0;
                }
            } else {
                if (_angle >= (2 * 3.14 + 3.14)) {
                    _counter++;
                    _angle = 3.14;
                }
            }
            if (_counter >= 3) {
                _execution_state = 2;
            }
        // finished, pick a new point to return to stand
        } else if (_execution_state == 2) {
            // @TODO: randomize x point distance
            _dest_point.y = _origin_point.y;
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                _dest_point.x = position.x - walk_range;
            } else {
                _dest_point.x = position.x + walk_range;
            }
            _execution_state = 3;
        // returning to stand
        } else if (_execution_state == 3) {
            if (move_to_point(_dest_point, move_speed, move_speed, true, false) == true) {
                _execution_state = 0;
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        }
    }
}

void artificial_inteligence::ia_action_teleport()
{
    // go teleporting
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {

        if (state.animation_type() != ANIM_TYPE_TELEPORT) { /// @TODO - must use teleport
            // remove all projectiles, so we don't end with a circle one around an emey that moved
            clean_effect_projectiles();
            set_animation_type(ANIM_TYPE_TELEPORT);
        } else {
            if (state.on_last_frame() == true) { // finished teleport animation
                _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
                state.invisible = true;


                // find wall to the left
                if (_parameter == AI_ACTION_TELEPORT_OPTION_LEFT || (_parameter == AI_ACTION_TELEPORT_OPTION_AHEAD && state.direction() == ANIM_DIRECTION_LEFT)) {
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(st_position(position.x, position.y+frameSize.height/2), st_size(walk_range, 0), ANIM_DIRECTION_LEFT);
//                    std::cout << "AI::teleport[left] - x[" << position.x <<"], dest_pos.x[" << dest_pos.x << "]" << std::endl;
                    position.x = dest_pos.x;
                // find wall to the right
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_RIGHT || (_parameter == AI_ACTION_TELEPORT_OPTION_AHEAD && state.direction() == ANIM_DIRECTION_RIGHT)) {
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(st_position(position.x+frameSize.width, position.y+frameSize.height/2), st_size(walk_range, 0), ANIM_DIRECTION_RIGHT);
//                    std::cout << "AI::teleport[right] - x[" << position.x <<"], dest_pos.x[" << dest_pos.x << "]" << std::endl;
                    position.x = dest_pos.x-frameSize.width;
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_TO_PLAYER) {
                    struct_player_dist dist_npc_player = dist_npc_players();
                    position.x = dist_npc_player.pObj->getPosition().x;
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_RANDOM_X) {
                    int rand_x = create_rand_x_point(walk_range);
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(st_position(position.x, position.y+frameSize.height/2), st_size(abs(position.x-rand_x), 0), (rand_x > position.x));
                    position.x = dest_pos.x;
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_RANDOM_Y) {
                    int rand_y = create_rand_y_point(walk_range);
                    int direction = ANIM_DIRECTION_UP;
                    st_position pos(position.x+frameSize.width/2, position.y);
                    if (rand_y > position.y) {
                        pos = st_position(position.x+frameSize.width/2, position.y+frameSize.height);
                        direction = ANIM_DIRECTION_DOWN;
                    }
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(pos, st_size(0, abs(position.y-rand_y)), direction);
                    if (direction == ANIM_DIRECTION_DOWN) {
                        position.y = dest_pos.y - frameSize.height;
                    } else {
                        position.y = dest_pos.y;
                    }
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_RANDOM_POINT) {
                    st_position rand_pos = create_rand_point(walk_range);

                    // if y is -TILESIZE or less, use half screen-height, as it can be a boss out of screen due to initial pos
                    if (position.y < -TILESIZE) {
                        position.y = RES_H/2;
                    }

                    int direction = ANIM_DIRECTION_LEFT;
                    st_position pos(position.x, position.y+frameSize.height/2);
                    if (rand_pos.y > position.y) {
                        if (rand_pos.x > position.x) {
                            direction = ANIM_DIRECTION_DOWN_RIGHT;
                            pos = st_position(position.x+frameSize.width, position.y+frameSize.height);
                        } else {
                            direction = ANIM_DIRECTION_DOWN_LEFT;
                            pos = st_position(position.x, position.y+frameSize.height);
                        }
                    } else {
                        if (rand_pos.x > position.x) {
                            direction = ANIM_DIRECTION_UP_RIGHT;
                            pos = st_position(position.x+frameSize.width, position.y);
                        } else {
                            direction = ANIM_DIRECTION_UP_LEFT;
                            pos = st_position(position.x, position.y);
                        }
                    }
                    int calc_y = abs(position.y-rand_pos.y);
                    position = gameControl.get_current_map_obj()->get_first_lock_in_direction(pos, st_size(abs(position.x-rand_pos.x), calc_y), direction);
                    if (direction == ANIM_DIRECTION_UP_RIGHT || direction == ANIM_DIRECTION_DOWN_RIGHT) {
                        position.x -= frameSize.width;
                    }
                    if (direction == ANIM_DIRECTION_DOWN_LEFT || direction == ANIM_DIRECTION_DOWN_RIGHT) {
                        position.y -= frameSize.height;
                    }
                }
                _ai_timer = timer.getTimer() + 600;
                state.set_animation_restarted(false);
                _ignore_gravity = true;
                state.set_animation_state(0);
            }
        }
    // return from teleporting
    } else {
        if (state.invisible == true) {
            state.invisible = false;
            state.set_animation_inverse(true);
            /// @TODO: to avoid this problem,c reate method to start inverse animation ///
            advance_to_last_frame(); // always call this once setting animation_inverse to true to avoid false state.animation_restarted()
            state.set_animation_restarted(false);
        }
        if (state.animation_restarted() == true) {
            set_animation_type(ANIM_TYPE_STAND);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _ignore_gravity = false;
            state.set_animation_inverse(false);
        }
    }
}
