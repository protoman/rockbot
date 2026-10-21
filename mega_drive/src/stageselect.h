#ifndef STAGE_SELECT_H
#define STAGE_SELECT_H

#include <genesis.h>

// Classic-style stage select (PC stage_select::pick_stage_classic_style,
// stage_select.cpp:357). Shows the 3x3 face grid, the blinking cursor and
// the BOSS/STAGE names, and returns the chosen stage index (1..8). The
// centre cell is the castle/player slot and is inert until the castle is
// unlocked (not modelled yet).
u8 stage_select_run(u8 player);   // player 0..3 (player-select choice)

#endif
