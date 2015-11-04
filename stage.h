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
    void showStage() const;
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
    void changeScrolling(st_position pos, bool check_lock=true) const;
    /**
     * @brief
     *
     * @return st_position
     */
    st_position getMapScrolling() const;
    /**
     * @brief
     *
     * @param scroll_y
     */
    void showAbove(int scroll_y=0) const;
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
    Uint8 get_current_map_number() const;
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
    Uint8 get_current_map_n() const;
    /**
     * @brief
     *
     * @param pos
     */
    void set_scrolling(st_position pos) const;

    void reset_scrolling();

    /**
     * @brief
     *
     * @param move_x
     * @param move_y
     */
    void move_map(const short int move_x, const short int move_y) const;
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
    void reset_current_map_objects() const;
    /**
     * @brief
     *
     */
    void reset_stage_objects() const;
    /**
     * @brief
     *
     */
    void clean_stage();
    /**
     * @brief
     *
     * @param is_close
     * @param nTiles
     * @param tileX
     * @param player_number
     */
    void redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number) const;
    /**
     * @brief
     *
     */
    void move_npcs() const;
    /**
     * @brief
     *
     */
    void show_npcs() const;
    /**
     * @brief
     *
     */
    void move_objects(bool paused=false) const;


    void show_objects(int adjust=0) const;
    /**
     * @brief
     *
     * @return bool
     */
    bool boss_hit_ground() const;
    /**
     * @brief
     *
     */
    void reset_stage_maps() const;
    /**
     * @brief
     *
     */
    void reset_stage_npcs() const;
    /**
     * @brief
     *
     * @param x_pos
     * @return int
     */
    int get_first_lock_on_left(int x_pos) const;

    int get_teleport_minimal_y(int xpos) const;

    void reset_objects_timers() const;

    bool subboss_alive_on_left(short tileX);

    void activate_final_boss_teleporter();


public:
    classMap* maps[PRELOAD_MAP_N]; /**< TODO */

private:
    Sint8 number; /**< TODO */
    Uint8 currentMap; /**< TODO */
    classPlayer* _player_ref; /**< TODO */
};

#endif // STAGE_H
