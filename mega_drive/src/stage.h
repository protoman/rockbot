#ifndef STAGE_H
#define STAGE_H

#include <genesis.h>

#include "stage_data.h"

// ---- sprite-engine generation ------------------------------------------
// SPR_end()/SPR_initEx() destroy the sprite pool, and SGDK leaves the freed
// Sprite structs' bytes (status ALLOCATED, prev/next links) in the heap block
// the new pool re-uses. A module that still holds a handle across that call
// and later SPR_releaseSprite()s it unlinks whatever sprite now occupies the
// slot - the player vanished and SPR_update() then walked a corrupt list
// (line-1111 crash). Every pool recreate/end bumps spr_gen; modules remember
// the generation their handles belong to and forget them when it changed.
extern u16 spr_gen;
void sprites_begin(u16 vram);   // SPR_initEx + new generation
void sprites_end(void);         // SPR_end + new generation

// Release a handle only if it belongs to the current sprite-engine generation
// (then always clear it).
static inline void spr_release_guarded(Sprite **p, u16 seen)
{
    if (*p && seen == spr_gen) SPR_releaseSprite(*p);
    *p = NULL;
}

// The stage tilemap lives on BG_A. BG_B carries the parallax background.
#define STAGE_PLANE BG_A
#define STAGE_PAL   PAL1

// Parallax background palette (PAL3, reloaded on stage_load).
//
// VRAM layout is computed PER MAP at stage_load: the stage tileset at
// TILE_USER_INDEX, the parallax tiles right after it, the animated frames
// after those, then SPR_TOOLTIP_TILE_BASE (12 tiles for the weapon tooltip),
// then the sprite pool, font at 1440..1535. The old fixed blocks (tileset
// <=144, bg at 160, anim at 520) overflowed on FROZEN LAIR map 1, whose
// rendered map uses 202 unique 8x8 tiles; deriving the bases from the actual
// resource sizes keeps every stage inside the sprite pool.

// Fixed tile blocks the game writes by hand must live OUTSIDE the sprite
// pool: the engine allocates from TILE_SPRITE_INDEX upward, so a hard-coded
// index inside the pool silently overwrites whatever sprites own those tiles.
// BUG-041: the weapon tooltip loaded its art at 912..923, which is where the
// HUD bars had landed once the stage's objects/enemies had taken the low
// tiles, so the HUD drew the tooltip box; the boss dialog panel was parked in
// the same range (it clobbers the HUD bars in boss rooms).
//
// Layout below the pool (the stage layout ends at <=573, tools/check_vram.py):
//   672..684  weapon tooltip  (SPR_TOOLTIP_TILE_BASE, 13 tiles: 9 box + 4 icon)
//   685..701  dialog panel    (dialogs.c DLG_PANEL_TILE_BASE, 17 tiles)
//   702       config screen   (config.c CFG_BLACK_TILE, 1 tile - reachable
//                               from the in-game menu, so the stage is live)
//   703..1439 sprite pool     (TILE_FONT_INDEX - SPRITE_VRAM_TILES)
#define SPR_TOOLTIP_TILE_BASE 672
#define SPR_DLG_TILE_BASE     685
#define SPR_CFG_BLACK_TILE    702
#define SPRITE_VRAM_TILES     737
#define BG_PAL        PAL3
#define STAGE_ANIM_SLOT_MAX 17
extern u16 stage_bg_tile_base;    // current map: first parallax tile
// Blank (all index-0) tile used for cells the parallax art does not cover.
u16 stage_sky_tile(void);
extern u16 stage_anim_tile_base;  // current map: first animated-frame tile
u16 stage_layout_bg(u8 map);      // first parallax tile of `map`
u16 stage_layout_anim(u8 map);    // first animated-frame tile of `map`

// PC character::process_special_map_points -> game::horizontal_screen_move:
// walking into a SCROLL_LOCK column slides the screen one width over ~40
// frames and carries the player 40px past the lock, so he can continue into
// the next section instead of being pinned at the screen edge.
void stage_autoscroll(s8 dir);

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
// One animated slot owns ONE 2x2 tile block: a frame change overwrites those
// 4 tiles in VRAM (single DMA) instead of pointing the map cells at a
// frame-specific block, which used to rewrite the tilemap of every visible
// cell of the slot on every frame (15 ms/frame on SEAHORSE).
#define TILE_ANIM_STRIDE 4

extern Map *stage_map;
extern Vect2D_s16 camera;
extern u8 stage_current_map;
// Optional camera X for the next stage_load()'s first full draw. The room
// transition already knows the destination camera; starting the new map at 0
// and fixing it afterwards shows one frame of the map's left edge (the flash
// seen right after a Mummy stair). -1 = normal start at 0.
extern s16 stage_pending_camera_x;
// TRUE for the stage_load() immediately following a room transition: the
// transition already shows the destination, so keep the display enabled to
// avoid a black/one-frame flash while the real map is rebuilt.
extern bool stage_load_in_place;

// Terrain value at a map tile (out of range: solid at the sides, sky below).
u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY);

// Load one map of the stage (0..STAGE0_MAP_COUNT-1) and reset the camera.
void stage_load(u8 map_index);

// FALSE when the last stage_load() could not create the map (no heap). The
// caller must not enter the stage loop in that case (BUG-040).
bool stage_map_ready(void);

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

// PC game::horizontal_screen_move: slide the view one screen sideways for a
// boss door. The player is pushed ~2.5 tiles in the same direction (he ends
// near the left edge of the new screen), so the caller must not carry him.
void stage_horizontal_screen_move(s16 dir);

// Re-load the stage tileset palette (PAL1) after a full-screen overlay
// (in-game menu) borrowed the palette line.
void stage_restore_palette(void);
// Re-apply the colourcycle band (PAL3) after a temporary palette use.
void stage_restore_cycle(void);

// Re-draw the parallax background plane (image, palette, scroll) after an
// overlay borrowed the background tile block.
void stage_redraw_bg(void);

// Current scroll of the parallax plane (BG_B), mirrored by stage.c so overlay
// art can place cells on that plane (the tooltip's opaque backing, BUG-042).
extern s16 stage_bg_hscroll;
extern s16 stage_bg_vscroll;
// PC E_BG_SCROLL_MODE: background offset that does not follow the camera.
s16 stage_bg_auto_hscroll(u8 map);
s16 stage_bg_auto_vscroll(u8 map);
void stage_bg_tick(void);       // once per frame (auto-scroll phase)
void stage_redraw_anim(void);

// Release the stage map (its deferred vblank updates would repaint the
// planes after a full-screen overlay or the title took over).
void stage_release_map(void);

// Direct camera set for a link arrival (PC set_scrolling), divider-safe.
void stage_arrival_camera(s16 focusX, s16 focusY);
// Same, but with the camera X the transition already computed, so the first
// frame after stage_load uses exactly the same scroll (no one-frame shift).
void stage_arrival_camera_at(s16 cam_x, s16 focusY);

// The camera X stage_arrival_camera() picks for `focus_x` on `map`, without
// touching the plane. The room transition needs it before the destination
// map is loaded, to know which screen to slide in.
s16 stage_arrival_camera_x(u8 map, s16 focus_x);
// ... with `focus_x` at screen column `screen_x` (room changes keep the
// player's screen X, see stage.c).
s16 stage_arrival_camera_keep(u8 map, s16 focus_x, s16 screen_x);

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
bool stage_link_band_hit(s16 left, s16 top, s16 right, s16 bottom);

// Horizontal offset of the player centre inside the band of the link that
// just fired (0..size*16). Wide scroll links use it to keep the player at the
// same place relative to the destination band (PC game.cpp:762).
extern s16 stage_link_band_off;

bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y, u8 *to_trans);

#endif
