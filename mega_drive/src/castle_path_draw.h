#ifndef CASTLE_PATH_DRAW_H
#define CASTLE_PATH_DRAW_H

#include <genesis.h>
#include "castle_path_data.h"

// PC draw::draw_castle_path (graphic/draw.cpp:1048) as a pure step generator:
// no VDP, so tests/host/castle_path_test.c compares it with an independent
// transliteration of the PC code (tools/convert_castle_path.py).
//
// A segment from castle_points[seg] to castle_points[seg + 1] is drawn as a
// vertical run (v iterations, towards the end point's Y) followed by a
// horizontal run (h iterations). Each iteration is three rectangle fills in PC
// screen coordinates (240 lines; subtract CASTLE_PATH_CROP_Y for the port).
typedef void (*CastlePlotRect)(void *ctx, s16 x, s16 y, u8 w, u8 h, u8 color);

// Number of vertical and horizontal iterations of segment `seg`.
void castle_seg_info(u8 seg, u16 *v, u16 *h);

// Run iteration `i` (0 .. v + h - 1) of segment `seg`.
void castle_seg_iter(u8 seg, u16 i, CastlePlotRect plot, void *ctx);

#endif
