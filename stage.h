#ifndef STAGE_H
#define STAGE_H

#ifdef ANDROID
#include <android/log.h>
#endif

#include "classmap.h"
#include "character/character.h"
#include "character/classplayer.h"

#define PRELOAD_MAP_N 3

/**
 * @brief
 *
 */
class stage
{
public:
    stage(int, classPlayer *set_player_ref);
    ~stage();
    int get_number();
    void setNumber(int);
    void load_stage();
    void show_stage();
    Uint8 getMapPointLock(struct st_position);
    void change_map_scroll(st_float_position pos, bool check_lock, bool ignore_auto_scroll);
    st_float_position getMapScrolling();
    void showAbove(int scroll_y=0, bool show_fg=true);
    classMap* get_current_map();
    Uint8 get_current_map_number();
    void set_current_map(int);
    Uint8 get_current_map_n();
    void set_scrolling(st_float_position pos);
    void reset_scrolling();
    void move_map(const short int move_x, const short int move_y);
    void reload_stage(); // reload maps npc and object list
    void print_map_objects_number(); // for debugging
    void reset_current_map();
    void reset_current_map_objects();
    void reset_stage_objects();
    void redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number);
    void move_npcs();
    void show_npcs();
    void show_npcs_to_left(int x);
    void move_objects(bool paused=false);
    void show_objects(int adjust=0);
    void show_above_objects(int adjust=0);
    bool boss_hit_ground(classnpc *npc_ref);
    classnpc* get_near_boss();
    void reset_stage_maps();
    void reset_stage_npcs();
    int get_first_lock_on_left(int x_pos);
    int get_first_lock_on_right(int x_pos);
    int get_first_ground(int x_pos);
    int get_teleport_minimal_y_tile(int xpos);
    void reset_objects_timers();
    void reset_objects_anim_timers();
    bool subboss_alive_on_left(short tileX);
    void activate_final_boss_teleporter();
    short get_current_map_gfx_mode();
    void add_autoscroll_delay();
    void reset_stage_timers();
    bool must_show_static_bg();                                 // method used to prevent showing enemies on transition if showing static-bg

private:
    void check_map_effect();




public:
    classMap maps[PRELOAD_MAP_N];

private:
    Sint8 number;
    Uint8 currentMap;
    classPlayer* _player_ref;
    unsigned int autoscroll_timer;
    bool stage_is_loaded;
};

#endif // STAGE_H
