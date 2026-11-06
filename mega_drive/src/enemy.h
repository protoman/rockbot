#ifndef ENEMY_H
#define ENEMY_H

#include <genesis.h>

#include "stage_data.h"

// Stage 10 map 2 spawns 27 enemies; the old cap of 16 silently dropped 11.
#define ENEMY_MAX 32

// PC shareddata: TOUCH_DAMAGE_SMALL (walk into an enemy).
#define TOUCH_DAMAGE_SMALL 2
#define TOUCH_DAMAGE_BIG   4

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
void enemies_freeze(u8 weapon);
// TRUE while a player FREEZE effect (DYNAMITE FLASH) is running.
bool enemies_freeze_active(void);
// QUAKE: damages every on-screen enemy standing on the ground (PC
// damage_ground_npcs); called each frame while the quake shot lives.
void enemies_quake_hit(u8 weapon);

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
// Live enemy count (profile line).
u8 enemies_live(void);
// Body box + facing of enemy `index` (CHAIN projectiles follow their owner).
bool enemies_get_box(u8 index, s16 *x, s16 *y, s16 *w, s16 *h, s8 *dir);
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
void enemies_anim_hold(bool hold);   // room slide: freeze / resume the enemies' animation

// Blocking horizontal scroll: re-place every live enemy sprite from its world
// position (the camera moved, the enemies did not).
void enemies_sync_sprites(void);

#endif
