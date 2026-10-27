#ifndef ESHOT_H
#define ESHOT_H

#include <genesis.h>

// Enemy projectiles (PC classnpc projectile_list). Straight-line shots only
// for stage 0 data (Normal + Semi-Charged are TRAJECTORY_LINEAR).
#define ESHOT_MAX 8

// Release all (called from stage_load with the other resets).
void eshots_reset(void);

// Fire projectile proj_id from (x, y) moving dir (-1/+1). FALSE if the
// per-type max_shots limit is reached (PC throw_projectile).
bool eshot_fire(s16 x, s16 y, s16 dir, u8 proj_id);
// Aimed variant (PC TARGET_DIRECTION): travels toward the player diagonally.
bool eshot_fire_aimed(s16 x, s16 y, u8 proj_id);
// Index of the enemy whose AI is running: every eshot_fire() call made from it
// tags the projectile with this owner, which CHAIN (follows its owner, PC
// projectilelib.cpp:643) and BOMB_RAIN (spawns children from it) need.
extern u8 eshot_current_owner;

// TRUE while enemy `owner` is shielded by its own held CENTERED projectile.
bool eshot_owner_has_circle(u8 owner);

// Move, tile/edge culling, player collision -> player_take_damage.
// Call once per frame after enemies_update().
void eshots_update(void);

// Hide/show every live enemy-shot sprite (in-game menu overlay).
void eshots_set_visible(bool visible);

// PC projectile QUAKE screen adjust (+/-2px, QUAKE_SCREEN_MOVE) while an enemy
// QUAKE projectile is alive; 0 otherwise. The player's own quake shots come
// from shot_shake_x(); stage_update_camera adds both.
s16 eshots_shake_x(void);

#endif
