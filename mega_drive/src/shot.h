#ifndef SHOT_H
#define SHOT_H

#include <genesis.h>

// game_projectile_list.dat record 0 "Normal": max_shots = 3.
#define SHOT_MAX 4   // P3 (BETA) can have 4 on screen

// Release all shots (called from stage_load).
void shots_reset(void);

// Fire from (x, y) with dir = facing (-1 / +1). Ignored while the cooldown
// runs or all SHOT_MAX slots are busy.
void shot_fire(s16 x, s16 y, s16 dir, s16 dy);

// Charged shot (PC projectiles 11 / 10): level 1 = semi (charged_semi.png,
// 16x16, damage 2), 2 = full (charged_shot.png, 31x30, damage 3); both
// LINEAR at speed 3, 2 frames at 100ms (character.cpp:735-738).
void shot_charge_fire(s16 x, s16 y, s16 dir, u8 level);

// Move shots, wall/screen cull, enemy hit tests. After enemies_update().
void shot_update(void);

// Hide/show every live shot sprite (in-game menu overlay).
void shots_set_visible(bool visible);

#endif
