#ifndef EFFECTS_H
#define EFFECTS_H

#include <genesis.h>

// Transient sprite effects the PC blits straight onto the game screen. Each
// one is a converted strip drawn on PAL0 (like explosion.c), so they share the
// enemy/UI palette line.
//
//   dust      character.cpp:1681  — when a slide/dash starts, 8x8, 160 ms,
//             three frames, mirrored by the facing. The PC art is 8x16 per
//             frame with the bottom half empty; only the painted 8 rows are
//             shipped (the PC's st_size(8, 8) does the same).
//   splash    projectilelib.cpp:129/851 — 32x23 art, 80 ms, two rounds; the
//             given y is the water surface, with 8 px of the art below it.
//   teleport  draw.cpp:429 (object.cpp:520) — 32x32, 100 ms, two frames,
//             shown while a teleporter is warping.
//   bubble    classmap.cpp:375 — one 4x4 bubble, rises 2 px per frame with a
//             4 px left/right wobble, gone after 3 s or when it leaves water.

// Slide/dash dust at the player's feet (x, y = top-left of the 8x8 art).
void fx_dust(s16 x, s16 y, s8 dir);

// Water splash at a water surface (y = the surface, not the art's top).
void fx_splash(s16 x, s16 y, s8 dir);

// Teleport beam at (x, y), art top-left.
void fx_teleport(s16 x, s16 y);

// Call every frame while the player is inside water at (x, y) — the PC calls
// classMap::add_bubble_animation there. It throttles itself to one bubble per
// 3 s, so calling it every frame is correct.
void fx_water_bubble(s16 x, s16 y);

// Advance ages/frames. Call once per frame before SPR_update().
void effects_update(void);

// Release every live effect (stage_load / leaving a stage).
void effects_reset(void);

#endif
