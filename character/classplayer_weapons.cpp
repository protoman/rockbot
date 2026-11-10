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

void classPlayer::init_weapon_colors()
{
    color_keys[0] = st_color(COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
    color_keys[1] = st_color(COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
    color_keys[2] = st_color(COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

    for (int i=0; i<MAX_WEAPON_N; i++) {
        weapon_colors[i] = GameMediator::get_instance()->player_list_v3_1[_number].weapon_colors[i];
	}
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

void classPlayer::consume_weapon(int value)
{
    if (game_save.items.weapons[selected_weapon] - value < 0) {
        game_save.items.weapons[selected_weapon] = 0;
    } else {
        game_save.items.weapons[selected_weapon] -= value;
    }
}

void classPlayer::change_player_color(bool full_change)
{
    int color_n = selected_weapon;
    if (selected_weapon > WEAPON_ITEM_JET) {
        color_n = 0;
    }
    if (full_change == false) {
        graphLib.change_surface_color(0, weapon_colors[color_n].color1, &(*sprite_set()).frames[state.direction()][state.animation_type()][state.animation_state()].frameSurface);
        graphLib.change_surface_color(1, weapon_colors[color_n].color2, &(*sprite_set()).frames[state.direction()][state.animation_type()][state.animation_state()].frameSurface);
        graphLib.change_surface_color(2, weapon_colors[color_n].color3, &(*sprite_set()).frames[state.direction()][state.animation_type()][state.animation_state()].frameSurface);
    } else {
        for (int i=0; i<CHAR_ANIM_DIRECTION_COUNT; i++) {
            for (int j=0; j<ANIM_TYPE_COUNT; j++) {
                for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    if (weapon_colors[0].color1.r != -1) {
                        graphLib.change_surface_color(0, weapon_colors[color_n].color1, &(*sprite_set()).frames[i][j][k].frameSurface);
                    }
                    if (weapon_colors[0].color2.r != -1) {
                        graphLib.change_surface_color(1, weapon_colors[color_n].color2, &(*sprite_set()).frames[i][j][k].frameSurface);
                    }
                    if (weapon_colors[0].color3.r != -1) {
                        graphLib.change_surface_color(2, weapon_colors[color_n].color3, &(*sprite_set()).frames[i][j][k].frameSurface);
                    }
                }
            }
        }
    }
}

void classPlayer::set_weapon(int weapon_n, bool show_tooltip_icon)
{
	selected_weapon = weapon_n;
    clean_projectiles();
    soundManager.stop_repeated_sfx();
    attack_button_last_state = 0;
    if (show_tooltip_icon == true) {
        draw_lib.add_weapon_tooltip(selected_weapon, realPosition, state.direction());
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

bool classPlayer::can_fly()
{
    return false;
}

int classPlayer::find_next_weapon(int current, int move)
{
    class_config config_manager;
    int res = config_manager.find_next_weapon(current, move);
    return res;
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
    if (can_double_jump() == true && character_max_jump_number == 1) {
        character_max_jump_number = 2;
    }
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true && game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[_number] == ARMOR_ABILITY_BODY_EXTENDEDIMMUNITY) {
        hit_duration = 4000;
    }
    int armor_attack_id = get_armor_arms_attack_id();
    if (armor_attack_id != -1) {
        _charged_shot_projectile_id = armor_attack_id;
    }

}
