#include <sstream>
#include <list>
#include <math.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include "classplayer.h"
#include "inputlib.h"
#include "game_mediator.h"

extern inputLib input;
#include "class_config.h"

#include "soundlib.h"
extern soundLib soundManager;

#include "game.h"
extern game gameControl;


#define PLAYER_MOVE_SPEED 1.25 // higher is faster
#include "file/file_io.h"
#include "classmap.h"


extern std::string FILEPATH;
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern CURRENT_FILE_FORMAT::file_io fio;
extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;
extern CURRENT_FILE_FORMAT::st_game_config game_config;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classPlayer::classPlayer(int playerNumber) : teleporter_n(-1), selected_weapon(WEAPON_DEFAULT), l_key_released(true), r_key_released(true)
{

    std::cout << "### PLAYER::CREATE::number[" << _number << "]" << std::endl;

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
    move_speed = PLAYER_MOVE_SPEED;

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

    //std::cout << "player.number[" << _number << "]" << std::endl;
    _charged_shot_projectile_id = GameMediator::get_instance()->player_list_v3_1[_number].full_charged_projectile_id;
    _normal_shot_projectile_id = GameMediator::get_instance()->player_list_v3_1[_number].normal_shot_projectile_id;

    std::cout << "classPlayer::initialize - player[" << _number << "][" << name << "], _normal_shot_projectile_id[" << _normal_shot_projectile_id << "]" << std::endl;

    _simultaneous_shots = GameMediator::get_instance()->player_list_v3_1[_number].simultaneous_shots;
    //std::cout << "classjump::set_acceleration - player[" << name << "], accel[" << GameMediator::get_instance()->player_list[_number].jump_gravity << "]" << std::endl;
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
		//std::cout << "classPlayer::get_item" << std::endl;
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
            game_save.items.weapon_tanks = 1; // max 1
			obj_info._object->set_finished(true);
			soundManager.play_sfx(SFX_GOT_ITEM);
            res = true;
			break;
        case OBJ_SPECIAL_TANK:
            game_save.items.special_tanks = 1; // max 1
            obj_info._object->set_finished(true);
            soundManager.play_sfx(SFX_GOT_ITEM);
            res = true;
            break;
        case OBJ_LIFE:
            game_save.items.lifes++;
            if (game_save.items.lifes > 9) {
                game_save.items.lifes = 9;
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
        default:
			//std::cout << "classPlayer::get_item - unknown item type: " << obj_info._object->get_type() << std::endl;
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


bool classPlayer::shoryuken()
{
    // trying to start command (can only start if standing)

    //std::cout << ">> SHOURYUKEN - up: " << moveCommands.up << ", dash: " << moveCommands.dash << ", jump: " << moveCommands.jump << std::endl;

    if (moveCommands.up != 0 && moveCommands.dash != 0 && state.animation_type == ANIM_TYPE_STAND)  {
        state.animation_type = ANIM_TYPE_SPECIAL_ATTACK;
        std::cout << ">>>>>>>>>>>>>>>>>>>> SHORYUKEN::START" << std::endl;
        soundManager.play_sfx(SFX_SHORYUKEN_GIRL);
        _obj_jump.start(true, TERRAIN_UNBLOCKED);
        return true;
    // is executing
    } else if (state.animation_type == ANIM_TYPE_SPECIAL_ATTACK) {
        std::cout << ">>>>>>>>>>>>>>>>>>>> SHORYUKEN::EXECUTE" << std::endl;
        _obj_jump.execute(TERRAIN_UNBLOCKED);
        int jump_speed = _obj_jump.get_speed();
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
            //std::cout << "jump::check_collision - i[" << i << "], map_lock["  << map_lock << "]" << std::endl;

            if (map_lock == BLOCK_UNBLOCKED || map_lock == BLOCK_WATER) {
                //std::cout << "jump.speed[" << speed_y << "]" << std::endl;
                position.y += speed_y;
                jump_moved = true;
                break;
            }
        }
        if (jump_speed != 0 && jump_moved == false) {
            //std::cout << "chat::jump - must interrupt because a collision happened" << std::endl;
            if (jump_speed < 0) {
                _obj_jump.interrupt();
            } else {
                _obj_jump.finish();
                state.animation_type = ANIM_TYPE_STAND;
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

        if (game_config.auto_charge_mode) {
            if (moveCommands.attack == 1) {
                std::cout << "auto-charge-attack" << std::endl;
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
        std::cout << "invalid weapon" << std::endl;
        return;
    }


    if (moveCommands.attack == 0 && attack_button_released == false) {
        //std::cout << ">>>>>>>>> attack_button_released[TRUE] #2 <<<<<<<<<<<<<" << std::endl;
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
        //std::cout << "########## attack_button_released[FALSE] #1 ##########" << std::endl;
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
        } else if (used_weapon == WEAPON_ITEM_JET) {
            if (gameControl.get_current_map_obj()->have_player_object() == true) {
                weapon_id = -1;
            } else {
                add_jet_object();
            }
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
            std::cout << "#### PLAYER::ATTACK - can't shot, weapon number reached ###" << std::endl;
            return;
        }


        //soundManager.play_sfx(SFX_PLAYER_SHOT);


        //std::cout << "############ weapon_id: " << weapon_id << std::endl;

        projectile_list.push_back(projectile(game_data.weapons[weapon_id].id_projectile, state.direction, get_attack_position(), is_player()));
        projectile &temp_proj = projectile_list.back();
        temp_proj.play_sfx(false);
        temp_proj.set_is_permanent();
        temp_proj.set_weapon_id(weapon_id);
        temp_proj.set_owner(this);


        //std::cout << "weapon_id: " << weapon_id << ", projectile_id: " << game_data.weapons[weapon_id].id_projectile << std::endl;

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
                //std::cout << "PLAYER::attack - could not find target" << std::endl;
                temp_proj.set_target_position(temp->get_position_ref());
            }
        } else if (weapon_trajectory == TRAJECTORY_TARGET_DIRECTION || weapon_trajectory == TRAJECTORY_TARGET_EXACT || weapon_trajectory == TRAJECTORY_ARC_TO_TARGET) {
            st_rectangle hitbox = get_hitbox();
            st_position player_pos(hitbox.x+hitbox.w/2, hitbox.y+hitbox.h/2);
            classnpc* temp = gameControl.get_current_map_obj()->find_nearest_npc_on_direction(player_pos, state.direction);
            if (temp != NULL) {
                //std::cout << "PLAYER::attack - could not find target" << std::endl;
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

        //std::cout << "Added projectile - id: " << game_data.weapons[weapon_id].id_projectile << std::endl;

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
		std::cout << "damage_ground_npcs - could not find projectile of QUAKE trajectory type" << std::endl;
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
		std::cout << "damage_ground_npcs - could not find weapon with projectile of id '" << weapon_n << "'" << std::endl;
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
	frameSize.width = 29;
    frameSize.height = 29;


    add_graphic();
    init_weapon_colors();

	graphicsLib_gSurface playerSpriteSurface;
	std::stringstream filename;
    filename << FILEPATH + "images/sprites/p" << (_number+1) << ".png";
    //filename << FILEPATH << "images/sprites/" << GameMediator::get_instance()->player_list[_number].graphic_filename;
    //playerSpriteSurface.show_debug = true;
	graphLib.surfaceFromFile(filename.str(), &playerSpriteSurface);
    if (playerSpriteSurface.get_surface() == NULL) {
		std::cout << "initFrames - Error loading player surface from file\n";
		return;
	}

    //animation_obj.init(name, filename, frameSize, GameMediator::get_instance()->player_list_v3_1.at(0).sprites);


    // @TODO - automatically add inverse direction (right) sprites

	// STAND
    addSpriteFrame(ANIM_TYPE_STAND, 3, playerSpriteSurface, 5000);
    addSpriteFrame(ANIM_TYPE_STAND, 4, playerSpriteSurface, 150);
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
    if (have_shoryuken() == true && shoryuken() == true) { // while doing shoryuken won't move
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

        //std::cout << "projectile.move_type: " << (*it)->get_move_type() << std::endl;

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
            std::cout << "### #1 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].hitbox[" << npc_hitbox.x << "," << npc_hitbox.y << "," << npc_hitbox.w << "," << npc_hitbox.h << "]" << std::endl;

            //classnpc* enemy = (*enemy_it);
            if ((*it).check_collision(npc_hitbox, st_position(moved.width, moved.height)) == true) {

                std::cout << "### #2 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].hit[TRUE]" << std::endl;

                // shielded NPC: reflects/finishes shot
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_intangible() == true) {
                    std::cout << "### #3 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].intangible[TRUE]" << std::endl;
                    continue;
                } else if (gameControl.get_current_map_obj()->_npc_list.at(i).is_shielded((*it).get_direction()) == true && (*it).get_trajectory() != TRAJECTORY_BOMB && (*it).get_trajectory() != TRAJECTORY_LIGHTING && (*it).get_trajectory() != TRAJECTORY_SLASH && (*it).get_vanishes_on_hit() == true) {
                    std::cout << "### #4 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].shielded[TRUE]" << std::endl;
                    if ((*it).get_trajectory() == TRAJECTORY_CHAIN) {
                        (*it).consume_projectile();
                    } else {
                        (*it).reflect();    // SHIELD reflect
                    }
                    continue;
                }
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_invisible() == true) { // invisible NPC -> ignore shot
                    std::cout << "### #5 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].invisible[TRUE]" << std::endl;
                    continue;
                }
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_teleporting() == true) { // executing AI-action TELEPORT
                    std::cout << "### #6 - enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].teleporting[TRUE]" << std::endl;
                    continue;
                }

                // check if have hit area, and if hit it
                st_rectangle npc_vulnerable_area = gameControl.get_current_map_obj()->_npc_list.at(i).get_vulnerable_area();

                std::cout << "### enemy[" << gameControl.get_current_map_obj()->_npc_list.at(i).get_name() << "].vulnerable_area[" << npc_vulnerable_area.x << "," << npc_vulnerable_area.y << "," << npc_vulnerable_area.w << "," << npc_vulnerable_area.h << "]" << std::endl;

                int temp_x = Sint16(npc_vulnerable_area.x-gameControl.get_current_map_obj()->getMapScrolling().x);

                if (npc_vulnerable_area.is_empty() == false && npc_vulnerable_area != npc_hitbox && (*it).check_collision(npc_vulnerable_area, st_position(moved.width, moved.height)) == false) { // hit body, but not the hit area -> reflect

                    std::cout << "### MISS-ENEMY VULNERABLE-AREA - projectile.x[" << (*it).get_position().x << "], enemy.pos.x[" << gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().x << "], enemy.pos.y[" << gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().y << "]"  << std::endl;
                    std::cout << "### npc_vulnerable_area x[" << npc_vulnerable_area.x << "], y[" << npc_vulnerable_area.y << "], w[" << npc_vulnerable_area.w << "], h[" << npc_vulnerable_area.h << "]" << std::endl;
                    std::cout << "### npc_hitbox x[" << npc_hitbox.x << "], y[" << npc_hitbox.y << "], w[" << npc_hitbox.w << "], h[" << npc_hitbox.h << "]" << std::endl;

                    (*it).reflect();        // HITAREA reflect
                    continue;
                } else {
                    std::cout << "### HIT-ENEMY VULNERABLE-AREA - enemy.pos.x[" << gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().x << "], enemy.pos.y[" << gameControl.get_current_map_obj()->_npc_list.at(i).getPosition().y << "]";
                }

                short wpn_id = (*it).get_weapon_id();

                if (wpn_id < 0) {
                    wpn_id = 0;
                }

                // NPC using cicrcle weapon, is only be destroyed by CHAIN, but NPC won't take damage
                if (gameControl.get_current_map_obj()->_npc_list.at(i).is_using_circle_weapon() == true) {
                    if ((*it).get_trajectory() == TRAJECTORY_CHAIN) {
                        std::cout << "PROJ::END #3" << std::endl;
                        gameControl.get_current_map_obj()->_npc_list.at(i).consume_projectile();
                    }
                    std::cout << "PROJ::END #4" << std::endl;
                    (*it).consume_projectile();
                    return;
                }

                if ((*it).get_damage() > 0) {
                    int multiplier = GameMediator::get_instance()->get_enemy(gameControl.get_current_map_obj()->_npc_list.at(i).get_number())->weakness[wpn_id].damage_multiplier;
                    if (multiplier <= 0) {
                        multiplier = 1;
                    }

                    std::cout << ">>>>>> weapon multiplier[" << multiplier << "], damage[" << (int)(*it).get_damage() << "]" << std::endl;

                    gameControl.get_current_map_obj()->_npc_list.at(i).damage((*it).get_damage() * multiplier, ignore_hit_timer);
                } else {
                    std::cout << "PLAYER::EXECUTE_PROJ - projectile damage is zero" << std::endl;
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
            //std::cout << "PLAYER::execute_projectiles - Have exploding bomb, checking objects that collide..." << std::endl;
            std::vector<object*> res_obj = gameControl.get_current_map_obj()->check_collision_with_objects((*it).get_area());
            if (res_obj.size() > 0) {
                //std::cout << "PLAYER::execute_projectiles - Found objects (" << res_obj.size() << ") that collides with bomb!" << std::endl;
                for (unsigned int i=0; i<res_obj.size(); i++) {
                    object* temp_obj = res_obj.at(i);
                    //std::cout << "PLAYER::execute_projectiles - OBJ[" << temp_obj->get_name() << "].type: " << temp_obj->get_type() << ", OBJ_DESTRUCTIBLE_WALL: " << OBJ_DESTRUCTIBLE_WALL << std::endl;
                    if (temp_obj->get_type() == OBJ_DESTRUCTIBLE_WALL) {
                        //std::cout << "PLAYER::execute_projectiles - Found destructible block!!!!" << std::endl;
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
        //std::cout << ">>> moveCommands.attack::RESET #1" << std::endl;
		moveCommands.attack = 0;
	}
	if (input.p1_input[BTN_SHIELD] == 1) {
		moveCommands.shield = 1;
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
            set_weapon((WEAPON_ICONS_ENUM)selected_weapon_c, true);
        }
		l_key_released = false;
        //std::cout << ">>> LBUTTON - selected_weapon: " << selected_weapon << ", selected_weapon_c: " << selected_weapon_c << ", WEAPON_COUNT: " << WEAPON_COUNT << std::endl;
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
            set_weapon((WEAPON_ICONS_ENUM)selected_weapon_c, true);
        }
		r_key_released = false;
        //std::cout << ">>> RBUTTON - selected_weapon: " << selected_weapon << ", selected_weapon_c: " << selected_weapon_c << ", WEAPON_COUNT: " << WEAPON_COUNT << std::endl;
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
		//std::cout << ">>> PLAYER SEND COMMAND FOR " << _platform->get_name() << ", type: " << _platform->get_type() << std::endl;
		if (moveCommands.up == 1) {
			_platform->command_up();
		}
		if (moveCommands.down == 1) {
			_platform->command_down();
		}
	}

    if (GameMediator::get_instance()->player_list_v3_1[_number].have_shield == true && moveCommands.up == 0 && moveCommands.down == 0 && moveCommands.left == 0 && moveCommands.right == 0 && moveCommands.jump == 0 && moveCommands.shield == 1) {
		if (state.animation_type != ANIM_TYPE_SHIELD) {
			std::cout << "playerClass::initShield CHANGE anim_type: " << state.animation_type << " to " << ANIM_TYPE_SHIELD << std::endl;
            set_animation_type(ANIM_TYPE_SHIELD);
			state.animation_timer = 0;
			state.animation_state = 0;
		}
		return;
	} else if (state.animation_type == ANIM_TYPE_SHIELD) {
        //std::cout << "playerClass::initShield REMOVE shield" << std::endl;
        if (is_player()) std::cout << "********* reset to STAND #15 **********" << std::endl;
        set_animation_type(ANIM_TYPE_STAND);
	}
    execute_projectiles();
}


void classPlayer::set_teleporter(int n)
{
	//std::cout << "******** classPlayer::set_teleporter - current: " << teleporter_n << ", new: " << n << std::endl;
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

    //std::cout << "PLAYER::death, x: " << position.x << std::endl;
    gameControl.get_current_map_obj()->print_objects_number();
    reset_charging_shot();
    gameControl.get_current_map_obj()->clear_animations();
    gameControl.get_current_map_obj()->print_objects_number();
    gameControl.get_current_map_obj()->reset_objects();
    gameControl.get_current_map_obj()->print_objects_number();
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
	state.direction = ANIM_DIRECTION_RIGHT;
    gameControl.remove_current_teleporter_from_list();
    draw_lib.add_fade_out_effect(171, 0, 19);
    gameControl.draw_explosion(realPosition.x, realPosition.y, false);
    draw_lib.remove_fade_out_effect();
    if (game_save.items.lifes == 0) {
        game_save.items.lifes = 3;
        std::cout << "GAME OVER" << std::endl;
        gameControl.game_over();
        return;
    }
    game_save.items.lifes--;
    std::cout << "PLAYER::DEATH::DONE" << std::endl;
}

void classPlayer::reset_hp()
{
	hitPoints.current = hitPoints.total;
}

void classPlayer::change_player_color(bool full_change)
{
    //std::cout << "PLAYER::change_player_color - selected_weapon: " << selected_weapon << std::endl;
	if (full_change == false) {
        graphLib.change_surface_color(0, weapon_colors[selected_weapon].color1, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
        graphLib.change_surface_color(1, weapon_colors[selected_weapon].color2, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
        graphLib.change_surface_color(2, weapon_colors[selected_weapon].color3, &(graphLib.character_graphics_list.find(name)->second).frames[state.direction][state.animation_type][state.animation_state].frameSurface);
	} else {
        for (int i=0; i<CHAR_ANIM_DIRECTION_COUNT; i++) {
			for (int j=0; j<ANIM_TYPE_COUNT; j++) {
				for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
					if (weapon_colors[selected_weapon].color1.r != -1) {
                        graphLib.change_surface_color(0, weapon_colors[selected_weapon].color1, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
					}
					if (weapon_colors[selected_weapon].color2.r != -1) {
                        graphLib.change_surface_color(1, weapon_colors[selected_weapon].color2, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
					}
					if (weapon_colors[selected_weapon].color3.r != -1) {
                        graphLib.change_surface_color(2, weapon_colors[selected_weapon].color3, &(graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface);
					}
				}
			}
		}
	}
}

void classPlayer::save_input()
{
    saved_move_commands = moveCommands;
    //std::cout << "PLAYER::save_input::ATTACK: " << moveCommands.attack << ", BTN-ATTACK: " << (int)saved_input[BTN_ATTACK] << std::endl;
}

void classPlayer::restore_input()
{
    //std::cout << "PLAYER::restore_input::OLD-ATTACK: " << saved_move_commands.attack << std::endl;
    moveCommands = saved_move_commands;
    //std::cout << "PLAYER::restore_input::ATTACK: " << moveCommands.attack << ", BTN-ATTACK: " << (int)input.p1_input[BTN_ATTACK] << std::endl;
}

void classPlayer::set_weapon(short weapon_n, bool show_tooltip_icon)
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
    //std::cout << "PLAYER::set_teleport_minimal_y[" << y << "]" << std::endl;
    _teleport_minimal_y = y-2;
}

bool classPlayer::can_fly()
{
    return false;
}


void classPlayer::add_coil_object()
{
    if (game_save.items.weapons[selected_weapon] > 0) {
        //std::cout << ">>>>>>> adding coil object" << std::endl;
		st_position obj_pos;
        obj_pos.y = position.y;
		if (state.direction == ANIM_DIRECTION_LEFT) {
            obj_pos.x = position.x - 2;
		} else {
            obj_pos.x = position.x + frameSize.width + 2;
		}


        object temp_obj(game_data.player_items[0], gameControl.get_current_map_obj(), st_position(position.x/TILESIZE, position.y/TILESIZE), st_position(-1, -1), -1);

        int first_unlocked_from_bottom = gameControl.get_current_map_obj()->get_first_lock_on_bottom(obj_pos.x, getPosition().y+get_size().height, temp_obj.get_size().width, temp_obj.get_size().height);
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
        //std::cout << ">>>>>>> adding JET object" << std::endl;
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
    //std::cout << "PLAYER::find_next_weapon - res: " << res << std::endl;
    return res;
}



void classPlayer::clean_move_commands()
{
    std::cout << ">>> moveCommands.attack::RESET #2" << std::endl;
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
        std::cout << "HARD-MODE, damage--[" << damage_points << "]" << std::endl;
    }
    int new_damage_points = damage_points;
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_HALFDAMAGE) {
        new_damage_points = damage_points/2;
        if (damage_points > 0 && new_damage_points <= 0) {
            new_damage_points = 1;
        }
        //std::cout << "ARMOR-HALF-DAMAGE, damage[" << damage_points << "], new_damage_points[" << new_damage_points << "]" << std::endl;
        character::damage(new_damage_points, ignore_hit_timer);
        return;
    }
    character::damage(damage_points, ignore_hit_timer);
}

void classPlayer::damage_spikes(bool ignore_hit_timer)
{
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_SPIKESIMMMUNE) {
        std::cout << "################## SPIKES Immunity" << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "####### SPIKES Immunity #######");
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
    //std::cout << ">>>>>>>>> attack_button_released[TRUE] #3 <<<<<<<<<<<<<" << std::endl;
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


