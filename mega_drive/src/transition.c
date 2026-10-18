// Room transition: the PC scrolls the old screen out and the new one in
// vertically (game::transition_screen, game.cpp:1067). See transition.h.
//
// On the Mega Drive both screens have to exist as tiles at once, so each
// screen's 320x224 window is read out of its MAP resource, its distinct tiles
// are copied above the animated-tile block (the only free VRAM at that point —
// the current map's tileset, parallax tiles and anim slots are all still in
// use) and its tilemap entries are rewritten to that base. The two windows are
// stacked in RAM and the visible slice is written to BG_A each frame.
//
// Plane placement matters: the VDP applies ONE horizontal scroll per scanline
// (HSCROLL_LINE during the slide, so each half gets its own), and it wraps the
// plane at 512px. Writing only the 40 visible columns with a NEGATIVE scroll
// (the obvious framing) made the wrap expose unwritten cells at the left edge
// and the last two columns at the right edge — the flickering fragments in the
// middle of a transition. So a screen is placed with a small NON-NEGATIVE
// scroll: the window's first map cell goes to plane cell 0 and the scroll is
// `cam - first_cell*8`, at most 16px, which keeps the whole 320px window
// inside the 42 cells we write (42*8 = 336 > 16 + 320).
//
// Deliberate deviations from the PC:
//   - The destination map's palette is not loaded (PAL1 holds the current
//     map's), so the incoming screen can be a shade off until stage_load.
//   - Animated cells show their map-PNG art (frame 0) instead of the live
//     animation frame.
//   - If the slice needs more tiles than the free VRAM block has, the caller
//     falls back to the fade. Only stage 2 map 2 -> map 1 does today: map 2
//     runs 16 anim slots, which leave 32 tiles (936 - (520 + 16*24)) while that
//     link's window needs 45..81. Every other link fits (the tightest is
//     stage 3 map 0 -> map 1: 104 free, 85 needed).

#include "transition.h"

#include <string.h>

#include "stage.h"
#include "stage_data.h"
#include "player.h"
#include "enemy.h"
#include "object.h"

#define TRANS_COLS 42     // plane cells written per row (>= 40 + 2 of slack)
#define TRANS_MCOLS 21    // metatiles requested = 42 cells
#define TRANS_ROWS 28     // plane rows (224px screen)
#define TRANS_MROWS 14    // metatile rows = map_h
#define TRANS_STEP 8      // pixels per frame (PC uses 4 at 6ms/step)
#define TRANS_TILES 160   // largest converted map tileset is 144 tiles

// per-map tile3 bit mask stride (same as stage.c)
#define TRANS_MASK_BYTES ((stage_def->map_w * stage_def->map_h + 7) / 8)

// One screen of scanlines (the port's maps are exactly one screen tall).
#define TRANS_LINES (TRANS_ROWS * 8)

// Combined tilemap: the outgoing screen on top, the incoming one below.
static u16 trans_buf[TRANS_ROWS * 2][TRANS_COLS];
// The destination tileset, unpacked (tile = 32 bytes = 8 u32).
static u32 trans_tiles[TRANS_TILES * 8];
// Destination tile index -> copied VRAM slot; 0xFF = not copied yet.
static u8 trans_remap[TRANS_TILES];
// Per-scanline horizontal scroll (HSCROLL_LINE): the outgoing and incoming
// screens sit at different sub-tile offsets, so each half needs its own.
static s16 trans_hs_a[TRANS_LINES];
static s16 trans_hs_b[TRANS_LINES];

// Highest VRAM tile the map that is currently loaded occupies: its tileset,
// then the parallax tiles, then the animated slots (see the VRAM map in
// stage.h). Everything above that is free up to the sprite pool.
static u16 vram_free_base(u8 map)
{
    u16 base = TILE_USER_INDEX + stage_def->tile_sets[map]->numTile;
    u16 bg = TILE_BG_INDEX + stage_def->bg_tiles_n[map];
    if (bg > base) base = bg;
    u16 anim = TILE_ANIM_BASE + (u16)stage_def->anim_slot_n[map] * TILE_ANIM_STRIDE;
    if (anim > base) base = anim;
    return base;
}

static s16 clamp_cam_x(s16 x)
{
    s16 max = stage_def->map_w * stage_def->tile - 320;
    if (x < 0) return 0;
    if (x > max) return max;
    return x;
}

// Load one screen into `buf`: the map content for metatiles mt0..mt0+20 read
// from `m`, placed at plane cells 0.. so that scrolling the plane by *hs puts
// map pixel `cam` at screen x 0. *hs is in [0,16] (never negative), which is
// what keeps the VDP's 512px plane wrap out of the visible window.
static void load_screen(Map *m, s16 cam, u16 *buf, s16 *hs, u16 *mt0_out)
{
    u16 t0 = (u16)(cam >> 3);              // first visible 8px map cell
    u16 mt0 = t0 >> 1;                     // metatile holding it
    if (mt0 > (u16)(stage_def->map_w - TRANS_MCOLS))
        mt0 = stage_def->map_w - TRANS_MCOLS;
    MAP_getTilemapRect(m, mt0, 0, TRANS_MCOLS, TRANS_MROWS, FALSE, buf);
    *hs = (s16)((u16)cam - mt0 * 16);
    *mt0_out = mt0;
}

// OR the above-player (tile3) priority bit into a screen's cells. The MAP
// resource does not carry it; the live plane gets it from stage_map_patch.
static void apply_tile3(u16 *buf, u8 map, u16 mt0)
{
    const u8 *t3 = stage_def->tile3_mask + map * TRANS_MASK_BYTES;
    for (u16 my = 0; my < TRANS_MROWS; my++) {
        for (u16 mx = 0; mx < TRANS_MCOLS; mx++) {
            const u16 n = my * stage_def->map_w + mt0 + mx;
            if (!(t3[n >> 3] & (1 << (n & 7)))) continue;
            u16 *e = &buf[my * 2 * TRANS_COLS + mx * 2];
            e[0] |= TILE_ATTR_PRIORITY_MASK;
            e[1] |= TILE_ATTR_PRIORITY_MASK;
            e[TRANS_COLS] |= TILE_ATTR_PRIORITY_MASK;
            e[TRANS_COLS + 1] |= TILE_ATTR_PRIORITY_MASK;
        }
    }
}

bool transition_scroll(u8 from_map, u8 to_map, s16 to_cam_x,
                       s16 player_x, s16 player_y, bool top_to_bottom)
{
    if (from_map == to_map) return FALSE;

    const u16 spare = vram_free_base(from_map);
    if (spare >= TILE_SPRITE_INDEX) return FALSE;

    const s16 dcam = clamp_cam_x(to_cam_x);

    u16 *newbuf = top_to_bottom ? trans_buf[TRANS_ROWS] : trans_buf[0];
    u16 *oldbuf = top_to_bottom ? trans_buf[0] : trans_buf[TRANS_ROWS];

    // ---- destination screen: tilemap window ------------------------------
    Map *dm = MAP_create(stage_def->map_defs[to_map], STAGE_PLANE,
                         TILE_ATTR_FULL(STAGE_PAL, FALSE, FALSE, FALSE,
                                        TILE_USER_INDEX));
    if (!dm) return FALSE;
    u16 nmt0;
    s16 new_hs;
    load_screen(dm, dcam, newbuf, &new_hs, &nmt0);
    MAP_release(dm);

    // ---- destination tiles: unpack, copy the ones the window uses ---------
    TileSet ts;
    ts.compression = COMPRESSION_NONE;
    ts.numTile = 0;
    ts.tiles = trans_tiles;
    if (!unpackTileSet(stage_def->tile_sets[to_map], &ts)) return FALSE;
    if (ts.numTile > TRANS_TILES) return FALSE;

    memset(trans_remap, 0xFF, ts.numTile);

    const u8 *t3 = stage_def->tile3_mask + to_map * TRANS_MASK_BYTES;
    u16 used = 0;
    for (u16 my = 0; my < TRANS_MROWS; my++) {
        for (u16 mx = 0; mx < TRANS_MCOLS; mx++) {
            const u16 n = my * stage_def->map_w + nmt0 + mx;
            const bool prio = (t3[n >> 3] & (1 << (n & 7))) != 0;
            for (u16 cy = 0; cy < 2; cy++) {
                for (u16 cx = 0; cx < 2; cx++) {
                    u16 *e = &newbuf[(my * 2 + cy) * TRANS_COLS + mx * 2 + cx];
                    u16 local = (*e & TILE_INDEX_MASK) - TILE_USER_INDEX;
                    if (local >= ts.numTile) continue;
                    if (trans_remap[local] == 0xFF) {
                        if (spare + used >= TILE_SPRITE_INDEX) return FALSE;
                        trans_remap[local] = (u8)used;
                        VDP_loadTileData(&trans_tiles[local * 8], spare + used,
                                         1, DMA_QUEUE);
                        used++;
                    }
                    *e = (*e & ~TILE_INDEX_MASK) | (u16)(spare + trans_remap[local]);
                    if (prio) *e |= TILE_ATTR_PRIORITY_MASK;
                }
            }
        }
    }

    // ---- outgoing screen: its tiles are already resident ------------------
    const s16 ocam = clamp_cam_x(camera.x);
    u16 omt0;
    s16 old_hs;
    load_screen(stage_map, ocam, oldbuf, &old_hs, &omt0);
    apply_tile3(oldbuf, from_map, omt0);

    // ---- slide ------------------------------------------------------------
    // BG_B reads the same table in this mode, so it is filled with the
    // parallax offset on every line.
    {
        const StageBgInfo *info = &stage_def->bg_info[from_map];
        s16 bg = 0;
        if (info->present && info->speed) {
            s32 s = -((s32)camera.x * info->speed) / 10;
            s %= 512;
            if (s > 0) s -= 512;
            bg = (s16)s;
        }
        for (u16 y = 0; y < TRANS_LINES; y++) trans_hs_b[y] = bg;
        VDP_setHorizontalScrollLine(BG_B, 0, trans_hs_b, TRANS_LINES, DMA_QUEUE);
    }
    VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
    VDP_setVerticalScroll(STAGE_PLANE, 0);

    const s16 player_sx = player_x - dcam;
    const u16 span = TRANS_LINES;                 // one screen in pixels
    const u16 steps = span / TRANS_STEP;
    for (u16 i = 0; i <= steps; i++) {
        const s16 off = top_to_bottom ? (s16)(i * TRANS_STEP)
                                      : (s16)(span - i * TRANS_STEP);
        VDP_setTileMapDataRect(STAGE_PLANE, &trans_buf[off / 8][0], 0, 0,
                               TRANS_COLS, TRANS_ROWS, TRANS_COLS, DMA_QUEUE);

        // The tilemap slice boundary and the scroll split are the same line.
        {
            const s16 split = (s16)(span - off);
            const s16 top_hs = top_to_bottom ? old_hs : new_hs;
            const s16 bot_hs = top_to_bottom ? new_hs : old_hs;
            for (u16 y = 0; y < TRANS_LINES; y++)
                trans_hs_a[y] = (y < (u16)split) ? top_hs : bot_hs;
            VDP_setHorizontalScrollLine(STAGE_PLANE, 0, trans_hs_a,
                                        TRANS_LINES, DMA_QUEUE);
        }

        // The outgoing screen takes its sprites with it (PC copies a bitmap);
        // the player belongs to the incoming screen and rides it in. Maps are
        // exactly one screen tall, so camera.y is always 0.
        const s16 old_dy = top_to_bottom ? -off : (s16)(span - off);
        enemies_shift_y(old_dy);
        objects_shift_y(old_dy);
        if (player.spr) {
            const s16 py = player_y + (top_to_bottom ? (s16)(span - off)
                                                     : (s16)-off);
            SPR_setPosition(player.spr, player_sx, py);
        }

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);   // see main.c: DMA clobbers the window regs
        VDP_setReg(18, 0);
    }

    // back to one offset per plane; the caller's stage_load re-scrolls both
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    return TRUE;
}
