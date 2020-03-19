#ifndef ST_CHARACTERSTATE_H
#define ST_CHARACTERSTATE_H

#include <stddef.h>
#include <vector>

#include "st_teleporter.h"
#include "st_projectile.h"
#include "st_platform.h"
#include "defines.h"

/**
 * @brief
 *
 */
struct st_characterState {
    int hit_timer; /**< TODO */
    Uint8 direction; /**< TODO */
    std::vector<struct st_projectile> projectile_list; /**< TODO */
    struct st_teleporter *last_used_teleporter; /**< TODO */
    int jump_timer; // used to control time of jump /**< TODO */
    int jump_mark; // holds the distance between jump origin and current position /**< TODO */
    unsigned int attack_timer; // used to control time of attack /**< TODO */
    struct st_platform *current_platform; /**< TODO */
    int animation_type; // animation type as walk, jump, attack /**< TODO */
    int animation_state; // current animation frame position /**< TODO */
    unsigned int animation_timer; // used to control animation speed /**< TODO */
    int slide_distance; // holds the amount of pixels character have slided /**< TODO */
    bool animation_inverse; // indicates if the frame animation must go from last to first frame /**< TODO */
    bool invisible; /**< TODO */
    bool frozen; /**< TODO */
    unsigned int frozen_timer; /**< TODO */
    bool was_animation_reset;
	// constructor
	st_characterState()
	{
		hit_timer = 0;
		direction = ANIM_DIRECTION_RIGHT;
		last_used_teleporter = NULL;
		jump_timer = 0;
		jump_mark = 0;
		current_platform = NULL;
		animation_type = ANIM_TYPE_STAND;
		animation_state = 0;
		animation_timer = 0;
		attack_timer = 0;
        slide_distance = 0;
		animation_inverse = false;
		invisible = false;
		frozen = false;
		frozen_timer = 0;
        was_animation_reset = false;
	}
};

#endif // ST_CHARACTERSTATE_H
