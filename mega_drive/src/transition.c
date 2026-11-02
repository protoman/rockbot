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
//     The incoming parallax is remapped to the outgoing PAL3 colours.
//   - Incoming enemies/objects are frozen first-frame previews (prev_*).
//   - Animated cells show their map-PNG art (frame 0) instead of the live
//     animation frame.
//   - If the slice needs more tiles than the free VRAM block has, the caller
//     falls back to the fade. Only stage 2 map 2 -> map 1 does today: map 2
//     runs 16 anim slots, which leave 32 tiles (936 - (520 + 16*24)) while that
//     link's window needs 45..81. Every other link fits (the tightest is
//     stage 3 map 0 -> map 1: 104 free, 85 needed).

#include "transition.h"
#include "dbglog.h"
#include "dbgprint.h"

#include <string.h>

#include "stage.h"
#include "stage_data.h"
#include "player.h"
#include "enemy.h"
#include "object.h"
#include "sprite_guard.h"

#define TRANS_COLS 42     // plane cells written per row (>= 40 + 2 of slack)
#define TRANS_MCOLS 21    // metatiles requested = 42 cells
#define TRANS_ROWS 28     // plane rows (224px screen)
#define TRANS_MROWS 14    // metatile rows = map_h
#define TRANS_STEP 4      // pixels per vblank (PC: 4 px per step); the plane is moved by VSCROLL
// Frames each step is held: 1 = a new picture offset every vblank (60 Hz). The PC moves
// 4 px per step; 56 steps of 4 px = 56 frames = 0.93 s.
#define TRANS_HOLD 1
// Sprites of the incoming room shown while it slides in (PC show_objects /
// temp_map->show_objects). Each costs a sprite slot, so the cap is small.
#define TRANS_PREV_MAX 14
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

// The buffers above are rebuilt by every room transition and idle otherwise; screens
// that run with almost no heap (the castle intro: 8 KB free in fragments) borrow them.
void *transition_scratch_tiles(u16 *bytes) { *bytes = sizeof(trans_tiles); return trans_tiles; }
void *transition_scratch_cells(u16 *bytes) { *bytes = sizeof(trans_buf); return trans_buf; }
// Per-scanline horizontal scroll (HSCROLL_LINE): the outgoing and incoming
// screens sit at different sub-tile offsets, so each half needs its own.
static s16 trans_hs_a[TRANS_LINES];
static s16 trans_hs_b[TRANS_LINES];

// Per-scanline scroll tables: what the VDP holds now (so a frame sends only the span
// of lines that changed) - the old code re-sent both 224-entry tables every step.
static s16 hs_sent_a[TRANS_LINES];
static s16 hs_sent_b[TRANS_LINES];

static void hs_send(VDPPlane plane, s16 *now, s16 *sent, bool all)
{
    s16 first = -1, last = -1;
    for (u16 y = 0; y < TRANS_LINES; y++) {
        if (all || now[y] != sent[y]) {
            if (first < 0) first = (s16)y;
            last = (s16)y;
            sent[y] = now[y];
        }
    }
    if (first >= 0)
        VDP_setHorizontalScrollLine(plane, (u16)first, now + first,
                                    (u16)(last - first + 1), DMA_QUEUE);
}

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

// ---- incoming room sprites ------------------------------------------------
static Sprite *prev_spr[TRANS_PREV_MAX];
static s16 prev_x[TRANS_PREV_MAX];     // screen X at the destination camera
static s16 prev_y[TRANS_PREV_MAX];     // world Y (maps are one screen tall)
static u8 prev_n;

static void prev_add(const SpriteDefinition *def, s16 x, s16 y, bool flip, s16 anim)
{
    if (prev_n >= TRANS_PREV_MAX || !def) return;
    Sprite *sp = SPR_addSprite(def, x, y + 224,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (!sp) return;
    SPR_setAutoAnimation(sp, FALSE);
    spr_set_anim(sp, anim);
    SPR_setHFlip(sp, flip);
    SPR_setVisibility(sp, HIDDEN);
    prev_spr[prev_n] = sp;
    prev_x[prev_n] = x;
    prev_y[prev_n] = y;
    prev_n++;
}

// Frozen first frame of every enemy and object the destination room starts
// with, inside the 320px window shown at `dcam`.
static void prev_build(u8 to_map, s16 dcam)
{
    prev_n = 0;
    for (u8 i = 0; i < stage_def->enemy_spawn_count; i++) {
        const EnemySpawn *sp = &stage_def->enemy_spawns[i];
        if (sp->map != to_map) continue;
        const s16 sx = sp->x - dcam;
        if (sx < -40 || sx >= 336) continue;
        const EnemyDef *d = &stage_def->enemy_defs[sp->def];
        prev_add(d->spr, sx, sp->y, sp->dir == 0, d->walk_n > 0 ? 1 : 0);
    }
    for (u8 i = 0; i < stage_def->object_spawn_count; i++) {
        const ObjectSpawn *sp = &stage_def->object_spawns[i];
        if (sp->map != to_map) continue;
        const ObjectDef *d = &stage_def->object_defs[sp->def];
        if (d->type == 30 || (d->type == 2 && d->timer)) continue;  // door / timed block
        const s16 sx = sp->x - dcam;
        if (sx + d->w <= 0 || sx >= 320) continue;
        prev_add(d->spr, sx, sp->y, FALSE, 0);
    }
}

// `dy`: how far below its resting place the destination screen still is.
static void prev_place(s16 dy)
{
    for (u8 i = 0; i < prev_n; i++) {
        const s16 y = (s16)(prev_y[i] + dy);
        SPR_setVisibility(prev_spr[i], (y > -48 && y < 224) ? VISIBLE : HIDDEN);
        SPR_setPosition(prev_spr[i], prev_x[i], y);
    }
}

static void prev_free(void)
{
    for (u8 i = 0; i < prev_n; i++) SPR_releaseSprite(prev_spr[i]);
    prev_n = 0;
    SPR_update();
}

#ifdef MD_TRANS_DEBUG
// Debug probes (build: make clean && make EXTRA_FLAGS="-DMD_TRANS_DEBUG").
// They compare what the VDP really holds (VRAM readback, tdump_read) with what
// the code meant to put there, and print one line per check on the KDEBUG
// console, so a single log answers "RAM right, VRAM wrong?" without pictures.
// Log lines start with "TR" and are parsed by tools/trans_log.py.
#include "tdump.h"
#include "dbglog.h"


static u8 dbg_slot_local[TRANS_TILES];   // incoming slot -> tileset tile
static u16 dbg_tile_used;                // incoming tile slots (no parallax)

// 16 VRAM words of tile `vt` against 32 bytes at `ram`; returns differing words
static u16 dbg_tile_bad(u16 vt, const u32 *ram, u16 *vram_out)
{
    u16 w[16];
    tdump_read((u16)(vt * 32), w, 16);
    const u16 *r = (const u16 *)ram;
    u16 bad = 0;
    for (u16 k = 0; k < 16; k++) if (w[k] != r[k]) bad++;
    if (vram_out) for (u16 k = 0; k < 4; k++) vram_out[k] = w[k];
    return bad;
}

// Outgoing map tileset: resident since stage_load, ROM copy is the reference.
static void dbg_check_out(const char *tag, u8 from_map)
{
    const TileSet *t = stage_def->tile_sets[from_map];
    u16 bad = 0, first = 0xFFFF;
    if (t->compression != COMPRESSION_NONE) {
        dbg_log("TR %s out: tileset compressed, skipped\n", tag);
        return;
    }
    for (u16 n = 0; n < t->numTile; n++) {
        if (dbg_tile_bad((u16)(TILE_USER_INDEX + n), &t->tiles[n * 8], NULL)) {
            if (!bad) first = n;
            bad++;
        }
    }
    dbg_log("TR %s out_bad=%u/%u first=%u\n", tag, bad, t->numTile, first);
}

// Incoming tile slots: trans_tiles (RAM) is the reference.
static void dbg_check_in(const char *tag, u16 spare)
{
    u16 bad = 0, shown = 0, v[4];
    for (u16 sl = 0; sl < dbg_tile_used; sl++) {
        const u32 *r = &trans_tiles[dbg_slot_local[sl] * 8];
        if (dbg_tile_bad((u16)(spare + sl), r, v)) {
            bad++;
            if (shown < 4) {
                const u16 *rw = (const u16 *)r;
                dbg_log("TR %s in slot=%u tile=%u vram=%04X %04X %04X %04X "
                        "ram=%04X %04X %04X %04X\n", tag, spare + sl,
                        dbg_slot_local[sl], v[0], v[1], v[2], v[3],
                        rw[0], rw[1], rw[2], rw[3]);
                shown++;
            }
        }
    }
    dbg_log("TR %s in_bad=%u/%u\n", tag, bad, dbg_tile_used);
}

// Per slide frame: VDP registers, queue state, the whole visible plane A
// window against trans_buf, and a sample of resident tiles.
static void dbg_frame(u16 i, u16 base, u16 spare, u8 from_map)
{
    u16 row[TRANS_COLS];
    u16 pbad = 0, fr = 0, fc = 0, fv = 0, fe = 0;
    for (u16 r = 0; r < TRANS_ROWS; r++) {
        tdump_read((u16)(VDP_getBGAAddress() + r * 128), row, TRANS_COLS);
        const u16 *want = &trans_buf[base + r][0];
        for (u16 c = 0; c < TRANS_COLS; c++) {
            if (row[c] != want[c]) {
                if (!pbad) { fr = r; fc = c; fv = row[c]; fe = want[c]; }
                pbad++;
            }
        }
    }
    u16 ibad = 0, obad = 0;
    const TileSet *t = stage_def->tile_sets[from_map];
    for (u16 k = 0; k < 8; k++) {
        if (dbg_tile_used)
            ibad += dbg_tile_bad((u16)(spare + (k * dbg_tile_used) / 8),
                &trans_tiles[dbg_slot_local[(k * dbg_tile_used) / 8] * 8],
                NULL) ? 1 : 0;
        if (t->compression == COMPRESSION_NONE)
            obad += dbg_tile_bad((u16)(TILE_USER_INDEX + (k * t->numTile) / 8),
                &t->tiles[((k * t->numTile) / 8) * 8], NULL) ? 1 : 0;
    }
    dbg_log("TRF i=%u base=%u r15=%02X r16=%02X r17=%02X r18=%02X r11=%02X "
            "q=%u\n", i, base, VDP_getReg(15), VDP_getReg(16), VDP_getReg(17),
            VDP_getReg(18), VDP_getReg(11), DMA_getQueueSize());
    dbg_log("TRF i=%u plane_bad=%u first r=%u c=%u vram=%04X want=%04X "
            "in_bad=%u/8 out_bad=%u/8\n", i, pbad, fr, fc, fv, fe, ibad,
            obad);
}
#endif

u8 transition_fail_code;
u8 transition_fail_map;
u16 transition_fail_free_kb;

bool transition_scroll(u8 from_map, u8 to_map, s16 to_cam_x,
                       s16 player_x, s16 player_y, bool top_to_bottom,
                       s16 old_shift)
{
    transition_fail_code = 0;
    if (from_map == to_map) { transition_fail_code = 1; return FALSE; }

    // Where the player is drawn NOW (old room, old camera): the slide carries him from
    // here to his arrival spot, like the PC (game.cpp:1085 moves him a little less than
    // the scroll every step) instead of making him vanish and re-enter from the edge.
    const s16 old_sy = (s16)(player.y - camera.y);
    const u16 spare = vram_free_base(from_map);
    if (spare >= TRANS_VRAM_END) { transition_fail_code = 2;
return FALSE; }

    const s16 dcam = clamp_cam_x(to_cam_x);
#ifdef MD_TRANS_DEBUG
    dbg_log("TRS from=%u to=%u spare=%u cam=%d dcam=%d t2b=%u qmax=%u\n",
            from_map, to_map, spare, camera.x, dcam, top_to_bottom,
            DMA_getMaxQueueSize());
    dbg_check_out("pre", from_map);
#endif

    u16 *newbuf = top_to_bottom ? trans_buf[TRANS_ROWS] : trans_buf[0];
    u16 *oldbuf = top_to_bottom ? trans_buf[0] : trans_buf[TRANS_ROWS];

    // ---- outgoing screen FIRST: its tiles are resident, and building it
    // needs the current stage map. Releasing that map here frees the heap the
    // destination Map needs: with the stage loaded, MEM_getFree() can be as low
    // as ~7 KB, which made MAP_create fail and every SEAHORSE link fall back
    // to the fade (BUG-023), leaving a half-loaded room behind (BUG-022/024).
    // The outgoing screen is shown as if its camera were ocam - old_shift, so the
    // old ladder lands on the destination ladder's screen column (see
    // transition.h). camera.x follows because the old half's enemies/objects
    // take their X from it every frame; stage_load() sets the real camera next.
    const s16 ocam = clamp_cam_x(clamp_cam_x(camera.x) - old_shift);
    camera.x = ocam;
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
#if defined(MD_LINK_DEBUG) && !defined(MD_TRANS_DEBUG)
    dbg_log("TRL ocam=%d dcam=%d old_hs=%d new_hs=%d omt0=%u nmt0=%u "
            "player_x=%d player_y=%d t2b=%u\n", ocam, dcam, old_hs, new_hs,
            omt0, nmt0, player_x, player_y, top_to_bottom);
#endif

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
#ifdef MD_TRANS_DEBUG
                        dbg_slot_local[used] = (u8)local;
#endif
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


#ifdef MD_TRANS_DEBUG
    dbg_tile_used = used;
#endif

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
        stage_def->bg_pal[from_map]) {
        const Palette *pa = stage_def->bg_pal[from_map];
        const Palette *pb = stage_def->bg_pal[to_map];
        bool same = TRUE;
        for (u8 i = 0; i < 16 && same; i++) same = (pa->data[i] == pb->data[i]);
        const u16 need = stage_def->bg_tiles_n[to_map];
        if (spare + used + need <= TRANS_VRAM_END) {
            // PAL3 holds the OUTGOING room's parallax colours for the whole
            // slide, so a destination with another palette is shown through
            // the nearest outgoing colour per pixel (a ~0.5 s approximation;
            // stage_load restores the true palette). The colourcycle band is
            // excluded: those entries rotate and would make the art flicker.
            u8 remap[16];
            const u8 lim = stage_def->cycle_n ? (u8)stage_def->cycle_base : 16;
            for (u8 c = 0; c < 16; c++) {
                remap[c] = c;
                if (same || c == 0) continue;
                const u16 w = pb->data[c];
                u16 best = 0xFFFF;
                for (u8 k = 1; k < lim; k++) {
                    const u16 v = pa->data[k];
                    s16 db = (s16)((w >> 9) & 7) - (s16)((v >> 9) & 7);
                    s16 dg = (s16)((w >> 5) & 7) - (s16)((v >> 5) & 7);
                    s16 dr = (s16)((w >> 1) & 7) - (s16)((v >> 1) & 7);
                    const u16 d = (u16)(db * db + dg * dg + dr * dr);
                    if (d < best) { best = d; remap[c] = k; }
                }
            }
            bg_tiles_new = (u16)(spare + used);
            // one entry per 32-tile block, so a big parallax set is chunked too
            for (u16 t = 0; t < need; t += TRANS_DMA_CHUNK) {
                const u16 n = (u16)((need - t > TRANS_DMA_CHUNK)
                                        ? TRANS_DMA_CHUNK : need - t);
                trans_queued = TRANS_DMA_CHUNK;   // force a drain before it
                trans_dma_room(1);
                const u32 *src = stage_def->bg_tiles[to_map] + t * 8;
                if (!same) {
                    // trans_tiles is idle by now (the window's tiles are in
                    // VRAM and the queue was just drained)
                    u32 *dst = trans_tiles;
                    for (u16 k = 0; k < n * 8; k++) {
                        const u32 v = src[k];
                        u32 o = 0;
                        for (u8 px = 0; px < 8; px++)
                            o |= (u32)remap[(v >> (px * 4)) & 15] << (px * 4);
                        dst[k] = o;
                    }
                    src = dst;
                }
                VDP_loadTileData(src, (u16)(bg_tiles_new + t), n, DMA_QUEUE);
            }
            used = (u16)(used + need);
            dest_bg = TRUE;
        }
    }

    // drain whatever tile transfers are still queued before the slide starts
    SYS_doVBlankProcess();
#ifdef MD_TRANS_DEBUG
    dbg_log("TRS tiles_used=%u numTile=%u dest_bg=%u bg_new=%u\n", used,
            ts.numTile, dest_bg, bg_tiles_new);
    dbg_check_in("post-load", spare);
    dbg_check_out("post-load", from_map);
#endif

    // Rewrite the outgoing half so it looks the same with BG_B's vertical
    // scroll taken out (rows for the incoming half are filled as they arrive).
    if (stage_def->bg_info[from_map].present) {
        static u16 bgrow[64];
        for (s16 r = 0; r < TRANS_ROWS; r++) {
            bg_row_cells(bgrow, from_map, r, stage_bg_tile_base, vscroll);
            VDP_setTileMapData(VDP_BG_B, bgrow, (u16)(r * 64), 64, 2, CPU);
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
    prev_build(to_map, dcam);
    const u16 span = TRANS_LINES;                 // one screen in pixels
    const u16 steps = span / TRANS_STEP;
    // STREAMING SLIDE (transition_plan.md phase 3): BG_A is a ring of 32 rows (256 px)
    // moved with the hardware vertical scroll, so the picture advances TRANS_STEP px
    // EVERY frame. `trans_buf` holds the two screens stacked (56 rows); stacked row
    // `s` lives in plane row s & 31, and only the row that scrolls into view is
    // written (one 84-byte DMA) instead of the whole 42x28 map every step.
    s16 w_lo = 32767, w_hi = -1;      // stacked rows already in the ring
    for (u16 i = 0; i <= steps; i++) {
        const s16 off = top_to_bottom ? (s16)(i * TRANS_STEP)
                                      : (s16)(span - i * TRANS_STEP);
        {
            const s16 r_lo = (s16)(off >> 3);
            const s16 r_hi = (s16)((off + TRANS_LINES - 1) >> 3);
            if (w_hi < 0) { w_lo = (s16)(r_hi + 1); w_hi = r_hi; }   // first frame: whole window
            while (w_lo > r_lo) {
                w_lo--;
                VDP_setTileMapDataRect(STAGE_PLANE, &trans_buf[w_lo][0], 0,
                                       (u16)(w_lo & 31), TRANS_COLS, 1,
                                       TRANS_COLS, DMA_QUEUE);
            }
            while (w_hi < r_hi) {
                w_hi++;
                VDP_setTileMapDataRect(STAGE_PLANE, &trans_buf[w_hi][0], 0,
                                       (u16)(w_hi & 31), TRANS_COLS, 1,
                                       TRANS_COLS, DMA_QUEUE);
            }
            // VSync variant: the write lands in the same vblank as the queued row
            // (an immediate VSRAM write would tear the frame still being drawn).
            VDP_setVerticalScrollVSync(STAGE_PLANE, (s16)(off & 255));
        }

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
                // BG_A: the boundary moves by pixels (stacked line z = off + y);
                // BG_B (parallax) switches by 8px rows, like its tile rows.
                const s16 z = (s16)(off + (s16)y);
                const bool a_new = top_to_bottom ? (z >= (s16)span) : (z < (s16)span);
                trans_hs_a[y] = a_new ? new_hs : old_hs;
                trans_hs_b[y] = is_new ? bg_new : bg_old;
            }
            // Only the lines whose value changed since the last frame go to the
            // VDP (the boundary between the halves moves a few lines per frame).
            hs_send(STAGE_PLANE, trans_hs_a, hs_sent_a, i == 0);
            hs_send(BG_B, trans_hs_b, hs_sent_b, i == 0);
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
                            bg_row_cells(bgrow, to_map, r, bg_tiles_new, vscroll_new);
                        } else {
                            for (u16 x = 0; x < 64; x++)
                                bgrow[x] = TILE_ATTR_FULL(
                                    BG_PAL, FALSE, FALSE, FALSE,
                                    stage_sky_tile());
                        }
                        VDP_setTileMapData(VDP_BG_B, bgrow, (u16)(r * 64), 64, 2,
                                           DMA_QUEUE);
                    }
                }
                from = (s16)((bg_filled_hi + 1 > lo) ? bg_filled_hi + 1 : lo);
                to = hi;
                if (from <= to) {
                    for (s16 r = from; r <= to; r++) {
                        if (dest_bg) {
                            bg_row_cells(bgrow, to_map, r, bg_tiles_new, vscroll_new);
                        } else {
                            for (u16 x = 0; x < 64; x++)
                                bgrow[x] = TILE_ATTR_FULL(
                                    BG_PAL, FALSE, FALSE, FALSE,
                                    stage_sky_tile());
                        }
                        VDP_setTileMapData(VDP_BG_B, bgrow, (u16)(r * 64), 64, 2,
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
            const s16 py = (s16)(old_sy + (s16)(((s32)(player_y - old_sy) * (s32)i) / (s32)steps));
            SPR_setPosition(player.spr, (s16)(player_sx + player_sprite_dx()), py);
        }

        prev_place(top_to_bottom ? (s16)(span - off) : (s16)-off);
#ifdef MD_LINK_TRACE
        if (i == 0 || i == steps)
            dbg_print("SLD i=%d off=%d dcam=%d ocam=%d old_sy=%d plr_y=%d", (int)i, (int)off,
                      (int)dcam, (int)ocam, (int)old_sy, (int)player_y);
#endif
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);   // see main.c: DMA clobbers the window regs
        VDP_setReg(18, 0);
        for (u8 h = 1; h < TRANS_HOLD; h++) {
            SYS_doVBlankProcess();
            VDP_setReg(17, 0);
            VDP_setReg(18, 0);
        }
#ifdef MD_TRANS_DEBUG
        dbg_frame(i, (u16)(off / 8), spare, from_map);
#endif
#ifdef MD_TRANS_FREEZE
        // Debug: single-step the slide. It holds at frame MD_TRANS_FREEZE and
        // every press of A runs 3 more frames, so BlastEm's own `p` screenshot
        // (a faithful framebuffer) can be taken at each stage of one transition.
        if (i >= MD_TRANS_FREEZE && ((i - MD_TRANS_FREEZE) % 3) == 0) {
            while (JOY_readJoypad(JOY_1) & BUTTON_A) SYS_doVBlankProcess();
            while (!(JOY_readJoypad(JOY_1) & BUTTON_A)) SYS_doVBlankProcess();
        }
#endif
    }

#ifdef MD_TRANS_DEBUG
    dbg_check_in("end", spare);
    dbg_check_out("end", from_map);
#endif
    prev_free();
    // back to one offset per plane; the caller's stage_load re-scrolls both
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setVerticalScroll(BG_B, vscroll);
    return TRUE;
}
