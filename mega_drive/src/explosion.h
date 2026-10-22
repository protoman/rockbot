#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <genesis.h>

// Enemy death puff (PC classmap.cpp:2232): when a regular enemy dies the PC
// adds images/animations/explosion_32.png at the NPC position with an (-8,-8)
// adjust — 3 frames of 32x32, 80ms per frame, repeated twice (~0.5s).
#define EXPLOSION_MAX 6

// Start a puff centred on (x, y) — the dead enemy's position, not its sprite
// origin; the (-8,-8) PC adjust is applied here. Silently ignored when the
// pool is full or the position is off screen (PC checks position.y < RES_H).
void explosion_spawn(s16 x, s16 y);

// Advance ages/frames and release finished puffs. Call once per frame before
// SPR_update().
void explosion_update(void);

// Release every live puff (stage_load / leaving a stage).
void explosion_reset(void);

// Boss-death ring (PC game::classic_style_draw_boss_death): eight explosions
// leave (cx, cy) and travel outward until they cross the screen. Also release
// the ring sprites, so call it from the stage teardown path.
void explosion_ring_start(s16 cx, s16 cy);
// Advance the ring one frame; FALSE once it finished (sprites released).
bool explosion_ring_update(void);
void explosion_ring_reset(void);

#endif
