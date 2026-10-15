#ifndef STAGE_H
#define STAGE_H

#include <genesis.h>

#include "stage0_data.h"

// The stage tilemap lives on BG_A. BG_B is left free for parallax later.
#define STAGE_PLANE BG_A
#define STAGE_PAL   PAL1

extern Map *stage_map;
extern Vect2D_s16 camera;
extern u8 stage_current_map;

// Load one map of the stage (0..STAGE0_MAP_COUNT-1) and reset the camera.
void stage_load(u8 map_index);

// Camera follow + plane scroll. Call once per frame before SPR_update().
void stage_update_camera(s16 focusX, s16 focusY);

// Force a complete MAP redraw. Call after the camera jumps (respawn, map link).
void stage_force_redraw(void);

// TRUE when the given world-space box overlaps a link origin tile of the
// current map. Fills the destination map index and pixel position.
bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y);

#endif
