#ifndef CLASSPLAYER_H
#define CLASSPLAYER_H

#include "character.h"
#include "classnpc.h"

#define JUMP_LOCK_DURATION 1000

class classnpc; // advance declaration

/**
 * @brief child of character, this class is used to store data for a player (human controlled character)
 */
class classPlayer : public character
{
public:
    void update_armor_properties(); // this will update certain properties of player with data of the armor pieces abilities
    /**
     * @brief constructor for player class
     * @param std::string the name of the player
     * @param int number of the player. to be later used when we re-add support for the simultaneous two player mode
     */
    classPlayer(int playerNumber);

    // called after game was picked and loaded, so we have player data
    void initialize();

    /**
     * @brief hardcoded method for setting each frame for a player (@TODO: replace by user driven data)
     */
    void initFrames();

    /**
     * @brief execute all player actions (move, jump, slide, damage, projectiles)
     */
    void execute();

    /**
     * @brief move the projectiles created by this player and damage any npcs that are in it's way
     */
    void execute_projectiles();
    void projectile_hit_npc(projectile &proj, classnpc &npc, st_rectangle npc_hitbox, st_float_size moved, bool ignore_hit_timer);
    void projectile_hit_objects(projectile &proj);
    bool object_is_affected_by_projectile(object* temp_obj, projectile& projectile);

    /**
     * @brief this is used when player enters a boss-teleport that teleport the player back to it's origin point
     * @param n id of the teleporter link
     */
    void set_teleporter(int n);

    /**
     * @brief returns the teleporter id the player is using
     * @return int teleporter-link id, -1 if none is being used
     */
    int get_teleporter();

    /**
     * @brief set the player HP (hit points) back to the default value
     */
    void reset_hp();

    /**
     * @brief change the weapon player is using
     * @param weapon_n id of the weapon to be set
     */
    void set_weapon(int weapon_n, bool show_tooltip_icon);

    /**
     * @brief get the number of energy a given weapon still has (the number decreases as the weapon is used)
     * @param weapon_n id of the weapon
     * @return short number of energy points the weapon still have
     */
    short int get_weapon_value(int weapon_n);

    /**
     * @brief change the energy value of a given weapon
     * @param weapon_n id of the weapon
     * @param value value to be set as energy points for this weapon
     */
    void set_weapon_value(Uint8 weapon_n, Uint8 value);

    /**
     * @brief return the weapon player is currently using
     * @return WEAPON_ICONS_ENUM enum for all weapons
     */
    short get_selected_weapon();

    short get_selected_weapon_value();

    /**
     * @brief fully recharge energy value of all weapons
     */
    void refill_weapons();

    /**
     * @brief virtual function from character. returns always false, as a player don't fly
     * @return bool returns false always
     */
    bool can_fly();

    /**
     * @brief
     *
     */
    void reset_charging_shot();

    bool is_teleporting();

    /**
     * @brief recharge player's HP or current weapon
     * @param _en_type HP or WEAPON to be recharged
     * @param value total amount of points that will be recharged
     */
    void recharge(e_energy_types _en_type, int value);

    void damage(unsigned int damage_points, bool ignore_hit_timer);

    void damage_spikes(bool ignore_hit_timer);
    void change_player_color(bool full_change);

    // to be used when game is paused
    void save_input();
    void restore_input();

    void block_jump();


private:
    void move();
    void read_move_commands();
    void switch_weapon_buttons();
    bool send_commands_to_platform();
    void death();
    void reset_lifes();
    void init_weapon_colors();
    bool get_item(object_collision& obj_info);
    void attack(bool dont_update_colors = false);
    void attack_with_default_weapon(bool always_charged);
    short int use_weapon_item(int used_weapon);
    void setup_weapon_projectile(projectile &temp_proj, int weapon_trajectory);

    /**
     * @brief damage all NPCs that are touching ground and inside game-screen area
     */
    void damage_ground_npcs();

    /**
     * @brief drop the COIL object into map
     */
    void add_coil_object();

    /**
     * @brief drop the JET object into map
     */
    void add_jet_object();

    /**
     * @brief used whe changing weapon with L/R buttons, find what the left left or right weapon is
     * @param current current weapon id
     * @param move 1 -> right, otherwise -> left
     * @return int weapon id of the next one
     */
    int find_next_weapon(int current, int move);

    bool can_shoot_diagonal();

    bool can_double_jump();



    bool can_air_dash();


    float get_hit_push_back_n();

    int get_armor_arms_attack_id();

    bool have_shoryuken();

    bool shouryuken();

    void consume_weapon(int value);



private:
    int teleporter_n; /**< current teleporter being used, -1 if none */
    int selected_weapon; /**< current selected weapon */
    CURRENT_FILE_FORMAT::file_weapon_colors weapon_colors[MAX_WEAPON_N]; /**< TODO */
    bool l_key_released; /**< avoid changing weapon continuously if L key is held */
    bool r_key_released;            // < avoid changing weapon continuously if R key is held
};

#endif // CLASSPLAYER_H
