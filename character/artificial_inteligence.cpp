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

std::vector<character*> *artificial_inteligence::player_list=NULL;


artificial_inteligence::artificial_inteligence() :  walk_range(TILESIZE*6), target(NULL), speed_y(max_speed), acceleration_y(0.05), _ai_timer(0), _ai_chain_n(0), _trajectory_parabola(NULL)
{
    max_speed = GRAVITY_MAX_SPEED * SharedData::get_instance()->get_movement_multiplier();
    _ghost_move_speed_reducer = 0;
    _did_shot = false;
    _reaction_state = 0;
    _reaction_type = 0;
    _last_jump_yinc = 0;
    _check_always_move_ahead = true;
    _initialized = 0;
    _ai_state.main_status = 0;
    _parameter = 0;
    _show_reset_stand = false;
    _auto_respawn_timer = timer.getTimer() + GameMediator::get_instance()->get_enemy(_number)->respawn_delay;
    _dest_point = position;
    _execution_state = 0;
    jump_attack_type = -1;
    _current_ai_type = -1;
    did_hit_player = false;
    _saved_point = st_float_position(-99, -99);

}


artificial_inteligence::~artificial_inteligence()
{

}


void artificial_inteligence::execute_ai()
{
    // initialize saved point, if none was set yet
    if (_saved_point.x == -99 && _saved_point.y == -99) {
        _saved_point = position;
    }
    if (_check_always_move_ahead == true) {
        _always_move_ahead = always_move_ahead();
        _check_always_move_ahead = false;
    }

    check_ai_reaction();

    if (is_stage_boss() && get_anim_type() == ANIM_TYPE_INTRO) {
        return;
    }
    if (timer.getTimer() < _ai_timer) {
        return;
    }
    if (SharedData::get_instance()->is_showing_boss_intro == true) {
        return;
    }

    // check if action is finished
    if (_current_ai_type == -1 || _ai_state.sub_status == IA_ACTION_STATE_FINISHED) {
        if (_current_ai_type != AI_ACTION_WAIT_RANDOM_TIME) { // this AI will set the delay itself
            if (_reaction_type == 0) {
                int delay = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to_delay;
                _ai_timer = timer.getTimer() + delay;
            } else {
                _ai_timer = timer.getTimer() + 200;
            }
            started_action_timer = _ai_timer;
        }

        _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
        define_ai_next_step();
    } else {
        execute_ai_step();
    }
    // reset flag
    did_hit_player = false;
}

void artificial_inteligence::hit_player()
{
    did_hit_player = true;
}

void artificial_inteligence::check_ai_reaction()
{
    // check and reset state if needed
    if (_reaction_state == 1) {
        if (_ai_state.sub_status == IA_ACTION_STATE_FINISHED) {
            _reaction_state = 0;
        }
        return; // do not check again if already executing
    }

    bool start_reaction = false;
    // near player
    struct_player_dist dist_players = dist_npc_players();
    int diff_y = abs((dist_players.pObj->getPosition().y+dist_players.pObj->get_size().height) - (position.y+frameSize.height));

    if (dist_players.dist < TILESIZE*4 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_ON_RANGE].action > 0) {
        _reaction_type = 0;
        start_reaction = true;
    // hit
    } else if (_was_hit == true && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_HIT].action > 0) {
        _reaction_type = 1;
        start_reaction = true;
    // dead
    } else if (hitPoints.current <= 0 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_DEAD].action > 0) {
        _reaction_type = 2;
        start_reaction = true;

        // if not sub-boss (that already have explosion), and dead-reaction is spawn npc, show explosions
        soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 1);
        st_float_position pos1(position.x+2, position.y+20);
        animation anim1(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos1, st_position(-8, -8), 80, 2, state.direction, st_size(56, 56), gameControl.get_current_map_obj()->get_map_scrolling_ref());
        gameControl.get_current_map_obj()->add_animation(anim1);

        st_float_position pos2(pos1.x+10, pos1.y-30);
        animation anim2(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos2, st_position(-8, -8), 80, 2, state.direction, st_size(56, 56), gameControl.get_current_map_obj()->get_map_scrolling_ref());
        anim2.set_initial_delay(500);
        gameControl.get_current_map_obj()->add_animation(anim2);

    } else if (dist_players.dist < walk_range && diff_y < 2 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_SAME_Y].action > 0) {
        _reaction_type = 3;
        start_reaction = true;
    } else if (dist_players.dist < walk_range/4 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_CLOSE].action > 0) {
        _reaction_type = 4;
        start_reaction = true;
    }

    _was_hit = false; // reset flag

    if (start_reaction == true) {

        // do not start a walk-reaction in middle air
        int react_type = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action;
        react_type--;
        if (react_type == AI_ACTION_WALK && hit_ground() == false && can_fly == false) {
            for (int i=0; i<RES_H; i++) {
                gravity(false);
                if (hit_ground() == true) {
                    break;
                }
            }
        }
        _reaction_state = 1;

        _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
        _ai_timer = timer.getTimer(); // start now, ignoring delay
        _current_ai_type = get_ai_type();
    }
}

void artificial_inteligence::define_ai_next_step()
{
    // could not execute shot, ignore next step
    bool must_ignore_next = (shot_success == false && _current_ai_type == AI_ACTION_SHOT_PROJECTILE_AHEAD);
    if (must_ignore_next == false && (_initialized == 0 || GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to == AI_ACTION_GOTO_CHANCE)) { // CHANCE
        _initialized = 1;
        int rand_n = rand() % 100;

        bool found_chance = false;
        int chance_sum = 0;
        for (int i=0; i<AI_MAX_STATES; i++) {
            chance_sum += GameMediator::get_instance()->ai_list.at(_number).states[i].chance;
            if (rand_n < chance_sum) {
                _ai_chain_n = i;
                found_chance = true;
                break;
            }
        }
        if (found_chance == false) {
            _ai_chain_n = 0;
        }
    } else {
        _ai_chain_n = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to-1;
        if (_ai_chain_n < 0) {
            _ai_chain_n = 0;
        }
    }
    _current_ai_type = get_ai_type();
    _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
}


void artificial_inteligence::execute_ai_step()
{
    _ai_timer = timer.getTimer() + 20;
    if (_current_ai_type == AI_ACTION_WALK) {
        execute_ai_step_walk();
    } else if (_current_ai_type == AI_ACTION_FLY) {
        execute_ai_step_fly();
    } else if (_current_ai_type == AI_ACTION_JUMP) {
        execute_ai_step_jump();
    } else if (_current_ai_type == AI_ACTION_WAIT_UNTIL_PLAYER_IS_IN_RANGE) {
        execute_ai_action_wait_until_player_in_range();
    } else if (_current_ai_type == AI_ACTION_SAVE_POINT) {
        execute_ai_save_point();
    } else if (_current_ai_type == AI_ACTION_SHOT_PROJECTILE_AHEAD) {
        execute_ai_action_trow_projectile(0, false);
    } else if (_current_ai_type == AI_ACTION_SHOT_PROJECTILE_PLAYER_DIRECTION) {
        execute_ai_action_trow_projectile(1, false);
    } else if (_current_ai_type == AI_ACTION_SHOT_PROJECTILE_INVERT_DIRECTION) {
        execute_ai_action_trow_projectile(1, true);
    } else if (_current_ai_type == AI_ACTION_AIR_WALK) {
        ia_action_air_walk();
    } else if (_current_ai_type == AI_ACTION_FALL_TO_GROUND) {
        ia_action_jump_fall();
    } else if (_current_ai_type == AI_ACTION_TELEPORT) {
        ia_action_teleport();
    } else if (_current_ai_type == AI_ACTION_DASH) {
        execute_ai_step_dash();
    } else if (_current_ai_type == AI_ACTION_GRAB_WALL) {
        execute_ai_step_jump_to_wall();
    } else if (_current_ai_type == AI_ACTION_SPAWN_NPC) {
        execute_ai_step_spawn_npc();
    } else if (_current_ai_type == AI_ACTION_CHANGE_MOVE_TYPE) {
        execute_ai_step_change_animation_type();
    } else if (_current_ai_type == AI_ACTION_REPLACE_NPC) {
        execute_ai_replace_itself(false);
    } else if (_current_ai_type == AI_ACTION_MORPH_INTO_NPC) {
        execute_ai_replace_itself(true);
    } else if (_current_ai_type == AI_ACTION_CIRCLE_PLAYER) {
        execute_ai_circle_player();
    } else if (_current_ai_type == AI_ACTION_CHANGE_MOVE_TYPE_REVERSE) {
        execute_ai_step_change_animation_type_reverse();
    } else if (_current_ai_type == AI_ACTION_JUMP_ATTACK_UP) {
        if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
            jump_attack_type = _parameter;
        }
        _parameter = AI_ACTION_JUMP_OPTION_UP;
        execute_ai_step_jump();
    } else if (_current_ai_type == AI_ACTION_JUMP_ATTACK_AHEAD_ONCE) {
        if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
            jump_attack_type = _parameter;
        }
        _parameter = AI_ACTION_JUMP_OPTION_ONCE;
        execute_ai_step_jump();
    } else if (_current_ai_type == AI_ACTION_WAIT_RANDOM_TIME) {
        execute_ai_wait_random_time();
    } else if (_current_ai_type == AI_ACTION_WALL_WALK_SHOOT || _current_ai_type == AI_ACTION_WALL_WALK) {
        if (_current_ai_type == AI_ACTION_WALL_WALK_SHOOT) {
            is_shooter = true;
            // define shooting direction
            shoot_direction = state.direction;
            if (name.find("[TOP]") != std::string::npos) {
                shoot_direction = ANIM_DIRECTION_DOWN;
            } else if (name.find("[BOTTOM]") != std::string::npos) {
                shoot_direction = ANIM_DIRECTION_UP;
            }
        }
        execute_ai_wall_walk();
    } else if (_current_ai_type == AI_ACTION_PLAY_SFX) {
        execute_play_sfx();
    } else {
        std::cout << "ERROR: ********** AI::UNKNOWN - number[" << (int)_number << "], pos[" << _ai_chain_n << "], _current_ai_type[" << (int)_current_ai_type << "] - NOT IMPLEMENTED *******" << std::endl;
        _current_ai_type = 0;
        _ai_chain_n = 0;
    }

    if (is_shooter) {
        if (shooter_timer < timer.getTimer()) {
            throw_projectile(0, false);
            shooter_timer = timer.getTimer() + SHOOTER_DELAY;
        }
    }
}



// ********************************************************************************************** //
// Find the nearest player and return a struct with distance and a reference to player            //
// ********************************************************************************************** //
struct_player_dist artificial_inteligence::dist_npc_players()
{
    int dist;
    struct struct_player_dist res;
    if (gameControl.get_current_map_obj() == NULL) {
        graphLib.show_debug_msg("EXIT #A.01");
        exception_manager::throw_param_exception(std::string("ERROR: trying to calc NPC-player distance before map pointer is set on NPC"), "");
    }
    if (gameControl.get_current_map_obj()->_player_ref == NULL) {
        graphLib.show_debug_msg("EXIT #01");
        exception_manager::throw_param_exception(std::string("ERROR: trying to calc NPC-player but there is no player on player_list."), "");
    }

    res.pObj = gameControl.get_current_map_obj()->_player_ref;
    float part1 = pow((position.x - res.pObj->getPosition().x), 2) + pow((position.y - res.pObj->getPosition().y), 2);
    dist = sqrt(part1);
    res.dist_xy.x = abs((float)position.x - res.pObj->getPosition().x);
    res.dist_xy.y = abs((float)position.y - res.pObj->getPosition().y);

    res.dist = dist;
    return res;

}




void artificial_inteligence::ground_damage_players()
{
    // check if player is on ground
    st_float_position npc_pos = gameControl.get_current_map_obj()->_player_ref->getPosition();
    npc_pos.x = (npc_pos.x + gameControl.get_current_map_obj()->_player_ref->get_size().width/2)/TILESIZE;
    npc_pos.y = (npc_pos.y + gameControl.get_current_map_obj()->_player_ref->get_size().height)/TILESIZE;
    int lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(npc_pos.x, npc_pos.y));
    if (lock != TERRAIN_UNBLOCKED && lock != TERRAIN_STAIR && lock != TERRAIN_WATER) {
        gameControl.get_current_map_obj()->_player_ref->damage(3, false);
    }
}

void artificial_inteligence::push_back_players(short direction)
{
    gameControl.get_current_map_obj()->_player_ref->push_back(direction);
}

void artificial_inteligence::pull_players(short direction)
{
    gameControl.get_current_map_obj()->_player_ref->pull(direction);
}

bool artificial_inteligence::auto_respawn() const
{
    if (GameMediator::get_instance()->get_enemy(_number)->respawn_delay > 0 && timer.getTimer() > _auto_respawn_timer)  {
        return true;
    }
    return false;
}


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

void artificial_inteligence::ia_action_jump_to_point(st_position point)
{
    int xinc = 0;
    if (state.direction == ANIM_DIRECTION_LEFT) {
        xinc = -move_speed*2; /// @TODO - check collision against walls (will have to "fake" the x position to continue jump movement)
    } else {
        xinc = move_speed*2; /// @TODO - check collision against walls (will have to "fake" the x position to continue jump movement)
    }

    if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_INITIAL) {
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
            _trajectory_parabola = new trajectory_parabola(position.x - point.x);
        } else {
            _trajectory_parabola = new trajectory_parabola(point.x - position.x);
        }
        _ai_state.initial_position.x = position.x;
        _ai_state.initial_position.y = position.y;
        set_animation_type(ANIM_TYPE_JUMP);
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_EXECUTING;
        _last_jump_yinc = 0;
        _ignore_gravity = true; // disable gravity
        _origin_point.x = 0; // used when can't move x to continue increasing value for the parabola method
    } else if (_ai_state.sub_action_sub_status == IA_ACTION_STATE_EXECUTING) {
        bool can_move_x = test_change_position(xinc, 0);
        if (can_move_x == false) { // hit a wall, fall
            _origin_point.x += xinc;
            xinc = 0;
        }

        int new_x = abs((position.x + _origin_point.x) - _ai_state.initial_position.x);
        int new_y = _ai_state.initial_position.y - _trajectory_parabola->get_y_point(new_x);



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
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    temp_xinc = -1;
                }
                int temp_yinc = 1;
                int temp_pos_x = new_x;

                while (true) { // search for the point when parabole changes signal
                    int temp_new_y = _ai_state.initial_position.y - _trajectory_parabola->get_y_point(temp_pos_x);
                    temp_yinc = position.y - temp_new_y;

                    if (temp_yinc >= 0) {
                        int temp_diff = position.x - temp_pos_x;
                        _origin_point.x += temp_diff;
                        break;
                    }
                    temp_pos_x += temp_xinc;
                }

                return;
            }
        }


        // jump-attack
        if (yinc < 0 && can_move_y == true && jump_attack_type != -1) {
            if (have_frame_graphic(state.direction, ANIM_TYPE_JUMP_ATTACK, 0) == true) {
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
                if (_trajectory_parabola != NULL) {
                    delete _trajectory_parabola;
                }
                _ignore_gravity = false; // enable gravity
                _ai_state.sub_action_sub_status = IA_ACTION_STATE_FINISHED;
                set_animation_type(ANIM_TYPE_STAND);
                moveCommands.right = 0;
                moveCommands.left = 0;
                _ai_timer = timer.getTimer() + 500;
                if (xinc == 0) { // if didn't moved x, then must change AI to other action
                    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                }
                return;
            }
        }
        position.y -= yinc;
        _last_jump_yinc = yinc;
    }
}

void artificial_inteligence::ia_action_jump_to_random()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        int dist = 0;
        int rand_x = 0;
        while (dist < TILESIZE*4) {
            rand_x = rand() % RES_W;
            dist = abs(position.x - rand_x);
        }
        _dest_point.x = rand() % RES_W;
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
        if (state.direction == ANIM_DIRECTION_LEFT) {
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
        if ( _ai_state.sub_action_sub_status == IA_ACTION_STATE_FINISHED) {
            // only stop when reached _origin_poin
            // check how much we need to jump yet
            if (abs(_origin_point.x - position.x) < TILESIZE*4) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                return;
            }
            if (state.direction == ANIM_DIRECTION_LEFT) {
                _dest_point.x = position.x - TILESIZE*4;
            } else {
                _dest_point.x = position.x + TILESIZE*4;
            }
            _ai_state.sub_action_sub_status = IA_ACTION_STATE_INITIAL;
        }
    }
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
        if (state.direction == ANIM_DIRECTION_LEFT) {
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
                if (have_frame_graphic(state.direction, ANIM_TYPE_JUMP_ATTACK, 0) == true) {
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

void artificial_inteligence::ia_action_quake_attack()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        _ai_state.initial_position.x = 0;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        _ai_state.initial_position.x++;
        if (_ai_state.initial_position.x % 20) {
            graphLib.set_screen_adjust(st_position(-QUAKE_SCREEN_MOVE, 0));
            ground_damage_players();
        } else if (_ai_state.initial_position.x % 25) {
            graphLib.set_screen_adjust(st_position(QUAKE_SCREEN_MOVE, 0));
            ground_damage_players();
        }
        _ai_state.timer = timer.getTimer() + 500;
        if (_ai_state.initial_position.x > 200) {
            graphLib.set_screen_adjust(st_position(0, 0));
            set_animation_type(ANIM_TYPE_STAND);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
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



void artificial_inteligence::ia_set_destiny_point(st_position dest_point)
{
    _ai_state.initial_position.x = position.x;
    _ai_state.initial_position.y  = position.y;
    _ai_state.secondary_position.x = dest_point.x;
    _ai_state.secondary_position.y = dest_point.y;
    if (_ai_state.secondary_position.x < _ai_state.initial_position.x) {
        set_direction(ANIM_DIRECTION_LEFT);
    } else {
        set_direction(ANIM_DIRECTION_RIGHT);
    }
}

// walk until reaching a given X coordinate
// @TODO - jump if needed
void artificial_inteligence::ia_walk_to_position()
{
    // @TODO - if can't walk, change action to jump to player
    //if (!test_change_position(-move_speed, 0)) {

    bool finished = false;
    if (_ai_state.secondary_position.x < _ai_state.initial_position.x) {
        position.x -= move_speed;
        if (position.x <= _ai_state.secondary_position.x) {
            finished = true;
        }
    } else {
        position.x += move_speed;
        if (position.x >= _ai_state.secondary_position.x) {
            finished = true;
        }
    }
    if (finished == true) {
        set_animation_type(ANIM_TYPE_STAND);
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    }
}

void artificial_inteligence::ia_float_to_position()
{
    bool finished = false;
    int xinc = 0;
    int yinc = 0;
    bool reached_x = false;
    bool reached_y = false;


    if (_ai_state.secondary_position.x < _ai_state.initial_position.x) {
        if (position.x <= _ai_state.secondary_position.x) {
            xinc = 0;
        } else {
            xinc -= move_speed;
        }
    } else {
        if (position.x >= _ai_state.secondary_position.x) {
            xinc = 0;
        } else {
            xinc += move_speed;
        }
    }
    if (_ai_state.secondary_position.y < _ai_state.initial_position.y) {
        if (position.y <= _ai_state.secondary_position.y) {
            yinc = 0;
        } else {
            yinc -= move_speed;
        }
    } else {
        if (position.y >= _ai_state.secondary_position.y) {
            yinc = 0;
        } else {
            yinc += move_speed;
        }
    }


    if (test_change_position(xinc, 0) == false) {
        xinc = 0;
    }
    if (test_change_position(0, yinc) == false) {
        yinc = 0;
    }
    if (xinc == 0 && yinc == 0) { // both X and Y blocked, leave
        finished = true;
    } else {
        position.x += xinc;
        position.y += yinc;
    }

    if (reached_x == true && reached_y == true) {
        finished = true;
    }
    // check if reached the point
    if (finished == true) {
        set_animation_type(ANIM_TYPE_STAND);
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    }
}

void artificial_inteligence::ia_dash()
{
    bool finished = false;
    if (_ai_state.secondary_position.x < _ai_state.initial_position.x) {
        position.x -= max_speed;
        if (position.x <= _ai_state.secondary_position.x) {
            finished = true;
        }
    } else {
        position.x += max_speed;
        if (position.x >= _ai_state.secondary_position.x) {
            finished = true;
        }
    }
    if (finished == true) {
        set_animation_type(ANIM_TYPE_STAND);
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    }
}





/// @TODO - jump if needed
void artificial_inteligence::execute_ai_step_walk()
{
    short move_type = _parameter;
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
            if (state.animation_type != ANIM_TYPE_TURN) {
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
            } else if (move_type == AI_ACTION_WALK_OPTION_HORIZONTAL_AHEAD) {
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    _dest_point.x = position.x - frameSize.width/2 - walk_range;
                } else {
                    _dest_point.x = position.x + frameSize.width/2 + walk_range;
                }
            } else if (move_type == AI_ACTION_WALK_OPTION_TO_RANDOM_DIRECTION) {
                set_direction(rand() % 2);
                if (state.direction == ANIM_DIRECTION_LEFT) {
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
            if (_is_last_frame == true) { // finished turn animation
                if (state.direction == ANIM_DIRECTION_LEFT) {
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

void artificial_inteligence::execute_ai_action_wait_until_player_in_range()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        set_animation_type(ANIM_TYPE_STAND);
    } else {
        struct_player_dist dist_players = dist_npc_players();
        int dist_player = abs((float)dist_players.pObj->getPosition().x - position.x);
        if (dist_player <= walk_range) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::execute_ai_action_trow_projectile(Uint8 n, bool invert_direction)
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.animation_type == ANIM_TYPE_WALK_AIR) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (is_on_attack_frame() == false){
            set_animation_type(ANIM_TYPE_ATTACK);
        }
        // face player to shoot, if parameter is not shot-ahead-only
        if (move_speed > 0 && _current_ai_type != AI_ACTION_SHOT_PROJECTILE_AHEAD) {
            struct_player_dist dist_players = dist_npc_players();
            if (dist_players.pObj->getPosition().x > position.x) {
                set_direction(ANIM_DIRECTION_RIGHT);
            } else {
                set_direction(ANIM_DIRECTION_LEFT);
            }
        }
        state.animation_state = 0;
        state.animation_timer = timer.getTimer() + (graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].delay;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        _did_shot = false;
    } else {
        if (_was_animation_reset == true && _did_shot == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _did_shot = false;
            if (state.animation_type == ANIM_TYPE_WALK_AIR || state.animation_type == ANIM_TYPE_JUMP || state.animation_type == ANIM_TYPE_JUMP_ATTACK) {
                set_animation_type(ANIM_TYPE_JUMP);
            } else {
                set_animation_type(ANIM_TYPE_STAND);
            }
        } else if ((_is_attack_frame == true || _is_last_frame == true) && _did_shot == false) { // only shoot when reached the last frame in animation attack
            shot_success = throw_projectile(_parameter, invert_direction);
            _did_shot = true;
            shot_timer = timer.getTimer();
        }
    }
}

// creates a projectile, return false if could not fire
bool artificial_inteligence::throw_projectile(int projectile_type, bool invert_direction)
{
    CURRENT_FILE_FORMAT::file_projectilev3 temp_projectile = GameMediator::get_instance()->get_projectile(projectile_type);
    // some projectile types are limited to one
    if (temp_projectile.trajectory == TRAJECTORY_CENTERED && projectile_list.size() > 0) {
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return false;
    }

    // can't fire a player's targeted projectile if move_speed zero (can't turn) and facing the wrong side
    if (move_speed == 0 && (temp_projectile.trajectory == TRAJECTORY_ARC_TO_TARGET || temp_projectile.trajectory == TRAJECTORY_TARGET_DIRECTION || temp_projectile.trajectory == TRAJECTORY_TARGET_EXACT)) {
        struct_player_dist dist_players = dist_npc_players();
        if ((dist_players.pObj->getPosition().x > position.x && state.direction == ANIM_DIRECTION_LEFT) || (dist_players.pObj->getPosition().x < position.x && state.direction == ANIM_DIRECTION_RIGHT)) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return false;
        }
    }

    unsigned int max_shots = 3;
    if (projectile_type != -1) {
        max_shots = temp_projectile.max_shots;
    }
    if (projectile_list.size() >= max_shots) {
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return false;
    }

    int proj_direction = state.direction;
    if (invert_direction == true) {
        proj_direction = !proj_direction;
    }

    if (is_shooter) {
        proj_direction = shoot_direction;
    }

    projectile_list.push_back(projectile(projectile_type, proj_direction, get_attack_position(), is_player()));
    projectile &temp_proj = projectile_list.back();
    temp_proj.play_sfx(true);
    temp_proj.set_owner(this);


    if (temp_projectile.trajectory == TRAJECTORY_CENTERED) {
        temp_proj.set_owner_direction(&state.direction);
        temp_proj.set_owner_position(&position);
    }

    if (temp_projectile.trajectory == TRAJECTORY_TARGET_DIRECTION || temp_projectile.trajectory == TRAJECTORY_TARGET_EXACT || temp_projectile.trajectory == TRAJECTORY_ARC_TO_TARGET || temp_projectile.trajectory == TRAJECTORY_FOLLOW) {
        if (!is_player() && gameControl.get_current_map_obj()->_player_ref != NULL) {
            character* p_player = gameControl.get_current_map_obj()->_player_ref;
            temp_proj.set_target_position(p_player->get_position_ref());
        }
    }



    return true;
}

void artificial_inteligence::execute_ai_step_fly()
{
    // INITIALIZATION
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        must_show_dash_effect = false;
        previous_position_list.clear();
        if (_parameter == AI_ACTION_FLY_OPTION_TO_PLAYER || _parameter == AI_ACTION_FLY_OPTION_DASH_TO_PLAYER) {
            struct_player_dist dist_players = dist_npc_players();
            _dest_point = dist_players.pObj->getPosition();
            must_show_dash_effect = true;
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
            if (state.direction == ANIM_DIRECTION_LEFT) {
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
            if (state.direction == ANIM_DIRECTION_LEFT) {
                _dest_point.x = position.x - frameSize.width/2 - walk_range;
            } else {
                _dest_point.x = position.x + frameSize.width/2 + walk_range;
            }
            _ai_state.initial_position.x = 0;
            _ai_state.initial_position.y = position.y;
            _dest_point.y = position.y;
        }

        set_animation_type(ANIM_TYPE_WALK_AIR);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;

    // EXECUTION
    } else {


        if (state.animation_type == ANIM_TYPE_TURN && have_frame_graphic(state.direction, state.animation_type, (state.animation_state+1)) == false) {
            set_animation_type(ANIM_TYPE_STAND);
            set_direction(!state.direction);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
        }

        if (_parameter == AI_ACTION_FLY_OPTION_HORIZONTAL_AHEAD) {
            if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        } else if (_parameter == AI_ACTION_FLY_OPTION_HORIZONTAL_TURN) {
            if (state.animation_type != ANIM_TYPE_TURN) {
                if (have_frame_graphic(state.direction, ANIM_TYPE_TURN, 0) == false) {
                    set_direction(!state.direction);
                    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
                } else {
                    set_animation_type(ANIM_TYPE_TURN);
                }
                state.animation_state = 0;
                distance.width = 0;
                state.animation_timer = timer.getTimer() + 200;
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
                _counter++;

                int proj_direction = state.direction;
                projectile_list.push_back(projectile(_parameter, proj_direction, get_attack_position(), is_player()));
                projectile &temp_proj = projectile_list.back();
                temp_proj.play_sfx(true);
                temp_proj.set_owner(this);

                if (GameMediator::get_instance()->get_projectile(_parameter).trajectory == TRAJECTORY_CENTERED) {
                    temp_proj.set_owner_direction(&state.direction);
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
            if (position.x != _dest_point.x) {
                if (_ai_state.initial_position.y == position.y) {
                    _dest_point.y = position.y - TILESIZE;
                } else if (position.y <= _ai_state.initial_position.y - TILESIZE) {
                    _dest_point.y = position.y + TILESIZE;
                }
            }

            int pos_x_before = position.x;

            if (move_to_point(_dest_point, move_speed, 2, is_ghost, false) == true || (move_speed != 0 && pos_x_before == position.x)) {
                set_direction(!state.direction);
                if (state.direction == ANIM_DIRECTION_LEFT) {
                    _dest_point.x = position.x - frameSize.width/2 - walk_range;
                } else {
                    _dest_point.x = position.x + frameSize.width/2 + walk_range;
                }
            }

        } else if (_parameter == AI_ACTION_FLY_OPTION_SIN_AHEAD) {

            float dist_y = abs(position.y - _dest_point.y);
            _sin_x += 0.12;
            float sin_value = (TILESIZE*3)*sin(_sin_x);

            if (position.x == _dest_point.x && dist_y < 0.2) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            } else {
                if (move_to_point(_dest_point, move_speed, 0, is_ghost, false) == true) {
                    if (dist_y > 0.2) {

                        int move_adjust_y = 0.1;
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
    }
}

void artificial_inteligence::execute_ai_save_point()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _saved_point = position;
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        last_execute_time = timer.getTimer() + 20;
    }
}

void artificial_inteligence::execute_ai_step_dash()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(ANIM_TYPE_SLIDE);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        if (_parameter == AI_ACTION_DASH_OPTION_AHEAD) {


            // check if can't move to the current direction and change to other, if needed
            if (state.direction == ANIM_DIRECTION_LEFT) {
                can_move_struct can_move_left = check_can_move_to_point(st_float_position(position.x-move_speed*4, position.y), -move_speed*2, 0, is_ghost, false);
                if (can_move_left.result != CAN_MOVE_SUCESS || can_move_left.xinc == 0 || can_move_left.can_move_x == false) {
                    set_direction(ANIM_DIRECTION_RIGHT);
                }
            } else if (state.direction == ANIM_DIRECTION_RIGHT) {
                can_move_struct can_move_right = check_can_move_to_point(st_float_position(position.x+move_speed*4, position.y), move_speed*2, 0, is_ghost, false);
                if (can_move_right.result != CAN_MOVE_SUCESS || can_move_right.xinc == 0 || can_move_right.can_move_x == false) {
                    set_direction(ANIM_DIRECTION_LEFT);
                }
            }

            if (state.direction == ANIM_DIRECTION_LEFT) {
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

void artificial_inteligence::execute_ai_step_change_animation_type()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(static_cast<ANIM_TYPE>(_parameter));
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        // some attacks keep enemy in current animation until it's effect is finished, so we check against the next.delay
        if (started_action_timer <= timer.getTimer()) {
            if (_is_last_frame == true) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        }
    }
}

void artificial_inteligence::execute_ai_step_change_animation_type_reverse()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(static_cast<ANIM_TYPE>(_parameter));
        state.animation_inverse = true;
        advance_to_last_frame(); // always call this once setting animation_inverse to true to avoid false _was_animation_reset
        _was_animation_reset = false;
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (_is_last_frame == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}

void artificial_inteligence::execute_ai_wait_random_time()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        int delay = rand() % _parameter;
        _ai_timer = timer.getTimer() + delay*1000;
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    }
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
            if (state.direction != ANIM_DIRECTION_LEFT) {
                must_turn = true;
            }
        } else if (_parameter == AI_ACTION_WALL_WALK_OPTION_RIGHT) {
            if (state.direction != ANIM_DIRECTION_RIGHT) {
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
            if (state.animation_type != ANIM_TYPE_TURN && have_frame_graphic(state.direction, ANIM_TYPE_TURN, (state.animation_state+1)) == true) {
                set_animation_type(ANIM_TYPE_TURN);
            } else {
                set_direction(!state.direction);
            }
        }

        set_animation_type(ANIM_TYPE_WALK_AIR);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;

    // EXECUTION
    } else {
        if (state.animation_type == ANIM_TYPE_TURN && have_frame_graphic(state.direction, state.animation_type, (state.animation_state+1)) == false) {
            set_animation_type(ANIM_TYPE_STAND);
            set_direction(!state.direction);
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

    //std::cout << "AI::move_to_point.x[" << position.x << "], move_inc.result[" << (int)move_inc.result << "], move_inc.can_move_x[" << move_inc.can_move_x << "]" << std::endl;

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
    if (xinc != 0 && position.x > dest_point.x && state.direction != ANIM_DIRECTION_LEFT) {
        set_direction(ANIM_DIRECTION_LEFT);
    }
    if (xinc != 0 && position.x < dest_point.x && state.direction != ANIM_DIRECTION_RIGHT) {
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
            if (state.direction == ANIM_DIRECTION_RIGHT) {
                map_point.x = (position.x + frameSize.width)/TILESIZE;
            }
            st_position current_map_point((position.x + frameSize.width/2)/TILESIZE, (position.y + frameSize.height + 3)/TILESIZE);
            int map_lock = gameControl.get_current_map_obj()->getMapPointLock(map_point);
            int current_map_lock = gameControl.get_current_map_obj()->getMapPointLock(current_map_point);

            if (map_lock != current_map_lock) {
                if (map_lock == TERRAIN_UNBLOCKED || map_lock == TERRAIN_WATER || (map_lock == TERRAIN_EASYMODEBLOCK && game_save.difficulty != DIFFICULTY_EASY) || (map_lock == TERRAIN_HARDMODEBLOCK && game_save.difficulty != DIFFICULTY_HARD)) {
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
    int rand_x = rand() % max_adjust*2;
    position.x = position.x + rand_x - max_adjust;
}

int artificial_inteligence::create_rand_x_point(int max_range)
{
    bool keep_going = true;
    int rand_x = 0;
    int scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
    while (keep_going) {
        rand_x = position.x + rand() % max_range*2 - max_range;
        if (rand_x < scroll_x+TILESIZE || rand_x+frameSize.width > scroll_x+RES_W-TILESIZE) {
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((rand_x+frameSize.width/2)/TILESIZE, (position.y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            keep_going = false;
        }
    }
    return rand_x;
}

int artificial_inteligence::create_rand_y_point(int max_range)
{
    bool keep_going = true;
    int rand_y = 0;
    while (keep_going) {
        rand_y = position.y + rand() % max_range*2 - max_range;
        if (rand_y < 0 || rand_y+frameSize.height > RES_H-TILESIZE) {
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((position.x+frameSize.width/2)/TILESIZE, (rand_y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            keep_going = false;
        }
    }
    return rand_y;
}

st_position artificial_inteligence::create_rand_point(int max_range)
{
    bool keep_going = true;
    int rand_x = 0;
    int rand_y = 0;
    int scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
    while (keep_going) {
        rand_x = position.x + rand() % max_range*2 - max_range;
        rand_y = position.y + rand() % max_range*2 - max_range;
        // TODO: check if new position X is out of visible screen
        if (rand_x < scroll_x+TILESIZE || rand_x+frameSize.width > scroll_x+RES_W-TILESIZE) {
            continue;
        }

        // check if new position Y is out of visible screen
        if (rand_y < 0 || rand_y+frameSize.height > RES_H-TILESIZE) {
            continue;
        }
        int point_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position((rand_x+frameSize.width/2)/TILESIZE, (rand_y+frameSize.height/2)/TILESIZE));
        if (point_lock == TERRAIN_WATER || point_lock == TERRAIN_UNBLOCKED) {
            keep_going = false;
        }
    }
    return st_position(rand_x, rand_y);
}

void artificial_inteligence::execute_ai_step_jump()
{
    if (_parameter == AI_ACTION_JUMP_OPTION_TO_ROOF) {
        ia_action_jump_to_roof();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_PLAYER) {
        ia_action_jump_to_player();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_AHEAD) {
        ia_action_jump_ahead();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_RANDOM_POINT) {
        ia_action_jump_to_random();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_ONCE) {
        ia_action_jump_once();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_UP) {
        ia_action_jump_up();
    } else if (_parameter == AI_ACTION_JUMP_OPTION_TO_PLAYER_DIRECTION) {
        ia_action_jump_once();
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
        int pos_x = find_wall(position.x, state.direction);
        if (pos_x == -1) { // no wall found to grab
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
        _dest_point.x = pos_x;
        _dest_point.y = RES_H/2;

        int dist_x = _dest_point.x - position.x;
        int dist_y = _dest_point.y - position.y;
        _diagonal_speed.x = move_speed*2;
        _diagonal_speed.y = (_diagonal_speed.x * dist_y) / dist_x;
        if (_diagonal_speed.y < 0) { // move_to_point always use positive values, and adjust depending on target itself
            _diagonal_speed.y = _diagonal_speed.y * -1;
        }
        _ignore_gravity = true; // disable gravity
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        set_animation_type(ANIM_TYPE_JUMP);
        state.animation_state = 0;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (move_to_point(_dest_point, _diagonal_speed.x, _diagonal_speed.y, is_ghost, false) == true) {
            state.animation_state = 0;
            set_animation_type(ANIM_TYPE_WALK_AIR);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            set_direction(!state.direction);
        }
    }
}

void artificial_inteligence::execute_ai_replace_itself(bool morph)
{
    // kills/remove itself
    _dead_state = 2;
    st_hit_points hp_copy = hitPoints;
    hitPoints.current = 0;
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    // spawn new npc
    classnpc* npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, st_position(position.x, position.y+frameSize.height/2), state.direction, false, false);
    // is executing reaction and is dying and is map-boss -> set child as new map-boss
    if (_reaction_state == 1 && _reaction_type == 2 && _is_stage_boss == true) {
        _is_stage_boss = false;
        npc_ref->set_stage_boss(true);
    // is morphing into the new NPC, copy some properties
    } else if (morph == true) {
        if (_is_stage_boss) {
            _is_stage_boss = false;
            npc_ref->set_stage_boss(true);
        }
        // @TODO: the boss HP HUD gets lost with morph //
        _dead_state = DEAD_STATE_IGNORE;
        npc_ref->npc_set_hp(hp_copy);
        // adjust Y post because of heigth difference //
        st_float_position new_pos = position;
        new_pos.y += frameSize.height - npc_ref->get_size().height;
        npc_ref->npc_set_position(new_pos);
        npc_ref->npc_set_direction(state.direction);
        npc_ref->npc_set_initialized(3);
    }
}


void artificial_inteligence::execute_ai_step_spawn_npc()
{
    // limit for spawed npcs

    // still spawning an NPC, leave
    if (gameControl.get_current_map_obj()->_npc_spawn_list.size() > 0) {
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }

    int child_count = gameControl.get_current_map_obj()->child_npc_count(get_number());
    if (child_count >= MAX_NPC_SPAWN) {
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }

    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.animation_type == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (is_on_attack_frame() == false){
            set_animation_type(ANIM_TYPE_ATTACK);
        }
        classnpc* npc_ref;
        if (name == "TOP HAT") {
            npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, st_position(position.x, position.y), state.direction, false, true);
        } else {
            npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, st_position(position.x, position.y+frameSize.height/2), state.direction, false, false);
        }

        if (npc_ref == NULL) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
        }

        npc_ref->set_parent_id(get_number());

        // is executing reaction and is dying and is map-boss -> set child as new map-boss
        if (_reaction_state == 1 && _reaction_type == 2 && _is_stage_boss == true) {
            RockbotLogger::get_instance()->write("AI::SPAWN, SET NEW BOSS");
            _is_stage_boss = false;
            npc_ref->set_stage_boss(true);
        }
    }
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
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
                if (state.direction == ANIM_DIRECTION_LEFT) {
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

            if (state.direction == ANIM_DIRECTION_RIGHT) {
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
            if (state.direction == ANIM_DIRECTION_LEFT) {
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

        if (state.animation_type != ANIM_TYPE_TELEPORT) { /// @TODO - must use teleport
            // remove all projectiles, so we don't end with a circle one around an emey that moved
            clean_effect_projectiles();
            set_animation_type(ANIM_TYPE_TELEPORT);
        } else {
            if (_is_last_frame == true) { // finished teleport animation
                _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
                state.invisible = true;


                // find wall to the left
                if (_parameter == AI_ACTION_TELEPORT_OPTION_LEFT || (_parameter == AI_ACTION_TELEPORT_OPTION_AHEAD && state.direction == ANIM_DIRECTION_LEFT)) {
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(st_position(position.x, position.y+frameSize.height/2), st_size(walk_range, 0), ANIM_DIRECTION_LEFT);
                    position.x = dest_pos.x;
                // find wall to the right
                } else if (_parameter == AI_ACTION_TELEPORT_OPTION_RIGHT || (_parameter == AI_ACTION_TELEPORT_OPTION_AHEAD && state.direction == ANIM_DIRECTION_RIGHT)) {
                    st_position dest_pos = gameControl.get_current_map_obj()->get_first_lock_in_direction(st_position(position.x+frameSize.width, position.y+frameSize.height/2), st_size(walk_range, 0), ANIM_DIRECTION_RIGHT);
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
                _was_animation_reset = false;
                _ignore_gravity = true;
                state.animation_state = 0;
            }
        }
    // return from teleporting
    } else {
        if (state.invisible == true) {
            state.invisible = false;
            state.animation_inverse = true;
            /// @TODO: to avoid this problem,c reate method to start inverse animation ///
            advance_to_last_frame(); // always call this once setting animation_inverse to true to avoid false _was_animation_reset
            _was_animation_reset = false;
        }
        if (_was_animation_reset == true) {
            set_animation_type(ANIM_TYPE_STAND);
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _ignore_gravity = false;
            state.animation_inverse = false;
        }
    }
}


int artificial_inteligence::get_ai_type() {
    int type = -1;

    // check for error
    if (_number < 0 || _number >= GameMediator::get_instance()->ai_list.size()) {
        char int_to_str[256];
        sprintf(int_to_str, "%d", _reaction_type);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid number[").append(std::string(int_to_str)).append(std::string("]")));
        return 0;
    }
    if (_reaction_type < 0 || _reaction_type >= MAX_AI_REACTIONS) {
        char int_to_str[256];
        sprintf(int_to_str, "%d", _reaction_type);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid reaction_type[").append(std::string(int_to_str)).append(std::string("]")));
        _reaction_type = 0;
        return 0;
    }

    if (_ai_chain_n < 0 || _ai_chain_n >= AI_MAX_STATES) {
        char int_to_str[256];
        sprintf(int_to_str, "%d", _ai_chain_n);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid _ai_chain_n[").append(std::string(int_to_str)).append(std::string("]")));
        _ai_chain_n = 0;
        return 0;
    }

    if (_reaction_state == 0 || GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action == -1) {
        type = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].action;
        _parameter = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter;
    } else {
        type = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action;
        _parameter = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].extra_parameter;
    }
    return type;
}

bool artificial_inteligence::always_move_ahead() const
{
    if (GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to-1 == _ai_chain_n) {
        return true;
    }
    return false;
}

bool artificial_inteligence::uses_fly_fall()
{
    for (int i=0; i<AI_MAX_STATES; i++) {
        if (GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].action == AI_ACTION_FLY && GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter == AI_ACTION_FLY_OPTION_FALL) {
            return true;
        }
    }
    return false;
}

void artificial_inteligence::invert_left_right_direction()
{
    if (state.direction == ANIM_DIRECTION_LEFT) {
        set_direction(ANIM_DIRECTION_RIGHT);
    } else if (state.direction == ANIM_DIRECTION_RIGHT) {
        set_direction(ANIM_DIRECTION_LEFT);
    }
}

void artificial_inteligence::execute_play_sfx()
{
    // TODO: add options
    soundManager.play_shared_sfx("dinosaur_growl.wav");
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
}

bool artificial_inteligence::is_teleporting()
{
    if (_current_ai_type == AI_ACTION_TELEPORT) {
        return true;
    }
    return false;
}
