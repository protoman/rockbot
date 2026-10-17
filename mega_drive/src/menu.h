#ifndef MENU_H
#define MENU_H

#include <genesis.h>

// PC in-game menu (class_config::execute_ingame_menu, class_config.cpp:290;
// panel drawn by draw::draw_in_game_menu, draw.cpp:855):
//   START toggles the pause menu, JUMP also closes it, DASH = LEAVE STAGE.
// MD renders the same layout with a black panel + font + the player
// portrait sprite (the 320x224 PC background image does not fit in the
// spare stage VRAM). Weapons: only the buster exists so far; the CONFIG
// entry is drawn but inert (no config system yet).
// Returns TRUE when the player picked LEAVE STAGE.
bool menu_open(void);

#endif
