#ifndef STAGE_H
#define STAGE_H

#include <genesis.h>

#include "stage_data.h"

// The stage tilemap lives on BG_A. BG_B carries the parallax background.
#define STAGE_PLANE BG_A
#define STAGE_PAL   PAL1

// Parallax background palette (PAL3, reloaded on stage_load).
//
// VRAM layout is computed PER MAP at stage_load: the stage tileset at
// TILE_USER_INDEX, the parallax tiles right after it, the animated frames
// after those, and the sprite pool at TILE_SPRITE_INDEX (1440 - SPR_initEx
// size; 504 -> 936..1439), font at 1440..1535. The old fixed blocks (tileset
// <=144, bg at 160, anim at 520) overflowed on FROZEN LAIR map 1, whose
// rendered map uses 202 unique 8x8 tiles; deriving the bases from the actual
// resource sizes keeps every stage inside the sprite pool.
#define BG_PAL        PAL3
#define STAGE_ANIM_SLOT_MAX 17
extern u16 stage_bg_tile_base;    // current map: first parallax tile
extern u16 stage_anim_tile_base;  // current map: first animated-frame tile
u16 stage_layout_bg(u8 map);      // first parallax tile of `map`
u16 stage_layout_anim(u8 map);    // first animated-frame tile of `map`

// Colourcycle (stage0_plan.md): animated cells drawn as a CRAM rotation
// instead of tile frames. Their art carries PAL3-relative pixel values, so the
// runtime switches the cell's palette line to this and rotates the band in
// PAL3. TILE_CYCLE_STABLE must match CYCLE_STABLE in tools/convert_stage.py.
#define STAGE_PAL_CYCLE 3
// PAL_setColor()/PAL_getColor() take a GLOBAL CRAM index (0..63): the lines are
// 16 entries each, so PAL3's entry N is CRAM 48 + N.
#define TILE_CYCLE_CRAM ((STAGE_PAL_CYCLE) << 4)
#define TILE_CYCLE_STABLE 15
#define STAGE_CYCLE_MAX 6

// Animated-tile frame slots: 1 slot per animation, 6 frames x 4 tiles,
// loaded at stage_anim_tile_base + slot*stride. The block must stay below the
// sprite pool — check_vram.py verifies the per-map sum.
#define TILE_ANIM_STRIDE 24

extern Map *stage_map;
extern Vect2D_s16 camera;
extern u8 stage_current_map;

// Terrain value at a map tile (out of range: solid at the sides, sky below).
u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY);

// PC projectile::check_map_collision (projectilelib.cpp:1193): every terrain
// except UNBLOCKED and WATER stops a projectile — so ladders (STAIR), spikes,
// doors, conveyor and the scroll-lock columns block shots too, not only SOLID
// tiles. (The PC's projectile data has no "passes through walls" flag.)
static inline bool terrain_blocks_projectile(u16 t)
{
    return t != TERRAIN_UNBLOCKED && t != TERRAIN_WATER;
}

// Load one map of the stage (0..STAGE0_MAP_COUNT-1) and reset the camera.
void stage_load(u8 map_index);

// Camera follow + plane scroll. Call once per frame before SPR_update().
void stage_update_camera(s16 focusX, s16 focusY);

// Animated tiles: advance frame timers, DMA new frames, rewrite visible
// cells. Call once per frame (60fps) in the game loop.
void stage_anim_update(void);

// Advance the colourcycle band (a few CRAM writes per step, no tile DMA).
// Call once per frame next to stage_anim_update().
void stage_cycle_update(void);

// Force a complete MAP redraw. Call after the camera jumps (respawn, map link).
void stage_force_redraw(void);

// Re-load the stage tileset palette (PAL1) after a full-screen overlay
// (in-game menu) borrowed the palette line.
void stage_restore_palette(void);

// Re-draw the parallax background plane (image, palette, scroll) after an
// overlay borrowed the background tile block.
void stage_redraw_bg(void);

// Release the stage map (its deferred vblank updates would repaint the
// planes after a full-screen overlay or the title took over).
void stage_release_map(void);

// Direct camera set for a link arrival (PC set_scrolling), divider-safe.
void stage_arrival_camera(s16 focusX, s16 focusY);

// The camera X stage_arrival_camera() picks for `focus_x` on `map`, without
// touching the plane. The room transition needs it before the destination
// map is loaded, to know which screen to slide in.
s16 stage_arrival_camera_x(u8 map, s16 focus_x);

// Boss teleporter (OBJ_STAGE_BOSS_TELEPORTER, PC game::object_teleport_boss
// game.cpp:2199 with must_return=false): reload dest_map, drop the player on
// the ground column of dest tile dest_tx (dest_ty unused — PC ignores it),
// divider-safe arrival camera. No fade/sfx yet.
void stage_teleport_boss(u8 dest_map, u8 dest_tx, u8 dest_ty);

// TRUE when the given world-space box overlaps a link origin tile of the
// current map. Fills the destination map index and pixel position, plus the
// transition direction (TRANS_T2B / TRANS_B2T) the room transition needs.
#define TRANS_T2B 0   // destination lies below (PC TRANSITION_TOP_TO_BOTTOM)
#define TRANS_B2T 1   // destination lies above (PC TRANSITION_BOTTOM_TO_TOP)
bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y, u8 *to_trans);

#endif
