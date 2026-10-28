// Weapon-changed tooltip — see tooltip.h.
#include "tooltip.h"

#include "stage.h"
#include "tooltip_data.h"
#include "transition.h"
#include "pal_safe.h"
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
static u8 frames;                   // frames left before the redraw
static u16 pal3_saved[16];
static u16 pal3_touched;            // PAL3 entries the tooltip wrote (restored at expiry)
static u8 remap[16];                // tooltip palette index -> PAL3 entry

// The tooltip does NOT take PAL3 over: the parallax is drawn with it, so
// replacing the line recoloured the background for the whole 1.5 s. Instead the
// art's colours are fitted into the line: a colour the parallax already has is
// reused, otherwise it goes into an entry the parallax leaves free, and only
// when there is none the nearest parallax colour stands in. The 9 box tiles and
// the 4 icon tiles are remapped to those entries on the way to the VDP; the
// name is drawn with PAL0 (ink = font white).
static u16 bg_used_for;             // cache key: map the mask below belongs to
static const void *bg_used_def;
static u16 bg_used;                 // PAL3 entries the parallax art uses

static u16 parallax_used_mask(void)
{
    const u16 key = (u16)(stage_current_map + 1);
    if (bg_used_def == stage_def && bg_used_for == key) return bg_used;
    u16 used = 1;
    if (stage_def->bg_pal[stage_current_map]) {
        const u32 *t = stage_def->bg_tiles[stage_current_map];
        const u16 n = (u16)(stage_def->bg_tiles_n[stage_current_map] * 8);
        for (u16 i = 0; i < n && used != 0xFFFF; i++) {
            u32 w = t[i];
            for (u8 k = 0; k < 8; k++, w >>= 4) used |= (u16)(1u << (w & 15));
        }
    }
    bg_used = used;
    bg_used_for = key;
    bg_used_def = stage_def;
    return used;
}

static u16 art_used_mask(void)
{
    u16 used = 0;
    const u32 *f = tooltip_frame_tiles;
    for (u16 i = 0; i < TOOLTIP_FRAME_TILE_COUNT * 8; i++) {
        u32 w = f[i];
        for (u8 k = 0; k < 8; k++, w >>= 4) used |= (u16)(1u << (w & 15));
    }
    const u32 *ic = &tooltip_icon_tiles[weapon * TOOLTIP_ICON_TILES * 8];
    for (u16 i = 0; i < TOOLTIP_ICON_TILES * 8; i++) {
        u32 w = ic[i];
        for (u8 k = 0; k < 8; k++, w >>= 4) used |= (u16)(1u << (w & 15));
    }
    return used & ~1u;              // index 0 is transparent
}

static u16 tooltip_color(u8 idx)
{
    if (idx >= TOOLTIP_ICON_PAL_FIRST && idx < TOOLTIP_ICON_PAL_FIRST + TOOLTIP_ICON_PAL_N &&
        weapon < TOOLTIP_ICON_N)
        return tooltip_icon_pal[weapon * TOOLTIP_ICON_PAL_N + (idx - TOOLTIP_ICON_PAL_FIRST)];
    return tooltip_pal_fixed[idx];
}

static u16 color_dist(u16 a, u16 b)
{
    s16 dr = (s16)((a >> 1) & 7) - (s16)((b >> 1) & 7);
    s16 dg = (s16)((a >> 5) & 7) - (s16)((b >> 5) & 7);
    s16 db = (s16)((a >> 9) & 7) - (s16)((b >> 9) & 7);
    return (u16)(dr * dr + dg * dg + db * db);
}

static void pal3_save(void)
{
    for (u8 i = 0; i < 16; i++) pal3_saved[i] = PAL_getColor(PAL3_CRAM + i);
}

static void pal3_restore(void)
{
    for (u8 i = 1; i < 15; i++)
        if (pal3_touched & (1u << i)) pal_color((u16)(PAL3_CRAM + i), pal3_saved[i]);
    pal3_touched = 0;
}

static void pal3_fit(void)
{
    const u16 used = parallax_used_mask();
    const u16 art = art_used_mask();
    // entries the colourcycle rotates or keeps stable are neither reusable nor free
    u16 band = 1u << TILE_CYCLE_STABLE;
    for (u8 i = 0; i < stage_def->cycle_n; i++)
        band |= (u16)(1u << (stage_def->cycle_base + i));
    u16 freemask = 0;
    for (u8 i = 1; i < 15; i++)
        if (!(used & (1u << i)) && !(band & (1u << i))) freemask |= (u16)(1u << i);
    const u16 reuse = (u16)(used & ~band & 0x7FFE);

    pal3_touched = 0;
    for (u8 idx = 1; idx < 16; idx++) {
        remap[idx] = 1;
        if (!(art & (1u << idx))) continue;
        const u16 c = tooltip_color(idx);
        s8 pick = -1;
        for (u8 e = 1; e < 15 && pick < 0; e++)
            if ((reuse & (1u << e)) && pal3_saved[e] == c) pick = (s8)e;
        if (pick < 0 && freemask) {
            for (u8 e = 1; e < 15; e++) {
                if (freemask & (1u << e)) {
                    pick = (s8)e;
                    freemask &= (u16)~(1u << e);
                    pal3_touched |= (u16)(1u << e);
                    pal_color((u16)(PAL3_CRAM + e), c);
                    break;
                }
            }
        }
        if (pick < 0) {                 // nothing free: the nearest parallax colour
            u16 best = 0xFFFF;
            for (u8 e = 1; e < 15; e++) {
                if (!(reuse & (1u << e))) continue;
                const u16 d = color_dist(pal3_saved[e], c);
                if (d < best) { best = d; pick = (s8)e; }
            }
        }
        remap[idx] = (u8)((pick < 0) ? 1 : pick);
    }
}

// Copy `count` tiles through remap[] and put them in VRAM at `tile`.
static void tiles_load_remapped(const u32 *src, u16 count, u16 tile)
{
    u16 bytes;
    u32 *buf = transition_scratch_tiles(&bytes);     // idle outside a room change
    for (u16 i = 0; i < count * 8; i++) {
        u32 w = src[i], o = 0;
        for (u8 k = 0; k < 8; k++, w >>= 4) o |= (u32)(w & 15 ? remap[w & 15] : 0) << (4 * k);
        buf[i] = o;
    }
    VDP_loadTileData(buf, tile, count, CPU);
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
    // BG_B is the SCROLLED parallax plane, so a screen cell (col,row) lives at
    // plane cell ((col*8 - hscroll)/8, (row*8 - vscroll)/8) - the fill is one
    // uniform tile, so a sub-tile offset only means one extra row/column of
    // coverage, never a seam.
    const u16 tile = (u16)(TOOLTIP_TILE_BASE + TOOLTIP_FILL_TILE);
    const s16 hx = (s16)(TOOLTIP_COL * 8 - stage_bg_hscroll);
    const s16 vy = (s16)(TOOLTIP_ROW * 8 - stage_bg_vscroll);
    const u16 c0 = (u16)((hx - 8) >> 3), c1 = (u16)((hx + TOOLTIP_FRAME_W * 8) >> 3);
    const u16 r0 = (u16)((vy - 8) >> 3), r1 = (u16)((vy + TOOLTIP_FRAME_H * 8) >> 3);
    for (u16 c = c0; c <= c1; c++)
        for (u16 r = r0; r <= r1; r++)
            VDP_setTileMapXY(BG_B,
                             TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, tile),
                             (u16)(c & 63), (u16)(r & 31));
}

static void box_draw(void)
{
    tiles_load_remapped(tooltip_frame_tiles, TOOLTIP_FRAME_TILE_COUNT, TOOLTIP_TILE_BASE);
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
    tiles_load_remapped(&tooltip_icon_tiles[weapon * TOOLTIP_ICON_TILES * 8],
                        TOOLTIP_ICON_TILES, TOOLTIP_ICON_TILE);
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
    pal3_restore();
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
        pal3_fit();
        box_draw();
        back_fill_draw();   // opaque backing: see back_fill_draw()
        icon_draw();
        // The glyph cells carry palette index 0 as their background, which is
        // the box interior in PAL3, and entry 15 is the ink - drawn with the
        // default PAL0 the name painted black cells over the box.
        VDP_setTextPalette(PAL0);   // ink = font white; the cell background is transparent
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
    pending = FALSE;
    drawn = FALSE;
    text = NULL;
    stage_redraw_bg();      // the box backing overwrote parallax cells
    stage_force_redraw();
}
