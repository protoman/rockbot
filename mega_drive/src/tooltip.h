#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <genesis.h>

// Weapon-changed tooltip (PC draw::add_weapon_tooltip / show_weapon_tooltip,
// draw.cpp:833/841): after the player picks a weapon in the menu the PC shows
// its name for 1500 ms in the lower left corner.
//
// On the MD the name is written into the stage plane at the current camera
// position (there is no screen-fixed text plane). The label therefore belongs
// to the map for its lifetime; when it expires the plane is rebuilt with
// stage_force_redraw(), which is exactly the trade-off the plan described.
// The PC also draws a small frame and the weapon icon there; the port has no
// art for those, so only the name is drawn.

#define TOOLTIP_FRAMES 90       // 1500 ms at 60 fps (PC _weapon_tooltip_timer)

// Show the name of weapons[weapon_n] on the next stage frame.
void tooltip_show(u8 weapon_n);

// Advance the tooltip; call once per frame from the stage loop.
void tooltip_update(void);

// Forget the tooltip without touching the plane (a full redraw is coming).
void tooltip_hide(void);

#endif
