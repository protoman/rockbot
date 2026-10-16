#ifndef STAGE_H
#define STAGE_H

#include <genesis.h>

#include "stage0_data.h"

// The stage tilemap lives on BG_A. BG_B carries the parallax background.
#define STAGE_PLANE BG_A
#define STAGE_PAL   PAL1

// Parallax background: own palette (PAL3, reloaded on stage_load) and a
// fixed VRAM block. Layout: system 0..15, stage tilesets from
// TILE_USER_INDEX (biggest stage-0 tileset = 127 tiles -> ends 143),
// bg tiles at 144 (bg0 = 768 tiles -> ends 911), animated-tile frames
// at 912+ (TILE_ANIM_BASE in stage0_data.h, 1 slot -> ends 936), sprite
// engine 1312..1951 (SPR_initEx(640) at TILE_SPRITE_INDEX = 1952-640),
// font 1952..2047.
#define BG_PAL        PAL3
#define TILE_BG_INDEX 144

extern Map *stage_map;
extern Vect2D_s16 camera;
extern u8 stage_current_map;

// Load one map of the stage (0..STAGE0_MAP_COUNT-1) and reset the camera.
void stage_load(u8 map_index);

// Camera follow + plane scroll. Call once per frame before SPR_update().
void stage_update_camera(s16 focusX, s16 focusY);

// Animated tiles: advance frame timers, DMA new frames, rewrite visible
// cells. Call once per frame (60fps) in the game loop.
void stage_anim_update(void);

// Force a complete MAP redraw. Call after the camera jumps (respawn, map link).
void stage_force_redraw(void);

// Direct camera set for a link arrival (PC set_scrolling), divider-safe.
void stage_arrival_camera(s16 focusX, s16 focusY);

// Boss teleporter (OBJ_STAGE_BOSS_TELEPORTER, PC game::object_teleport_boss
// game.cpp:2199 with must_return=false): reload dest_map, drop the player on
// the ground column of dest tile dest_tx (dest_ty unused — PC ignores it),
// divider-safe arrival camera. No fade/sfx yet.
void stage_teleport_boss(u8 dest_map, u8 dest_tx, u8 dest_ty);

// TRUE when the given world-space box overlaps a link origin tile of the
// current map. Fills the destination map index and pixel position.
bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y);

#endif
