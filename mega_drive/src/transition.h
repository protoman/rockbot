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
bool transition_scroll(u8 from_map, u8 to_map, s16 to_cam_x,
                       s16 player_x, s16 player_y, bool top_to_bottom);

#endif
