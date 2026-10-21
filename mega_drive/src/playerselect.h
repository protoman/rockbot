#ifndef PLAYERSELECT_H
#define PLAYERSELECT_H

#include <genesis.h>

// PC scenesLib::select_player (sceneslib.cpp:546): full-screen
// player_selection bg + the per-player portrait at y=50, name under it,
// left/right to cycle, START to confirm. Returns the chosen player 0..3.
u8 player_select_run(void);

#endif
