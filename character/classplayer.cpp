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

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classPlayer::classPlayer(int playerNumber) : teleporter_n(-1), selected_weapon(WEAPON_DEFAULT), l_key_released(true), r_key_released(true)
{

    _number = playerNumber;
    if (_number == 3 || _number == 0) {
        //_obj_jump.set_jump_acceleration(0.95);
        _obj_jump.set_jump_limit(50);
    }
    position.y = -TILESIZE;
	position.x = 80;
    hit_duration = 2000;
    hitPoints.total = PLAYER_INITIAL_HP;
	hitPoints.current = hitPoints.total;
	shield_type = SHIELD_FRONT; /// @TODO: from editor
	// load items from save
    selected_weapon = 0;
    move_speed = PLAYER_MOVE_SPEED * SharedData::get_instance()->get_movement_multiplier();

    reset_charging_shot();
}

void classPlayer::set_player_name(std::string set_name)
{
    name = set_name;
}

void classPlayer::initialize()
{
    _number = game_save.selected_player;
    char temp_name[30];
    sprintf(temp_name, "PLAYER_%d", _number);
    name = std::string(temp_name);
    if (_number == 3 || _number == 0) {
        //_obj_jump.set_jump_acceleration(0.95);
        _obj_jump.set_jump_limit(50);
    }

    max_projectiles = GameMediator::get_instance()->player_list_v3_1[_number].max_shots;
    // it is a player, can't have zero projectiles!!
    if (max_projectiles < 1) {
        max_projectiles = 1;
    }
    if (GameMediator::get_instance()->player_list_v3_1[_number].can_slide == true) {
        slide_type = 1;
    }

    _charged_shot_projectile_id = GameMediator::get_instance()->player_list_v3_1[_number].full_charged_projectile_id;
    _normal_shot_projectile_id = GameMediator::get_instance()->player_list_v3_1[_number].normal_shot_projectile_id;
    _simultaneous_shots = GameMediator::get_instance()->player_list_v3_1[_number].simultaneous_shots;

    if (can_double_jump() == true) {
        _jumps_number = 2;
    } else {
        _jumps_number = 1;
    }
    _damage_modifier = GameMediator::get_instance()->player_list_v3_1[_number].damage_modifier;
    update_armor_properties();
}


void classPlayer::init_weapon_colors()
{
    color_keys[0] = st_color(COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
    color_keys[1] = st_color(COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
    color_keys[2] = st_color(COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

    for (int i=0; i<MAX_WEAPON_N; i++) {
        weapon_colors[i] = GameMediator::get_instance()->player_list_v3_1[_number].weapon_colors[i];
	}
}

bool classPlayer::get_item(object_collision &obj_info)
{
    if (state.animation_type == ANIM_TYPE_TELEPORT) {
        return false;
    }
	if (character::get_item(obj_info)) {
		return true;
	}

	bool res = false;
	// deal with non-blocking items
	if (obj_info._object != NULL && obj_info._object->finished() == false) {
		switch (obj_info._object->get_type()) {
		case OBJ_ENERGY_TANK:
            if (game_save.items.energy_tanks < 9) { // max 9
                game_save.items.energy_tanks++;
            }
			obj_info._object->set_finished(true);
			soundManager.play_sfx(SFX_GOT_ITEM);
            res = true;
			break;
		case OBJ_WEAPON_TANK:
            if (game_save.items.weapon_tanks < 9) { // max 9
                game_save.items.weapon_tanks++;
            }
			obj_info._object->set_finished(true);
			soundManager.play_sfx(SFX_GOT_ITEM);
            res = true;
			break;
        case OBJ_SPECIAL_TANK:
            if (game_save.items.special_tanks < 9) { // max 9
                game_save.items.special_tanks++;
            }
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            res = true;
            break;
        case OBJ_WEAPON_PILL_BIG:
			obj_info._object->set_finished(true);
			recharge(ENERGY_TYPE_WEAPON, ENERGY_ITEM_BIG);
            res = true;
			break;
		case OBJ_WEAPON_PILL_SMALL:
			obj_info._object->set_finished(true);
			recharge(ENERGY_TYPE_WEAPON, ENERGY_ITEM_SMALL);
            res = true;
			break;
        case OBJ_ARMOR_ARMS:
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            game_save.armor_pieces[ARMOR_TYPE_ARMS] = true;
            show();
            gameControl.showGotArmorDialog(ARMOR_TYPE_ARMS);
            update_armor_properties();
            res = true;
            break;
        case OBJ_ARMOR_BODY:
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            game_save.armor_pieces[ARMOR_TYPE_BODY] = true;
            show();
            gameControl.showGotArmorDialog(ARMOR_TYPE_BODY);
            update_armor_properties();
            res = true;
            break;
        case OBJ_ARMOR_LEGS:
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            game_save.armor_pieces[ARMOR_TYPE_LEGS] = true;
            show();
            gameControl.showGotArmorDialog(ARMOR_TYPE_LEGS);
            update_armor_properties();
            res = true;
            break;
        case OBJ_LIFE:
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            if (game_save.items.lifes < 9) {
                game_save.items.lifes++; // max 9
            }
        default:
			break;
		}
	}
	return res;
}

void classPlayer::recharge(e_energy_types _en_type, int value)
{
	if (_en_type == ENERGY_TYPE_HP) {
		character::recharge(_en_type, value);
	} else if (_en_type == ENERGY_TYPE_WEAPON) {
        if (game_save.items.weapons[selected_weapon] < PLAYER_INITIAL_HP) {
            if (game_save.items.weapons[selected_weapon] + value <= PLAYER_INITIAL_HP) {
                game_save.items.weapons[selected_weapon] += value;
			} else {
                game_save.items.weapons[selected_weapon] = PLAYER_INITIAL_HP;
			}
			soundManager.play_sfx(SFX_GOT_ENERGY);
			if (value > ENERGY_ITEM_SMALL) {
				soundManager.play_sfx(SFX_GOT_ENERGY);
			}
		}
	}
}


bool classPlayer::shouryuken()
{
    // trying to start command (can only start if standing)
    if (moveCommands.up != 0 && moveCommands.dash != 0 && state.animation_type == ANIM_TYPE_STAND)  {
        set_animation_type(ANIM_TYPE_SPECIAL_ATTACK);
        _can_execute_airdash = false;
        soundManager.play_sfx(SFX_SHORYUKEN_GIRL);
        _obj_jump.start(true, TERRAIN_UNBLOCKED);
        return true;
    // is executing
    } else if (state.animation_type == ANIM_TYPE_SPECIAL_ATTACK) {
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

void classPlayer::consume_weapon(int value)
{
    if (game_save.items.weapons[selected_weapon] - value < 0) {
        game_save.items.weapons[selected_weapon] = 0;
    } else {
        game_save.items.weapons[selected_weapon] -= value;
    }
}

Uint8 classPlayer::get_max_hp()
{
    return fio.get_heart_pieces_number(game_save);
}


void classPlayer::attack(bool dont_update_colors)
{
    UNUSED(dont_update_colors);
    st_position proj_pos;


    if (state.animation_type == ANIM_TYPE_HIT) { // can't fire when hit
        return;
    }

    if (state.animation_type == ANIM_TYPE_SHIELD) { // can't attack when using shield
        return;
    }

    bool always_charged = false;
    // player with armor-special-type changes to auto-carged instead of charging shot
    if (game_save.armor_pieces[ARMOR_TYPE_ARMS] == true && _simultaneous_shots > 1) {
        always_charged = true;
    }

    if (selected_weapon == WEAPON_DEFAULT) {
        /// @NOTE: desabilitei o tiro em diagonal pois vai precisar mudanças no sistema de arquivos para comportar as poses/frames de ataque para cima e para baixo

        if (SharedData::get_instance()->game_config.auto_charge_mode) {
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


        if (state.direction == ANIM_DIRECTION_LEFT) {
            proj_pos = st_position(position.x, position.y+frameSize.height/2);
        } else {
            proj_pos = st_position(position.x+frameSize.width-TILESIZE*2, position.y+frameSize.height/2);
        }

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

        if (weapon_id == 0) { /// @TODO - this is a temporary exit to handle incomplete weapons
            return;
        } else if (weapon_id == -1) {
            weapon_id = 0;
        }

        // check if projectiles limit from weapon/projectile os not reached
        if (projectile_list.size() >= GameMediator::get_instance()->get_projectile(game_data.weapons[weapon_id].id_projectile).max_shots) {
            return;
        }

        projectile_list.push_back(projectile(game_data.weapons[weapon_id].id_projectile, state.direction, get_attack_position(), is_player()));
        projectile &temp_proj = projectile_list.back();
        temp_proj.play_sfx(false);
        temp_proj.set_is_permanent();
        temp_proj.set_weapon_id(weapon_id);
        temp_proj.set_owner(this);


        int weapon_trajectory = GameMediator::get_instance()->get_projectile(game_data.weapons[weapon_id].id_projectile).trajectory;
        if (weapon_trajectory == TRAJECTORY_CENTERED || weapon_trajectory == TRAJECTORY_SLASH) {
            temp_proj.set_owner_direction(&state.direction);
            temp_proj.set_owner_position(&position);
        } else if (weapon_trajectory == TRAJECTORY_CHAIN) {
            temp_proj.set_owner_position(&position);
            temp_proj.set_owner_direction(&state.direction);

        } else if (weapon_trajectory == TRAJECTORY_FOLLOW) {
            st_rectangle hitbox = get_hitbox();
            classnpc* temp = gameControl.get_current_map_obj()->find_nearest_npc(st_position(hitbox.x+hitbox.w/2, hitbox.y+hitbox.h/2));
            if (temp != NULL) {
                temp_proj.set_target_position(temp->get_position_ref());
            }
        } else if (weapon_trajectory == TRAJECTORY_TARGET_DIRECTION || weapon_trajectory == TRAJECTORY_TARGET_EXACT || weapon_trajectory == TRAJECTORY_ARC_TO_TARGET) {
            st_rectangle hitbox = get_hitbox();
            st_position player_pos(hitbox.x+hitbox.w/2, hitbox.y+hitbox.h/2);
            classnpc* temp = gameControl.get_current_map_obj()->find_nearest_npc_on_direction(player_pos, state.direction);
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
        if (state.animation_type == ANIM_TYPE_STAND) {
            set_animation_type(ANIM_TYPE_ATTACK);
        } else if (state.animation_type == ANIM_TYPE_JUMP) {
            set_animation_type(ANIM_TYPE_JUMP_ATTACK);
        } else if (state.animation_type == ANIM_TYPE_STAIRS || state.animation_type == ANIM_TYPE_STAIRS_SEMI || state.animation_type == ANIM_TYPE_STAIRS_MOVE) {
            set_animation_type(ANIM_TYPE_STAIRS_ATTACK);
        } else if (state.animation_type == ANIM_TYPE_WALK) {
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



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classPlayer::initFrames()
{
    frameSize.width = GameMediator::get_instance()->player_list_v3_1[_number].sprite_size.width;
    frameSize.height = GameMediator::get_instance()->player_list_v3_1[_number].sprite_size.height;

    add_graphic();
    init_weapon_colors();

	graphicsLib_gSurface playerSpriteSurface;
	std::stringstream filename;
    filename << FILEPATH + "images/sprites/p" << (_number+1) << ".png";
    //filename << FILEPATH << "images/sprites/" << GameMediator::get_instance()->player_list[_number].graphic_filename;
    //playerSpriteSurface.show_debug = true;
	graphLib.surfaceFromFile(filename.str(), &playerSpriteSurface);
    if (playerSpriteSurface.get_surface() == NULL) {
        std::cout << "ERROR: initFrames - Error loading player surface from file\n";
		return;
	}

    //animation_obj.init(name, filename, frameSize, GameMediator::get_instance()->player_list_v3_1.at(0).sprites);


    // @TODO - automatically add inverse direction (right) sprites

	// STAND
    addSpriteFrame(ANIM_TYPE_STAND, 3, playerSpriteSurface, 2000);
    addSpriteFrame(ANIM_TYPE_STAND, 4, playerSpriteSurface, 100);
	// WALK
    addSpriteFrame(ANIM_TYPE_WALK, 7, playerSpriteSurface, WALK_FRAME_DELAY);
    addSpriteFrame(ANIM_TYPE_WALK, 8, playerSpriteSurface, WALK_FRAME_DELAY);
    addSpriteFrame(ANIM_TYPE_WALK, 7, playerSpriteSurface, WALK_FRAME_DELAY);
    addSpriteFrame(ANIM_TYPE_WALK, 6, playerSpriteSurface, WALK_FRAME_DELAY);

	// JUMP
    addSpriteFrame(ANIM_TYPE_JUMP, 9, playerSpriteSurface, 150);
	// ATTACK
    addSpriteFrame(ANIM_TYPE_ATTACK, 11, playerSpriteSurface, 150);
	// ATTACK + JUMP
    addSpriteFrame(ANIM_TYPE_JUMP_ATTACK, 10, playerSpriteSurface, 80);
	// ATTACK + WALK
    addSpriteFrame(ANIM_TYPE_WALK_ATTACK, 12, playerSpriteSurface, 150);
    addSpriteFrame(ANIM_TYPE_WALK_ATTACK, 13, playerSpriteSurface, 150);
    addSpriteFrame(ANIM_TYPE_WALK_ATTACK, 14, playerSpriteSurface, 150);
	// HIT
    addSpriteFrame(ANIM_TYPE_HIT, 15, playerSpriteSurface, 100);
    addSpriteFrame(ANIM_TYPE_HIT, 16, playerSpriteSurface, 100);
	// TELEPORT
    addSpriteFrame(ANIM_TYPE_TELEPORT, 0, playerSpriteSurface, 200);
    addSpriteFrame(ANIM_TYPE_TELEPORT, 1, playerSpriteSurface, 100);
    addSpriteFrame(ANIM_TYPE_TELEPORT, 2, playerSpriteSurface, 100);
    // STAIRS
    addSpriteFrame(ANIM_TYPE_STAIRS, 17, playerSpriteSurface, 5000);
	// stairs semi
    addSpriteFrame(ANIM_TYPE_STAIRS_SEMI, 18, playerSpriteSurface, 5000);
    addSpriteFrame(ANIM_TYPE_STAIRS_SEMI, 18, playerSpriteSurface, 5000);
    addSpriteFrame(ANIM_TYPE_STAIRS, 17, playerSpriteSurface, 5000);
	// stairs + move
    addSpriteFrame(ANIM_TYPE_STAIRS_MOVE, 17, playerSpriteSurface, 200);
    // stairs + attack
    addSpriteFrame(ANIM_TYPE_STAIRS_ATTACK, 19, playerSpriteSurface, 500);

    // slide
    addSpriteFrame(ANIM_TYPE_SLIDE, 20, playerSpriteSurface, 1000);

    // throw
    //addSpriteFrame(ANIM_TYPE_THROW, 21, 0, playerSpriteSurface, 1000);


	// shield
    addSpriteFrame(ANIM_TYPE_SHIELD, 22, playerSpriteSurface, 100);

    // shoot-diagonal-up
    addSpriteFrame(ANIM_TYPE_ATTACK_DIAGONAL_UP, 23, playerSpriteSurface, 100);

    // shoot-diagonal-down
    addSpriteFrame(ANIM_TYPE_ATTACK_DIAGONAL_DOWN, 24, playerSpriteSurface, 100);

    // comemoration/got-weapon
    addSpriteFrame(ANIM_TYPE_GOT_WEAPON, 26, playerSpriteSurface, 200);
    addSpriteFrame(ANIM_TYPE_GOT_WEAPON, 27, playerSpriteSurface, 200);

    addSpriteFrame(ANIM_TYPE_SPECIAL_ATTACK, 29, playerSpriteSurface, 200);


    playerSpriteSurface.freeGraphic();

    change_player_color(true);

}


void classPlayer::execute()
{
    if (freeze_weapon_effect != FREEZE_EFFECT_PLAYER || timer.getTimer() < hit_duration+last_hit_time) {
        move();
    } else {
        clear_move_commands();
        input.clean();
    }
    if (have_shoryuken() == true && shouryuken() == true) { // while doing shoryuken won't move
        return;
    }

    charMove();
    attack();


}

void classPlayer::execute_projectiles()
{
    // animate projectiles
    vector<projectile>::iterator it;
    bool ignore_hit_timer = false;
    if (_simultaneous_shots > 1) {
        ignore_hit_timer = true;
    }

    for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        if ((*it).is_finished == true) {
            projectile_list.erase(it);
            break;
        }
        st_size moved = (*it).move();

        /// @TODO projectiles that are tele-guided
        if ((*it).get_move_type() == TRAJECTORY_QUAKE) {
            damage_ground_npcs();
            continue;
        }
        (*it).draw();
        if ((*it).is_reflected == true) {
            continue;
        }

        // check collision against enemies
        for (int i=0; i<gameControl.get_current_map_obj()->_npc_list.size(); i++) {
            if ((*it).is_finished == true) {
                projectile_list.erase(it);
                break;
            }
            if (gameControl.get_current_map_obj()->_npc_list.at(i).is_on_visible_screen() == false) {
                continue;
            }
            if (gameControl.get_current_map_obj()->_npc_list.at(i).is_dead() == true) {
                continue;
            }


            // collision against whole body
            st_rectangle npc_hitbox = gameControl.get_current_map_obj()->_npc_list.at(i).get_hitbox();

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
                int temp_x = Sint16(npc_vulnerable_area.x-gameControl.get_current_map_obj()->getMapScrolling().x);

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
                    return;
                }

                if ((*it).get_damage() > 0) {
                    int multiplier = GameMediator::get_instance()->get_enemy(gameControl.get_current_map_obj()->_npc_list.at(i).get_number())->weakness[wpn_id].damage_multiplier;

                    if (multiplier <= 0) {
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


        // if projectile is a bomb, check collision against objects
        if ((*it).get_effect_n() == 1 && ((*it).get_move_type() == TRAJECTORY_BOMB || (*it).get_move_type() == TRAJECTORY_FALL_BOMB) || (*it).is_explosive() == true) {
            std::vector<object*> res_obj = gameControl.get_current_map_obj()->check_collision_with_objects((*it).get_area());
            if (res_obj.size() > 0) {
                for (unsigned int i=0; i<res_obj.size(); i++) {
                    object* temp_obj = res_obj.at(i);
                    if (temp_obj->get_type() == OBJ_DESTRUCTIBLE_WALL) {
                        temp_obj->set_finished(true);
                        if ((*it).is_explosive() == true) {
                            (*it).transform_into_explosion();
                        }
                    }
                }
            }
        }
    }
    projectile_list.insert(projectile_list.end(), projectile_to_be_added_list.begin(), projectile_to_be_added_list.end());
    projectile_to_be_added_list.clear();
}

void classPlayer::move()
{
	if (input.p1_input[BTN_DOWN] == 1) {
		moveCommands.down = 1;
	} else {
		moveCommands.down = 0;
	}
	if (input.p1_input[BTN_UP] == 1) {
		moveCommands.up = 1;
	} else {
		moveCommands.up = 0;
	}
	if (input.p1_input[BTN_LEFT] == 1) {
		moveCommands.left = 1;
	} else {
		moveCommands.left = 0;
	}
	if (input.p1_input[BTN_RIGHT] == 1) {
		moveCommands.right = 1;
	} else {
		moveCommands.right = 0;
	}
	if (input.p1_input[BTN_JUMP] == 1) {
		moveCommands.jump = 1;
	} else {
		moveCommands.jump = 0;
	}
	if (input.p1_input[BTN_ATTACK] == 1) {
		moveCommands.attack = 1;
	} else {
		moveCommands.attack = 0;
	}
	if (input.p1_input[BTN_SHIELD] == 1) {
		moveCommands.shield = 1;
        moveCommands.left = 0;
        moveCommands.right = 0;
        moveCommands.up = 0;
        moveCommands.down = 0;
	} else {
		moveCommands.shield = 0;
	}
	if (input.p1_input[BTN_DASH] == 1) {
		moveCommands.dash = 1;
	} else {
		moveCommands.dash = 0;
	}

    // players that shoot on diagonal can't move shile attacking
    if (can_shoot_diagonal()) {
        if (is_on_attack_frame()) {
            if (state.animation_type != ANIM_TYPE_JUMP_ATTACK) {
                if (state.animation_type == ANIM_TYPE_WALK_ATTACK) {
                    set_animation_type(ANIM_TYPE_ATTACK);
                }
                moveCommands.left = 0;
                moveCommands.right = 0;
            }
        }
    }

	if (input.p1_input[BTN_L] != 1 && l_key_released == false) {
		l_key_released = true;
	}
	int wpn_max = WEAPON_COUNT;
    //wpn_max--;
	if (input.p1_input[BTN_L] == 1 && l_key_released == true) {
        int selected_weapon_c = find_next_weapon(selected_weapon, -1);
        if (selected_weapon_c != -1) {
            set_weapon(selected_weapon_c, true);
        }
		l_key_released = false;
	}

	if (input.p1_input[BTN_R] != 1 && r_key_released == false) {
		r_key_released = true;
	}
	if (input.p1_input[BTN_R] == 1 && r_key_released == true) {
        int selected_weapon_c = find_next_weapon(selected_weapon, 1);
		if (selected_weapon_c >= wpn_max) {
			selected_weapon_c = WEAPON_DEFAULT;
		}
        if (selected_weapon_c != -1) {
            set_weapon(selected_weapon_c, true);
        }
		r_key_released = false;
	}


	// send commands to the platform in special cases
	if (_platform != NULL) {
		if (_platform->get_type() == OBJ_ITEM_FLY && timer.getTimer() > _platform->get_timer()) {
            consume_weapon(1);
            if (game_save.items.weapons[selected_weapon] == 0) {
                _platform->set_finished(true);
                _platform = NULL;
                return;
            }
            _platform->set_timer(timer.getTimer()+240);
		}
		if (moveCommands.up == 1) {
			_platform->command_up();
		}
		if (moveCommands.down == 1) {
			_platform->command_down();
		}
	}

    if (GameMediator::get_instance()->player_list_v3_1[_number].have_shield == true && moveCommands.up == 0 && moveCommands.down == 0 && moveCommands.left == 0 && moveCommands.right == 0 && moveCommands.jump == 0 && moveCommands.shield == 1) {
		if (state.animation_type != ANIM_TYPE_SHIELD) {
            set_animation_type(ANIM_TYPE_SHIELD);
			state.animation_timer = 0;
			state.animation_state = 0;
		}
		return;
	} else if (state.animation_type == ANIM_TYPE_SHIELD) {
        set_animation_type(ANIM_TYPE_STAND);
	}
    execute_projectiles();
}


void classPlayer::set_teleporter(int n)
{
	teleporter_n = n;
}

int classPlayer::get_teleporter()
{
	return teleporter_n;
}



void classPlayer::death()
{

    soundManager.stop_music();
    soundManager.play_sfx(SFX_PLAYER_DEATH);
    gameControl.draw_player_death(realPosition);

    game_save.items.lifes--;
    if (game_save.items.lifes < 0) {
        reset_lifes();
        gameControl.game_over();
        return;
    }

    reset_charging_shot();
    gameControl.get_current_map_obj()->clear_animations();
    gameControl.get_current_map_obj()->reset_objects();
	dead = true;
    _obj_jump.interrupt();
    _obj_jump.finish();
    freeze_weapon_effect = FREEZE_EFFECT_NONE;

    last_hit_time = 0;

    selected_weapon = 0;
    change_player_color(true);

    set_weapon(WEAPON_DEFAULT, false);
    _inertia_obj.stop();
    clear_move_commands();
	input.clean();
    set_direction(ANIM_DIRECTION_RIGHT);
    gameControl.remove_current_teleporter_from_list();
}

void classPlayer::reset_lifes()
{
    if (game_save.difficulty != DIFFICULTY_EASY) {
        game_save.items.lifes = 9;
    } else if (game_save.difficulty != DIFFICULTY_NORMAL) {
        game_save.items.lifes = 5;
    } else {
        game_save.items.lifes = 3;
    }
}

void classPlayer::reset_hp()
{
	hitPoints.current = hitPoints.total;
}

void classPlayer::change_player_color(bool full_change)
{
    int color_n = selected_weapon;
    if (selected_weapon > WEAPON_ITEM_JET) {
        color_n = 0;
    }
    if (full_change == false) {
        graphLib.change_surface_color(0, weapon_colors[color_n].color1, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
        graphLib.change_surface_color(1, weapon_colors[color_n].color2, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
        graphLib.change_surface_color(2, weapon_colors[color_n].color3, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
    } else {
        for (int i=0; i<CHAR_ANIM_DIRECTION_COUNT; i++) {
            for (int j=0; j<ANIM_TYPE_COUNT; j++) {
                for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    if (weapon_colors[0].color1.r != -1) {
                        graphLib.change_surface_color(0, weapon_colors[color_n].color1, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
                    }
                    if (weapon_colors[0].color2.r != -1) {
                        graphLib.change_surface_color(1, weapon_colors[color_n].color2, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
                    }
                    if (weapon_colors[0].color3.r != -1) {
                        graphLib.change_surface_color(2, weapon_colors[color_n].color3, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
                    }
                }
            }
        }
    }
}

void classPlayer::save_input()
{
    saved_move_commands = moveCommands;
}

void classPlayer::restore_input()
{
    moveCommands = saved_move_commands;
}

void classPlayer::set_weapon(int weapon_n, bool show_tooltip_icon)
{
	selected_weapon = weapon_n;
    clean_projectiles();
    soundManager.stop_repeated_sfx();
    attack_button_last_state = 0;
    if (show_tooltip_icon == true) {
        draw_lib.add_weapon_tooltip(selected_weapon, realPosition, state.direction);
    }
    change_player_color(true);
}

short classPlayer::get_weapon_value(int weapon_n)
{
	if (weapon_n == 0) {
		return hitPoints.current;
	}
    return game_save.items.weapons[weapon_n];
}

void classPlayer::set_weapon_value(Uint8 weapon_n, Uint8 value)
{
    game_save.items.weapons[weapon_n] = value;
}



CURRENT_FILE_FORMAT::file_weapon_colors classPlayer::get_weapon_colors(short int weapon_n)
{
	return weapon_colors[weapon_n];
}

short classPlayer::get_selected_weapon()
{
    return selected_weapon;
}

short classPlayer::get_selected_weapon_value()
{
    return get_weapon_value(get_selected_weapon());
}

void classPlayer::refill_weapons()
{
	for (int i=0; i<WEAPON_COUNT; i++) {
        game_save.items.weapons[i] = PLAYER_INITIAL_HP;
	}
}

void classPlayer::set_teleport_minimal_y(int y)
{
    _teleport_minimal_y = y-2;
}

bool classPlayer::can_fly()
{
    return false;
}


void classPlayer::add_coil_object()
{
    if (game_save.items.weapons[selected_weapon] > 0) {
		st_position obj_pos;
        obj_pos.y = position.y;
		if (state.direction == ANIM_DIRECTION_LEFT) {
            obj_pos.x = position.x - 2;
		} else {
            obj_pos.x = position.x + frameSize.width + 2;
		}


        object temp_obj(game_data.player_items[0], gameControl.get_current_map_obj(), st_position(position.x/TILESIZE, position.y/TILESIZE), st_position(-1, -1), -1);

        int first_unlocked_from_bottom = gameControl.get_current_map_obj()->get_first_lock_on_bottom(obj_pos.x, getPosition().y+frameSize.height+4, temp_obj.get_size().width, temp_obj.get_size().height);

        obj_pos.y = (first_unlocked_from_bottom+1)*TILESIZE - temp_obj.get_size().height;

        temp_obj.set_precise_position(obj_pos, state.direction);
		temp_obj.set_duration(2500);
        temp_obj.enable_teleport_animation();
        temp_obj.set_collision_mode(COLlISION_MODE_Y);
        temp_obj.set_direction(state.direction);
        gameControl.get_current_map_obj()->add_object(temp_obj);
        consume_weapon(1);
    }
}

void classPlayer::add_jet_object()
{
    if (game_save.items.weapons[selected_weapon] > 0) {
		st_position obj_pos;
        obj_pos.y = position.y + TILESIZE;
        if (state.direction == ANIM_DIRECTION_LEFT) {
            obj_pos.x = position.x - 2;
        } else {
            obj_pos.x = position.x + frameSize.width + 2;
        }
        object temp_obj(game_data.player_items[1], gameControl.get_current_map_obj(), st_position(position.x/TILESIZE, position.y/TILESIZE), st_position(-1, -1), -1);
        temp_obj.set_precise_position(obj_pos, state.direction);
        temp_obj.enable_teleport_animation();
		temp_obj.set_duration(4500);
		temp_obj.set_direction(state.direction);
        gameControl.get_current_map_obj()->add_object(temp_obj);
	}
}

int classPlayer::find_next_weapon(int current, int move)
{
    class_config config_manager;
    int res = config_manager.find_next_weapon(current, move);
    return res;
}



void classPlayer::clean_move_commands()
{
    moveCommands.attack = 0;
    moveCommands.dash = 0;
    moveCommands.down = 0;
    moveCommands.jump = 0;
    moveCommands.left = 0;
    moveCommands.right = 0;
    moveCommands.shield = 0;
    moveCommands.start = 0;
    moveCommands.up = 0;
}

bool classPlayer::can_shoot_diagonal()
{
    if (GameMediator::get_instance()->player_list_v3_1[_number].can_shot_diagonal) {
        return true;
    }
    // armor-pieces checking


    return false;
}

bool classPlayer::can_double_jump()
{
    if (GameMediator::get_instance()->player_list_v3_1[_number].can_double_jump) {
        return true;
    }
    // -------------------- armor-pieces checking -------------------- //
    if (game_save.armor_pieces[ARMOR_TYPE_LEGS] == true && game_data.armor_pieces[ARMOR_TYPE_LEGS].special_ability[_number] == ARMOR_ABILITY_LEGS_DOUBLEJUMP) {
        return true;
    }
    return false;
}

bool classPlayer::can_air_dash()
{
    if (GameMediator::get_instance()->player_list_v3_1[_number].can_air_dash == true) {
        return true;
    }

    if (game_save.armor_pieces[ARMOR_TYPE_LEGS] == true && game_data.armor_pieces[ARMOR_TYPE_LEGS].special_ability[_number] == ARMOR_ABILITY_LEGS_AIRDASH) {
        return true;
    }
    return false;
}

void classPlayer::damage(unsigned int damage_points, bool ignore_hit_timer)
{
    if (damage_points > 1 && game_save.difficulty == DIFFICULTY_EASY) {
        damage_points--;
    }

    int new_damage_points = damage_points;
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_HALFDAMAGE) {
        new_damage_points = damage_points/2;
        if (damage_points > 0 && new_damage_points <= 0) {
            new_damage_points = 1;
        }
        character::damage(new_damage_points, ignore_hit_timer);
        return;
    }
    character::damage(damage_points, ignore_hit_timer);
}

void classPlayer::damage_spikes(bool ignore_hit_timer)
{
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_SPIKESIMMMUNE) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "####### SPIKES Immunity #######");
#endif
        return;
    }
    classPlayer::damage(SPIKES_DAMAGE, ignore_hit_timer);
}

float classPlayer::get_hit_push_back_n()
{
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_NOPUSHBACK) {
        return 0;
    } else {
        return character::get_hit_push_back_n();
    }
}

int classPlayer::get_armor_arms_attack_id()
{
    if (game_save.armor_pieces[ARMOR_TYPE_ARMS] == true) {
         return game_data.armor_pieces[ARMOR_TYPE_ARMS].special_ability[_number];
    }
    return -1;
}


bool classPlayer::have_shoryuken()
{
    if (game_save.armor_pieces[ARMOR_TYPE_LEGS] == true && game_data.armor_pieces[ARMOR_TYPE_LEGS].special_ability[_number] == ARMOR_ABILITY_LEGS_SHORYUKEN) {
        return true;
    }
    return false;
}

void classPlayer::update_armor_properties()
{
    if (can_double_jump() == true && _jumps_number == 1) {
        _jumps_number = 2;
    }
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_EXTENDEDIMMUNITY) {
        hit_duration = 4000;
    }
    int armor_attack_id = get_armor_arms_attack_id();
    if (armor_attack_id != -1) {
        _charged_shot_projectile_id = armor_attack_id;
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
    if (state.animation_type == ANIM_TYPE_SLIDE) {
        set_animation_type(ANIM_TYPE_WALK);
        state.slide_distance = 0;
    }
}

bool classPlayer::is_teleporting()
{
    if (state.animation_type == ANIM_TYPE_TELEPORT) {
        return true;
    }
    return false;
}


