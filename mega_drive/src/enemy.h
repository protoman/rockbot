#ifndef ENEMY_H
#define ENEMY_H

#include <genesis.h>

#include "stage0_enemies.h"

#define ENEMY_MAX 16

// PC shareddata: TOUCH_DAMAGE_SMALL (walk into an enemy).
#define TOUCH_DAMAGE_SMALL 2

// Release the current map's enemies and spawn the ones listed for
// stage_current_map (called from stage_load).
void enemies_reset(void);

// Patrol movement, gravity (fly = 0), animation, contact damage.
// Call once per frame after the camera update, before SPR_update().
void enemies_update(void);

// Shot AABB against the enemy hitboxes. TRUE = one enemy was hit
// (1 damage; the enemy dies when hp reaches 0).
bool enemies_try_hit(s16 left, s16 top, s16 right, s16 bottom);

#endif
