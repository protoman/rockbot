#ifndef STAGE_SELECT_H
#define STAGE_SELECT_H

#include <genesis.h>

// Classic-style stage select (PC stage_select::pick_stage_classic_style,
// stage_select.cpp:357). Shows the 3x3 face grid, the blinking cursor and
// the BOSS/STAGE names, and returns the chosen stage index (1..8). The
// centre cell is the castle: once all 8 bosses are beaten it returns the
// first unbeaten castle #1 stage (9..13, PC get_last_castle_stage).
u8 stage_select_run(u8 player, u16 stage_mask);  // player 0..3, beaten bits

void stage_select_reset_cursor(void);            // new game / load: cursor back to the centre

#endif
