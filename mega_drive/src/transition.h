#ifndef TRANSITION_H
#define TRANSITION_H

#include <genesis.h>

// PC room change (game::transition_screen, game.cpp:1067): the current screen
// slides out and the destination screen slides in, vertically, in one motion.
//
// The two screens are stacked into a RAM tilemap (40x56 plane cells), the
// destination screen's tiles are copied into the free VRAM block above the
// animated-tile slots, and the visible 40x28 slice is rewritten each frame
// while the offset moves. Enemies/objects stay frozen (PC copies a bitmap)
// and are shifted with the outgoing screen; the player rides the incoming one.
//
//   from_map      : map currently loaded (its tileset/bg/anim are resident)
//   to_map        : destination map
//   to_cam_x      : camera X the destination screen is shown at
//                   (stage_arrival_camera_x)
//   player_x/y    : player's destination WORLD position (player_place args)
//   top_to_bottom : TRUE when the destination lies below (origin tile_y >
//                   dest tile_y, PC TRANSITION_TOP_TO_BOTTOM)
//
// Returns FALSE when the destination screen's tiles do not fit in the free
// VRAM block; the caller must then fall back to the palette fade. On TRUE the
// destination screen is the one on screen and the caller must finish the room
// change: stage_load(to_map), player_place(player_x, player_y),
// stage_arrival_camera(), player_sync_sprite().
//
// `old_shift`: the destination camera cannot always put the arrival where the
// player stands (map edge, scroll lock), so the OUTGOING screen is moved by the
// residual instead (dest screen X of the player minus his old screen X): the two
// ladders stay on one column through the slide. The outgoing screen is leaving
// and the player belongs to the incoming one, so only the old half moves.
bool transition_scroll(u8 from_map, u8 to_map, s16 to_cam_x,
                       s16 player_x, s16 player_y, bool top_to_bottom,
                       s16 old_shift);

// Diagnostic: why the last transition fell back to the fade.
// 1 same map, 2 no VRAM, 3 MAP_create, 4 unpack, 5 >256 tiles, 6 no room.
extern u8 transition_fail_code;
extern u8 transition_fail_map;
extern u16 transition_fail_free_kb;

// Scratch RAM lent while no transition runs (contents are not preserved).
void *transition_scratch_tiles(u16 *bytes);
void *transition_scratch_cells(u16 *bytes);

// Scratch for building one screen's tile map before it is written to the plane.
// The screens used to own a private 40x28 buffer each (10 x 2240 B of BSS, ~22 KB of the
// ~20 KB heap budget - BUG-045: the DMA queue allocation failed). They are never on screen
// together and never during a room change, so they share the transition's cell buffer.
// NULL when `words` does not fit (the caller skips the draw).
static inline u16 *screen_cells(u16 words)
{
    u16 bytes;
    u16 *p = (u16 *)transition_scratch_cells(&bytes);
    return ((u32)words * 2 <= bytes) ? p : (u16 *)0;
}

#endif
