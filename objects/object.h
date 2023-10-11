#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <map>
#include "../graphicslib.h"

class classMap; // forward declaration

#define TELEPORT_TIME 400
#define STATUS_TIMER_DELAY 500
#define TIMED_BOMB_DELAY 800
#define TIMED_BOMB_EXPLOSION_DURATION 2000

enum e_object_teleport_states {
    e_object_teleport_state_initial,
    e_object_teleport_state_teleport_in,
    e_object_teleport_state_waiting,
    e_object_teleport_state_teleport_out,
    e_object_teleport_state_count
};

enum e_OBJECT_BOSS_DOOR_STATES {
    e_OBJECT_BOSS_DOOR_STATE_NONE,
    e_OBJECT_BOSS_DOOR_STATE_OPENING,
    e_OBJECT_BOSS_DOOR_STATE_OPENED,
    e_OBJECT_BOSS_DOOR_STATE_CLOSING,
    e_OBJECT_BOSS_DOOR_STATE_COUNT
};

enum e_OBJECT_CRUSHER_STATE {
    e_OBJECT_CRUSHER_STATE_INIT,
    e_OBJECT_CRUSHER_STATE_FOUND_FREE_WALL,
    e_OBJECT_CRUSHER_STATE_FOUND_HIT_GROUND,
    e_OBJECT_CRUSHER_STATE_RETURNING,
    e_OBJECT_CRUSHER_STATE_COUNT
};

enum e_OBJECT_DIAGONAL_PLATFORM_STATE {
    e_OBJECT_DIAGONAL_PLATFORM_STATE_INIT,
    e_OBJECT_DIAGONAL_PLATFORM_STATE_MOVING,
    e_OBJECT_DIAGONAL_PLATFORM_STATE_START_FALL,
    e_OBJECT_DIAGONAL_PLATFORM_STATE_FALLING,
    e_OBJECT_DIAGONAL_PLATFORM_STATE_COUNT
};


/**
 * @brief
 *
 */
class object
{
public:
    //object(Uint8 id, struct CURRENT_FILE_FORMAT::file_object temp_obj); // game object constructor
    object(short _id, classMap *set_map, st_position map_pos, st_position teleporter_dest, short map_dest, short set_direction); // map object constructor
    ~object();
    void reset();
    void reset_timer();
    void reset_obj_anim_timer();
    void show(int adjust_y=0, int adjust_x=0);
    void show_vertical_ray(int adjust_x, int adjust_y=0);
    void show_horizontal_ray(int adjust_x, int adjust_y=0);
    void show_track_platform(int adjust_x, int adjust_y=0);
    void show_deathray_vertical(int adjust_x=0, int adjust_y=0);
    void show_deathray_horizontal(int adjust_x=0, int adjust_y=0);
    void show_boss_door(int adjust_x=0, int adjust_y=0);
    void show_crusher();
    bool is_platform();                                             // tell if object is of platform type
    void move(bool paused);
    void move_crusher();
    void add_explosion();
    bool check_player_crushed();
    void reset_animation();
    void stop();
    void execute(bool paused);
    bool object_type_needs_reset_offscreen();

    st_position get_position() const;
    st_position get_start_position() const;
    st_rectangle get_area();
    st_size get_size();
    Uint8 get_type() const;
    bool is_active_platform();
    Uint8 get_id() const;
    Uint8 get_direction() const;
    void set_direction(int);
    int get_distance() const;
    void set_distance(int);
    void set_timer(int);
    unsigned int get_timer() const;
    bool finished() const;
    void set_finished(bool is_finished);
    void invert_direction_x();
    void invert_direction_y();
    void set_state(short obj_state);
    short get_state() const;
    void set_duration(int duration);
    void start();
    void start_timed_bomb(int xinc, int yinc, st_rectangle player_hitbox);
    void command_up();															// some objects can be controlled by the player
    void command_down();														// some objects can be controlled by the player
    std::string get_name() const;
    bool is_hidden() const;
    void set_hidden(bool hide);
    bool is_started() const;
    bool is_on_screen();
    bool is_on_visible_screen();
    bool is_on_expanded_visible_screen();
    void set_collision_mode(enum collision_modes collision_mode); // if player uses this as platform, move him
    enum collision_modes get_collision_mode() const;
    void reset_timers();
    void set_teleport_out();
    bool is_consumable();                                       // if item is a energy, tank, life, that player can get
    void enable_teleport_animation();                           // when this is called, object will show a teleporting in-animation
    void set_precise_position(st_position pos, int direction);                                // used to get a fine-tuning positioning instead of map-position
    void set_position(st_position pos);
    void remove_graphic();
    st_position get_boss_teleporter_dest();
    Uint8 get_boss_teleport_map_dest();
    Uint8 get_obj_map_id();
    void set_obj_map_id(Uint8 id);
    bool is_teleporting();
    void set_is_dropped(bool dropped);
    bool get_is_dropped();
    void inc_status();


private:
    void add_graphic();
    void gravity();
    bool test_change_position(short int xinc, short int yinc);
    void check_player_move(int xinc, int yinc); // if player uses this as platform, move him


private:
    short _id;
    std::string name;
    std::string graphic_filename;												// graphic file used on it
    short type;
    unsigned int obj_timer;														// used as time for disapearing block
    unsigned int _obj_frame_timer;
    int _frame_duration;
    int speed;																	// used as speed for moving platform
    int limit;																	// used as range (pixels) for moving platform, visible time for disapearing block
    short direction;																// used to check if moving away from oiginalpoint or moving to it
    short original_direction;
    int distance;
    int framesize_w;
    int framesize_h;
    short frame;																	// indicates what is the used frame
    struct st_position start_point;
    struct st_position position;
    classMap *map;																// reference to the map this object is in
    bool _started;																// some object types will only start to act/move after player interaction
    unsigned int _start_timer;                                                           // holds the time of the activation (used for initial delay)
    bool _finished;																// indicates to map->show() that the object must be deleted
    Sint8 _state;
    unsigned int _duration;
    unsigned int _timer_limit;
    bool _command_up;
    bool _command_down;
    bool _animation_finished;
    bool _animation_reversed;
    bool _hidden;
    enum collision_modes _collision_mode;
    bool _must_play_appearing_sfx;                                              // used by disappearing blocks to play the sfx just once
    short _teleport_state;                                                      // used to control when start/finish the teleport
    short _ray_state;                                                           // controls death-ray movement
    st_position _boss_teleporter_dest;
    short _boss_teleporter_map_dest;
    short _obj_map_id;                                                            // used for map-objects, so we can get them in stage_data.objects[N]
    bool _expanding;
    int _size;
    int max_frames;
    bool is_dropped;
    bool show_teleport;
    long teleport_max_timer;
    bool item_jet_started = false;
    float crusher_speed = 0;
    unsigned long active_platform_timer = 0;
    unsigned long status_timer = 0;
    unsigned long last_execute_time = 0;
    int zigzag_graph_pos_x = 0;
};

#endif // OBJECT_H
