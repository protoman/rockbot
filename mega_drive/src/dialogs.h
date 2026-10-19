#ifndef DIALOGS_H
#define DIALOGS_H

#include <genesis.h>

// PC boss-room dialogs (game::map_present_boss -> dialogs::show_boss_dialog).
// The stage-start half of the dialog files (lines 0..29) is NOT used by the
// PC game at all; only the boss block (30..59) is shown, when the boss first
// appears in the arena.
void dialog_show_boss(u8 stage, u8 player);

#endif
