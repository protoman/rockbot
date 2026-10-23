// Weapon-changed tooltip — see tooltip.h.
#include "tooltip.h"

#include "stage.h"
#include "tooltip_data.h"
#include "weapon_data.h"

// The box/icon tiles live in the 12 tiles the sprite pool leaves free at
// SPR_TOOLTIP_TILE_BASE (stage.h): the sprite engine allocates from
// TILE_FONT_INDEX - SPRITE_VRAM_TILES upward, so a fixed write INSIDE that
// range overwrites whatever sprites own those tiles. BUG-041: loading the art
// at 912 (the old dialog-panel window) hit the HUD bars once the stage's
// objects had taken the low tiles, and the HUD then displayed the tooltip box.
#define TOOLTIP_TILE_BASE SPR_TOOLTIP_TILE_BASE
#define TOOLTIP_ICON_TILE (TOOLTIP_TILE_BASE + TOOLTIP_FRAME_TILE_COUNT)

// PC layout on the 224 px screen: box at (10,192), icon at (14,200), name at
// (34,200). Plane cells are 8 px, so the box starts on cell 1 (pixel 8), the
// icon on cells 1..2 and the name on cell 4 - the icon ends up 6 px left of
// the PC one, everywhere else matches.
#define TOOLTIP_COL      1
#define TOOLTIP_ROW      24
#define TOOLTIP_ICON_COL 1
#define TOOLTIP_ICON_ROW 25
#define TOOLTIP_TILE_X   4
#define TOOLTIP_TILE_Y   25

#define PAL3_CRAM 48            // PAL3 * 16

static const char *text;
static u8 weapon;                   // selected weapon, for the icon
static u8 pending;                  // selected, not drawn yet
static u8 drawn;                    // box + label are in the plane
static u8 owns_pal3;                // PAL3 holds the tooltip palette
static u8 frames;                   // frames left before the redraw
static u16 pal3_saved[16];

bool tooltip_owns_pal3(void)
{
    return owns_pal3;
}

static void pal3_save(void)
{
    for (u8 i = 0; i < 16; i++) pal3_saved[i] = PAL_getColor(PAL3_CRAM + i);
}

static void pal3_restore(void)
{
    PAL_setColors(PAL3_CRAM, pal3_saved, 16, DMA_QUEUE);
}

static void pal3_load(void)
{
    static u16 pal[16];
    for (u8 i = 0; i < 16; i++) pal[i] = tooltip_pal_fixed[i];
    if (weapon < TOOLTIP_ICON_N) {
        for (u8 i = 0; i < TOOLTIP_ICON_PAL_N; i++)
            pal[TOOLTIP_ICON_PAL_FIRST + i] =
                tooltip_icon_pal[weapon * TOOLTIP_ICON_PAL_N + i];
    }
    PAL_setPalette(PAL3, pal, DMA_QUEUE);
}

// Write one 8x8 cell of the box/icon art at screen cell (col,row). The plane
// holds world tiles and wraps at 64 cells, so the position becomes
// (camera tile + screen cell) & 63 - dialogs.c does the same for its panels.
static void plane_cell(VDPPlane plane, u16 tile, u8 col, u8 row)
{
    const u16 base = (u16)((camera.x >> 3) & 63);
    const u16 top = (u16)(((camera.y >> 3) + row) & 31);
    VDP_setTileMapXY(plane,
                     TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, tile),
                     (u16)((base + col) & 63), top);
}

static void cell_draw(u16 tile, u8 col, u8 row)
{
    plane_cell(STAGE_PLANE, tile, col, row);
}

// The box art's interior and the font's glyph background are palette index 0,
// and a BG pixel with index 0 is TRANSPARENT on the MD: the parallax showed
// through the box and through every character cell (BUG-042). The box is
// backed by a solid tile on the parallax plane; plane A then only carries the
// opaque parts (borders, icon ink, glyph ink).
static void back_fill_draw(void)
{
    const u16 tile = (u16)(TOOLTIP_TILE_BASE + TOOLTIP_FILL_TILE);
    for (u8 r = 0; r < TOOLTIP_FRAME_H; r++)
        for (u8 c = 0; c < TOOLTIP_FRAME_W; c++)
            plane_cell(BG_B, tile, (u8)(TOOLTIP_COL + c),
                       (u8)(TOOLTIP_ROW + r));
}

static void box_draw(void)
{
    VDP_loadTileData(tooltip_frame_tiles, TOOLTIP_TILE_BASE,
                     TOOLTIP_FRAME_TILE_COUNT, DMA_QUEUE);
    u16 i = 0;
    for (u8 r = 0; r < TOOLTIP_FRAME_H; r++)
        for (u8 c = 0; c < TOOLTIP_FRAME_W; c++, i++)
            cell_draw((u16)(TOOLTIP_TILE_BASE + tooltip_frame_map[i]),
                      (u8)(TOOLTIP_COL + c), (u8)(TOOLTIP_ROW + r));
}

// PC draws the ACTIVE half of the icon cell (draw_weapon_tooltip_icon with
// disabled = true reads region (0,0), which load_icons() fills from the top
// half of icons.png). Only the selected icon's four tiles are loaded.
static void icon_draw(void)
{
    if (weapon >= TOOLTIP_ICON_N) return;
    VDP_loadTileData(&tooltip_icon_tiles[weapon * TOOLTIP_ICON_TILES * 8],
                     TOOLTIP_ICON_TILE, TOOLTIP_ICON_TILES, DMA_QUEUE);
    for (u8 r = 0; r < 2; r++)
        for (u8 c = 0; c < 2; c++)
            cell_draw((u16)(TOOLTIP_ICON_TILE + r * 2 + c),
                      (u8)(TOOLTIP_ICON_COL + c),
                      (u8)(TOOLTIP_ICON_ROW + r));
}

void tooltip_show(u8 weapon_n)
{
    if (weapon_n >= WEAPON_N) return;
    text = weapon_data[weapon_n].name;
    weapon = weapon_n;
    pending = TRUE;
    drawn = FALSE;
}

void tooltip_hide(void)
{
    if (owns_pal3) {
        pal3_restore();
        owns_pal3 = FALSE;
    }
    text = NULL;
    pending = FALSE;
    drawn = FALSE;
    frames = 0;
}

void tooltip_update(void)
{
    if (!pending) return;

    if (!drawn) {
        // The START menu leaves a FULL plane redraw queued in SGDK's DMA
        // buffer (menu_open -> stage_force_redraw -> MAP_scrollToEx ->
        // setMapRow/Column -> DMA_allocateTemp). Those cells land at the next
        // vblank, i.e. after anything written here, and painted over the box
        // and its text: the tooltip never appeared (BUG-041). Flush the queue
        // first -- the stage start sequence does the same before drawing
        // "MISSION STARTED!". DmaQueue / vblank counters are unchanged by it.
        SYS_doVBlankProcess();
        pal3_save();
        pal3_load();
        owns_pal3 = TRUE;
        box_draw();
        back_fill_draw();   // opaque backing: see back_fill_draw()
        icon_draw();
        // The glyph cells carry palette index 0 as their background, which is
        // the box interior in PAL3, and entry 15 is the ink - drawn with the
        // default PAL0 the name painted black cells over the box.
        VDP_setTextPalette(PAL3);
        VDP_drawTextBG(STAGE_PLANE, text,
                       (u16)((((camera.x >> 3) & 63) + TOOLTIP_TILE_X) & 63),
                       (u16)((((camera.y >> 3) & 31) + TOOLTIP_TILE_Y) & 31));
        VDP_setTextPalette(PAL0);
        drawn = TRUE;
        frames = TOOLTIP_FRAMES;
        return;
    }

    if (--frames) return;

    // Gone: give PAL3 back and rebuild the plane so the box cells get their
    // map art back. One full redraw per tooltip, never per frame.
    pal3_restore();
    owns_pal3 = FALSE;
    pending = FALSE;
    drawn = FALSE;
    text = NULL;
    stage_redraw_bg();      // the box backing overwrote parallax cells
    stage_force_redraw();
}
