#ifndef CLASSNPC_H
#define CLASSNPC_H

#include <string>
#include "character.h"
#include "file/format.h"
#include "character/artificial_inteligence.h"

class classPlayer;


/**
 * @brief
 *
 */
struct npc_frameset {
    int frameset;				// holds information about the type of this frame /**< TODO */
    unsigned int frameset_time;		// the number of milisseconds this frame leasts /**< TODO */
    struct st_size framesize; /**< TODO */
};




/**
 * @brief
 *
 */
class classnpc : public artificial_inteligence
{
public:
	classnpc();
    ~classnpc();
    classnpc(std::string set_name);
    classnpc(int stage_id, int map_id, int main_id, int id); // load data from game_data and create a new npc
    classnpc(int stage_id, int map_id, int main_id, st_position npc_pos, short int direction, bool player_friend); // spawned npc
    void initFrames();
    void execute();
    void init_animation();
    void boss_move();
    void copy(classnpc *from);
    void move();
    void move_projectiles();
    void show_projectiles();
    virtual bool is_boss();
    void set_is_boss(bool set_boss);
    bool is_player_friend();
    void set_stage_boss(bool boss_flag);
    void revive();
    short get_dead_state();
    bool is_able_to_fly();
    bool is_spawn();
    bool is_subboss();
    void reset_position();
    st_position get_start_position();
    st_position get_bg_position();
    void show();
    // those are needed for AI to use because of linker not reconizing character class
    void npc_set_hp(st_hit_points new_hp);
    void npc_set_position(st_float_position pos);
    void npc_set_direction(short dir);
    void npc_set_initialized(short init);
    void set_parent_id(int parent_id);
    int get_parent_id();
    void reset_timers();
    bool is_static();





protected:
    /**
     * @brief
     *
     */
    void death();
    /**
     * @brief
     *
     */
    void invert_direction();
    /**
     * @brief
     *
     * @param stage_id
     * @param map_id
     * @param main_id
     */
    void build_basic_npc(int stage_id, int map_id, int main_id);



protected:
	// W A R N I N G -----------------> new members must be reflected in copy() method
    short int facing;									// defines the side npc is facing before start moving (also used by LINEWALK behavior) /**< TODO */
    std::string graphic_filename;						// graphic file used on it /**< TODO */
    bool first_run; /**< TODO */
    bool _is_player_friend;								// player spawned npcs must not hit him, but other npcs instead /**< TODO */
    bool _is_spawn;                                     // indicates that is a spawn, so won't drop items, etc


    // boss member variables
    short _initialized; /**< TODO */
    bool _screen_blinked; /**< TODO */
    int _parent_id;
    st_position static_bg_pos;
};

#endif // CLASSNPC_H
