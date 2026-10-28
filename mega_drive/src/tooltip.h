#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <genesis.h>

// Weapon-changed tooltip (PC draw::add_weapon_tooltip / show_weapon_tooltip,
// draw.cpp:833/841): after the player picks a weapon in the menu the PC shows
// a frame box, the weapon icon and its name for 1500 ms in the lower left
// corner (graphicslib.cpp:1370 draw_weapon_changed_tooltip).
//
// On the MD the box is written into the stage plane at the current camera
// position (there is no screen-fixed text plane). It therefore belongs to the
// map for its lifetime; when it expires the plane is rebuilt with
// stage_force_redraw(), which is exactly the trade-off the plan described.
// The box/tiles come from tools/convert_tooltip.py (src/tooltip_data.*).
//
// The art's colours are fitted into PAL3 next to the parallax's own (reused when
// equal, else placed in entries the parallax leaves free, else the nearest
// parallax colour), so the background keeps its colours; the entries written
// are restored at expiry.

#define TOOLTIP_FRAMES 90       // 1500 ms at 60 fps (PC _weapon_tooltip_timer)

// Show the name of weapons[weapon_n] on the next stage frame.
void tooltip_show(u8 weapon_n);

// Advance the tooltip; call once per frame from the stage loop.
void tooltip_update(void);

// Forget the tooltip without touching the plane (a full redraw is coming).
void tooltip_hide(void);

#endif
