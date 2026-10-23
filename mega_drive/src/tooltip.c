// Weapon-changed tooltip — see tooltip.h.
#include "tooltip.h"

#include "stage.h"
#include "weapon_data.h"

// PC graphicsLib::draw_weapon_changed_tooltip (graphicslib.cpp:1370) puts the
// name at pixel (34, RES_H-22); the MD screen is 224 px tall (28 rows), so
// that is tile 4, row 25.
#define TOOLTIP_TILE_X 4
#define TOOLTIP_TILE_Y 25

static const char *text;
static u8 pending;                  // selected, not drawn yet
static u8 drawn;                    // label is in the plane
static u8 frames;                   // frames left before the redraw

void tooltip_show(u8 weapon_n)
{
    if (weapon_n >= WEAPON_N) return;
    text = weapon_data[weapon_n].name;
    pending = TRUE;
    drawn = FALSE;
}

void tooltip_hide(void)
{
    text = NULL;
    pending = FALSE;
    drawn = FALSE;
    frames = 0;
}

void tooltip_update(void)
{
    if (!pending) return;

    if (!drawn) {
        // The plane holds world tiles and wraps at 64 tiles, so the screen
        // position becomes (camera tile + screen tile) & 63 (dialogs.c:180
        // does the same for its panels).
        u16 tx = (u16)(((camera.x >> 3) + TOOLTIP_TILE_X) & 63);
        u16 ty = (u16)(((camera.y >> 3) + TOOLTIP_TILE_Y) & 31);
        u16 pal = VDP_getTextPalette();
        VDP_setTextPalette(PAL0);    // PAL0 entry 15 is the font white in-game
        VDP_drawTextBG(STAGE_PLANE, text, tx, ty);
        VDP_setTextPalette(pal);
        drawn = TRUE;
        frames = TOOLTIP_FRAMES;
        return;
    }

    if (--frames) return;

    // Gone: rebuild the plane so the label's cells get their map art back.
    // One full redraw per tooltip, never per frame.
    pending = FALSE;
    drawn = FALSE;
    text = NULL;
    stage_force_redraw();
}
