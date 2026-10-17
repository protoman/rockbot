#ifndef MENU_H
#define MENU_H

#include <genesis.h>

// PC in-game menu (class_config::execute_ingame_menu, class_config.cpp:290;
// panel drawn by draw::draw_in_game_menu, draw.cpp:855):
//   START toggles the pause menu, JUMP also closes it, DASH = LEAVE STAGE,
//   LEFT/RIGHT cycle the owned weapons (PC Up/Down/Left/Right + the weapon
//   grid). `weapon_mask` = owned weapons, bit n (see save_slot_weapon_mask).
// MD renders the same layout with a black panel + font + the player
// portrait sprite. The CONFIG entry is drawn but inert (no config system).
// Returns TRUE when the player picked LEAVE STAGE.
bool menu_open(u16 weapon_mask);

#endif
