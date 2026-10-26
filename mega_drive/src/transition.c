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
// Transition tiles must stay BELOW the tooltip block: 672..702 hold the tooltip
// art, the dialog panel and the sky tile (702, which BG_B cells point at), and
// 703.. is the sprite pool. TILE_SPRITE_INDEX (703) is not a free-VRAM limit.
#define TRANS_VRAM_END SPR_TOOLTIP_TILE_BASE
#define TRANS_TILES 256   // FROZEN LAIR map 1 has 202 unique 8x8 tiles

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

// Highest VRAM tile a map occupies: its tileset, the parallax tiles right
// after it, then the animated slots (stage.h computes the same bases when the
// map is loaded). Everything above that is free up to the sprite pool.
static u16 vram_free_base(u8 map)
{
    return stage_layout_anim(map) +
           (u16)stage_def->anim_slot_n[map] * TILE_ANIM_STRIDE;
}

// The SGDK DMA queue holds DMA_QUEUE_SIZE_DEFAULT (80) transfers and silently
// DROPS the rest (dma.c). A window needs 45..200 single-tile transfers plus the
// parallax block, so queueing them all before the first vblank lost tiles: the
// VRAM kept whatever it held (noise bands, broken tiles, stray dots). Load in
// chunks and let a vblank drain the queue between them.
#define TRANS_DMA_CHUNK 32
static u16 trans_queued;

static void trans_dma_room(u16 entries)
{
    if (trans_queued + entries > TRANS_DMA_CHUNK) {
        SYS_doVBlankProcess();
        trans_queued = 0;
    }
    trans_queued += entries;
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
    // VDP scroll is the negated source X (MAP_scrollToEx does -camera.x):
    // screen pixel 0 displays plane pixel (0 - scroll). The window starts at
    // map pixel mt0*16, so the scroll must be mt0*16 - cam, NOT cam - mt0*16.
    // The old positive value shifted both halves by up to 30px (1-2 tiles).
    *hs = (s16)((s16)(mt0 * 16) - cam);
    *mt0_out = mt0;
}

// Parallax offset for a camera position (same formula as stage_update_camera).
static s16 bg_offset(u8 map, s16 cam)
{
    const StageBgInfo *info = &stage_def->bg_info[map];
    if (!info->present) return 0;
    // auto-scroll maps ignore the camera (PC classmap.cpp:592); stage.c owns
    // the phase so both screens agree mid-transition.
    if (info->auto_scroll == 3 || info->auto_scroll == 4 ||
        info->auto_scroll == 5)
        return stage_bg_auto_hscroll(map);
    if (!info->speed) return 0;
    s32 v = -((s32)cam * info->speed) / 10;
    v %= 512;
    if (v > 0) v -= 512;
    return (s16)v;
}

// One BG_B plane row (64 cells) showing screen row `srow` of map `map`, whose
// tiles live at `tile_base`. `vscroll` rotates the plane so the row lands where
// the stage would have drawn it with its own vertical scroll.
static void bg_row_cells(u16 *row, u8 map, s16 srow, u16 tile_base, s16 vscroll)
{
    const s16 plane_row = (s16)((srow + vscroll) & 31);
    const s16 h = (s16)stage_def->bg_map_h[map];
    const u16 *m = stage_def->bg_maps[map];
    const u16 sky = TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                   stage_sky_tile());
    for (u16 x = 0; x < 64; x++) {
        row[x] = (plane_row >= 0 && plane_row < h)
                     ? TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                      tile_base + m[plane_row * 64 + x])
                     : sky;
    }
}

// OR the above-player (tile3) priority bit into a screen's cells. The MAP
// resource does not carry it; the live plane gets it from stage_map_patch.
static void apply_tile3(u16 *buf, u8 map, u16 mt0)
{
    const u8 *t3 = stage_def->tile3_mask + map * TRANS_MASK_BYTES;
    const u8 *p3 = stage_def->pal3_mask
                       ? stage_def->pal3_mask + map * TRANS_MASK_BYTES : NULL;
    for (u16 my = 0; my < TRANS_MROWS; my++) {
        for (u16 mx = 0; mx < TRANS_MCOLS; mx++) {
            const u16 n = my * stage_def->map_w + mt0 + mx;
            u16 *e = &buf[my * 2 * TRANS_COLS + mx * 2];
            if (t3[n >> 3] & (1 << (n & 7))) {
                e[0] |= TILE_ATTR_PRIORITY_MASK;
                e[1] |= TILE_ATTR_PRIORITY_MASK;
                e[TRANS_COLS] |= TILE_ATTR_PRIORITY_MASK;
                e[TRANS_COLS + 1] |= TILE_ATTR_PRIORITY_MASK;
            }
            // colourcycled cell: PAL3-relative art needs the PAL3 bits here
            // too, or the incoming screen shows it with the map palette
            if (p3 && (p3[n >> 3] & (1 << (n & 7)))) {
                for (u16 k = 0; k < 4; k++) {
                    u16 *c2 = &e[(k & 1) + ((k >> 1) * TRANS_COLS)];
                    *c2 = (*c2 & ~TILE_ATTR_PALETTE_MASK) |
                          ((u16)STAGE_PAL_CYCLE << TILE_ATTR_PALETTE_SFT);
                }
            }
        }
    }
}

u8 transition_fail_code;
u8 transition_fail_map;
u16 transition_fail_free_kb;

bool transition_scroll(u8 from_map, u8 to_map, s16 to_cam_x,
                       s16 player_x, s16 player_y, bool top_to_bottom)
{
    transition_fail_code = 0;
    if (from_map == to_map) { transition_fail_code = 1; return FALSE; }

    const u16 spare = vram_free_base(from_map);
    if (spare >= TRANS_VRAM_END) { transition_fail_code = 2;
return FALSE; }

    const s16 dcam = clamp_cam_x(to_cam_x);

    u16 *newbuf = top_to_bottom ? trans_buf[TRANS_ROWS] : trans_buf[0];
    u16 *oldbuf = top_to_bottom ? trans_buf[0] : trans_buf[TRANS_ROWS];

    // ---- outgoing screen FIRST: its tiles are resident, and building it
    // needs the current stage map. Releasing that map here frees the heap the
    // destination Map needs: with the stage loaded, MEM_getFree() can be as low
    // as ~7 KB, which made MAP_create fail and every SEAHORSE link fall back
    // to the fade (BUG-023), leaving a half-loaded room behind (BUG-022/024).
    const s16 ocam = clamp_cam_x(camera.x);
    u16 omt0;
    s16 old_hs;
    load_screen(stage_map, ocam, oldbuf, &old_hs, &omt0);
    apply_tile3(oldbuf, from_map, omt0);
    stage_release_map();

    // ---- destination screen: tilemap window ------------------------------
    Map *dm = MAP_create(stage_def->map_defs[to_map], STAGE_PLANE,
                         TILE_ATTR_FULL(STAGE_PAL, FALSE, FALSE, FALSE,
                                        TILE_USER_INDEX));
    if (!dm) {
        transition_fail_code = 3;
        transition_fail_map = to_map;
        transition_fail_free_kb = (u16)(MEM_getFree() / 1024);
        return FALSE;
    }
    u16 nmt0;
    s16 new_hs;
    load_screen(dm, dcam, newbuf, &new_hs, &nmt0);
    MAP_release(dm);

    // ---- destination tiles: unpack, copy the ones the window uses ---------
    TileSet ts;
    ts.compression = COMPRESSION_NONE;
    ts.numTile = 0;
    ts.tiles = trans_tiles;
    if (!unpackTileSet(stage_def->tile_sets[to_map], &ts)) { transition_fail_code = 4;
return FALSE; }
    if (ts.numTile > TRANS_TILES) { transition_fail_code = 5;
return FALSE; }

    memset(trans_remap, 0xFF, ts.numTile);
    trans_queued = 0;

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
                        if (spare + used >= TRANS_VRAM_END) { transition_fail_code = 6;
return FALSE; }
                        trans_remap[local] = (u8)used;
                        trans_dma_room(1);
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


    // ---- destination parallax --------------------------------------------
    // The incoming screen must arrive with ITS room's background: the old code
    // left BG_B with the outgoing room's art and scroll for the whole slide, so
    // the next room was painted over the previous room's sky (report
    // 2026-10-24). Both halves share PAL3, so the destination art can only be
    // shown when the two bg palettes are identical - otherwise the incoming
    // half is left as sky (the backdrop colour) instead of the wrong art.
    u16 bg_tiles_new = 0;
    bool dest_bg = FALSE;
    const s16 vscroll = (s16)(-((s16)stage_def->bg_info[from_map].adjust_y - 16));
    const s16 vscroll_new = (s16)(-((s16)stage_def->bg_info[to_map].adjust_y - 16));
    if (stage_def->bg_info[to_map].present && stage_def->bg_pal[to_map] &&
        stage_def->bg_pal[from_map] && vscroll == vscroll_new) {
        const Palette *pa = stage_def->bg_pal[from_map];
        const Palette *pb = stage_def->bg_pal[to_map];
        bool same = TRUE;
        for (u8 i = 0; i < 16 && same; i++) same = (pa->data[i] == pb->data[i]);
        const u16 need = stage_def->bg_tiles_n[to_map];
        if (same && spare + used + need <= TRANS_VRAM_END) {
            bg_tiles_new = (u16)(spare + used);
            // one entry per 32-tile block, so a big parallax set is chunked too
            for (u16 t = 0; t < need; t += TRANS_DMA_CHUNK) {
                const u16 n = (u16)((need - t > TRANS_DMA_CHUNK)
                                        ? TRANS_DMA_CHUNK : need - t);
                trans_queued = TRANS_DMA_CHUNK;   // force a drain before it
                trans_dma_room(1);
                VDP_loadTileData(stage_def->bg_tiles[to_map] + t * 8,
                                 (u16)(bg_tiles_new + t), n, DMA_QUEUE);
            }
            used = (u16)(used + need);
            dest_bg = TRUE;
        }
    }

    // drain whatever tile transfers are still queued before the slide starts
    SYS_doVBlankProcess();

    // Rewrite the outgoing half so it looks the same with BG_B's vertical
    // scroll taken out (rows for the incoming half are filled as they arrive).
    if (stage_def->bg_info[from_map].present) {
        static u16 bgrow[64];
        for (s16 r = 0; r < TRANS_ROWS; r++) {
            bg_row_cells(bgrow, from_map, r, stage_bg_tile_base, vscroll);
            VDP_setTileMapData(BG_B, bgrow, (u16)(r * 64), 64, 1, CPU);
        }
    }
    const s16 bg_old = bg_offset(from_map, camera.x);
    const s16 bg_new = bg_offset(to_map, dcam);
    for (u16 y = 0; y < TRANS_LINES; y++) trans_hs_b[y] = bg_old;

    VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
    VDP_setVerticalScroll(STAGE_PLANE, 0);
    VDP_setVerticalScroll(BG_B, 0);
    // Rows of BG_B already moved to the incoming half. The incoming band grows
    // UPWARD for T2B and DOWNWARD for B2T, so a single "last row" marker left
    // the far side of the band holding the outgoing room's art - the stray
    // tiles seen on ladder transitions (report 2026-10-24).
    s16 bg_filled_lo = TRANS_ROWS;
    s16 bg_filled_hi = -1;

    const s16 player_sx = player_x - dcam;
    const u16 span = TRANS_LINES;                 // one screen in pixels
    const u16 steps = span / TRANS_STEP;
    for (u16 i = 0; i <= steps; i++) {
        const s16 off = top_to_bottom ? (s16)(i * TRANS_STEP)
                                      : (s16)(span - i * TRANS_STEP);
        VDP_setTileMapDataRect(STAGE_PLANE, &trans_buf[off / 8][0], 0, 0,
                               TRANS_COLS, TRANS_ROWS, TRANS_COLS, DMA_QUEUE);

        // Which source row every scanline shows, derived from the same index
        // the tilemap write above used (base = off/8 into the stacked buffer).
        // T2B puts the incoming screen BELOW the outgoing one, B2T ABOVE, so
        // the answer differs per direction - the old `y < split` split applied
        // the outgoing offset to the incoming half on B2T, which is the 1..15px
        // horizontal jump seen in upward room changes (report 2026-10-24).
        {
            const u16 base = (u16)(off / 8);
            const bool new_on_top = !top_to_bottom;
            for (u16 y = 0; y < TRANS_LINES; y++) {
                const u16 prow = (u16)(base + (y >> 3));
                const bool is_new = new_on_top ? (prow < TRANS_ROWS)
                                               : (prow >= TRANS_ROWS);
                trans_hs_a[y] = is_new ? new_hs : old_hs;
                trans_hs_b[y] = is_new ? bg_new : bg_old;
            }
            VDP_setHorizontalScrollLine(STAGE_PLANE, 0, trans_hs_a,
                                        TRANS_LINES, DMA_QUEUE);
            VDP_setHorizontalScrollLine(BG_B, 0, trans_hs_b, TRANS_LINES,
                                        DMA_QUEUE);
            // Parallax rows that just became incoming: the destination art (or
            // sky when its palette could not be shown).
            s16 lo = TRANS_ROWS, hi = -1;
            for (u16 r = 0; r < TRANS_ROWS; r++) {
                const u16 prow = (u16)(base + r);
                const bool is_new = new_on_top ? (prow < TRANS_ROWS)
                                               : (prow >= TRANS_ROWS);
                if (is_new) {
                    if ((s16)r < lo) lo = (s16)r;
                    if ((s16)r > hi) hi = (s16)r;
                }
            }
            if (hi >= lo) {
                static u16 bgrow[64];
                // Fill the two ends the band grew into since the last frame.
                s16 from = lo, to = (s16)((bg_filled_lo - 1 > hi)
                                          ? hi : bg_filled_lo - 1);
                if (from <= to) {
                    for (s16 r = from; r <= to; r++) {
                        if (dest_bg) {
                            bg_row_cells(bgrow, to_map, r, bg_tiles_new, 0);
                        } else {
                            for (u16 x = 0; x < 64; x++)
                                bgrow[x] = TILE_ATTR_FULL(
                                    BG_PAL, FALSE, FALSE, FALSE,
                                    stage_sky_tile());
                        }
                        VDP_setTileMapData(BG_B, bgrow, (u16)(r * 64), 64, 1,
                                           DMA_QUEUE);
                    }
                }
                from = (s16)((bg_filled_hi + 1 > lo) ? bg_filled_hi + 1 : lo);
                to = hi;
                if (from <= to) {
                    for (s16 r = from; r <= to; r++) {
                        if (dest_bg) {
                            bg_row_cells(bgrow, to_map, r, bg_tiles_new, 0);
                        } else {
                            for (u16 x = 0; x < 64; x++)
                                bgrow[x] = TILE_ATTR_FULL(
                                    BG_PAL, FALSE, FALSE, FALSE,
                                    stage_sky_tile());
                        }
                        VDP_setTileMapData(BG_B, bgrow, (u16)(r * 64), 64, 1,
                                           DMA_QUEUE);
                    }
                }
                if (lo < bg_filled_lo) bg_filled_lo = lo;
                if (hi > bg_filled_hi) bg_filled_hi = hi;
            }
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
    VDP_setVerticalScroll(BG_B, vscroll);
    return TRUE;
}
