#include "class_config.h"

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "game.h"
extern game gameControl;



extern CURRENT_FILE_FORMAT::st_save game_save;

class_config::class_config() : ingame_menu_active(false)
{
	ingame_menu_pos.x = 0;
    ingame_menu_pos.y = 0;
    for (int i=0; i<WPN_COLS; i++) {
        for (int j=0; j<WPN_ROWS; j++) {
            _weapons_matrix[i][j] = false;
            _weapons_array[i*j] = false;
        }
    }
    player_ref = NULL;

}



void class_config::set_player_ref(classPlayer* set_player_ref)
{
	player_ref = set_player_ref;
}


// column1: normal, ape,    daisie, dynamite, mummy,     spike
// column2:         techno, mage,   seahorse, item coil, item jet
// column3: e-tank, w-tank *(must be changed, as currently are two rows)
void class_config::move_cursor(Sint8 x_inc, Sint8 y_inc) {
	// left/right: if position exists, just move. if not then go to first item in nexct column or stays in place
	bool moved = false;
    st_position res;

    //std::cout << ">>>>> class_config::move_cursor - xinc: " << x_inc << ", y_inc: " << y_inc << std::endl;

    if (ingame_menu_pos.y != 6) { // weapons positions
        if (x_inc > 0) {
            if (ingame_menu_pos.x == 0) {
                res = move_weapon_curstor_right();
                if (res.x != -1) {
                    ingame_menu_pos = res;
                    moved = true;
                }
            } else {
                res = move_weapon_curstor_left(); // right on right column - move left
                if (res.x != -1) {
                    ingame_menu_pos = res;
                    moved = true;
                }
			}
		} else if (x_inc < 0 && ingame_menu_pos.y != 0) {
            if (ingame_menu_pos.x == 0) {
                res = move_weapon_curstor_right(); // left on left column - move right
                if (res.x != -1) {
                    ingame_menu_pos = res;
                    moved = true;
                }
            } else {
                res = move_weapon_curstor_left();
                if (res.x != -1) {
                    ingame_menu_pos = res;
                    moved = true;
                }
			}

		}
    } else { // energy drinks position
		if (x_inc > 0 && ingame_menu_pos.y != 0) {
			if (ingame_menu_pos.x < 2) {
				ingame_menu_pos.x++;
			} else {
				ingame_menu_pos.x = 0;
			}
			moved = true;
		} else if (x_inc < 0) {
			if (ingame_menu_pos.x == 0) {
				ingame_menu_pos.x = 2;
			} else {
				ingame_menu_pos.x--;
			}
			moved = true;
		}
	}





	if (y_inc > 0) {
		if (ingame_menu_pos.y < 6) {
            res = move_weapon_curstor_down();
            if (res.y != -1) {
                ingame_menu_pos = res;
                moved = true;
            }
        } else {
            ingame_menu_pos.x = 0;
            ingame_menu_pos.y = 0;
		}
		moved = true;
    } else if (y_inc < 0) {
        if ((ingame_menu_pos.x == 0 && ingame_menu_pos.y == 0) || (ingame_menu_pos.x == 1 && ingame_menu_pos.y == 1)) { // when on top, go to energy drinks section
            std::cout << "MOVE CURSOR #1" << std::endl;
			ingame_menu_pos.y = 6;
		} else {
            if (ingame_menu_pos.y == 6 && ingame_menu_pos.x == 2) { // just fix x, as there is no third column on weapons section as there is in the energy drinks section
				ingame_menu_pos.x = 1;
			}
            res = move_weapon_curstor_up();
            if (res.y != -1) {
                ingame_menu_pos = res;
                moved = true;
            }
		}
	}
	if (moved == true) {
		soundManager.play_sfx(SFX_CURSOR);
    }
}

st_position class_config::move_weapon_curstor_left() const // moving from right-column to left-column
{
    /*
    int stage_pos = convert_menu_pos_to_weapon_n(st_position(0, ingame_menu_pos.y));
    if (game_save.stages[stage_pos] == 1) { // have the same-line weapon on the other columns
        return st_position(0, ingame_menu_pos.y);
    } else { // use the first weapon (normal) from left column
        return st_position(0, 0);
    }
    */
    int new_x = 1;
    if (ingame_menu_pos.x == 1) {
        new_x = 0;
    }
    // search for a existing slot to move to
    for (int i=ingame_menu_pos.y; i<WPN_ROWS; i++) { // start on current y
        if (_weapons_matrix[new_x][i] == true) {
            return st_position(new_x, i);
        }
    }
    for (int i=0; i<ingame_menu_pos.y; i++) { // remaining y positions
        if (_weapons_matrix[new_x][i] == true) {
            return st_position(new_x, i);
        }
    }
    return st_position(-1, -1);
}

st_position class_config::move_weapon_curstor_right() const // move from left to right column
{
    return move_weapon_curstor_left(); // both are the same code
}

st_position class_config::move_weapon_curstor_up()
{
    // from position to top
    //std::cout << "$$$$$ move_weapon_curstor_up - ini: " << (ingame_menu_pos.y-1) << std::endl;
    for (int i=ingame_menu_pos.y-1; i>=0; i--) {
        //std::cout << "move_weapon_curstor_up - looking slot [" << ingame_menu_pos.x << "][" << i << "] value: " << _weapons_matrix[ingame_menu_pos.x][i] << std::endl;
        if (_weapons_matrix[ingame_menu_pos.x][i] == true) {
            //std::cout << "move_weapon_curstor_up - found slot at [" << ingame_menu_pos.x << "][" << i << "]" << std::endl;
            return st_position(ingame_menu_pos.x, i);
        }
    }
    // if not on energy tanks, move to it
    if (ingame_menu_pos.y != 6) {
        return st_position(ingame_menu_pos.x, 6);
    }

    // from bottom to position
    for (int i=WPN_ROWS-1; i>ingame_menu_pos.y; i--) {
        if (_weapons_matrix[ingame_menu_pos.x][i] == true) {
            return st_position(ingame_menu_pos.x, i);
        }
    }

    return st_position(-1, -1);


}

st_position class_config::move_weapon_curstor_down()
{

    // from position to bottom
    for (int i=ingame_menu_pos.y+1; i<WPN_ROWS; i++) {
        int stage_id = i;
        //std::cout << "CONFIG::move_weapon_curstor_down - slot[" << ingame_menu_pos.x << "][" << stage_id << "]: " << _weapons_matrix[ingame_menu_pos.x][stage_id] << std::endl;
        if (_weapons_matrix[ingame_menu_pos.x][stage_id] == true) {
            //std::cout << "CONFIG::move_weapon_curstor_down - found slot at [" << ingame_menu_pos.x << "][" << i << "]" << std::endl;
            return st_position(ingame_menu_pos.x, stage_id);
        }
    }
    // if not on energy tanks, move to it
    if (ingame_menu_pos.y != 6) {
        return st_position(ingame_menu_pos.x, 6);
    }

    // from top to position
    for (int i=0; i<ingame_menu_pos.y; i++) {
        if (_weapons_matrix[ingame_menu_pos.x][i] == true) {
            return st_position(ingame_menu_pos.x, i);
        }
    }

    return st_position(-1, -1);

}

void class_config::weapon_menu_show_player()
{
    graphLib.copyArea(st_position(26, 195), player_ref->get_char_frame(ANIM_DIRECTION_RIGHT, ANIM_TYPE_ATTACK, 0), &graphLib.gameScreen);
}

void class_config::use_tank(int tank_type)
{
	int n = 0;

    // check tank number
    if (tank_type == TANK_ENERGY && game_save.items.energy_tanks == 0) {
        return;
    }
    if (tank_type == TANK_WEAPON && game_save.items.weapon_tanks == 0) {
        return;
    }
    if (tank_type == TANK_SPECIAL && game_save.items.special_tanks == 0) {
        return;
    }

	// no need for tank usage
	if (tank_type == TANK_ENERGY && player_ref->get_hp().current == player_ref->get_hp().total) {
		return;
	}
	if (tank_type == TANK_ENERGY || tank_type == TANK_SPECIAL) {
		while (player_ref->get_hp().current < player_ref->get_hp().total) {
            input.read_input();
			player_ref->set_current_hp(1);
			if (n == 0 || n % 6 == 0) {
				soundManager.play_sfx(SFX_GOT_ENERGY);
			}
			n++;
            graphLib.draw_weapon_cursor(st_position(0, 0), player_ref->get_hp().current, -1, player_ref->get_max_hp());
            draw_lib.update_screen();
			timer.delay(50);
		}
	}
	if (tank_type == TANK_SPECIAL || tank_type == TANK_WEAPON) {
		st_position weapon_pos(0, 0);
		for (int i=0; i<WEAPON_COUNT; i++) {
			n = 0;
			short unsigned int value = player_ref->get_weapon_value(i);
			if (value < player_ref->get_hp().total) {
				while (value < player_ref->get_hp().total) {
                    input.read_input();
					value++;
					player_ref->set_weapon_value(i, value);
					if (n == 0 || n % 6 == 0) {
						soundManager.play_sfx(SFX_GOT_ENERGY);
					}
					n++;
                    graphLib.draw_weapon_cursor(weapon_pos, player_ref->get_weapon_value(i), -1, player_ref->get_max_hp());
                    draw_lib.update_screen();
					timer.delay(50);
				}
			}
			weapon_pos.y = weapon_pos.y+1;
			if (weapon_pos.y > 5) {
				weapon_pos.y = 1;
				weapon_pos.x = 1;
			}
		}
	}
    // consume tank
    if (tank_type == TANK_ENERGY) {
        game_save.items.energy_tanks--;
    }
    if (tank_type == TANK_WEAPON) {
        game_save.items.weapon_tanks--;
    }
    if (tank_type == TANK_SPECIAL) {
        game_save.items.special_tanks--;
    }
}



void class_config::draw_ingame_menu()
{
    ingame_menu_pos = convert_stage_n_to_menu_pos(player_ref->get_selected_weapon());
    graphLib.draw_weapon_menu_bg(player_ref->get_current_hp(), player_ref->get_char_frame(ANIM_DIRECTION_RIGHT, ANIM_TYPE_ATTACK, 0), player_ref->get_max_hp());
    graphLib.draw_weapon_icon(convert_menu_pos_to_weapon_n(ingame_menu_pos), ingame_menu_pos, true);
    graphLib.draw_weapon_cursor(ingame_menu_pos, player_ref->get_weapon_value(convert_menu_pos_to_weapon_n(ingame_menu_pos)), player_ref->get_number(), player_ref->get_max_hp());
}

bool class_config::execute_ingame_menu()
{
    st_position old_pos;


    if (input.p1_input[BTN_START] == 1) {

        input.clean();
        timer.delay(100);

        ingame_menu_active = !ingame_menu_active;

        if (ingame_menu_active) {
            gameControl.game_pause();
            generate_weapons_matrix();
            draw_lib.fade_out_screen(0, 0, 0, 300);
            draw_ingame_menu();
        } else {
            // left menu, change player color/weapon and remove pause
            if (ingame_menu_pos.y != 6) {
                player_ref->set_weapon(convert_menu_pos_to_weapon_n(ingame_menu_pos), false);
                gameControl.game_unpause();
            } else {
                // use item
                use_tank(ingame_menu_pos.x);
                ingame_menu_active = !ingame_menu_active; // keep itself inside the menu
                generate_weapons_matrix();
                draw_ingame_menu();
            }
        }
    }

    if (ingame_menu_active) {
        old_pos.x = ingame_menu_pos.x;
        old_pos.y = ingame_menu_pos.y;
        if (input.p1_input[BTN_UP] == 1) {
            move_cursor(0, -1);
        } else if (input.p1_input[BTN_DOWN] == 1) {
            move_cursor(0, 1);
        } else if (input.p1_input[BTN_LEFT] == 1) {
            move_cursor(-1, 0);
        } else if (input.p1_input[BTN_RIGHT] == 1) {
            move_cursor(1, 0);
        } else if (input.p1_input[BTN_R] == 1) {
            if (gameControl.show_config(game_save.stages[gameControl.currentStage]) == true) { // player picked "leave stage" option
                ingame_menu_active = false;
                gameControl.game_unpause();
                return true;
            }
            draw_ingame_menu();
        }
        if (old_pos.x != ingame_menu_pos.x || old_pos.y != ingame_menu_pos.y) {
            //std::cout << ">> old_pos.y: " << old_pos.y << ", ingame_menu_pos.y: " << ingame_menu_pos.y << std::endl;
            if (old_pos.y != 6) {
                graphLib.draw_weapon_cursor(old_pos, player_ref->get_weapon_value(convert_menu_pos_to_weapon_n(old_pos)), -1, player_ref->get_max_hp());
                graphLib.draw_weapon_icon(convert_menu_pos_to_weapon_n(old_pos), old_pos, false);
            } else {
                graphLib.erase_menu_item(old_pos.x);
            }
            if (ingame_menu_pos.y != 6) {
                graphLib.draw_weapon_cursor(ingame_menu_pos, player_ref->get_weapon_value(convert_menu_pos_to_weapon_n(ingame_menu_pos)), player_ref->get_number(), player_ref->get_max_hp());
                graphLib.draw_weapon_icon(convert_menu_pos_to_weapon_n(ingame_menu_pos), ingame_menu_pos, true);
                player_ref->set_weapon(convert_menu_pos_to_weapon_n(ingame_menu_pos), false);
                weapon_menu_show_player();
            } else {
                graphLib.draw_menu_item(ingame_menu_pos.x);
            }
        }
        input.clean();
        //timer.delay(MENU_CHANGE_DELAY);
    }

    return ingame_menu_active;
}

short class_config::convert_menu_pos_to_weapon_n(st_position menu_pos) const
{
    short res = WEAPON_DEFAULT;
    if (menu_pos.x == 0) {
        res = menu_pos.y;
    } else {
        res = menu_pos.y + 5;
    }
    //std::cout << "*** CONFIG::convert_menu_pos_to_weapon_n - res: " << res << ", menu_pos.x: " << menu_pos.x << ", menu_pos.y: " << menu_pos.y << std::endl;
    return res;

    /*
	if (menu_pos.x == 1 && menu_pos.y == 4) {
		return WEAPON_ITEM_COIL;
	}
	if (menu_pos.x == 1 && menu_pos.y == 5) {
		return WEAPON_ITEM_JET;
	}
    */
}

st_position class_config::convert_stage_n_to_menu_pos(short stage_n) const
{
	st_position res_pos;

    if (stage_n < 6) {
        res_pos.x = 0;
        res_pos.y = stage_n;
    } else {
        res_pos.x = 1;
        res_pos.y = stage_n - 5;
    }

    //std::cout << "*** CONFIG::convert_menu_pos_to_weapon_n - res.x: " << res_pos.x << ", res.y: " << res_pos.y << std::endl;

    return res_pos;
}


Sint8 class_config::find_next_weapon(Uint8 current, Uint8 move) const
{
    if (move == 1) {
        for (int i=current+1; i<WEAPON_COUNT; i++) { // from position to end
            //std::cout << "#0 CONFIG::find_next_weapon - wpnId: " << i << ", save: " << game_save.stages[i] << std::endl;
            if (game_save.stages[i] == 1) {
                //std::cout << "#0 CONFIG::find_next_weapon - OK[" << i << "]" << std::endl;
                return i;
            }
        }
        for (int i=0; i<current; i++) { // from start to position
            if (game_save.stages[i] == 1) {
                return i;
            }
        }
    } else {
        for (int i=current-1; i>=0; i--) { // from position to start
            if (game_save.stages[i] == 1) {
                //std::cout << ">>#1 CONFIG::find_next_weapon - wpnId: " << i << std::endl;
                return i;
            }
        }
        for (int i=WEAPON_COUNT-1; i>current; i--) { // from end to position
            if (game_save.stages[i] == 1) {
                //std::cout << ">>#2 CONFIG::find_next_weapon - wpnId: " << i << std::endl;
                return i;
            }
        }
    }
    return -1;
}

void class_config::disable_ingame_menu()
{
    ingame_menu_active = false;
}

void class_config::generate_weapons_array()
{
    _weapons_array[0] = true;       // normal weapon always exists
    /*
    APE         - TECHNOBOT
    DAISIEBOT   - MAGEBOT
    DYNAMITEBOT - SEAHORSEBOT
    MUMMYBOT    - COIL_GOT_STAGE
    SPIKEBOT    - JET_GOT_STAGE
    */
    for (int i=1; i<9; i++) {
        if (game_save.stages[i] == 1) {
            _weapons_array[i] = true;
        }
    }
    if (game_save.stages[COIL_GOT_STAGE] == 1) {
        _weapons_array[WEAPON_ITEM_COIL] = true;
    }
    if (game_save.stages[JET_GOT_STAGE] == 1) {
        _weapons_array[WEAPON_ITEM_JET] = true;
    }
}

void class_config::generate_weapons_matrix()
{
    _weapons_matrix[0][0] = true;       // normal weapon always exists
    _weapons_matrix[1][0] = false;      // second column do not have a weapon on first row
    /*
    APE         - TECHNOBOT
    DAISIEBOT   - MAGEBOT
    DYNAMITEBOT - SEAHORSEBOT
    MUMMYBOT    - COIL_GOT_STAGE
    SPIKEBOT    - JET_GOT_STAGE
    */
    for (int i=1; i<=9; i++) {

        if (game_save.stages[i] == 1) {
            st_position pos = convert_stage_n_to_menu_pos(i);
            //std::cout << "CONFIG::generate_weapons_matrix - stage[" << i << "]: " << game_save.stages[i] << ", pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;
            _weapons_matrix[pos.x][pos.y] = true;
        }
    }
    if (game_save.stages[COIL_GOT_STAGE] == 1) {
        _weapons_matrix[1][4] = true;
    }
    if (game_save.stages[JET_GOT_STAGE] == 1) {
        _weapons_matrix[1][5] = true;
    }
}
