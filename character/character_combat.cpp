// character: shooting, charging, damage and effects (split from character.cpp, no change of behaviour)
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


// return 0 if must not attack, 1 for normal attack, 2 for semi-charged and 3 for fully charged
ATTACK_TYPES character::check_must_attack(bool always_charged)
{

    // capture button timer even if can't shoot, so we avoid always charging
    if (moveCommands.attack != 0 && attack_button_last_state == 0) {
        attack_button_pressed_timer = timer.getTimer();
    }

    if (timer.is_paused()) {
        return ATTACK_TYPE_NOATTACK;
    }

    if (state.animation_type() == ANIM_TYPE_TELEPORT) {
        return ATTACK_TYPE_NOATTACK;
    }
    if (sprite_set() == NULL) {
        return ATTACK_TYPE_NOATTACK;
    }

    if (state.animation_type() == ANIM_TYPE_SLIDE) {
        return ATTACK_TYPE_NOATTACK;
    }

    if (max_projectiles <= get_projectile_count()) {
        return ATTACK_TYPE_NOATTACK;
    }

    if (is_player() == true && get_projectile_max_shots(always_charged) <= projectile_list.size()) {
        return ATTACK_TYPE_NOATTACK;
    }

    int now_timer = timer.getTimer();
    int time_diff = now_timer - attack_button_pressed_timer;

    if (SharedData::get_instance()->game_config.turbo_mode == true && moveCommands.attack != 0) {
        if (now_timer < state.attack_timer + TURBO_ATTACK_INTERVAL) {
            return ATTACK_TYPE_NOATTACK;
        } else {
            return ATTACK_TYPE_NORMAL;
        }
    }

    // button changed from released to pressed
    if (moveCommands.attack != 0 && attack_button_last_state == 0) {
        return ATTACK_TYPE_NORMAL;
    // button changed from pressed to released and char can use charged attacks
    } else if (SharedData::get_instance()->game_config.turbo_mode == false && _charged_shot_projectile_id > 0 && moveCommands.attack == 0 && attack_button_last_state == 1) {
        // @TODO use super charged time also
        if (time_diff >= CHARGED_SHOT_TIME) {
            return ATTACK_TYPE_FULLYCHARGED;
        } else if (time_diff >= CHARGED_SHOT_INITIAL_TIME) {
            return ATTACK_TYPE_SEMICHARGED;
        }
    }
    return ATTACK_TYPE_NOATTACK;
}


void character::check_charging_colors(bool always_charged)
{
    // change player colors if charging attack
    int now_timer = timer.getTimer();
    int attack_diff_timer = now_timer-attack_button_pressed_timer;

    if (SharedData::get_instance()->game_config.turbo_mode == true) {
        return;
    }

    // don't charge if can't shot
    if (max_projectiles <= get_projectile_count()) {
        // reset time, so we start counting only when all projectiles are gone
        return;
    }

    if (is_player() == true && get_projectile_max_shots(always_charged) <= projectile_list.size()) {
        // reset time, so we start counting only when all projectiles are gone
        return;
    }

    if (_charged_shot_projectile_id > 0 && attack_diff_timer > CHARGED_SHOT_INITIAL_TIME && attack_diff_timer < CHARGED_SHOT_TIME && attack_button_last_state == 1 && moveCommands.attack == 1 && _simultaneous_shots < 2) {
        if (is_player() && soundManager.is_playing_repeated_sfx() == false) {
            soundManager.play_repeated_sfx(SFX_CHARGING1, 0);
        }
        if (color_keys[0].r != -1) {
            if (charging_color_timer < timer.getTimer()) {
                charging_color_n++;
                if (charging_color_n > 2) {
                    charging_color_n = 0;
                }
                charging_color_timer = timer.getTimer()+200;
                if (charging_color_n == 0) {
                    change_char_color(0, st_color(171, 0, 19), false);
                    change_char_color(1, st_color(231, 0, 91), false);
                } else if (charging_color_n == 1) {
                    change_char_color(0, st_color(231, 0, 91), false);
                    change_char_color(1, st_color(255, 119, 183), false);
                } else if (charging_color_n == 2) {
                    change_char_color(0, st_color(255, 119, 183), false);
                    change_char_color(1, st_color(171, 0, 19), false);
                }
            }
        }
    }
    if (_charged_shot_projectile_id > 0 && is_player() && attack_diff_timer >= CHARGED_SHOT_TIME && attack_button_last_state == 1 && moveCommands.attack == 1 && !is_dead()) {
        if (soundManager.is_playing_repeated_sfx() == true && soundManager.get_repeated_sfx_n() == SFX_CHARGING1) {
            soundManager.stop_repeated_sfx();
            soundManager.play_repeated_sfx(SFX_CHARGING2, 255);
        }
        if (color_keys[0].r != -1) {
            if (charging_color_timer < timer.getTimer()) {
                charging_color_n++;
                if (charging_color_n > 2) {
                    charging_color_n = 0;
                }
                charging_color_timer = timer.getTimer()+100;
                if (charging_color_n == 0) {
                    change_char_color(0, st_color(219, 43, 0), false);
                    change_char_color(1, st_color(255, 155, 59), false);
                } else if (charging_color_n == 1) {
                    change_char_color(0, st_color(255, 155, 59), false);
                    change_char_color(1, st_color(255, 234, 0), false);
                } else if (charging_color_n == 2) {
                    change_char_color(0, st_color(255, 234, 0), false);
                    change_char_color(1, st_color(219, 43, 0), false);
                }
            }
        }
    }
}


st_position character::get_attack_position()
{
    return get_attack_position(state.direction());
}



st_position character::get_attack_position(short direction)
{
    st_position proj_pos;
    if (direction == ANIM_DIRECTION_LEFT) {
        proj_pos = st_position(position.x+TILESIZE/3, position.y+frameSize.height/2);
    } else {
        proj_pos = st_position(position.x+frameSize.width-TILESIZE/2, position.y+frameSize.height/2);
    }
    if (is_player() == false) {
        st_position_int8 attack_arm_pos = GameMediator::get_instance()->get_enemy(_number)->attack_arm_pos;
        if (attack_arm_pos.x != 0 || attack_arm_pos.y >= 1) {
            if (direction == ANIM_DIRECTION_LEFT) {
                proj_pos = st_position(position.x + attack_arm_pos.x, position.y + attack_arm_pos.y);
            } else {
                proj_pos = st_position(position.x + frameSize.width - attack_arm_pos.x, position.y + attack_arm_pos.y);
            }
        }
    }
    return proj_pos;

}


/// @TODO: this must be moved to player, as character attack must be very simple
// projectile id of the shot, by the kind of attack (normal, semi-charged, fully charged). -1 = none
int character::choose_attack_projectile(ATTACK_TYPES must_attack, bool always_charged)
{
    int attack_id = -1;

    if (must_attack == ATTACK_TYPE_NORMAL) {
        if (always_charged == true) {
            attack_id = game_data.semi_charged_projectile_id;
        } else {
            if (_normal_shot_projectile_id > 0) {
                attack_id = _normal_shot_projectile_id;
            } else {
                attack_id = 0;
            }
        }
    } else if (must_attack == ATTACK_TYPE_SEMICHARGED) {
        attack_id = game_data.semi_charged_projectile_id;
    } else if (must_attack == ATTACK_TYPE_FULLYCHARGED) {
        attack_id = _charged_shot_projectile_id;
    }
    return attack_id;
}

// shots that follow the owner (centered, slash) or aim at the player/nearest enemy
void character::link_projectile_to_owner_and_target(projectile &temp_proj, int proj_trajectory)
{
    if (proj_trajectory == TRAJECTORY_CENTERED || proj_trajectory == TRAJECTORY_SLASH) {
        temp_proj.set_owner_direction(state.direction_ref());
        temp_proj.set_owner_position(&position);
		}
    if (proj_trajectory == TRAJECTORY_TARGET_DIRECTION || proj_trajectory == TRAJECTORY_TARGET_EXACT || proj_trajectory == TRAJECTORY_ARC_TO_TARGET || proj_trajectory == TRAJECTORY_FOLLOW) {
        // NPC
        if (!is_player() && gameControl.get_current_map_obj()->_player_ref != NULL) {
            character* p_player = gameControl.get_current_map_obj()->_player_ref;
            temp_proj.set_target_position(p_player->get_position_ref());
        // PLAYER
        } else {
            classnpc* temp_npc = NULL;
            if (proj_trajectory == TRAJECTORY_TARGET_DIRECTION || proj_trajectory == TRAJECTORY_TARGET_EXACT || proj_trajectory == TRAJECTORY_ARC_TO_TARGET) {
                temp_npc = gameControl.get_current_map_obj()->find_nearest_npc(st_position(position.x, position.y));
            } else {
                temp_npc = gameControl.get_current_map_obj()->find_nearest_npc_on_direction(st_position(position.x, position.y), state.direction());
            }
            if (temp_npc != NULL) {
                temp_proj.set_target_position(temp_npc->get_position_ref());
            }
        }
		}
}

void character::attack(bool dont_update_colors, short updown_trajectory, bool always_charged)
{
    if (attack_state != ATTACK_NOT && (timer.getTimer()-state.attack_timer) >= (*sprite_set()).frame_delay(state.direction(), state.animation_type(), state.animation_state())) {
		attack_state = ATTACK_NOT;
	}

    ATTACK_TYPES must_attack = check_must_attack(always_charged);
    check_charging_colors(always_charged);
    attack_button_last_state = moveCommands.attack;


    if (must_attack == ATTACK_TYPE_NOATTACK) {
        return;
    }
    int attack_id = choose_attack_projectile(must_attack, always_charged);

    if (attack_id != -1) {
        if (attack_id == _charged_shot_projectile_id || attack_id == game_data.semi_charged_projectile_id) {
			if (is_player() && soundManager.is_playing_repeated_sfx() == true) {
				soundManager.stop_repeated_sfx();
			}
		}

        st_position proj_pos = get_attack_position();

        projectile_list.push_back(projectile(attack_id, state.direction(), proj_pos, is_player(), _number));
        projectile &temp_proj = projectile_list.back();
        temp_proj.set_is_permanent();
        temp_proj.play_sfx(!is_player());
        temp_proj.set_owner(this);

        _player_must_reset_colors = true;


        if (attack_id == 0 || attack_id == _normal_shot_projectile_id) { // handle normal attack differences depending on player
            if (updown_trajectory == 1) {
                temp_proj.set_trajectory(TRAJECTORY_DIAGONAL_UP);
                set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_UP);
            } else if (updown_trajectory == -1) {
                temp_proj.set_trajectory(TRAJECTORY_DIAGONAL_DOWN);
                set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_DOWN);
            } else if (is_on_attack_frame() == true && updown_trajectory == 0 && (state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_UP || state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_DOWN)) { // not shooting diagonal, but animation is on diagonal -> reset to normal attack
                set_animation_type(ANIM_TYPE_ATTACK);
            }

        }

        int proj_trajectory = GameMediator::get_instance()->get_projectile(attack_id).trajectory;
        temp_proj.set_owner(this);
        link_projectile_to_owner_and_target(temp_proj, proj_trajectory);

        // second projectile for player that fires multiple ones. It is added after the first one is fully set up:
        // push_back can reallocate the list, which left temp_proj dangling for the code above
        if ((attack_id == 0 || attack_id == _normal_shot_projectile_id || (attack_id == game_data.semi_charged_projectile_id && always_charged == true)) && is_player() && _simultaneous_shots > 1) { /// @TODO - move number of simultaneous shots to character/data-file
            int pos_x_second = proj_pos.x+TILESIZE;
            if (state.direction() == ANIM_DIRECTION_RIGHT) {
                pos_x_second = proj_pos.x-TILESIZE;
            }
            projectile_list.push_back(projectile(attack_id, state.direction(), st_position(pos_x_second, proj_pos.y+5), is_player(), _number));
            projectile &temp_proj2 = projectile_list.back();
            temp_proj2.set_is_permanent();
            temp_proj2.set_owner(this);
        }

		attack_state = ATTACK_START;
		state.attack_timer = timer.getTimer();
		if (state.animation_type() == ANIM_TYPE_STAND) {
            set_animation_type(ANIM_TYPE_ATTACK);
		} else if (state.animation_type() == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
		} else if (is_in_stairs_frame()) {
            set_animation_type(ANIM_TYPE_STAIRS_ATTACK);
		} else if (state.animation_type() == ANIM_TYPE_WALK) {
            set_animation_type(ANIM_TYPE_WALK_ATTACK);
		}
    }
}


void character::consume_projectile()
{
    if (projectile_list.size() > 0) {
        projectile_list.at(0).consume_projectile();
    }

}


void character::recharge(e_energy_types _en_type, int value)
{
	if (_en_type == ENERGY_TYPE_HP) {
        if (hitPoints.current < hitPoints.total) {
            if (hitPoints.current + value <= hitPoints.total) {
				hitPoints.current += value;
			} else {
                hitPoints.current = hitPoints.total;
			}

            if (value > ENERGY_ITEM_SMALL) {
                soundManager.play_sfx(SFX_GOT_ENERGY_BIG);
            } else {
                soundManager.play_sfx(SFX_GOT_ENERGY);
            }
        }
	}
}




bool character::get_item(object_collision& obj_info)
{
	bool res = false;
	// deal with non-blocking items
	if (obj_info._object != NULL && obj_info._object->finished() == false) {
		switch (obj_info._object->get_type()) {
            case OBJ_ENERGY_PILL_SMALL:
                recharge(ENERGY_TYPE_HP, ENERGY_ITEM_SMALL);
                res = true;
                obj_info._object->set_finished(true);
                break;
            case OBJ_ENERGY_PILL_BIG:
                recharge(ENERGY_TYPE_HP, ENERGY_ITEM_BIG);
                res = true;
                obj_info._object->set_finished(true);
                break;
            default:
                break;
		}
	}
    return res;
}


// returns type, or -1 if none
int character::is_executing_effect_weapon()
{
    vector<projectile>::iterator it;
    for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        int move_type = (*it).get_move_type();
        if (move_type == TRAJECTORY_BOMB) {
            return TRAJECTORY_BOMB;
        } else if (move_type == TRAJECTORY_QUAKE) {
            return TRAJECTORY_QUAKE;
        } else if (move_type == TRAJECTORY_FREEZE) {
            return TRAJECTORY_FREEZE;
        } else if (move_type == TRAJECTORY_CENTERED) {
            return TRAJECTORY_CENTERED;
        } else if (move_type == TRAJECTORY_PUSH_BACK) {
            return TRAJECTORY_PUSH_BACK;
        } else if (move_type == TRAJECTORY_PULL) {
            return TRAJECTORY_PULL;
        }
    }
    return -1;
}


// is all projectiles are normal (-1 or 0) return the character's max_shots,
// otherwise, find the lowest between all fired projectiles
Uint8 character::get_projectile_max_shots(bool always_charged)
{
    bool all_projectiles_normal = true;
    vector<projectile>::iterator it;
    short max_proj = 9;
    for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        short id = (*it).get_id();
        // if always charged, and projectile is semi-charged, count as normal
        if (id != -1 && id != 0) {
            if (always_charged == true && id == game_data.semi_charged_projectile_id) {
                continue;
            }
            all_projectiles_normal = false;
            short proj_max = (*it).get_max_shots();
            if (max_proj > 0 && proj_max < max_proj) {
                max_proj = proj_max;
            }
        }
    }
    if (all_projectiles_normal == true) {
        return max_projectiles;
    }
    return max_proj;
}


void character::push_back(short direction)
{
    int xinc = -(move_speed-0.2);
    if (direction == ANIM_DIRECTION_LEFT) {
        xinc = (move_speed-0.2);
    }

    if (test_change_position(xinc, 0)) {
        position.x += xinc;
    }
}


void character::pull(short direction)
{
    int xinc = (move_speed-0.2);
    if (direction == ANIM_DIRECTION_LEFT) {
        xinc = -(move_speed-0.2);
    }

    if (test_change_position(xinc, 0)) {
        position.x += xinc;
    }
}




void character::remove_freeze_effect()
{
    state.frozen = false;
    state.frozen_timer = 0;
}


void character::add_projectile(short id, st_position pos, int trajectory, int direction)
{
    projectile_to_be_added_list.push_back(projectile(id, direction, pos, is_player(), _number));
    projectile &new_projectile = projectile_to_be_added_list.back();
    new_projectile.set_is_permanent();
    new_projectile.set_trajectory(trajectory);
}



unsigned int character::get_projectile_count()
{
	int pcount = 0;
    vector<projectile>::iterator it;
	for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        pcount++;
	}
	return pcount;
}


void character::clean_projectiles()
{
	while (!projectile_list.empty()) {
        projectile_list.at(0).finish();
		projectile_list.erase(projectile_list.begin());
    }
    if (is_player() && freeze_weapon_effect == FREEZE_EFFECT_NPC) {
        freeze_weapon_effect = FREEZE_EFFECT_NONE;
    }
}


void character::clean_effect_projectiles()
{
    while (true) {
        bool found_item = false;
        for (unsigned int i=0; i<projectile_list.size(); i++) {
            Uint8 move_type = projectile_list.at(i).get_move_type();
            if (move_type == TRAJECTORY_QUAKE || move_type == TRAJECTORY_FREEZE || move_type == TRAJECTORY_CENTERED || move_type == TRAJECTORY_PUSH_BACK || move_type == TRAJECTORY_PULL) {
                found_item = true;
                projectile_list.at(i).finish();
                projectile_list.erase(projectile_list.begin()+i);
                break;
            }
        }
        if (found_item == false) {
            return;
        }
    }
}


void character::damage(unsigned int damage_points, bool ignore_hit_timer = false)
{
    UNUSED(ignore_hit_timer);
    const bool instant_kill = (damage_points >= 999); // crushers, death rays, "inside an object": the hit invulnerability must not save from them
	if (is_player() && GAME_FLAGS[FLAG_INVINCIBLE] == true) {
		return;
	}
    if (game_save.difficulty == DIFFICULTY_HARD && is_player() == false) {
        damage_points = damage_points * 0.5;
    }
    if (damage_points < 1) { // minimum damage is 1. if you don't want damage, don't call this method, ok? :)
        damage_points = 1;
    }
    if (damage_points + _damage_modifier > 0) {
        damage_points += _damage_modifier;
    }

    if (hitPoints.current <= 0) { // already dead
		return;
	}

    if (state.frozen == true && is_player()) {
        state.frozen_timer = 0;
		state.frozen = false;
	}

	unsigned int now_timer = timer.getTimer();
    if (instant_kill == false && now_timer < hit_duration+last_hit_time) { // is still intangible from last hit
        return;
    }

    _was_hit = true;

    if (is_in_stairs_frame() == true) {
        _dropped_from_stairs = true;
    }

    last_hit_time = now_timer;
    if (now_timer > hit_duration+last_hit_time) {
        hit_animation_timer = now_timer+HIT_BLINK_ANIMATION_LAPSE;
    }

    if (!is_player() || GAME_FLAGS[FLAG_INFINITE_HP] == false) {
        hitPoints.current -= damage_points;
    }


    if (is_player() == true && state.animation_type() != ANIM_TYPE_HIT) {
        soundManager.play_sfx(SFX_PLAYER_HIT);
        set_animation_type(ANIM_TYPE_HIT);
        if (_obj_jump.is_started() == true) {
            hit_moved_back_n = get_hit_push_back_n()/2;
            _obj_jump.finish();
        } else {
            hit_moved_back_n = 0;
        }
        jump_button_released = false;
        if (gameControl.get_current_map_obj() != NULL) {
            int hit_anim_x = 0;
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                hit_anim_x = 3;
            }
        }
	}

	// TODO: add hit animation
	if (hitPoints.current <= 0) {
		hitPoints.current = 0;
		death();
    }
}


void character::damage_spikes(bool ignore_hit_timer)
{
    character::damage(SPIKES_DAMAGE);
}


bool character::is_dead() const
{
	return (hitPoints.current <= 0);
}



st_hit_points character::get_hp() const
{
	return hitPoints;
}


void character::set_current_hp(Uint8 value)
{
    // it was "current += inc": using an energy tank (set_current_hp(total)) gave current+total, above the maximum
    if (value > hitPoints.total) {
        value = hitPoints.total;
    }
    hitPoints.current = value;
}



bool character::is_shielded(int projectile_direction) const
{
	if (is_player()) {
        if (input.p1_input[BTN_SHIELD] == 1 && state.animation_type() == ANIM_TYPE_SHIELD) { // player is on SHIELD animation and is keeping the shield button pressed
            if (shield_type == SHIELD_FULL || shield_type == SHIELD_FRONT) { // player can use shield
                return true;
            }
        }
		return false;
	} else {
        //std::cout << "NPC[" << name << "], shield_type[" << shield_type << "], SHIELD_BACK[" << (int)SHIELD_BACK << "], anim-type[" << state.animation_type() << "], ANIM_TYPE_SHIELD[" << ANIM_TYPE_SHIELD << "]" << std::endl;
        if (shield_type == SHIELD_FULL || (shield_type == SHIELD_FRONT && projectile_direction != state.direction() && (state.animation_type() == ANIM_TYPE_STAND || state.animation_type() == ANIM_TYPE_WALK  || state.animation_type() == ANIM_TYPE_WALK_AIR)) || (shield_type == SHIELD_STAND && state.animation_type() == ANIM_TYPE_STAND)) {
			return true;
		}
        if (shield_type == SHIELD_STAND_AND_WALK && projectile_direction != state.direction() && (state.animation_type() == ANIM_TYPE_STAND || state.animation_type() == ANIM_TYPE_WALK)) {
            return true;
        }
        if (shield_type == SHIELD_STAND_FRONT && projectile_direction != state.direction() && state.animation_type() == ANIM_TYPE_STAND) {
            return true;
        }
        if (shield_type == SHIELD_BACK && projectile_direction == state.direction()) {
            return true;
        }
        if (shield_type == SHIELD_USING_SHIELD && state.animation_type() == ANIM_TYPE_SHIELD) {
            return true;
        }
        return false;
    }
}


// contact damage: only a disguised enemy can't hurt by touch; the blink after being hit (is_intangible) does not void it
bool character::is_intangible_to_touch()
{
    if (is_player()) {
        return false;
    }
    return (state.animation_type() == ANIM_TYPE_STAND && shield_type == SHIELD_DISGUISE);
}


bool character::is_intangible()
{
    if (is_player()) {
        return false;
    }
    if (state.animation_type() == ANIM_TYPE_STAND && shield_type == SHIELD_DISGUISE) {
        return true;
    }
    if (timer.getTimer() < hit_duration+last_hit_time) { // is still intangible from last hit
        return true;
    }
    return false;
}



bool character::is_using_circle_weapon()
{
    if (projectile_list.size() == 1) {
        if (projectile_list.at(0).get_move_type() == TRAJECTORY_CENTERED) {
            return true;
        }
    }
    return false;
}


void character::inc_effect_weapon_status()
{
    if (projectile_list.size() == 1) {
        int move_type = projectile_list.at(0).get_move_type() ;
        if (move_type == TRAJECTORY_CENTERED || move_type == TRAJECTORY_BOMB) {
            projectile_list.at(0).inc_status();
        }
    }
}


bool character::is_weak_to_freeze()
{
    if (freeze_weapon_id == -1) {
        return false;
    }
    int wpn_id = -1;
    for (int i=0; i<MAX_WEAPON_N; i++) {
        if (game_data.weapons[i].id_projectile == freeze_weapon_id) {
            wpn_id = i;
        }
    }
    if (wpn_id == -1) {
        return false;
    }
    if (GameMediator::get_instance()->get_enemy(_number)->weakness[wpn_id].damage_multiplier == 0) {
        return false;
    }
    return true;
}


float character::get_hit_push_back_n()
{
    //return TILESIZE*0.8;
    return TILESIZE * 1.2;
}


bool character::have_shoryuken()
{
    return false;
}


int character::get_armor_arms_attack_id()
{
    return -1;
}
