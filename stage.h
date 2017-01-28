#ifndef STAGE_H
#define STAGE_H

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
    /**
     * @brief
     *
     */
    ~stage();
    int get_number();
    void setNumber(int);

    void loadStage();
    /**
     * @brief
     *
     */
    void showStage();
    /**
     * @brief
     *
     * @param st_position
     * @return int
     */
    Uint8 getMapPointLock(struct st_position);
    /**
     * @brief
     *
     * @param pos
     * @param check_lock
     */


    void change_map_scroll(st_float_position pos, bool check_lock, bool ignore_auto_scroll);
    /**
     * @brief
     *
     * @return st_position
     */
    st_float_position getMapScrolling();
    /**
     * @brief
     *
     * @param scroll_y
     */
    void showAbove(int scroll_y=0);
    /**
     * @brief
     *
     * @return classMap
     */
    classMap* get_current_map();
    /**
     * @brief
     *
     * @return int
     */
    Uint8 get_current_map_number();
    /**
     * @brief
     *
     * @param int
     */
    void set_current_map(int);
    /**
     * @brief
     *
     * @return short
     */
    Uint8 get_current_map_n();
    /**
     * @brief
     *
     * @param pos
     */
    void set_scrolling(st_float_position pos);

    void reset_scrolling();

    /**
     * @brief
     *
     * @param move_x
     * @param move_y
     */
    void move_map(const short int move_x, const short int move_y);
    /**
     * @brief
     *
     */
    void reload_stage(); // reload maps npc and object list
    /**
     * @brief
     *
     */

    void print_map_objects_number(); // for debugging

    void reset_current_map();
    /**
     * @brief
     *
     */
    void reset_current_map_objects();
    /**
     * @brief
     *
     */
    void reset_stage_objects();

    void redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number);
    /**
     * @brief
     *
     */
    void move_npcs();
    /**
     * @brief
     *
     */
    void show_npcs();
    /**
     * @brief
     *
     */
    void move_objects(bool paused=false);


    void show_objects(int adjust=0);
    /**
     * @brief
     *
     * @return bool
     */
    bool boss_hit_ground();
    /**
     * @brief
     *
     */
    void reset_stage_maps();
    /**
     * @brief
     *
     */
    void reset_stage_npcs();
    /**
     * @brief
     *
     * @param x_pos
     * @return int
     */
    int get_first_lock_on_left(int x_pos);

    int get_first_lock_on_right(int x_pos);

    int get_teleport_minimal_y_tile(int xpos);

    void reset_objects_timers();

    bool subboss_alive_on_left(short tileX);

    void activate_final_boss_teleporter();

    short get_current_map_gfx_mode();

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
