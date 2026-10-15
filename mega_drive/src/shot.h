#ifndef SHOT_H
#define SHOT_H

#include <genesis.h>

// game_projectile_list.dat record 0 "Normal": max_shots = 3.
#define SHOT_MAX 3

// Release all shots (called from stage_load).
void shots_reset(void);

// Fire from (x, y) with dir = facing (-1 / +1). Ignored while the cooldown
// runs or all SHOT_MAX slots are busy.
void shot_fire(s16 x, s16 y, s16 dir);

// Move shots, wall/screen cull, enemy hit tests. After enemies_update().
void shot_update(void);

#endif
