#ifndef CASTLE_PATH_H
#define CASTLE_PATH_H

#include <genesis.h>

// Castle intro path overlay (PC classic_style_castle_intro): the line between the
// castle's stage points, drawn on BG_A over the castle art on BG_B. `idx` is the
// castle stage index (0 = first castle stage): segments 0..idx-1 appear at once,
// segment idx is drawn over ~1s like the PC (CASTLE_PATH_DURATION 1000ms), and a
// marker is stamped on every reached point. The caller has loaded the castle art
// and its four palette lines; this only uses PAL0 entries (grey/dark/marker colours).
void castle_path_run(u8 idx);

#endif
