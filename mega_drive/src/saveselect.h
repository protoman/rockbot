#ifndef SAVESELECT_H
#define SAVESELECT_H

#include <genesis.h>

// PC scenesLib::select_save (sceneslib.cpp:755): 5 slots, d-pad to move,
// START/JUMP confirms, ATTACK cancels. For a NEW GAME any slot is allowed
// (it is cleared before starting); for LOAD GAME empty slots are rejected.
// Returns the slot 0..4, or -1 when the player backs out.
s16 save_selector(bool is_new_game);

#endif
