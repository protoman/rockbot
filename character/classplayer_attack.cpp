#include <sstream>
#include <list>
#include <math.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include "classplayer.h"
#include "../inputlib.h"
#include "../game_mediator.h"

extern inputLib input;
#include "../class_config.h"

#include "../soundlib.h"
extern soundLib soundManager;

#include "../game.h"
extern game gameControl;


#define PLAYER_MOVE_SPEED 1.65 // higher is faster
#include "../file/file_io.h"
#include "../classmap.h"


extern std::string FILEPATH;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern CURRENT_FILE_FORMAT::file_io fio;
extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;

bool classPlayer::shouryuken()
{
    // trying to start command (can only start if standing on the ground: against a wall the animation can fall back to STAND in the air, which allowed to chain shoryukens forever)
    if (moveCommands.up != 0 && moveCommands.dash != 0 && state.animation_type() == ANIM_TYPE_STAND && hit_ground() == true)  {
        set_animation_type(ANIM_TYPE_SPECIAL_ATTACK);
        _can_execute_airdash = false;
        soundManager.play_sfx(SFX_SHORYUKEN_GIRL);
        _obj_jump.start(true, TERRAIN_UNBLOCKED);
        return true;
    // is executing
    } else if (state.animation_type() == ANIM_TYPE_SPECIAL_ATTACK) {
        _obj_jump.execute(TERRAIN_UNBLOCKED);
        float jump_speed = _obj_jump.get_speed();
        bool jump_moved = false;

        // check collision
        for (int i=abs((float)jump_speed); i>0; i--) {
            int speed_y = 0;
            if (jump_speed > 0) {
                speed_y = i;
            } else {
                speed_y = i*-1;
            }
            st_map_collision map_col = map_collision(0, speed_y, gameControl.get_current_map_obj()->getMapScrolling());
            int map_lock = map_col.block;

            if (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER) {
                position.y += speed_y;
                jump_moved = true;
                break;
            }
        }
        if (jump_speed != 0 && jump_moved == false) {
            if (jump_speed < 0) {
                _obj_jump.interrupt();
            } else {
                _obj_jump.finish();
                set_animation_type(ANIM_TYPE_STAND);
            }
        }



        if (_obj_jump.is_started() == false) {
            return false;
        }
        return true;
    }
    return false;
}

// shot with the default weapon (buster): charge, diagonal aim, and the color reset after a charged shot
void classPlayer::attack_with_default_weapon(bool always_charged)
{
    /// @NOTE: desabilitei o tiro em diagonal pois vai precisar mudanças no sistema de arquivos para comportar as poses/frames de ataque para cima e para baixo

    if (SharedData::get_instance()->game_config.auto_charge_mode && !is_dead()) {
        if (moveCommands.attack == 1) {
            moveCommands.attack = 0;
            attack_button_released = true;
        } else {
            moveCommands.attack = 1;
            attack_button_released = false;
        }
    }

    bool is_on_ground = hit_ground();
    if (can_shoot_diagonal() == true && moveCommands.up != 0 && is_on_ground == true) {
        character::attack(false, 1, always_charged);
    } else if (can_shoot_diagonal() == true && moveCommands.down != 0 && is_on_ground == true) {
        character::attack(false, -1, always_charged);
    } else {
        character::attack(false, 0, always_charged);
    }
    if (_player_must_reset_colors == true) {
        _player_must_reset_colors = false;
        change_player_color(true);
    }
}

// coil/jet/tanks are "weapons" in the list: uses the item. Returns the weapon id to shoot (0 = nothing to shoot, -1 = item that
// is only a shot, like the coil that cannot be placed because another player object exists)
short int classPlayer::use_weapon_item(int used_weapon)
{
    short int weapon_id = 0;

    if (used_weapon == WEAPON_ITEM_COIL) {
        if (gameControl.get_current_map_obj()->have_player_object() == true) {
            weapon_id = -1;
        } else {
            add_coil_object();
        }
        //set_weapon(WEAPON_DEFAULT, true);
    } else if (used_weapon == WEAPON_ITEM_JET) {
        if (gameControl.get_current_map_obj()->have_player_object() == true) {
            weapon_id = -1;
        } else {
            add_jet_object();
        }
        //set_weapon(WEAPON_DEFAULT, true);
    } else if (used_weapon == WEAPON_ITEM_ETANK) {
        class_config config_manager;
        config_manager.set_player_ref(this);
        config_manager.use_tank(TANK_ENERGY);
        set_weapon(WEAPON_DEFAULT, true);
    } else if (used_weapon == WEAPON_ITEM_WTANK) {
        class_config config_manager;
        config_manager.set_player_ref(this);
        config_manager.use_tank(TANK_WEAPON);
        set_weapon(WEAPON_DEFAULT, true);
    } else if (used_weapon == WEAPON_ITEM_STANK) {
        class_config config_manager;
        config_manager.set_player_ref(this);
        config_manager.use_tank(TANK_SPECIAL);
        set_weapon(WEAPON_DEFAULT, true);
    } else {
        weapon_id = used_weapon;
    }
    return weapon_id;
}

// links the new projectile to what it needs: owner, nearest enemy as target, and the aiming animation
void classPlayer::setup_weapon_projectile(projectile &temp_proj, int weapon_trajectory)
{
    if (weapon_trajectory == TRAJECTORY_CENTERED || weapon_trajectory == TRAJECTORY_SLASH) {
        temp_proj.set_owner_direction(state.direction_ref());
        temp_proj.set_owner_position(&position);
    } else if (weapon_trajectory == TRAJECTORY_CHAIN) {
        temp_proj.set_owner_position(&position);
        temp_proj.set_owner_direction(state.direction_ref());

    } else if (weapon_trajectory == TRAJECTORY_FOLLOW) {
        st_rectangle hitbox = get_hitbox();
        classnpc* temp = gameControl.get_current_map_obj()->find_nearest_npc(st_position(hitbox.x+hitbox.w/2, hitbox.y+hitbox.h/2));
        if (temp != NULL) {
            temp_proj.set_target_position(temp->get_position_ref());
        }
    } else if (weapon_trajectory == TRAJECTORY_TARGET_DIRECTION || weapon_trajectory == TRAJECTORY_TARGET_EXACT || weapon_trajectory == TRAJECTORY_ARC_TO_TARGET) {
        st_rectangle hitbox = get_hitbox();
        st_position player_pos(hitbox.x+hitbox.w/2, hitbox.y+hitbox.h/2);
        classnpc* temp = gameControl.get_current_map_obj()->find_nearest_npc_on_direction(player_pos, state.direction());
        if (temp != NULL) {
            temp_proj.set_target_position(temp->get_position_ref());
        }
        if (weapon_trajectory == TRAJECTORY_ARC_TO_TARGET) {
            set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_UP);
        } else if (temp != NULL && (weapon_trajectory == TRAJECTORY_TARGET_DIRECTION || weapon_trajectory == TRAJECTORY_TARGET_EXACT)) {
            st_rectangle npc_hitbox = temp->get_hitbox();
            st_position npc_pos(npc_hitbox.x + (npc_hitbox.w/2), npc_hitbox.y + (npc_hitbox.h/2));
            // check if fire if diagonal (distY > distX)
            int distY = abs(npc_pos.y-(player_pos.y-TILESIZE*2));
            if (distY > 0) {
                if (npc_pos.y > player_pos.y) {
                    set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_DOWN);
                } else {
                    set_animation_type(ANIM_TYPE_ATTACK_DIAGONAL_UP);
                }
            }
        }
    }
}

void classPlayer::attack(bool dont_update_colors)
{
    UNUSED(dont_update_colors);
    st_position proj_pos;


    if (state.animation_type() == ANIM_TYPE_HIT) { // can't fire when hit
        return;
    }

    if (state.animation_type() == ANIM_TYPE_SHIELD) { // can't attack when using shield
        return;
    }

    bool always_charged = false;
    // player with armor-special-type changes to auto-carged instead of charging shot
    if (game_save.armor_pieces[ARMOR_TYPE_ARMS] == true && _simultaneous_shots > 1) {
        always_charged = true;
    }

    if (selected_weapon == WEAPON_DEFAULT) {
        attack_with_default_weapon(always_charged);
        return;
    } else if (game_save.items.weapons[selected_weapon] <= 0) {
        return;
    }


    if (moveCommands.attack == 0 && attack_button_released == false) {
        attack_button_released = true;
        return;
    }

    int effect_type = is_executing_effect_weapon();
    int used_weapon = selected_weapon;
    if (effect_type == TRAJECTORY_FREEZE) { // freeze can shoot normal projectiles
        if (max_projectiles > get_projectile_count()) {
            character::attack(true, 0, always_charged);
        }
        return;
    } else if (effect_type != -1) {
        if (moveCommands.attack != 0 && (timer.getTimer()-state.attack_timer) > 100 && attack_button_released == true) {
            inc_effect_weapon_status(); // this method have a filter to inc only the types that are effect (centered, bomb, etc)
        }
        return;
    }
    if (max_projectiles <= get_projectile_count()) {
        return;
    }

    if (moveCommands.attack != 0 && (timer.getTimer()-state.attack_timer) > 100 && attack_button_released == true) {
        attack_button_released = false;


        if (state.direction() == ANIM_DIRECTION_LEFT) {
            proj_pos = st_position(position.x, position.y+frameSize.height/2);
        } else {
            proj_pos = st_position(position.x+frameSize.width-TILESIZE*2, position.y+frameSize.height/2);
        }

        short int weapon_id = use_weapon_item(used_weapon);

        if (weapon_id == 0) { /// @TODO - this is a temporary exit to handle incomplete weapons
            return;
        } else if (weapon_id == -1) {
            weapon_id = 0;
        }

        // check if projectiles limit from weapon/projectile os not reached
        if (projectile_list.size() >= GameMediator::get_instance()->get_projectile(game_data.weapons[weapon_id].id_projectile).max_shots) {
            return;
        }

        projectile_list.push_back(projectile(game_data.weapons[weapon_id].id_projectile, state.direction(), get_attack_position(), is_player(), _number));
        projectile &temp_proj = projectile_list.back();
        temp_proj.play_sfx(false);
        temp_proj.set_is_permanent();
        temp_proj.set_weapon_id(weapon_id);
        temp_proj.set_owner(this);


        int weapon_trajectory = GameMediator::get_instance()->get_projectile(game_data.weapons[weapon_id].id_projectile).trajectory;
        setup_weapon_projectile(temp_proj, weapon_trajectory);

        if (selected_weapon != WEAPON_DEFAULT) {
            if (weapon_trajectory == TRAJECTORY_QUAKE) {
                consume_weapon(4);
            } else if (weapon_trajectory == TRAJECTORY_FREEZE) {
                consume_weapon(2);
            } else if (used_weapon != WEAPON_ITEM_COIL && used_weapon != WEAPON_ITEM_JET) {
                consume_weapon(1);
            }
        }

        attack_state = ATTACK_START;
        state.attack_timer = timer.getTimer();
        if (state.animation_type() == ANIM_TYPE_STAND) {
            set_animation_type(ANIM_TYPE_ATTACK);
        } else if (state.animation_type() == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (state.animation_type() == ANIM_TYPE_STAIRS || state.animation_type() == ANIM_TYPE_STAIRS_SEMI || state.animation_type() == ANIM_TYPE_STAIRS_MOVE) {
            set_animation_type(ANIM_TYPE_STAIRS_ATTACK);
        } else if (state.animation_type() == ANIM_TYPE_WALK) {
            set_animation_type(ANIM_TYPE_WALK_ATTACK);
        }
    }
}

void classPlayer::damage_ground_npcs()
{
	/// @TODO - this part must be done only ONCE
	// find quake in projectiles list
	int projectile_n = -1;
    for (int i =0; i<GameMediator::get_instance()->get_projectile_list_size(); i++) {
        if (GameMediator::get_instance()->get_projectile(i).trajectory == TRAJECTORY_QUAKE) {
			projectile_n = i;
			break;
		}
	}
	if (projectile_n == -1) {
		return;
	}
	// find quake weapon in weapons list
	int weapon_n = -1;
	for (int i = 0; i<MAX_WEAPON_N; i++) {
		if (game_data.weapons[i].id_projectile == projectile_n) {
			weapon_n = i;
			break;
		}
	}
	// could not find the weapon
	if (weapon_n == -1) {
		return;
	}

    for (int i=0; i<gameControl.get_current_map_obj()->_npc_list.size(); i++) {
        if (gameControl.get_current_map_obj()->_npc_list.at(i).is_on_visible_screen() == false) {
			continue;
		}
        if (gameControl.get_current_map_obj()->_npc_list.at(i).is_player_friend() == true) {
            continue;
        }

		// check if NPC is vulnerable to quake (all bosses except the one with weakness are not)
        short damage = GameMediator::get_instance()->get_enemy(gameControl.get_current_map_obj()->_npc_list.at(i).get_number())->weakness[weapon_n].damage_multiplier;

		// check if NPC is on ground
        st_position npc_pos(gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().x, gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().y);
        npc_pos.x = (npc_pos.x + gameControl.get_current_map_obj()->_npc_list.at(i).get_size().width/2)/TILESIZE;
        npc_pos.y = (npc_pos.y + gameControl.get_current_map_obj()->_npc_list.at(i).get_size().height)/TILESIZE;
        int lock = gameControl.get_current_map_obj()->getMapPointLock(npc_pos);
		if (lock == TERRAIN_UNBLOCKED || lock == TERRAIN_STAIR || lock == TERRAIN_WATER) {
			continue;
		} else {
            gameControl.get_current_map_obj()->_npc_list.at(i).damage(damage, false);
		}
	}
}

void classPlayer::execute_projectiles()
{
    // animate projectiles
    vector<projectile>::iterator it;
    bool ignore_hit_timer = false;
    if (_simultaneous_shots > 1) {
        ignore_hit_timer = true;
    }

    // erasing a projectile used to "break" out of the loop (and the circle-weapon case even returned), so the other
    // projectiles skipped their move/collision in that frame: the loop now goes on with the next one
    it = projectile_list.begin();
    while (it != projectile_list.end()) {
        if ((*it).is_finished == true) {
            it = projectile_list.erase(it);
            continue;
        }
        st_float_size moved = (*it).move();

        /// @TODO projectiles that are tele-guided
        if ((*it).get_move_type() == TRAJECTORY_QUAKE) {
            damage_ground_npcs();
            ++it;
            continue;
        }
        (*it).draw();
        if ((*it).is_reflected == true) {
            ++it;
            continue;
        }

        // check collision against enemies
        bool projectile_erased = false;
        for (unsigned int i=0; i<gameControl.get_current_map_obj()->_npc_list.size(); i++) {
            if ((*it).is_finished == true) {
                it = projectile_list.erase(it);
                projectile_erased = true;
                break;
            }
            if (gameControl.get_current_map_obj()->_npc_list.at(i).is_on_visible_screen() == false) {
                continue;
            }
            if (gameControl.get_current_map_obj()->_npc_list.at(i).is_dead() == true) {
                continue;
            }
            if (gameControl.get_current_map_obj()->_npc_list.at(i).is_player_friend() == true) {
                continue;
            }

            // collision against whole body
            st_rectangle npc_hitbox = gameControl.get_current_map_obj()->_npc_list.at(i).get_vulnerable_area();

            if ((*it).check_collision(npc_hitbox, st_position(moved.width, moved.height)) == true) {
                // shielded NPC: reflects/finishes shot
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_intangible() == true) {
                    (*it).consume_projectile();
                    continue;
                } else if (gameControl.get_current_map_obj()->_npc_list.at(i).is_shielded((*it).get_direction()) == true && (*it).get_trajectory() != TRAJECTORY_BOMB && (*it).get_trajectory() != TRAJECTORY_LIGHTING && (*it).get_trajectory() != TRAJECTORY_SLASH && (*it).get_vanishes_on_hit() == true) {
                    if ((*it).get_trajectory() == TRAJECTORY_CHAIN) {
                        (*it).consume_projectile();
                    } else {
                        (*it).reflect();    // SHIELD reflect
                    }
                    continue;
                }
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_invisible() == true) { // invisible NPC -> ignore shot
                    continue;
                }
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_teleporting() == true) { // executing AI-action TELEPORT
                    continue;
                }

                // check if have hit area, and if hit it
                st_rectangle npc_vulnerable_area = gameControl.get_current_map_obj()->_npc_list.at(i).get_vulnerable_area();

                if (npc_vulnerable_area.is_empty() == false && npc_vulnerable_area != npc_hitbox && (*it).check_collision(npc_vulnerable_area, st_position(moved.width, moved.height)) == false) { // hit body, but not the hit area -> reflect
                    (*it).reflect();        // HITAREA reflect
                    continue;
                }

                short wpn_id = (*it).get_weapon_id();

                if (wpn_id < 0) {
                    wpn_id = 0;
                }

                // NPC using cicrcle weapon, is only be destroyed by CHAIN, but NPC won't take damage
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_using_circle_weapon() == true) {
                    if ((*it).get_trajectory() == TRAJECTORY_CHAIN) {
                        gameControl.get_current_map_obj()->_npc_list.at(i).consume_projectile();
                    }
                    (*it).consume_projectile();
                    continue; // was "return", which skipped every other projectile (and the queued ones)
                }

                if ((*it).get_damage() > 0) {
                    int multiplier = GameMediator::get_instance()->get_enemy(gameControl.get_current_map_obj()->_npc_list.at(i).get_number())->weakness[wpn_id].damage_multiplier;

                    if (multiplier <= 0) { // immune to this weapon: the shot is used up instead of flying through the enemy
                        soundManager.play_sfx(SFX_SHOT_REFLECTED); // the "no effect" sound (same as a reflected shot)
                        (*it).consume_projectile();
                        continue;
                    }
                    gameControl.get_current_map_obj()->_npc_list.at(i).damage((*it).get_damage() * multiplier, ignore_hit_timer);
                }
                if ((*it).get_damage() > 0) {
                    if ((*it).get_vanishes_on_hit() == true) {
                        (*it).consume_projectile();
                    }
                    soundManager.play_sfx(SFX_NPC_HIT);
                }
            }
        }


        if (projectile_erased) { // "it" already points to the next projectile
            continue;
        }

        // check collision against objects

        // if projectile is a bomb, check collision against objects
        std::vector<object*> res_obj = gameControl.get_current_map_obj()->check_collision_with_objects((*it).get_area());
        if (res_obj.size() > 0) {
            for (unsigned int i=0; i<res_obj.size(); i++) {
                object* temp_obj = res_obj.at(i);
                if (object_is_affected_by_projectile(temp_obj, (*it))) {
                    if (temp_obj->get_type() == OBJ_DESTRUCTIBLE_NO_DROP) {
                        temp_obj->set_hidden(true);
                    } else if (temp_obj->get_type() == OBJ_JUMP_SHOOT_DESTRUCTIBLE_NO_DROP) {
                        temp_obj->inc_status();
                        continue;
                    } else {
                        temp_obj->set_finished(true);
                    }
                    if ((*it).is_explosive() == true) {
                        (*it).transform_into_explosion();
                    } else {
                        st_float_position obj_pos(temp_obj->get_position().x, temp_obj->get_position().y);
                        gameControl.get_current_map_obj()->add_animation(ANIMATION_STATIC, &graphLib.explosion16, obj_pos, st_position(0, 0), 80, 2, ANIM_DIRECTION_RIGHT, st_size(16, 16));
                    }
                    if (temp_obj->get_type() == OBJ_DESTRUCTIBLE_WITH_DROP) {
                        st_position drop_pos(temp_obj->get_position().x+temp_obj->get_size().width/2, (temp_obj->get_position().y));
                        gameControl.get_current_map_obj()->drop_random_item(drop_pos);
                    }
                }
            }
        }
        ++it;
    }
    projectile_list.insert(projectile_list.end(), projectile_to_be_added_list.begin(), projectile_to_be_added_list.end());
    projectile_to_be_added_list.clear();
}

bool classPlayer::object_is_affected_by_projectile(object *temp_obj, projectile &projectile)
{
    if (projectile.get_effect_n() == 1 && (projectile.get_move_type() == TRAJECTORY_BOMB || projectile.get_move_type() == TRAJECTORY_FALL_BOMB) || projectile.is_explosive() == true && temp_obj->get_type() == OBJ_DESTRUCTIBLE_WALL) {
        return true;
    }
    if (temp_obj->get_type() == OBJ_DESTRUCTIBLE_NO_DROP || temp_obj->get_type() == OBJ_DESTRUCTIBLE_WITH_DROP || temp_obj->get_type() == OBJ_JUMP_SHOOT_DESTRUCTIBLE_NO_DROP) {
        return true;
    }
    return false;
}

void classPlayer::add_coil_object()
{
    if (game_save.items.weapons[selected_weapon] > 0) {
		st_position obj_pos;
        obj_pos.y = position.y;
		if (state.direction() == ANIM_DIRECTION_LEFT) {
            obj_pos.x = position.x - 2;
		} else {
            obj_pos.x = position.x + frameSize.width + 2;
		}


        if (TILESIZE == 0 || !gameControl.get_current_map_obj()) {
            return;
        }
        if (game_data.player_items[0] < 0 || game_data.player_items[0] >= (int)GameMediator::get_instance()->object_list.size()) {
            return;
        }

        object temp_obj(game_data.player_items[0], gameControl.get_current_map_obj(), st_position(position.x/TILESIZE, position.y/TILESIZE), st_position(-1, -1), -1, state.direction());

        int first_unlocked_from_bottom = gameControl.get_current_map_obj()->get_first_lock_on_bottom(obj_pos.x, getPosition().y+frameSize.height+4, temp_obj.get_size().width, temp_obj.get_size().height);

        obj_pos.y = (first_unlocked_from_bottom+1)*TILESIZE - temp_obj.get_size().height;

        temp_obj.set_precise_position(obj_pos, state.direction());
		temp_obj.set_duration(2500);
        temp_obj.enable_teleport_animation();
        temp_obj.set_collision_mode(COLlISION_MODE_Y);
        gameControl.get_current_map_obj()->add_object(temp_obj);
        consume_weapon(1);
    }
}

void classPlayer::add_jet_object()
{
    if (game_save.items.weapons[selected_weapon] > 0) {
		st_position obj_pos;
        obj_pos.y = position.y + TILESIZE;
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            obj_pos.x = position.x - 2;
        } else {
            obj_pos.x = position.x + frameSize.width + 2;
        }
        
        if (TILESIZE == 0 || !gameControl.get_current_map_obj()) {
            return;
        }
        if (game_data.player_items[1] < 0 || game_data.player_items[1] >= (int)GameMediator::get_instance()->object_list.size()) {
            return;
        }
        
        object temp_obj(game_data.player_items[1], gameControl.get_current_map_obj(), st_position(position.x/TILESIZE, position.y/TILESIZE), st_position(-1, -1), -1, state.direction());
        temp_obj.set_precise_position(obj_pos, state.direction());
        temp_obj.enable_teleport_animation();
		temp_obj.set_duration(4500);
        gameControl.get_current_map_obj()->add_object(temp_obj);
	}
}

void classPlayer::reset_charging_shot()
{
    if (selected_weapon != WEAPON_DEFAULT) { // only do this, if using normal weapon
        return;
    }
    state.attack_timer = 0;
    attack_button_released = true;
    soundManager.stop_repeated_sfx();

    change_player_color(true);

    /*
    if (color_keys[0].r != -1) {
        change_char_color(0, color_keys[0], true);
    }
    if (color_keys[1].r != -1) {
        change_char_color(1, color_keys[1], true);
    }
    */
    // also reset slide/dash
    if (state.animation_type() == ANIM_TYPE_SLIDE) {
        set_animation_type(ANIM_TYPE_WALK);
        state.slide_distance = 0;
    }
}
