#ifndef CLASS_CONFIG_H
#define CLASS_CONFIG_H

#include "file/format/st_common.h"
#include "character/classplayer.h"

#define WPN_COLS 2
#define WPN_ROWS 6

/**
 * @brief
 *
 */
enum e_tank_type {TANK_ENERGY, TANK_WEAPON, TANK_SPECIAL};

// ********************************************************************************************** //
// This class handles all configuration menus and in-game weapon and item selection screen        //
// ********************************************************************************************** //
/**
 * @brief
 *
 */
class class_config
{
public:
	class_config();
    /**
     * @brief
     */
    void draw_ingame_menu();
    /**
     * @brief
     * @param set_player_ref
     * @return bool
     */
    bool execute_ingame_menu();
    /**
     * @brief
     * @param set_player_ref
     */
    void set_player_ref(classPlayer* set_player_ref);
    /**
     * @brief
     *
     * @param menu_pos
     * @return WEAPON_ICONS_ENUM
     */
    short convert_menu_pos_to_weapon_n(st_position menu_pos) const;
    /**
     * @brief
     *
     * @param wpn_n
     * @return st_position
     */
    st_position convert_stage_n_to_menu_pos(short stage_n) const;


    Sint8 find_next_weapon(Uint8 current, Uint8 move) const; // used by L/R buttons

    void disable_ingame_menu();


private:
    /**
     * @brief
     *
     * @param x_inc
     * @param y_inc
     */
    void move_cursor(Sint8 x_inc, Sint8 y_inc);
    /**
     * @brief
     *
     * @return st_position
     */
    st_position move_weapon_curstor_left() const;
    /**
     * @brief
     *
     * @return st_position
     */
    st_position move_weapon_curstor_right() const;
    /**
     * @brief
     *
     * @return st_position
     */
    st_position move_weapon_curstor_up();
    /**
     * @brief
     *
     * @return st_position
     */
    st_position move_weapon_curstor_down();


    void weapon_menu_show_player();

    void use_tank(int tank_type);

    void generate_weapons_matrix();

    // used  to create an array with all weapons that have have depending on save
    void generate_weapons_array();


private:
    bool ingame_menu_active; /**< TODO */
    st_position ingame_menu_pos; /**< Cursor porion in menu. X = 0 means left column, x=1 means right column */
    classPlayer* player_ref; /**< TODO */
    bool _weapons_matrix[WPN_COLS][WPN_ROWS]; // this indicates if a weapon is present in that position or not
    bool _weapons_array[WEAPON_COUNT]; // this indicates if a weapon is present in that position or not
};

#endif // CLASS_CONFIG_H
