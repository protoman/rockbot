#ifndef OBJECT_H
#define OBJECT_H

#include <genesis.h>

#include "stage_data.h"

// Stage 13 map 0 places 27 objects (the boss door is the last one); the old
// cap of 24 silently dropped it, so the DR. DESTRIN room had no door.
#define OBJECT_MAX 32

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

// Enemy death drop (PC classMap::drop_item): random pill at a world position,
// timed out after 4.5s like the PC object duration.
void objects_drop_item(s16 cx, s16 cy, bool subboss);

// Selected item weapon (PC add_coil_object / add_jet_object): COIL FROG
// (type 13, super-jump spring) or JET EAGLE (type 12, flying platform).
bool objects_spawn_weapon_item(bool jet);

// Platform state machines (fall/fly), pickups, checkpoint, animation.
// Call once per frame BEFORE player_update (carry moves the player first).
void objects_update(void);
// Player shot vs destructible wall; TRUE = consumed.
// `bomb`: the hit comes from an exploding bomb (the only thing that can
// break OBJ_DESTRUCTIBLE_WALL type 22 - PC classplayer.cpp:800).
bool objects_try_hit(s16 l, s16 t, s16 r, s16 b, u8 dmg, bool bomb);
// Solid map objects (blocks, doors) inside the box: enemies and the
// player hit-pushback use it the same way terrain does.
bool objects_block_box(s16 l, s16 t, s16 r, s16 b);

// Descending-player query: first fall/fly platform whose top the box crosses
// this frame (head ended at/above it, feet below, any horizontal touch —
// inclusive X so a side-clamped corner entry still catches). Returns the top
// Y or -1. Does not start the platform; call objects_player_land(l, r, y)
// when it
// wins against the terrain landing.
s16 objects_land_check(s16 l, s16 r, s16 head_new, s16 feet_new);

// COIL FROG spring: TRUE when a descending player (feet_old -> feet_new) steps
// on a frog; the caller then calls player_super_jump().
bool objects_spring_check(s16 l, s16 r, s16 feet_old, s16 feet_new);

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
// Returned by objects_head_block() when nothing stops the head. It must be below
// any reachable y: the room top is 0 and a jump may carry the head above it.
#define OBJ_NO_HEAD_BLOCK (-32768)
s16 objects_head_block(s16 l, s16 r, s16 head_new, s16 feet);

// Debug: row tile_y = "PXnnnn PYnnn CXnnnn", row tile_y+1 =
// "A<state><started>:<y> ..." for up to 3 platforms.
void objects_debug_text(u8 tile_y, s16 px, s16 py, s16 camx);

// TRUE while a boss door (type 30) still occupies the given tile column and
// has not been opened. The camera lock uses this live state instead of the
// static spawn list, so opening the door releases the scroll.
bool objects_boss_door_locked(s16 tile_x);
bool objects_boss_door_present(s16 tile_x);
void objects_reset_doors(void);

// Hide/show every live object sprite (in-game menu overlay).
void objects_set_visible(bool visible);
// Live object count (profile line).
u8 objects_live(void);

// Room transition: redraw every live object sprite shifted down by `dy`
// screen pixels (the outgoing screen takes its objects with it).
void objects_shift_y(s16 dy);

// Blocking horizontal scroll: re-place every live object sprite from its
// world position (the camera moved, the objects did not).
void objects_sync_sprites(void);

#endif
