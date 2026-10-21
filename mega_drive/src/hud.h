#ifndef HUD_H
#define HUD_H

#include <genesis.h>

// Player HP as 5 energy balls at the top-left (PC draw::show_hud modern
// style: draw_enery_ball at x=3, y=3, one ball per 20% in 4 fill steps).
void hud_init(void);

// Refresh the balls from the current hp. Cheap: only touched sprites get
// a new frame. Call once per frame with player.hp.
void hud_update(u8 hp);

// Boss HP row: 5 balls at RES_W-55 (PC draw.cpp:1237, boss_hp_ball frames).
// hp < 0 hides the row (PC set_boss_hp(-99)); fill = hp / BOSS_INITIAL_HP.
// Call once per frame.
void hud_boss(s16 hp);

// Weapon gauge row: 5 balls at x=62 (PC draw.cpp:1217). Hidden while the
// default weapon is selected (weapon 0); fill = ammo / PLAYER_INITIAL_HP.
void hud_weapon(u8 weapon, u8 ammo);

// Hide/show the HP rows (in-game menu overlay).
void hud_set_visible(bool visible);

#endif
