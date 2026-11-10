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



void artificial_inteligence::ia_action_jump_to_player()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        struct_player_dist dist_npc_player = dist_npc_players();
        // do not execute this action if distance is less than 3 tiles
        if (dist_npc_player.dist < TILESIZE*3) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        } else {
            int px = dist_npc_player.pObj->getPosition().x + dist_npc_player.pObj->get_size().width/2;
            _dest_point.x = px - frameSize.width/2;
            _dest_point.y = dist_npc_player.pObj->getPosition().y + dist_npc_player.pObj->get_size().height;
            _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        }
    } else {
        ia_action_jump_to_point(st_position(_dest_point.x, _dest_point.y));
        if ( _ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
             _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

// start of a jump to a point: direction, parabola and the starting position (too short jumps are skipped)
void artificial_inteligence::jump_to_point_start(st_position point, float xinc)
{
    int jump_dist = abs(position.x - point.x);
    if (jump_dist < TILESIZE/2) { // avoid too short jumps
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }
    if (position.x < point.x) {
        set_direction(ANIM_DIRECTION_RIGHT);
    } else {
        set_direction(ANIM_DIRECTION_LEFT);
    }
    if (xinc <= 0) {
        jump_pos_x = position.x - point.x;
    } else {
        jump_pos_x = point.x - position.x;
    }
    _trajectory_parabola = trajectory_parabola(jump_pos_x);
    _ai_state.initial_position.x = position.x;
    _ai_state.initial_position.y = position.y;
    set_animation_type(ANIM_TYPE_JUMP);
    _ai_state.sub_action_sub_status = IA_ACTION_STATE_EXECUTING;
    _last_jump_yinc = 0;
    _ignore_gravity = true; // disable gravity
    _origin_point.x = 0; // used when can't move x to continue increasing value for the parabola method
}

// one step of the jump along the parabola: wall and ceiling hits, jump attack, landing
void artificial_inteligence::jump_to_point_step(float xinc, float jump_yinc_multiplier)
{
    bool can_move_x = test_change_position(xinc, 0);
    if (can_move_x == false) { // hit a wall, fall
        _origin_point.x += xinc;
        xinc = 0;
    }

    // TODO - add multiplier

    int new_x = abs((position.x + _origin_point.x) - _ai_state.initial_position.x);
    int new_y = _ai_state.initial_position.y - (_trajectory_parabola.get_y_point(new_x) * jump_yinc_multiplier);
    int yinc = position.y - new_y;

    if (abs(yinc) >= TILESIZE) {
        if (yinc > 0) {
            yinc = TILESIZE-1;
        } else {
            yinc = -(TILESIZE-1);
        }
    }
    if (yinc >= TILESIZE-1) { // never move more than tilesize, no matter if will change jump behavior or else you could end up out of screen
        yinc = TILESIZE-2;
    }

    position.x += xinc;
    bool can_move_y = false;
    can_move_y = test_change_position(0, yinc*-1);

    // check hit-head when going up
    if (yinc > 0 && can_move_y == false) { // falling. @NOTE: y values are inverted, so > 0 meand going up and < 0 meand going down
        bool found_point = false;
        if (_last_jump_yinc > 0 || yinc != 0) {
            // reduce Y in case hit head
            for (int i=yinc*-1; i>0; i--) {
                can_move_y = test_change_position(0, i);
                if (can_move_y == true) {
                    yinc = i;
                    found_point = true;
                    break;
                }
            }
        }
        // ignore Y movement if can't move in this axis
        /// @TODO: increase X virtual position until the point the NPC starts falling
        if (found_point == false) {
            int temp_xinc = 1;
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                temp_xinc = -1;
            }
            int temp_yinc = 1;
            int temp_pos_x = new_x;

            int loop_count = 0;
            while (loop_count < 1000) { // search for the point when parabole changes signal
                int temp_new_y = _ai_state.initial_position.y - _trajectory_parabola.get_y_point(temp_pos_x);
                temp_yinc = position.y - temp_new_y;

                if (temp_yinc >= 0) {
                    int temp_diff = position.x - temp_pos_x;
                    _origin_point.x += temp_diff;
                    break;
                }
                temp_pos_x += temp_xinc;
                loop_count++;
            }
            //std::cout << "AI::ia_action_jump_to_point - LEAVE #1" << std::endl;
            return;
        }
    }


    // jump-attack
    if (yinc < 0 && can_move_y == true && jump_attack_type != -1) {
        if (have_frame_graphic(state.direction(), ANIM_TYPE_JUMP_ATTACK, 0) == true) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        }
        throw_projectile(jump_attack_type, false);
        jump_attack_type = -1;
    }


    // check ground when falling
    if (yinc < 0 && can_move_y == false) { // falling. @NOTE: y values are inverted, so > 0 meand going up and < 0 meand going down
        bool found_point = false;
        if (_last_jump_yinc > 0 || yinc != 0) {
            // reduce Y in case hit ground
            for (int i=yinc; i>=0; i--) {
                can_move_y = test_change_position(0, i);
                if (can_move_y == true) {
                    yinc = i;
                    found_point = true;
                    break;
                }
            }
        }
        if (found_point == false) {
            _ignore_gravity = false; // enable gravity
            _ai_state.sub_action_sub_status = IA_ACTION_STATE_FINISHED;
            set_animation_type(ANIM_TYPE_STAND);
            moveCommands.right = 0;
            moveCommands.left = 0;
            _ai_timer = timer.getTimer() + 500;
            if (xinc == 0) { // if didn't moved x, then must change AI to other action
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
            //std::cout << "AI::ia_action_jump_to_point - FINISH" << std::endl;
            return;
        }
    }
    position.y -= yinc;
    _last_jump_yinc = yinc;
    //std::cout << "AI::ia_action_jump_to_point - xinc[" << xinc << "], yinc[" << yinc << "]" << std::endl;
}

void artificial_inteligence::ia_action_jump_to_point(st_position point)
{
    float xinc = 0;
    float jump_xinc_multiplier = GameMediator::get_instance()->get_enemy_extra_data(_number)->jump_speed_x;
    float jump_yinc_multiplier = GameMediator::get_instance()->get_enemy_extra_data(_number)->jump_speed_y;

    // hack to keep the old behavior for enemies that have not set a custom value
    if (move_speed == 1 && jump_xinc_multiplier == 1 && jump_yinc_multiplier == 1) {
        jump_xinc_multiplier = 4;
    }

    if (state.direction() == ANIM_DIRECTION_LEFT) {
        xinc = -move_speed*jump_xinc_multiplier; /// @TODO - check collision against walls (will have to "fake" the x position to continue jump movement)
    } else {
        xinc = move_speed*jump_xinc_multiplier; /// @TODO - check collision against walls (will have to "fake" the x position to continue jump movement)
    }

    if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_INITIAL) {
        jump_to_point_start(point, xinc);
    } else if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_EXECUTING) {
        jump_to_point_step(xinc, jump_yinc_multiplier);
    }
}

void artificial_inteligence::ia_action_jump_to_random()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        int dist = 0;
        int rand_x = position.x;
        int loop_count = 0;
        while (dist < TILESIZE*4 && loop_count < 100) {
            rand_x = rand() % RES_W;
            dist = abs(position.x - rand_x);
            loop_count++;
        }
        _dest_point.x = rand_x;
        _dest_point.y = position.y;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else {
        ia_action_jump_to_point(st_position(_dest_point.x, _dest_point.y));
        if ( _ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
             _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::ia_action_jump_ahead()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _origin_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _origin_point.x = position.x + frameSize.width/2 + walk_range;
        }
        _dest_point.x = calc_jump_pos_x(move_speed);
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        _dest_point.y = position.y;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else {
        ia_action_jump_to_point(st_position(_dest_point.x, _dest_point.y));
        //std::cout << "AI::ia_action_jump_ahead::EXECUTE, status[" << _ai_state.sub_action_sub_status << "]" << std::endl;
        if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
            /*
            // only stop when reached _origin_poin
            if (abs(_origin_point.x - position.x) < TILESIZE*4) { // check how much we need to jump yet
                std::cout << "AI::ia_action_jump_ahead::FINISHED" << std::endl;
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                return;
            }
            _dest_point.x = calc_jump_pos_x(move_speed);
            std::cout << "AI::ia_action_jump_ahead::RESTART" << std::endl;
            _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
            */
        }
    }
}

void artificial_inteligence::ia_action_jump_long_ahead()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _origin_point.x = position.x - frameSize.width/2 - walk_range;
        } else {
            _origin_point.x = position.x + frameSize.width/2 + walk_range;
        }
        _dest_point.x = calc_jump_pos_x(LONG_JUMP_DIST);
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        _dest_point.y = position.y;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else {
        ia_action_jump_to_point(st_position(_dest_point.x, _dest_point.y));
        if ( _ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
            if (abs(_origin_point.x - position.x) < TILESIZE*4) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                return;
            }
            _dest_point.x = calc_jump_pos_x(LONG_JUMP_DIST);
            _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        }
    }
}

int artificial_inteligence::calc_jump_pos_x(int distance)
{
    if (distance < 1) {
        distance = 1;
    }
    int dest_point_x = position.x + TILESIZE*distance;
    if (state.direction() == ANIM_DIRECTION_LEFT) {
        dest_point_x = position.x - TILESIZE*distance;
    }
    return dest_point_x;
}

void artificial_inteligence::ia_action_jump_once()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (_parameter == AI_ACTION_JUMP_OPTION_TO_PLAYER_DIRECTION) {
            struct_player_dist dist_players = dist_npc_players();
            if (dist_players.pObj->getPosition().x > position.x) {
                set_direction(ANIM_DIRECTION_RIGHT);
            } else {
                set_direction(ANIM_DIRECTION_LEFT);
            }
        }
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            _origin_point.x = position.x - frameSize.width/2 - walk_range;
            _dest_point.x = position.x - TILESIZE*4;
        } else {
            _origin_point.x = position.x + frameSize.width/2 + walk_range;
            _dest_point.x = position.x + TILESIZE*4;
        }
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        _dest_point.y = position.y;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else {
        ia_action_jump_to_point(st_position(_dest_point.x, _dest_point.y));
        if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::ia_action_jump_up()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        _ai_state.action_status = 0;
        set_animation_type(ANIM_TYPE_JUMP);
        _obj_jump.start(false, TERRAIN_UNBLOCKED);
        moveCommands.jump = 1;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (_ai_state.action_status == 0) {
            moveCommands.jump = 1;
            _ai_state.action_status++;
        } else {
            moveCommands.jump = 1;
            float speed = _obj_jump.get_speed();
            // execute attack, if needed
            if (speed >= 0 && jump_attack_type != -1) {
                if (have_frame_graphic(state.direction(), ANIM_TYPE_JUMP_ATTACK, 0) == true) {
                    set_animation_type(ANIM_TYPE_JUMP_ATTACK);
                }
                throw_projectile(jump_attack_type, false);
                jump_attack_type = -1;
            } else if (speed >= 0 && hit_ground() == true) {
                moveCommands.jump = 0;
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                set_animation_type(ANIM_TYPE_STAND);
                _ai_state.action_status = 0;
                _obj_jump.finish();
            }
        }
    }
}

void artificial_inteligence::ia_action_jump_teleport_in()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(ANIM_TYPE_TELEPORT);
        first_unlocked_from_bottom = gameControl.get_current_map_obj()->get_first_lock_on_bottom(position.x, position.y, frameSize.width, frameSize.height);
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (position.y + move_speed*2 <= first_unlocked_from_bottom) {
            position.y += move_speed*2;
        } else {
            position.y = first_unlocked_from_bottom - frameSize.height;
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::ia_action_jump_teleport_out()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        teleporting_out++;
        set_animation_type(ANIM_TYPE_TELEPORT);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        position.y -= move_speed*2;
        if (position.y + frameSize.height < 0) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            hitPoints.current = 0;
            dead = true;
            teleporting_out++;
        }
    }
}

void artificial_inteligence::ia_action_jump_to_roof()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(ANIM_TYPE_JUMP);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;

        // find the end-point of the jump-up
        int limit_y = 0;
        for (int y=position.y; y>=0; y--) {
            st_position new_pos((position.x+frameSize.width/2)/TILESIZE, (y)/TILESIZE);
            if (gameControl.get_current_map_obj()->is_point_solid(new_pos) == true) {
                limit_y = y;
                break;
            }
        }
        if (limit_y == 0) {
            limit_y = 1;
        }
        _ai_state.secondary_position.y = limit_y;
        _ai_state.step = (limit_y - position.y)/max_speed;
        _ignore_gravity = true; // disable gravity
        speed_y = max_speed*2;
        return;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        // executing
        if (position.y > _ai_state.secondary_position.y) {
            ia_accelerate_up();
        } else {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::ia_action_air_walk()
{

    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(ANIM_TYPE_WALK_AIR);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        struct_player_dist dist_npc_player = dist_npc_players();
        _dest_point.x = dist_npc_player.pObj->getPosition().x;
        if (_dest_point.x > position.x) {
            state.set_direction(ANIM_DIRECTION_RIGHT);
        } else {
            state.set_direction(ANIM_DIRECTION_LEFT);
        }
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        // IURI - colocar verificação de colisão aqui
        /// @TODO use move speed and reducer for last part
        if (move_to_point(_dest_point, move_speed, 0, false, false) == true) {
            set_animation_type(ANIM_TYPE_JUMP);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::ia_action_jump_fall()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        set_animation_type(ANIM_TYPE_JUMP);
        speed_y = 5;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        ia_accelerate_down();
        if (hit_ground() == true) {
            set_animation_type(ANIM_TYPE_STAND);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _ai_timer = timer.getTimer() + 1200;
            _ignore_gravity = false;
        }
    }
}

void artificial_inteligence::ia_accelerate_up()
{
    speed_y -= speed_y*acceleration_y;
    if (speed_y < JUMP_ROOF_MIN_SPEED) {
        speed_y = JUMP_ROOF_MIN_SPEED;
    } else if (speed_y > TILESIZE) {
        speed_y = TILESIZE-2;
    } else if (speed_y > max_speed*2) {
        speed_y = max_speed*2;
    }
    position.y -= speed_y;
    /// @TODO - adjustment when ground is near
}

void artificial_inteligence::ia_accelerate_down()
{
    speed_y += speed_y*acceleration_y*4;
    if (speed_y < 1) {
        speed_y = acceleration_y*4;
    } else if (speed_y > TILESIZE-1) {
        speed_y = TILESIZE-1;
    }
    // adjust fall to hit ground exactly on the point
    if (will_hit_ground(speed_y) == true) {
        for (int i=speed_y-1; i>=0; i--) {
            if (will_hit_ground(i-1) == false) {
                speed_y = i;
                break;
            }
        }
    }
    position.y += speed_y;
    /// @TODO - adjustment when ground is near
}

void artificial_inteligence::execute_ai_step_jump()
{
    if (_parameter == AI_ACTION_JUMP_OPTION_TO_ROOF) {
        ia_action_jump_to_roof();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_PLAYER) {
        ia_action_jump_to_player();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_AHEAD) {
        ia_action_jump_ahead();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_LONG_AHEAD) {
        ia_action_jump_long_ahead();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_RANDOM_POINT) {
        ia_action_jump_to_random();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_ONCE) {
        ia_action_jump_once();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_UP) {
        ia_action_jump_up();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_PLAYER_DIRECTION) {
        ia_action_jump_once();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TELEPORT_IN) {
        ia_action_jump_teleport_in();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TELEPORT_OUT) {
        ia_action_jump_teleport_out();
    }

}

int artificial_inteligence::find_wall(float initial_x, int direction)
{
    int ini_x = initial_x / TILESIZE;
    int pos_x = -1;
    if (direction == ANIM_DIRECTION_LEFT) {
        pos_x = gameControl.get_current_map_obj()->get_first_lock_on_left(ini_x);
    } else if (direction == ANIM_DIRECTION_RIGHT) {
        pos_x = gameControl.get_current_map_obj()->get_first_lock_on_right(ini_x);
    } else if (direction == ANIM_DIRECTION_UP) {
        pos_x = gameControl.get_current_map_obj()->get_first_lock_on_bottom(ini_x, -1);
    } else if (direction == ANIM_DIRECTION_DOWN) {
        pos_x = gameControl.get_current_map_obj()->get_first_lock_on_right(ini_x);
    }
    return pos_x;
}

void artificial_inteligence::execute_ai_step_jump_to_wall()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        struct_player_dist dist_players = dist_npc_players();
        if (dist_players.pObj->getPosition().x > position.x) {
            set_direction(ANIM_DIRECTION_RIGHT);
        } else {
            set_direction(ANIM_DIRECTION_LEFT);
        }
        int pos_x = find_wall(position.x, state.direction());
        if (pos_x == -1) { // no wall found to grab
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return; // it went on and overwrote the state, jumping towards x=-1
        }
        _dest_point.x = pos_x;
        _dest_point.y = RES_H/2;

        int dist_x = _dest_point.x - position.x;
        int dist_y = _dest_point.y - position.y;
        if (dist_x == 0) { // already at the wall (the speed calculation divides by it)
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
        }
        _diagonal_speed.x = move_speed*2;
        _diagonal_speed.y = (_diagonal_speed.x * dist_y) / dist_x;
        if (_diagonal_speed.y < 0) { // move_to_point always use positive values, and adjust depending on target itself
            _diagonal_speed.y = _diagonal_speed.y * -1;
        }
        _ignore_gravity = true; // disable gravity
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        set_animation_type(ANIM_TYPE_JUMP);
        state.set_animation_state(0);
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (move_to_point(_dest_point, _diagonal_speed.x, _diagonal_speed.y, is_ghost, false) == true) {
            state.set_animation_state(0);
            set_animation_type(ANIM_TYPE_WALK_AIR);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            turn_around();
        }
    }
}
