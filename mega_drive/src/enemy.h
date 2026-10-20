#ifndef ENEMY_H
#define ENEMY_H

#include <genesis.h>

#include "stage_data.h"

#define ENEMY_MAX 16

// PC shareddata: TOUCH_DAMAGE_SMALL (walk into an enemy).
#define TOUCH_DAMAGE_SMALL 2

// PC defines.h:438 (set by classnpc.cpp:707 for is_boss NPCs; the converter
// already writes it into the boss EnemyDef.hp).
#define BOSS_INITIAL_HP 36

// Release the current map's enemies and spawn the ones listed for
// stage_current_map (called from stage_load).
void enemies_reset(void);

// Patrol movement, gravity (fly = 0), animation, contact damage.
// Call once per frame after the camera update, before SPR_update().
void enemies_update(void);

// Shot AABB against the enemy hitboxes. TRUE = one enemy was hit.
// damage: 1 = normal, 2 = semi-charged, 3 = full-charged (piercing).
// Test a shot box against the enemies. `shot_dir` is the shot's travel
// direction (1 = right, -1 = left). Returns 0 = miss, 1 = hit (damage
// applied), 2 = blocked by a shield (the caller reflects the shot).
u8 enemies_try_hit(s16 left, s16 top, s16 right, s16 bottom, u8 damage,
                   s8 shot_dir, bool charged, u8 weapon);

// ---- stage boss (def->is_boss) ------------------------------------------
// Current HP for the HUD ball row; 0 when the boss is absent or dead.
u8 enemies_boss_hp(void);
// TRUE while the boss is alive and on screen (PC
// classMap::is_boss_on_extended_screen). Gates the HP row.
bool enemies_boss_on_screen(void);
// TRUE once the boss has died since this map's enemies_reset (PC
// classmap.cpp:2273 -> got_weapon()).
bool enemies_boss_died(void);
// Centre of the boss body (for the death explosion).
void enemies_boss_death_pos(s16 *x, s16 *y);
// Nearest live on-screen enemy centre (DESERT STORM FOLLOW shots).
bool enemies_nearest(s16 x, s16 y, s16 *tx, s16 *ty);
// PC classMap::subboss_alive_on_left: boss/sub-boss alive in the screen-wide
// band left of a boss door.
bool enemies_subboss_alive_left(s16 tile_x);

// Hide/show every live enemy sprite (in-game menu overlay).
void enemies_set_visible(bool visible);

// Room transition: redraw every live enemy sprite shifted down by `dy`
// screen pixels. The outgoing screen takes its enemies with it (the PC
// copies a frozen bitmap of it), so the caller passes the screen offset.
void enemies_shift_y(s16 dy);

#endif
