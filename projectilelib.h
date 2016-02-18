#ifndef PROJECTILELIB_H
#define PROJECTILELIB_H

#include "file/format/st_common.h"
#include "defines.h"
#include <string>
#include "timerlib.h"
#include "graphicslib.h"

#include "aux_tools/trajectory_parabola.h"

class classMap; // advance declaration


#include "graphic/draw.h"
extern draw draw_lib;

extern timerLib timer; /**< TODO */

/**
 * @brief
 *
 */
struct quake_info {
    unsigned int timer; /**< TODO */
    short int counter1; /**< TODO */
    short int counter2; /**< TODO */
	quake_info() {
		timer = 0;
		counter1 = 0;
		counter2 = 0;
	}
};



class projectile
{
public:
    projectile(Uint8 id, Uint8 set_direction, st_position set_position, classMap *set_map, bool _owner_is_player); // common constructor
    void set_is_permanent();
    void set_default_values(); // set some common values for all constructors
    ~projectile();
    st_size move();
    void draw();
    void set_map(classMap *set_map);
    bool check_colision(st_rectangle enemy_pos, st_position pos_inc) const;
    bool check_map_colision(st_position pos_inc) const;
    Uint8 get_direction() const;
    void reflect();
    Uint8 get_move_type() const;
    void set_y(int sety);
    void consume_projectile();


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
    void play_sfx();

private:
    // methods that return properties taking in account id -1 (default projectile)
    /**
     * @brief
     *
     * @return st_size
     */
    st_size_int8 get_size() const;

    void move_ahead(st_size &moved);
    void position_to_ground();
    void set_direction_from_xyinc(int xinc, int yinc);
    void set_direction_from_targetpos(int middle_tolerance);


public:
    bool is_finished; /**< TODO */
    bool is_reflected; /**< TODO */

private:
    Sint8 _id; /**< TODO */
    st_size_int8 _size; /**< TODO */
    Uint8 animation_pos; /**< TODO */
    unsigned int animation_timer; /**< TODO */
    Uint8 direction; /**< TODO */
    unsigned int move_timer; /**< TODO */
    int move_delay; /**< TODO */
    Uint8 status; // status of animation (going/returning, etc) /**< TODO */
    Uint8 _move_type; /**< TODO */

    st_float_position position; /**< TODO */
    st_position realPosition; /**< TODO */
	// used for trajectory calculations
    st_position position0; /**< TODO */
    st_position position1; /**< TODO */
    st_position map_position; /**< TODO */
    classMap *map; /**< TODO */
    st_float_position _diagonal_speed;

    bool diagonal_flag; // used to control diagonal shot angle /**< TODO */
    Uint8 _max_frames; // number of frames for the projectile /**< TODO */
    float angle; /**< TODO */
    short int radius; /**< TODO */

	// used for quake effect
    quake_info _quake_info; /**< TODO */

	// general effect timer
    unsigned int _effect_timer; /**< TODO */
    short _effect_n; // used to control steps /**< TODO */
    short _points; // duration /**< TODO */
    short _weapon_id; /**< TODO */

    st_float_position *_owner_position; /**< TODO */
    Uint8 *_owner_direction; /**< TODO */

    st_float_position *_target_position; /**< TODO */

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
