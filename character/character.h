#ifndef CHARACTER_H
#define CHARACTER_H

#include <iostream>
#include <string>
#include <map>

// local includes
#include "../file/format/st_hitPoints.h"
#include "../file/format/st_common.h"
#include "../file/format/st_characterState.h"
#include "../graphicslib.h"
#include "../projectilelib.h"
#include "movement/jump.h"
#include "movement/inertia.h"
#include "character_animation.h"



extern graphicsLib graphLib;

class object; // forward declaration

/**
 * @brief
 *
 */
enum ATTACK_STATES {
	ATTACK_NOT,
	ATTACK_START,
	ATTACK_WAIT
};


enum ATTACK_TYPES {
    ATTACK_TYPE_NOATTACK,
    ATTACK_TYPE_NORMAL,
    ATTACK_TYPE_SEMICHARGED,
    ATTACK_TYPE_FULLYCHARGED,
    ATTACK_TYPE_SUPERCHARGED,
    ATTACK_TYPE_COUNT
};

class classMap;		// advance declaration
struct object_collision;


/**
 * @brief
 *
 */
struct st_characterMovements { // this instruction holds commands passed from player (joystick) and npc (AI) and is used to execute basic movements
    short up;
    short down;
    short left;
    short right;
    short attack;
    short jump;
    short start;
    short dash;
    short shield;
	st_characterMovements()
	{
		up = 0;
		down = 0;
		left = 0;
		right = 0;
		attack = 0;
		jump = 0;
/**
 * @brief
 *
 */
        start = 0;
		dash = 0;
		shield = 0;
	}
};



/**
 * @brief
 *
 */
class character
{
public:
	character();
    virtual ~character();
    void setName(std::string);
    std::string get_name(void) const;
    void setHitPoints(st_hit_points);
    virtual void initFrames() = 0; // pure virtual
    virtual void execute() = 0; // pure virtual
    st_float_position getPosition() const;
    struct st_position get_real_position() const;
    void set_position(struct st_position);
    void inc_position(float inc_x, float inc_y);
    void addSpriteFrame(int, int, graphicsLib_gSurface&, int);
    void set_is_player(bool set_player);
    bool is_player() const;
    void advance_frameset(); // changes the state for the next (or previous) frame
    void show();
    void show_previous_sprites();
    void reset_dash_effect();
    void show_at(st_position pos);
    void show_sprite();
    void reset_sprite_animation_timer();
    void show_sprite_graphic(short direction, short type, short n, st_position pos);
    graphicsLib_gSurface* get_current_frame_surface(short direction, short type, short frame_n);
    st_size get_size() const;
    st_rectangle get_hitbox(int anim_type=-1);          // used for collision with map/objects
    st_rectangle get_vulnerable_area(int anim_type=-1);         // used for collision agains projectiles, takes vulnerable_area into account
    void set_platform(object*);
    object* get_platform();
    int get_direction() const;
    void set_direction(int direction);
    void clean_projectiles();
    void clean_effect_projectiles();
    void char_update_real_position();
    st_float_position get_screen_position_from_point(st_float_position pos);
    virtual void damage(unsigned int damage_points, bool ignore_hit_timer);
    virtual void damage_spikes(bool ignore_hit_timer);
    bool is_dead() const;
    st_hit_points get_hp() const;
    Uint8 get_current_hp() const;
    void set_current_hp(Uint8 inc);
    void execute_jump();								// execute a complete jump
    void execute_jump_up();					// execute jump until reaches the maximum height
    void fall();								// falls until reaching ground or leaving screen /// @TODO
    void fall_to_ground();
    void initialize_boss_position_to_ground();
    bool change_position(short int xinc, short int yinc);
    void change_position_x(short int xinc);
    int change_position_y(short int yinc);
    bool test_change_position(short int xinc, short int yinc);
    bool is_shielded(int projectile_direction) const;
    bool is_intangible();
    short get_anim_type() const;
    graphicsLib_gSurface* get_char_frame(int direction, int type, int frame);
    st_color get_color_key(short int key_n) const;
    short int get_number() const;
    void charMove();
    void store_previous_position();
    void clear_move_commands();
    void reset_gravity_speed();
    bool gravity(bool boss_demo_mode);							// returns true if finished (reached ground)
    bool hit_ground();
    /**
     * @brief
     *
     * @return bool
     */

    bool will_hit_ground(int y_change) const;

    bool is_on_screen(); // indicates if the character is inside screen area
    /**
     * @brief
     *
     * @return bool
     */
    bool is_on_visible_screen(); // ignore if near, only return true for 0-RES_W


    bool is_entirely_on_screen(); // whole body is on screen area, ignoring one tile left and right
    /**
     * @brief
     *
     * @return bool
     */
    bool is_invisible() const;

    /**
     * @brief
     *
     */
    void activate_super_jump();
    /**
     * @brief
     *
     */
    void activate_force_jump();
    /**
     * @brief
     *
     * @return st_position
     */
    st_float_position *get_position_ref();
    /**
     * @brief
     *
     */
    void reset_jump();


    void consume_projectile();

    bool is_using_circle_weapon();

    void inc_effect_weapon_status();


    void set_animation_type(enum ANIM_TYPE type);
    void set_animation_frame(unsigned int frame);

    void set_progressive_appear_pos(int pos);
    bool is_stage_boss();
    bool has_final_game_boss();
    void clean_character_graphics_list();
    bool have_background_graphics();
    void cancel_slide();
    virtual float get_hit_push_back_n();
    virtual bool have_shoryuken();
    virtual int get_armor_arms_attack_id();
    void remove_freeze_effect();
    void push_back(short direction);
    void pull(short direction);
    bool get_can_fly();
    bool animation_has_restarted();
    void set_animation_has_restarted(bool restarted);
    st_position get_int_position(); // converts float position to integer position

    void add_projectile(short id, st_position pos, int trajectory, int direction);
    st_position get_attack_position();
    st_position get_attack_position(short direction);
    bool is_on_last_animation_frame();
    bool have_frame_graphic(int direction, int type, int pos);  // indicates if the given frame graphic exits


private:
    ATTACK_TYPES check_must_attack(bool always_charged);
    void check_charging_colors(bool always_charged);

protected:
    // updown_trajectory: updown -1 is down, 0 is none, 1 is up
    // auto_charged: true will use charged (if have) or semi-charged as default projetile
    virtual void attack(bool dont_update_colors, short updown_trajectory, bool always_charged);
    void change_char_color(Sint8 colorkey_n, st_color new_color, bool full_change);
    bool slide(st_float_position mapScrolling);
    bool jump(int, st_float_position);
    st_map_collision map_collision(const float incx, const short int incy, st_float_position mapScrolling, int hitbox_anim_type=-1);
    bool is_on_teleporter_capsulse(object* object);
    bool is_on_teleport_platform(object* object);
    void check_map_collision_point(int &map_block, int &new_map_lock, int &old_map_lock, int mode_xy);
    bool process_special_map_points(int map_lock, int incx, int incy, st_position map_pos);
    void check_platform_move(short map_lock);
    void add_graphic();
    virtual void death() = 0;
    bool have_frame_graphics();
    bool is_in_stairs_frame() const; // indicates if the character is on some of the STAIRS animation types
    bool is_on_attack_frame();
    virtual void recharge(e_energy_types _en_type, int value);
    virtual bool get_item(object_collision& obj_info);
    unsigned int get_projectile_count(); // returns the number of projectiles (some special attacks count as max)
    int frames_count(); // find out the number of frames in the current direction/type
    void advance_to_last_frame();
    int is_executing_effect_weapon(); // returns type, or -1 if none
    void check_reset_stand();
    bool is_weak_to_freeze();                           // checks that this NPC is weak against the freeze weapon
    virtual bool can_air_dash();
    Uint8 get_projectile_max_shots(bool always_charged);




// members
public:
	// projectile list
    std::vector<projectile> projectile_list;
    std::vector<projectile> projectile_to_be_added_list;

protected:
	// members static that can be moved to use game_data
    std::string name;
    struct st_size frameSize;
    unsigned int max_projectiles; // maximum number of simultaneuous projectiles the character can shot
    float move_speed; // how many pixels the character moves by cycle (default value)

	// dynamic members
    st_hit_points hitPoints;
    unsigned int last_hit_time; /// time of last being hit
    unsigned int hit_duration; /// time the character is intangible after being hit (in milisseconds)
    unsigned int hit_animation_timer; /// used to control "blinking" effect when hit
    unsigned int hit_animation_count;   // to control number of times blinking uses
    st_float_position position;
    st_position realPosition;
    unsigned int last_execute_time;
    struct st_characterMovements moveCommands;
    struct st_characterMovements saved_move_commands;
    Uint8 saved_input[BTN_COUNT];

	// characteristics members
    float accel_speed_y; // used for acceleration in jump/gravity
    float gravity_y; // used for acceleration in jump/gravity
    float max_speed; // used for acceleration in jump/gravity
    bool can_fly;                       // indicates if the character can fly in order to not be affected by gravity
    bool _ignore_gravity;               // ignore gravity while this flag is true
    unsigned int attack_state; // indicates if the player is attacking and what is the stage of the attack
    bool attack_button_released; // indicates if attack button was released between shots

    unsigned int attack_button_pressed_timer;
    short attack_button_last_state;

    int charging_color_n; // holds information used when changing char colors on charging shot
    unsigned int charging_color_timer; // holds information used when changing char colors on charging shot
    bool jump_button_released; // indicates if jump button was released between jumps
    bool _dash_button_released; // indicates if dash button (or jump+down) was released between jumps
    /**
     * @brief
     *
     * @param pos
     * @return st_position
     */
    st_position is_on_stairs(st_rectangle pos);		// check is character in over a staircase

	// control members
    st_characterState state;
    bool is_player_type; // used for some small tweaks


	// DEBUG
    float hit_moved_back_n;
	// external members

    object* _platform = NULL; // used to move player when object moves

    bool dead;

    st_color color_keys[3];

    short int       shield_type;                        // same as for NPC, 0->no-shield, 1->full_shield, 2->front-shield

    unsigned int    _moving_platform_timer;             // used to control character movement when is over a moving platform

    int             _number;                            // used for player or npc_id

    bool            _super_jump;                        // indicates if the jump is super high
    bool            _force_jump;                        // when character is catapulted by an object, ignores jump button state until jump is finished (no matter if the jump button is released in the middle of jump, it will jump until reaching top)
    std::string     _debug_char_name;

    int             _teleport_minimal_y;                // do not stop teleport before reaching this point. (used on player's death)

    bool _is_falling;

    int _dead_state; // 0 - alive, 1 - just died, 2 dead
    short slide_type; // 0 - dash (24 px height), 1 - slide (16px height)
    bool _water_splash;									// used to prevent making a new splash until completaly inside or outside water
    bool _has_background;
    st_position background_pos;
    short _stairs_stopped_count; // used to prevent stopping stairs animation because of a single frame without player input
    short _charged_shot_projectile_id;
    short _normal_shot_projectile_id;
    short _hit_move_back_dist;
    bool _was_animation_reset;                               // inform that animation "looped" once
    bool _is_last_frame;                                     // inform that reached the end of that animation loop
    bool _is_attack_frame;                                   // inform that NPC reached the attack-frame number (so it can attack before attack animation reaches the final frame)
    short _attack_frame_n;                                   // what is the attack frame number, by default it is -1, that means we use the last one
    bool _was_hit;                                           // indicates if the character was hit during the current cicle/loop
    short int _simultaneous_shots;                            // how many shots at a time the character can fire
    bool _is_boss;
    bool _always_move_ahead;                                // indicates to character class that the AI is to always move on
    bool _check_always_move_ahead;                          // used to prevent setting _always_move_ahead each time we can AI exec
    int _progressive_appear_pos;                            // used by spawn-npc to show just a part of the NPC
    bool _is_stage_boss;                                    // used in NPC class. Indicates if this is the stage-boss
    bool _has_final_game_boss = false;                      // this enemy is the final game boss or spawns it
    bool _dropped_from_stairs;                              // used to avoid grabbing stairs again when dropped from it
    classjump _obj_jump;
    short _jumps_number;                                    // used for double or triple jumping
    short _damage_modifier;                                 // used in players
    st_float_position _previous_position;                   // this is used so we can avoid calculating things like hit_gound() if position did not changed
    bool _hit_ground;
    inertia _inertia_obj;                                   // handles inertia over ice
    bool _dashed_jump;                                      // adds horizontal acceleration fo X movement if jump was made during a dash (not slide) until player reaches ground again
    bool _can_execute_airdash;                                 // prevents dashing multiple-times in middle-air
    bool _player_must_reset_colors;                         // inform the player class that he must get back into default-weapon colors
    long _stairs_falling_timer;                              // controls time when player can again grab the staircase
    bool is_ghost;                                          // if can shoot or not /**< TODO */
    st_rectangle vulnerable_area_box;                       // hitarea, set by classnpc
    character_animation animation_obj;

    std::vector<st_float_position> previous_position_list;
    bool must_show_dash_effect;
    graphicsLib_gSurface dash_effect_shadow_surface_frame;
    short int facing = ANIM_DIRECTION_LEFT;					    // defines the side npc is facing before start moving (also used by LINEWALK behavior) /**< TODO */


};

#endif // CHARACTER_H
