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

// Charged shot: level 1 = semi (damage 2), 2 = full (damage 3). 16x16
// pulsing sprite from res/shot_charge.png (character.cpp:735-738).
void shot_charge_fire(s16 x, s16 y, s16 dir, u8 level);

// Move shots, wall/screen cull, enemy hit tests. After enemies_update().
void shot_update(void);

#endif
