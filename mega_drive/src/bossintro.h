#ifndef BOSSINTRO_H
#define BOSSINTRO_H

#include <genesis.h>

// PC scenesLib::classic_style_boss_intro (sceneslib.cpp:681): full-screen
// boss_intro_bg, the stage boss sprite centred, the boss name near the
// bottom, ~6.4s (skippable with a button). Skipped for stages already
// beaten (PC: game_save.stages[stage] != 0).
void boss_intro_run(u8 stage);

#endif
