#ifndef ARTIFICIAL_INTELIGENCE_H
#define ARTIFICIAL_INTELIGENCE_H

#include "character.h"
#include <cmath>


/**
 * @brief AI sub-state, indicates if the current state is starting (must set initial values), executing or finishing (must set next step and set values)
 */
enum e_action_states { IA_ACTION_STATE_INITIAL, IA_ACTION_STATE_EXECUTING, IA_ACTION_STATE_FINISHED  };

/**
 * @brief holds information about the distance of the current NPC for the closest player
 */
struct struct_player_dist {
    int dist; /**< distance (diagonal), in pixels, to the player */
    character *pObj; /**< pointer to the player object */
    st_position dist_xy; /**< distance separated for X and Y axis */
};


/**
 * @brief
 */
struct struct_ia_state {
    short int main_status;										/**< AI main state */
    short int sub_status;                                       /**< TODO */
    short int sub_action_sub_status;                            // used when a action calls several sub-actions
    short int action_status;                                    /**< TODO */
    st_position initial_position;                               /**< TODO */
    st_position secondary_position;                             /**< TODO */
    int timer;                                                  /**< TODO */
    int step;                                                   /**< TODO */
	struct_ia_state()
	{
		main_status = 0;
        sub_status = IA_ACTION_STATE_FINISHED;                  // setting it as finished forces us to defined next step on start
		action_status = IA_ACTION_STATE_INITIAL;
		timer = 0;
		step = 0;
        sub_action_sub_status = 0;
	}
};

/**
 * @brief class, child of character and parent of classNPC, that holds artificial inteligence methods used by NPC class
 */
class artificial_inteligence : public character
{

public:
    /**
     * @brief constructor
     */
    artificial_inteligence();
    ~artificial_inteligence();

    // AI methods @TODO: deprecated, must be replaced by AI chain system

    /**
     * @brief
     */
    void ground_damage_players();

    void push_back_players(short direction);

    bool auto_respawn() const;

    // NEW AI
    /**
     * @brief
     */
    void execute_ai();

    void hit_player();


protected:
    /**
     * @brief
     * @return struct_player_dist
     */
    struct_player_dist dist_npc_players();

    // AI ACTIONS
    /**
     * @brief
     */
    void ia_action_jump_to_player();

    void ia_action_jump_to_point(st_position point);

    void ia_action_jump_to_random();

    void ia_action_jump_ahead();


    void ia_action_jump_once();

    void ia_action_jump_up();

    /**
     * @brief
     */
    void ia_action_jump_to_roof();

    /**
     * @brief
     */
    void ia_action_air_walk();

    /**
     * @brief
     */
    void ia_action_jump_fall();

    /**
     * @brief
     */
    void ia_action_quake_attack();

    /**
     * @brief
     */
    void ia_accelerate_up();

    /**
     * @brief
     */
    void ia_accelerate_down();


    /**
     * @brief
     * @param dest_point
     */
    void ia_set_destiny_point(st_position dest_point);

    /**
     * @brief
     */
    void ia_walk_to_position();

    /**
     * @brief same as walk to position, but for floaters
     */
    void ia_float_to_position();

    /**
     * @brief
     */
    void ia_dash();

    /**
     * @brief
     */
    void execute_ai_step_jump_to_wall();


    void execute_ai_replace_itself();

    void execute_ai_step_spawn_npc();

    void execute_ai_circle_player();

    void ia_action_teleport();




    void check_ai_reaction();

    /**
     * @brief
     */
    void define_ai_next_step();

    /**
     * @brief
     */
    void execute_ai_step();

    /**
     * @brief
     */
    void execute_ai_step_walk();

    /**
     * @brief
     */
    void execute_ai_action_wait_until_player_in_range();

    /**
     * @brief
     * @param n
     * @param invert_direction
     */
    void execute_ai_action_trow_projectile(Uint8 n, bool invert_direction);

    bool throw_projectile(int projectile_type, bool invert_direction);

    /**
     * @brief
     */
    void execute_ai_step_fly();

    void execute_ai_save_point();

    /**
     * @brief
     */
    void execute_ai_step_dash();

    void execute_ai_step_change_animation_type();

    void execute_ai_step_change_animation_type_reverse();

    void execute_ai_wait_random_time();

    /**
     * @brief moves to a point, returns true when point was reached or can't reach the target
     * @param dest_point point of destiny
     * @param speed_x how fast must move on X axis
     * @param speed_y how fast must move on Y axis
     * @param can_pass_walls the NPC can cross walls like a ghost or not
     * @return bool true -> point reached
     */
    bool move_to_point(st_float_position dest_point, float speed_x, float speed_y, bool can_pass_walls);


    void randomize_x_point(int max_adjust);

    int create_rand_x_point(int max_range);
    int create_rand_y_point(int max_range);
    st_position create_rand_point(int max_range);

    void execute_ai_step_jump();

    int find_wall(float initial_x, int direction);

    int get_ai_type();

    bool always_move_ahead() const; // indicates if this NPC will always go on in AI

    bool uses_fly_fall(); // uses to respawn it inside holes



protected:
    st_size distance; /**< TODO */
    short int walk_range;								// defines how long from the start point it can go /**< TODO */
    character *target;									// number of the player that is target /**< TODO */
    float speed_y; /**< TODO */
    float acceleration_y; /**< TODO */
    st_position _origin_point;							// used as target for moving /**< TODO */
    st_float_position _dest_point; /**< TODO */


    struct_ia_state _ai_state; /**< TODO */


	// "external" members
    static std::vector<character*> *player_list;					// reference to the players list /**< TODO */
    unsigned int _ai_timer;										// used to not execute AI before a while /**< TODO */
    short _ai_chain_n;							// indicates witch one of the four AI actions is the NPC on /**< TODO */
    short _ghost_move_speed_reducer;           // some NPCs walk slowly when throught walls
    struct st_position start_point; /**< TODO */
    unsigned int _auto_respawn_timer;                            // used to check if enought time has passed to auto-respawn

    trajectory_parabola *_trajectory_parabola;          // used for jumping to a specific point
    bool _did_shot;                                         // used to avoid shooting multiple times
    st_float_position _diagonal_speed;
    int _reaction_state;                                // used to control reaction so it won't execute two times or over an executing state
                                                        // 0 - waiting, 1 - executing
    int _reaction_type;                                 // 1: near, 2: hit, 3: dead
    int _parameter;                                     // stores the extra-parameter
    int _last_jump_yinc;                                 // with this, we can check if NPC was falling when yinc becomes = 0
    bool _initialized;                                  // used to indicated that is not yet initialized, thus we need to get change to pick next action
    int _counter;                                       // used for counting times, such as how many shots were fired
    int _current_ai_type;                               // holds the current AI type, can be changed to another one if needed (as jumping over obstacles)
    bool _show_reset_stand;
    st_position _target_point;							// used to store player position in circle player
    float _angle;                                         // used to store angle in circle movement
    int _execution_state;                               // controlw flow of execution
    int radius;
    short int jump_attack_type;                               // used by jump attack to store attack-type, if any. otherwise, it is set as -1
    bool did_hit_player;                                 // when player collides with this enemy, it will set this flag as on, so the enemy knows it
};

#endif // ARTIFICIAL_INTELIGENCE_H
