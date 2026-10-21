#ifndef OBJECT_H
#define OBJECT_H

#include <genesis.h>

#include "stage_data.h"

#define OBJECT_MAX 24

// Temp test-room overlay: PX/PY/CX line + per-platform state lines.
// Remove together with the test-room spawns (stage0_plan backlog).
#define DEBUG_OBJECTS 1

// PC OBJ_STATE (defines.h:417)
#define OBJ_STATE_STAND  0
#define OBJ_STATE_MOVE   1
#define OBJ_STATE_RETURN 2

// PC INITIAL_ACTIVATION_DELAY 220ms -> 13 frames at 60fps (object.cpp:36)
#define OBJ_START_DELAY 13

// Release the current map's objects and spawn the ones listed for
// stage_current_map (called from stage_load).
void objects_reset(void);

// Platform state machines (fall/fly), pickups, checkpoint, animation.
// Call once per frame BEFORE player_update (carry moves the player first).
void objects_update(void);

// Descending-player query: first fall/fly platform whose top the box crosses
// this frame (head ended at/above it, feet below, any horizontal touch —
// inclusive X so a side-clamped corner entry still catches). Returns the top
// Y or -1. Does not start the platform; call objects_player_land(l, r, y)
// when it
// wins against the terrain landing.
s16 objects_land_check(s16 l, s16 r, s16 head_new, s16 feet_new);

// The player landed on the platform whose top is at `top`: start it
// (STAND -> MOVE + activation delay), as PC does on the landing collision.
void objects_player_land(s16 left, s16 right, s16 top);

// TRUE when a fall/fly platform top is exactly under the feet (standing
// on a platform, including one that is carrying the player).
bool objects_stand_check(s16 l, s16 r, s16 feet);

// TRUE when the player box at player-x nx would overlap a platform body
// (PC COLlISION_MODE_XY: solid from the side). *out_x receives the player-x
// that clamps flush against the blocking face.
bool objects_block_x(s16 nx, s16 t, s16 b, s16 dx, s16 *out_x);

// Rising into a platform bottom: returns the player-y that stops the head
// flush under it, or -1 when free. feet = current (pre-move) player bottom.
s16 objects_head_block(s16 l, s16 r, s16 head_new, s16 feet);

// Debug: row tile_y = "PXnnnn PYnnn CXnnnn", row tile_y+1 =
// "A<state><started>:<y> ..." for up to 3 platforms.
void objects_debug_text(u8 tile_y, s16 px, s16 py, s16 camx);

// Hide/show every live object sprite (in-game menu overlay).
void objects_set_visible(bool visible);

// Room transition: redraw every live object sprite shifted down by `dy`
// screen pixels (the outgoing screen takes its objects with it).
void objects_shift_y(s16 dy);

#endif
