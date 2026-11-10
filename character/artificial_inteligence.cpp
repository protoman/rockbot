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
    if (GameMediator::get_instance()->get_enemy(_number)->respawn_delay > 0) {
//        std::cout << ">>>>>>>>>> AI::CONSTRUCTOR[" << name << "], id[" <<_number << "], respawn_delay[" << GameMediator::get_instance()->get_enemy(_number)->respawn_delay << "], auto_respawn_timer[" << (int)_auto_respawn_timer << "], current_time[" << (int)timer.getTimer() << "]" << std::endl;
    }

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
            if (_reaction_state == 0) { // not in a reaction (reaction type 0 is a valid reaction, PLAYER_ON_RANGE)
                int delay = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to_delay;
                _ai_timer = timer.getTimer() + delay;
            } else {
                // TODO, use reaction time
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


// a reaction is running: when its action ended, goes to the next state of the chain (or back to the normal AI)
void artificial_inteligence::continue_ai_reaction()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_FINISHED) {
        int reaction_goto = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].go_to;
        if (reaction_goto == AI_ACTION_GOTO_CHANCE) {
            //std::cout << "AI::execute_ai::check_ai_reaction - EXIT" << std::endl;
            _reaction_state = 0;
            _reaction_type = 0;
            reaction_loop_check.clear();
        } else {
            //std::cout << "AI::execute_ai::check_ai_reaction - CONTINUE" << std::endl;
            if (reaction_loop_check.size() == 0) {
                _ai_chain_n = reaction_goto-1;
                _current_ai_type = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].action;
                _parameter = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter;
                reaction_loop_check.push_back(_current_ai_type);
                //std::cout << "AI::execute_ai::check_ai_reaction - FIRST, reaction_goto[" << reaction_goto << "], _current_ai_type[" << _current_ai_type << "], _parameter[" << _parameter << "]" << std::endl;
            } else {
                int next_item = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to;
                //std::cout << "AI::execute_ai::check_ai_reaction - CHECK - next_item[" << next_item << "], ai_chain_n[" << _ai_chain_n << "], current_ai_type[" << _current_ai_type << "], param[" << _parameter << "]" << std::endl;
                if (next_item != AI_ACTION_GOTO_CHANCE) {
                    _ai_chain_n = next_item-1;
                    _current_ai_type = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].action;
                    _parameter = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter;
                    reaction_loop_check.push_back(_current_ai_type);
                    _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
                    //std::cout << "AI::execute_ai::check_ai_reaction - NEXT - next_item[" << next_item << "], ai_chain_n[" << _ai_chain_n << "], current_ai_type[" << _current_ai_type << "], param[" << _parameter << "]" << std::endl;
                    execute_ai();
                } else {
                    //std::cout << "AI::execute_ai::check_ai_reaction - ENDED - _ai_chain_n[" << _ai_chain_n << "], _current_ai_type[" << _current_ai_type << "]" << std::endl;
                    _reaction_state = 0;
                    _reaction_type = 0;
                    reaction_loop_check.clear();
                    define_ai_next_step();
                }
            }
            _ai_state.sub_status = IA_ACTION_STATE_INITIAL;
            execute_ai_step();
        }
    }
}

// dead-reaction that is not a sub-boss (it already has an explosion): two explosions while the next enemy spawns
void artificial_inteligence::show_death_reaction_explosions()
{
    // if not sub-boss (that already have explosion), and dead-reaction is spawn npc, show explosions
    soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 1);
    st_float_position pos1(position.x+2, position.y+20);
    animation anim1(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos1, st_position(-8, -8), 80, 2, state.direction(), st_size(56, 56), gameControl.get_current_map_obj()->get_map_scrolling_ref());
    gameControl.get_current_map_obj()->add_animation(anim1);

    st_float_position pos2(pos1.x+10, pos1.y-30);
    animation anim2(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos2, st_position(-8, -8), 80, 2, state.direction(), st_size(56, 56), gameControl.get_current_map_obj()->get_map_scrolling_ref());
    anim2.set_initial_delay(500);
    gameControl.get_current_map_obj()->add_animation(anim2);
}

void artificial_inteligence::check_ai_reaction()
{
    // check and reset state if needed
    if (_reaction_state == 1) {
        continue_ai_reaction();
        return; // do not check again if already executing
    }

    bool start_reaction = false;
    // near player
    struct_player_dist dist_players = dist_npc_players();
    int diff_y = abs((dist_players.pObj->getPosition().y+dist_players.pObj->get_size().height) - (position.y+frameSize.height));
    int diff_x = 0;
    if (dist_players.pObj->getPosition().x <= position.x) { // player is on the left
        diff_x = abs((dist_players.pObj->getPosition().x+dist_players.pObj->get_size().width) - position.x);
    } else { // player is on the right
        diff_x = abs(dist_players.pObj->getPosition().x - (position.x+frameSize.width));
    }

    if (state.direction() == ANIM_DIRECTION_LEFT && dist_players.pObj->get_direction() == ANIM_DIRECTION_RIGHT) {
        diff_x = abs((dist_players.pObj->getPosition().x+dist_players.pObj->get_size().width) - position.x);
    }

    if (dist_players.dist < TILESIZE*4 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_ON_RANGE].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_PLAYER_ON_RANGE" << std::endl;
        _reaction_type = AI_REACTION_PLAYER_ON_RANGE;
        start_reaction = true;
    // dead
    } else if (hitPoints.current <= 0 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_DEAD].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_DEAD" << std::endl;
        _reaction_type = AI_REACTION_DEAD;
        start_reaction = true;

        show_death_reaction_explosions();
    // hit (comes after death to avoid not firing that reaction that has priority)
    } else if (_was_hit == true && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_HIT].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_HIT" << std::endl;
        _reaction_type = AI_REACTION_HIT;
        start_reaction = true;
    } else if (dist_players.dist < walk_range && diff_y < 2 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_SAME_Y].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_PLAYER_SAME_Y" << std::endl;
        _reaction_type = AI_REACTION_PLAYER_SAME_Y;
        start_reaction = true;
    } else if (dist_players.dist < walk_range && diff_x < TILESIZE && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_SAME_X].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_PLAYER_SAME_X" << std::endl;
        _reaction_type = AI_REACTION_PLAYER_SAME_X;
        start_reaction = true;
    } else if (dist_players.dist < walk_range/4 && GameMediator::get_instance()->ai_list.at(_number).reactions[AI_REACTION_PLAYER_CLOSE].action >= 0) {
//        std::cout << "AI::execute_ai::check_ai_reaction::AI_REACTION_PLAYER_CLOSE" << std::endl;
        _reaction_type = AI_REACTION_PLAYER_CLOSE;
        start_reaction = true;
    }

    _was_hit = false; // reset flag

    if (start_reaction == true) {
        //std::cout << "AI::execute_ai::check_ai_reaction::start_reaction" << std::endl;
        // do not start a walk-reaction in middle air
        // the reaction action is the AI_ACTION value itself (-1 = none, as the editor saves it)
        int react_type = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action;
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
    // a failed shot also goes to a random state, instead of following the state's go_to
    if (must_ignore_next == true || _initialized == 0 || GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].go_to == AI_ACTION_GOTO_CHANCE) { // CHANCE
        _initialized = 1;
        reaction_loop_check.clear();
        int rand_n = rand() % 100;

        //std::cout << "AI::define_ai_next_step[" << name << "] - rand_n[" << rand_n << "]" << std::endl;

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
    //std::cout << "AI::execute_ai::execute_ai_step[" << _current_ai_type << "]" << std::endl;
    _ai_timer = timer.getTimer() + 20;
    if (_current_ai_type != AI_ACTION_FLY) {
        must_show_dash_effect = false; // the fly dash trail must not stay on after the action ends
    }
    if (_current_ai_type == AI_ACTION_WALK) {
        execute_ai_step_walk();
    } else if (_current_ai_type == AI_ACTION_FLY) {
        //std::cout << "AI::execute_ai_step::execute_ai_step_fly" << std::endl;
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
            shoot_direction = state.direction();
            if (name.find("[TOP]") != std::string::npos) {
                shoot_direction = ANIM_DIRECTION_DOWN;
            } else if (name.find("[BOTTOM]") != std::string::npos) {
                shoot_direction = ANIM_DIRECTION_UP;
            }
        }
        execute_ai_wall_walk();
    } else if (_current_ai_type == AI_ACTION_PLAY_SFX) {
        execute_play_sfx();
    } else if (_current_ai_type == AI_ACTION_SHOT_MULTIPLE_PROJECTILE) {
        execute_shot_multiple_projectile();
    } else if (_current_ai_type == AI_ACTION_EXPLODE_ITSELF) {
        execute_explode_itself();
    } else if (_current_ai_type == AI_ACTION_THROW_ITEM) {
        execute_throw_item();
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
        gameControl.get_current_map_obj()->_player_ref->block_jump();
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


void artificial_inteligence::execute_ai_save_point()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        _saved_point = position;
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        last_execute_time = timer.getTimer() + 20;
    }
}


void artificial_inteligence::execute_ai_step_change_animation_type()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(static_cast<ANIM_TYPE>(_parameter));
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        // some attacks keep enemy in current animation until it's effect is finished, so we check against the next.delay
        if (started_action_timer <= timer.getTimer()) {
            if (state.on_last_frame() == true) {
                _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            }
        }
    }
}


void artificial_inteligence::execute_ai_step_change_animation_type_reverse()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        set_animation_type(static_cast<ANIM_TYPE>(_parameter));
        state.set_animation_inverse(true);
        advance_to_last_frame(); // always call this once setting animation_inverse to true to avoid false state.animation_restarted()
        state.set_animation_restarted(false);
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
    } else if (_ai_state.sub_status == IA_ACTION_STATE_EXECUTING) {
        if (state.on_last_frame() == true) {
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



int artificial_inteligence::get_ai_type() {
    int type = -1;

    // check for error
    if (_number < 0 || _number >= GameMediator::get_instance()->ai_list.size()) {
        char int_to_str[256];
        snprintf(int_to_str, sizeof(int_to_str), "%d", _reaction_type);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid number[").append(std::string(int_to_str)).append(std::string("]")));
        return 0;
    }
    if (_reaction_type < 0 || _reaction_type >= MAX_AI_REACTIONS) {
        char int_to_str[256];
        snprintf(int_to_str, sizeof(int_to_str), "%d", _reaction_type);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid reaction_type[").append(std::string(int_to_str)).append(std::string("]")));
        _reaction_type = 0;
        return 0;
    }

    if (_ai_chain_n < 0 || _ai_chain_n >= AI_MAX_STATES) {
        char int_to_str[256];
        snprintf(int_to_str, sizeof(int_to_str), "%d", _ai_chain_n);
        RockbotLogger::get_instance()->write(std::string("AI::get_ai_type, invalid _ai_chain_n[").append(std::string(int_to_str)).append(std::string("]")));
        _ai_chain_n = 0;
        return 0;
    }

    if (_reaction_state == 0 || GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action == -1) {
        type = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].action;
        _parameter = GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter;
        //std::cout << "#1 PARAM set to [" << _parameter << "]" << std::endl;
    } else if (reaction_loop_check.size() == 0) {
        type = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].action;
        _parameter = GameMediator::get_instance()->ai_list.at(_number).reactions[_reaction_type].extra_parameter;
        //std::cout << "#2 PARAM set to [" << _parameter << "]" << std::endl;
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
    if (state.direction() == ANIM_DIRECTION_LEFT) {
        set_direction(ANIM_DIRECTION_RIGHT);
    } else if (state.direction() == ANIM_DIRECTION_RIGHT) {
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
