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

// Move, tile/edge culling, player collision -> player_take_damage.
// Call once per frame after enemies_update().
void eshots_update(void);

// Hide/show every live enemy-shot sprite (in-game menu overlay).
void eshots_set_visible(bool visible);

#endif
