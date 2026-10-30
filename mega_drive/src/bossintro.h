#ifndef BOSSINTRO_H
#define BOSSINTRO_H

#include <genesis.h>

// PC scenesLib::classic_style_boss_intro (sceneslib.cpp:681): full-screen
// boss_intro_bg, the stage boss sprite centred, the boss name near the
// bottom, ~6.4s (skippable with a button). Nothing at all - not the screen and
// not the jingle - for a stage already beaten (PC: game_save.stages[stage] != 0
// returns before the music, sceneslib.cpp:682).
//
// `beaten_mask` is the slot's completed-stage mask (save_slot_stages); the gate
// lives here, not in the caller, so the music cannot be reached without the
// presentation. Returns FALSE when the intro was skipped.
bool boss_intro_run(u8 stage, u16 beaten_mask);

#endif
