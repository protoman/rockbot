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

void classPlayer::initialize()
{
    _number = game_save.selected_player;
    char temp_name[30];
    snprintf(temp_name, sizeof(temp_name), "PLAYER_%d", _number);
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
        character_max_jump_number = 2;
    } else {
        character_max_jump_number = 1;
    }
    _damage_modifier = GameMediator::get_instance()->player_list_v3_1[_number].damage_modifier;
    update_armor_properties();
}

bool classPlayer::get_item(object_collision &obj_info)
{
    if (state.animation_type() == ANIM_TYPE_TELEPORT) {
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
            res = true; // picked up: without this the 1UP was reported as a solid object and blocked the player's move
            break;
        default:
			break;
		}
	}
	return res;
}

void classPlayer::block_jump()
{
    if (hit_ground() == false) {
        return;
    }
    jump_lock_timer = timer.getTimer() + JUMP_LOCK_DURATION;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classPlayer::initFrames()
{
    frameSize.width = GameMediator::get_instance()->player_list_v3_1[_number].sprite_size.width;
    frameSize.height = GameMediator::get_instance()->player_list_v3_1[_number].sprite_size.height;
    total_frame_size.width = frameSize.width;
    total_frame_size.height = frameSize.height;

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
            if (state.animation_type() != ANIM_TYPE_JUMP_ATTACK) {
                if (state.animation_type() == ANIM_TYPE_WALK_ATTACK) {
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
		if (state.animation_type() != ANIM_TYPE_SHIELD) {
            set_animation_type(ANIM_TYPE_SHIELD);
			state.set_animation_timer(0);
			state.set_animation_state(0);
		}
		return;
	} else if (state.animation_type() == ANIM_TYPE_SHIELD) {
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
    clean_projectiles();
    clean_effect_projectiles();
    soundManager.stop_music();
    soundManager.stop_repeated_sfx();
    soundManager.play_sfx(SFX_PLAYER_DEATH);
    gameControl.draw_player_death(realPosition);

    // boss rush has no lives: dying just restarts the fight
    if (gameControl.is_boss_rush() == false) {
        game_save.items.lifes--;
        if (game_save.items.lifes < 0) {
            reset_lifes();
            gameControl.game_over();
            return;
        }
    }

    reset_charging_shot();
    gameControl.get_current_map_obj()->clear_animations();
    gameControl.get_current_map_obj()->reset_objects();
	dead = true;
    _obj_jump.interrupt();
    _obj_jump.finish();
    //std::cout << "### PLAYER::REMOVE PLATFORM #3" << std::endl;
    set_platform(NULL);
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
    // easy 9 lives, normal 5, hard 3 (the tests were "!=", so normal and hard got 9 and easy got 5)
    if (game_save.difficulty == DIFFICULTY_EASY) {
        game_save.items.lifes = 9;
    } else if (game_save.difficulty == DIFFICULTY_NORMAL) {
        game_save.items.lifes = 5;
    } else {
        game_save.items.lifes = 3;
    }
}

void classPlayer::reset_hp()
{
	hitPoints.current = hitPoints.total;
}

void classPlayer::save_input()
{
    saved_move_commands = moveCommands;
}

void classPlayer::restore_input()
{
    moveCommands = saved_move_commands;
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

bool classPlayer::is_teleporting()
{
    if (state.animation_type() == ANIM_TYPE_TELEPORT) {
        return true;
    }
    return false;
}
