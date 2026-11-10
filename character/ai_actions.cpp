// artificial intelligence: shooting, spawning and replacing NPCs (split from artificial_inteligence.cpp, no change of behaviour)
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


void artificial_inteligence::execute_ai_action_trow_projectile(Uint8 n, bool invert_direction)
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.animation_type() == ANIM_TYPE_WALK_AIR) {
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
        state.set_animation_state(0);
        state.set_animation_timer(timer.getTimer() + (*sprite_set()).frame_delay(state.direction(), state.animation_type(), state.animation_state()));
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        _did_shot = false;
    } else {
        if (state.animation_restarted() == true && _did_shot == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _did_shot = false;
            if (state.animation_type() == ANIM_TYPE_WALK_AIR || state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) {
                set_animation_type(ANIM_TYPE_JUMP);
            } else {
                set_animation_type(ANIM_TYPE_STAND);
            }
        } else if ((state.on_attack_frame() == true || state.on_last_frame() == true) && _did_shot == false) { // only shoot when reached the last frame in animation attack
            shot_success = throw_projectile(_parameter, invert_direction);
            _did_shot = true;
            shot_timer = timer.getTimer();
        }
    }
}


void artificial_inteligence::execute_shot_multiple_projectile()
{
    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.animation_type() == ANIM_TYPE_WALK_AIR) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (is_on_attack_frame() == false){
            set_animation_type(ANIM_TYPE_ATTACK);
        }
        state.set_animation_state(0);
        state.set_animation_timer(timer.getTimer() + (*sprite_set()).frame_delay(state.direction(), state.animation_type(), state.animation_state()));
        _ai_state.sub_status = IA_ACTION_STATE_EXECUTING;
        _did_shot = false;
    } else {
        if (state.animation_restarted() == true && _did_shot == true) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            _did_shot = false;
            if (state.animation_type() == ANIM_TYPE_WALK_AIR || state.animation_type() == ANIM_TYPE_JUMP || state.animation_type() == ANIM_TYPE_JUMP_ATTACK) {
                set_animation_type(ANIM_TYPE_JUMP);
            } else {
                set_animation_type(ANIM_TYPE_STAND);
            }
        } else if ((state.on_attack_frame() == true || state.on_last_frame() == true) && _did_shot == false) { // only shoot when reached the last frame in animation attack
            if (_parameter == AI_ACTION_SHOT_MULTIPLE_PROJECTILE_5_UP) {
                throw_direction_projectile(ANIM_DIRECTION_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_UP_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_UP);
                throw_direction_projectile(ANIM_DIRECTION_UP_RIGHT);
                throw_direction_projectile(ANIM_DIRECTION_RIGHT);
            } else if (_parameter == AI_ACTION_SHOT_MULTIPLE_PROJECTILE_8) {
                throw_direction_projectile(ANIM_DIRECTION_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_UP_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_UP);
                throw_direction_projectile(ANIM_DIRECTION_UP_RIGHT);
                throw_direction_projectile(ANIM_DIRECTION_RIGHT);
                throw_direction_projectile(ANIM_DIRECTION_DOWN_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_DOWN);
                throw_direction_projectile(ANIM_DIRECTION_DOWN_RIGHT);
            } else if (_parameter == AI_ACTION_SHOT_MULTIPLE_PROJECTILE_3_UP) {
                throw_direction_projectile(ANIM_DIRECTION_LEFT);
                throw_direction_projectile(ANIM_DIRECTION_UP);
                throw_direction_projectile(ANIM_DIRECTION_RIGHT);
            } else if (_parameter == AI_ACTION_SHOT_MULTIPLE_PROJECTILE_3_AHEAD) {
                if (state.direction() == ANIM_DIRECTION_LEFT) {
                    throw_direction_projectile(ANIM_DIRECTION_LEFT);
                    throw_direction_projectile(ANIM_DIRECTION_UP_LEFT);
                    throw_direction_projectile(ANIM_DIRECTION_DOWN_LEFT);
                } else {
                    throw_direction_projectile(ANIM_DIRECTION_RIGHT);
                    throw_direction_projectile(ANIM_DIRECTION_UP_RIGHT);
                    throw_direction_projectile(ANIM_DIRECTION_DOWN_RIGHT);
                }
            }
            _did_shot = true;
            shot_timer = timer.getTimer();
            //_ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        }
    }
}


void artificial_inteligence::execute_explode_itself()
{
    CURRENT_FILE_FORMAT::file_projectilev3 temp_projectile = GameMediator::get_instance()->get_projectile(0);
    projectile_list.push_back(projectile(0, state.direction(), get_attack_position(), is_player(), _number));
    projectile &temp_proj = projectile_list.back();
    temp_proj.transform_into_explosion();
    hitPoints.current = 0;
}


void artificial_inteligence::execute_throw_item()
{
    int x_pos = position.x + frameSize.width + TILESIZE;
    if (state.direction() == ANIM_DIRECTION_LEFT) {
        x_pos = position.x - TILESIZE;
    }
    gameControl.get_current_map_obj()->drop_great_random_item(st_position(x_pos, position.y - TILESIZE));
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
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
        if ((dist_players.pObj->getPosition().x > position.x && state.direction() == ANIM_DIRECTION_LEFT) || (dist_players.pObj->getPosition().x < position.x && state.direction() == ANIM_DIRECTION_RIGHT)) {
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

    int proj_direction = state.direction();
    if (invert_direction == true) {
        proj_direction = opposite_direction(proj_direction);
    }

    if (is_shooter) {
        proj_direction = shoot_direction;
    }

    projectile_list.push_back(projectile(projectile_type, proj_direction, get_attack_position(), is_player(), _number));
    projectile &temp_proj = projectile_list.back();
    temp_proj.play_sfx(true);
    temp_proj.set_owner(this);


    if (temp_projectile.trajectory == TRAJECTORY_CENTERED) {
        temp_proj.set_owner_direction(state.direction_ref());
        temp_proj.set_owner_position(&position);
    }

    if (temp_projectile.trajectory == TRAJECTORY_TARGET_DIRECTION || temp_projectile.trajectory == TRAJECTORY_TARGET_EXACT || temp_projectile.trajectory == TRAJECTORY_ARC_TO_TARGET || temp_projectile.trajectory == TRAJECTORY_FOLLOW) {
        if (!is_player() && gameControl.get_current_map_obj()->_player_ref != NULL) {
            character* p_player = gameControl.get_current_map_obj()->_player_ref;
            //std::cout << "AI::throw_projectile::set_target_pos[" << p_player->get_position_ref()->x << "][" << p_player->get_position_ref()->y << "], real_pos[" << p_player->get_real_position().x << "][" << p_player->get_real_position().y << "]" << std::endl;
            temp_proj.set_target_position(p_player->get_position_ref());
        }
    }

    return true;
}


void artificial_inteligence::throw_direction_projectile(int direction)
{
    projectile_list.push_back(projectile(0, direction, get_attack_position(), is_player(), _number));
    projectile &temp_proj = projectile_list.back();
}


void artificial_inteligence::execute_ai_replace_itself(bool morph)
{
    // kills/remove itself
    _dead_state = 2;
    st_hit_points hp_copy = hitPoints;
    hitPoints.current = 0;
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
    // spawn new npc
    classnpc* npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, st_position(position.x, position.y+frameSize.height/2), state.direction(), false, false);
    if (npc_ref == NULL) { // could not create the replacement
        return;
    }

    // is executing reaction and is dying and is map-boss -> set child as new map-boss
    if (_reaction_state == 1 && _reaction_type == 2 && _is_stage_boss == true) {
        _is_stage_boss = false;
        npc_ref->set_stage_boss(true);
        hp_copy.total = BOSS_INITIAL_HP;
        hp_copy.current = hitPoints.total;
        npc_ref->npc_set_hp(hp_copy);
    // is morphing into the new NPC, copy some properties
    } else if (morph == true) {
        if (_is_stage_boss) {
            _is_stage_boss = false;
            npc_ref->set_stage_boss(true);
        }
        // @TODO: the boss HP HUD gets lost with morph //
        _dead_state = DEAD_STATE_IGNORE;
        npc_ref->npc_set_hp(hp_copy);
    }
    st_float_position new_pos = position;
    //new_pos.y += frameSize.height - npc_ref->get_size().height; // adjust Y post because of heigth difference //
    npc_ref->npc_set_position(new_pos);
    npc_ref->npc_set_initialized(4);
}



void artificial_inteligence::execute_ai_step_spawn_npc()
{
    // still spawning an NPC, leave
    if (gameControl.get_current_map_obj()->_npc_spawn_list.size() > 0) {
        //std::cout << "AI::execute_ai_step_spawn_npc - still waiting for last spawn, LEAVE!" << std::endl;
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }

    int child_count = gameControl.get_current_map_obj()->child_npc_count(start_point);
    if (child_count >= MAX_NPC_SPAWN) {
        //std::cout << "AI::execute_ai_step_spawn_npc - reached MAX_NPC_SPAWN, LEAVE!" << std::endl;
        _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
        return;
    }

    if (_ai_state.sub_status == IA_ACTION_STATE_INITIAL) {
        if (state.animation_type() == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (is_on_attack_frame() == false){
            set_animation_type(ANIM_TYPE_ATTACK);
        }
        classnpc* npc_ref;
        if (name == "TOP HAT") {
            npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, st_position(position.x, position.y), state.direction(), false, true);
        } else {
            st_position_int8 attack_arm_pos = GameMediator::get_instance()->get_enemy(_number)->attack_arm_pos;
            st_position proj_pos;
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                proj_pos = st_position(position.x + attack_arm_pos.x, position.y + attack_arm_pos.y);
            } else {
                proj_pos = st_position(position.x + frameSize.width - attack_arm_pos.x, position.y + attack_arm_pos.y);
            }
            proj_pos.x -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x;
            proj_pos.y -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y;
            npc_ref = gameControl.get_current_map_obj()->spawn_map_npc(_parameter, proj_pos, state.direction(), false, false);
        }

        if (npc_ref == NULL) {
            _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
            return;
        }

        npc_ref->set_parent_id(start_point);

        // is executing reaction and is dying and is map-boss -> set child as new map-boss
        if (_reaction_state == 1 && _reaction_type == 2 && _is_stage_boss == true) {
            RockbotLogger::get_instance()->write("AI::SPAWN, SET NEW BOSS");
            _is_stage_boss = false;
            npc_ref->set_stage_boss(true);
        }
    }
    _ai_state.sub_status = IA_ACTION_STATE_FINISHED;
}
