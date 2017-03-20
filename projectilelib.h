#ifndef PROJECTILELIB_H
#define PROJECTILELIB_H

#include "file/format/st_common.h"
#include "defines.h"
#include <string>
#include "graphicslib.h"

#include "aux_tools/trajectory_parabola.h"

#include "graphic/draw.h"
extern draw draw_lib;

#include "timerlib.h"
extern timerLib timer;

/**
 * @brief
 *
 */
struct quake_info {
    unsigned int timer;
    short int counter1;
    short int counter2;
	quake_info() {
		timer = 0;
		counter1 = 0;
		counter2 = 0;
	}
};



class projectile
{
public:
    projectile(Uint8 id, Uint8 set_direction, st_position set_position, bool _owner_is_player); // common constructor
    void set_is_permanent();
    void set_default_values(); // set some common values for all constructors
    ~projectile();
    st_size move();
    void draw();
    bool check_collision(st_rectangle enemy_pos, st_position pos_inc) const;
    bool check_map_collision(st_position pos_inc) const;
    Uint8 get_direction() const;
    void reflect();
    Uint8 get_move_type() const;
    bool is_explosive();
    void set_y(int sety);
    void consume_projectile();
    void transform_into_explosion();


    void finish(); // force finish, used in char::clean_projectiles, handles effects that should be removed
    void set_weapon_id(short wpn_id);
    short get_weapon_id() const;
    short get_effect_n() const;
    void set_owner_position(st_float_position *owner_position);
    void set_owner_direction(Uint8 *owner_direction);
    Uint8 get_speed() const;
    Uint8 get_damage() const;
    Uint8 get_trajectory() const;
    void set_trajectory(short new_trajectory);
    void set_target_position(st_float_position *pos);
    graphicsLib_gSurface* get_surface();
    void inc_status(); // increase status, so we can explode a bomb or change centered (star circle) to linear
    st_rectangle get_area();
    short get_max_shots();
    short get_id();
    void play_sfx(bool called_from_npc);

private:
    // methods that return properties taking in account id -1 (default projectile)
    /**
     * @brief
     *
     * @return st_size
     */
    st_size get_size() const;

    void move_ahead(st_size &moved);
    void position_to_ground();
    void set_direction_from_xyinc(int xinc, int yinc);
    void set_direction_from_targetpos(int middle_tolerance);


public:
    bool is_finished;
    bool is_reflected;

private:
    Sint8 _id;
    st_size _size;
    Uint8 animation_pos;
    unsigned int animation_timer;
    Uint8 direction;
    unsigned int move_timer;
    int move_delay;
    Uint8 status; // status of animation (going/returning, etc)
    Uint8 _move_type;

    st_float_position position;
    st_position realPosition;
	// used for trajectory calculations
    st_position position0;
    st_position position1;
    st_position map_position;
    st_float_position _diagonal_speed;

    bool diagonal_flag; // used to control diagonal shot angle
    Uint8 _max_frames; // number of frames for the projectile
    float angle;
    short int radius;

	// used for quake effect
    quake_info _quake_info;

	// general effect timer
    unsigned int _effect_timer;
    short _effect_n; // used to control steps
    short _points; // duration
    short _weapon_id;

    st_float_position *_owner_position;
    Uint8 *_owner_direction;

    st_float_position *_target_position;

    Uint8 _speed;
    trajectory_parabola _trajectory_parabola;
    bool _owner_is_player;

    bool _is_temporary;                                 // this is needed because C++ lists create a copy, so we must know that this little one here is NOT the one that is inside the list
    short _change_direction_counter;                    // used to prevent changing frames too fast
    float _sin_x;                                       // used for sinoidal movement
    int _chain_width;
    float _accel_x;                                       // used for arc
    float _speed_x;                                       // used for arc

};
#endif // PROJECTILELIB_H
