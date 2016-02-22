#pragma GCC diagnostic ignored "-Wreorder"

#ifndef CHARACTER_H
#define CHARACTER_H

#include <iostream>
#include <string>
#include <map>

// local includes
#include "file/format/st_hitPoints.h"
#include "file/format/st_common.h"
#include "file/format/st_characterState.h"
#include "graphicslib.h"
#include "projectilelib.h"
#include "character/movement/jump.h"
#include "character/movement/inertia.h"

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

class classMap;		// advance declaration
struct object_colision;

/**
 * @brief
 *
 */
struct st_spriteFrame {
    unsigned int delay; // time in milisseconds this frame will be shown
    graphicsLib_gSurface frameSurface;
	st_spriteFrame() {
        frameSurface.set_surface(NULL);
		delay = 20;
	}
    void setDelay(int newDelay)
	{
		delay = newDelay;
	}
    void setSurface(graphicsLib_gSurface& newSurface)
	{
		graphLib.initSurface(st_size(newSurface.width, newSurface.height), &frameSurface);
		graphLib.copyArea(st_position(0, 0), &newSurface, &frameSurface);
	}
};


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
    /**
     * @brief
     *
     */
    /**
     * @brief
     *
     */
    virtual ~character();
    /**
     * @brief
     *
     * @param std::string
     */
    void setName(std::string);
    /**
     * @brief
     *
     * @return std::string
     */
    std::string getName(void) const;
    /**
     * @brief
     *
     * @param st_hit_points
     */
    void setHitPoints(st_hit_points);
    /**
     * @brief
     *
     */
    virtual void initFrames() = 0; // pure virtual
    /**
     * @brief
     *
     */
    virtual void execute() = 0; // pure virtual
    /**
     * @brief
     *
     * @return st_position
     */
    st_float_position getPosition() const;
    /**
     * @brief
     *
     * @return st_position
     */
    struct st_position get_real_position() const;
    /**
     * @brief
     *
     * @param st_position
     */
    void set_position(struct st_position);
    /**
     * @brief
     *
     * @param int
     * @param int
     * @param int
     * @param int
     * @param
     * @param int
     */
    void addSpriteFrame(int, int, graphicsLib_gSurface&, int);
    /**
     * @brief
     *
     * @param set_map
     */
    void set_map(classMap *set_map);
    /**
     * @brief
     *
     * @return classMap
     */
    classMap *get_map();
    /**
     * @brief
     *
     * @param set_player
     */
    void set_is_player(bool set_player);
    /**
     * @brief
     *
     * @return bool
     */
    bool is_player() const;
    /**
     * @brief
     *
     */
    void advance_frameset(); // changes the state for the next (or previous) frame
    /**
     * @brief
     *
     */
    void show();


    /**
     * @brief
     *
     */
    void show_sprite();

    /**
     * @brief
     *
     * @param direction
     * @param type
     * @param n
     */
    void show_sprite_graphic(short direction, short type, short n);

    /**
     * @brief
     *
     * @return st_size
     */
    st_size get_size() const;
    /**
     * @brief
     *
     * @param
     */
    void set_platform(object*);
    /**
     * @brief
     *
     * @return object
     */
    object* get_platform();
    /**
     * @brief
     *
     * @return int
     */
    int get_direction() const;
    /**
     * @brief
     *
     * @param direction
     */
    void set_direction(int direction);
    classMap *map;										// reference to the map this npc is in
    /**
     * @brief
     *
     */
    void clean_projectiles();
    /**
     * @brief
     *
     */
    void char_update_real_position();
    /**
     * @brief
     *
     * @param damage_points
     */
    virtual void damage(unsigned int damage_points, bool ignore_hit_timer);
    /**
     * @brief
     *
     * @return bool
     */
    bool is_dead() const;
    /**
     * @brief
     *
     * @return st_hit_points
     */
    st_hit_points get_hp() const;
    /**
     * @brief
     *
     * @return Uint8
     */
    Uint8 get_current_hp() const;
    /**
     * @brief
     *
     * @param inc
     */
    void set_current_hp(Uint8 inc);
    /**
     * @brief
     *
     */
    void execute_jump();								// execute a complete jump
    /**
     * @brief
     *
     */
    void execute_jump_up();					// execute jump until reaches the maximum height
    /**
     * @brief
     *
     */
    void fall();								// falls until reaching ground or leaving screen /// @TODO
    /**
     * @brief
     *
     */
    void teleport_out();

    /**
     * @brief
     *
     * @param xinc
     * @param yinc
     * @return bool
     */
    bool change_position(short int xinc, short int yinc);


    /**
     * @brief
     *
     * @param xinc
     * @return bool
     */
    void change_position_x(short int xinc);

    /**
     * @brief
     *
     * @param yinc
     * @return bool
     */
    int change_position_y(short int yinc);

    /**
     * @brief
     *
     * @param xinc
     * @param yinc
     * @return bool
     */
    bool test_change_position(short int xinc, short int yinc);
    /**
     * @brief
     *
     * @param xinc
     * @param yinc
     * @return bool
     */
    bool test_change_position_debug(short int xinc, short int yinc);
    /**
     * @brief
     *
     * @param projectile_direction
     * @return bool
     */
    bool is_shielded(int projectile_direction) const;
    /**
     * @brief
     *
     * @return short
     */
    short get_anim_type() const;
    /**
     * @brief
     *
     * @param direction
     * @param type
     * @param frame
     * @return graphicsLib_gSurface
     */
    graphicsLib_gSurface* get_char_frame(int direction, int type, int frame);
    /**
     * @brief
     *
     * @param key_n
     * @return st_color
     */
    st_color get_color_key(short int key_n) const;
    /**
     * @brief
     *
     * @return short
     */
    short int get_number() const;
    /**
     * @brief
     *
     */
    void charMove();
    /**
     * @brief
     *
     */
    void clear_move_commands();
    /**
     * @brief
     *
     * @param boss_demo_mode
     * @return bool
     */


    void reset_gravity_speed();

    bool gravity(bool boss_demo_mode);							// returns true if finished (reached ground)
    /**
     * @brief
     *
     * @return bool
     */
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


    /**
     * @brief remove the first projectile from the list
     *
     */
    void consume_projectile();

    /**
     * @brief identifies if the player is using some kind of cicle weapon (TRAJECTORY_CENTERED), so enemy projectiles will damage the player projectile, not himself
     * @return bool true if using circle-type weapon
     */
    bool is_using_circle_weapon();

    void inc_effect_weapon_status();


    void set_animation_type(enum ANIM_TYPE type);

    void set_show_hp(bool show);

    void set_progressive_appear_pos(int pos);

    /**
     * @brief
     *
     * @return bool
     */
    bool is_stage_boss();

    void clean_character_graphics_list();

    void cancel_slide();

    virtual float get_hit_push_back_n();

    virtual bool have_shoryuken();

    virtual int get_armor_arms_attack_id();

    void remove_freeze_effect();

    void push_back(short direction);


protected:
    /**
     * @brief
     *
     */
    // updown_trajectory: updown -1 is down, 0 is none, 1 is up
    // auto_charged: true will use charged (if have) or semi-charged as default projetile
    virtual void attack(bool dont_update_colors, short updown_trajectory, bool auto_charged);
    /**
     * @brief
     *
     * @param key
     * @param new_color
     * @param full_change
     */
    void change_char_color(Sint8 colorkey_n, st_color new_color, bool full_change);

    /**
     * @brief
     *
     * @param mapScrolling
     * @return bool
     */
    bool slide(st_position mapScrolling);
    /**
     * @brief
     *
     * @param int
     * @param st_position
     * @return bool
     */
    bool jump(int, st_position);


    /**
     * @brief
     *
     * @param incx
     * @param incy
     * @param mapScrolling
     * @return short
     */
    st_map_colision map_colision(const float incx, const short int incy, st_position mapScrolling);


    bool is_on_teleporter_capsulse(object* object);

    /**
     * @brief
     *
     * @param map_block
     * @param new_map_lock
     * @param mode_xy
     * @param map_pos
     */
    void check_map_colision_point(int &map_block, int &new_map_lock, int mode_xy, st_position map_pos);
    /**
     * @brief
     *
     * @param map_lock
     * @param incx
     * @param incy
     * @param map_pos
     * @return bool
     */
    bool process_special_map_points(int map_lock, int incx, int incy, st_position map_pos);

    void check_platform_move(short map_lock);

    /**
     * @brief
     *
     */
    void add_graphic();
    /**
     * @brief
     *
     */
    virtual void death() = 0;
    /**
     * @brief
     *
     * @return bool
     */
    bool have_frame_graphics();



    /**
     * @brief
     *
     * @return bool
     */
    bool have_background_graphics();
    /**
     * @brief
     *
     * @param direction
     * @param type
     * @param pos
     * @return bool
     */
    bool have_frame_graphic(int direction, int type, int pos);  // indicates if the given frame graphic exits
    /**
     * @brief
     *
     * @return bool
     */
    bool is_in_stairs_frame() const; // indicates if the character is on some of the STAIRS animation types


    bool is_on_attack_frame();

    /**
     * @brief
     *
     * @param _en_type
     * @param value
     */
    virtual void recharge(e_energy_types _en_type, int value);
    /**
     * @brief
     *
     * @param obj_info
     * @return bool
     */
    virtual bool get_item(object_colision& obj_info);
    /**
     * @brief
     *
     * @return unsigned int
     */
    unsigned int get_projectile_count(); // returns the number of projectiles (some special attacks count as max)
    /**
     * @brief
     *
     * @return int
     */
    int frames_count(); // find out the number of frames in the current direction/type
    /**
     * @brief
     *
     */
    void advance_to_last_frame();

    virtual void show_hp(); // player and boss will implement this

    int is_executing_effect_weapon(); // returns type, or -1 if none

    st_position get_int_position(); // converts float position to integer position

    void check_reset_stand();

    bool is_weak_to_freeze();                           // checks that this NPC is weak against the freeze weapon

    virtual bool can_air_dash();

    Uint8 get_projectile_max_shots();




// members
public:
	// projectile list
    std::vector<projectile> projectile_list;
    //struct st_spriteFrame sprite_list[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]; // TODO - move to protected


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

	// TODO - graphics list map, used in order to avoid duplication of graphics
    static std::map<std::string, st_spriteFrame[CHAR_ANIM_DIRECTION_COUNT][ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]> character_graphics_list;
    static std::map<std::string, graphicsLib_gSurface> _character_graphics_background_list;

    object* _platform; // used to move player when object moves

    bool dead;

    st_color color_keys[3];

    short int       shield_type;                        // same as for NPC, 0->no-shield, 1->full_shield, 2->front-shield

    unsigned int    _moving_platform_timer;             // used to control character movement when is over a moving platform

    int             _number;                            // used for player or npc_id

    bool            _super_jump;                        // indicates if the jump is super high
    bool            _force_jump;                        // when character is catapulted by an object, ignores jump button state until jump is finished (no matter if the jump button is released in the middle of jump, it will jump until reaching top)
    std::string     _debug_char_name;

    int             _teleport_minimal_y;                // do not stop teleport before reaching this point. (used on player's death)

    int             _projectile_type_id;				// indicates the projectile ID (relation with file_projectile)

    bool _is_falling;

    int _dead_state; // 0 - alive, 1 - just died, 2 dead
    short slide_type; // 0 - dash (24 px height), 1 - slide (16px height)
    bool _water_splash;									// used to prevent making a new splash until completaly inside or outside water
    bool _has_background;
    st_position _frame_pos_adjust;
    short _stairs_stopped_count; // used to prevent stopping stairs animation because of a single frame without player input
    short _charged_shot_projectile_id;
    short _hit_move_back_dist;
    bool _was_animation_reset;                               // inform that animation "looped" once
    bool _is_last_frame;
    bool _was_hit;                                           // indicates if the character was hit during the current cicle/loop
    short int _simultaneous_shots;                            // how many shots at a time the character can fire
    bool _is_boss;
    bool _show_hp;                                          // show or hide the HP bat
    bool _always_move_ahead;                                // indicates to character class that the AI is to always move on
    bool _check_always_move_ahead;                          // used to prevent setting _always_move_ahead each time we can AI exec
    int _progressive_appear_pos;                            // used by spawn-npc to show just a part of the NPC
    bool _is_stage_boss;                                    // used in NPC class. Indicates if this is the stage-boss
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

};

#endif // CHARACTER_H
