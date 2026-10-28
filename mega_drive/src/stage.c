#include "stage.h"
#include "pal_safe.h"
#include "sfx.h"

#include "resources.h"
#include "enemy.h"
#include "explosion.h"
#include "effects.h"
#include "tooltip.h"
#include "object.h"
#include "player.h"
#include "shot.h"
#include "eshot.h"


Map *stage_map;
Vect2D_s16 camera;
u8 stage_current_map;
s16 stage_pending_camera_x = -1;
bool stage_load_in_place;

#define STAGE_MASK_BYTES ((stage_def->map_w * stage_def->map_h + 7) / 8)

static u8 anim_frame[STAGE_ANIM_SLOT_MAX];
static u16 anim_timer[STAGE_ANIM_SLOT_MAX];

u16 stage_bg_tile_base;
u16 stage_anim_tile_base;

// Sprite-engine generation (see stage.h): every pool destroy/create bumps it,
// so modules can tell that the handles they hold are stale and must not be
// released into the new pool.
u16 spr_gen = 1;

void sprites_begin(u16 vram)
{
    SPR_initEx(vram);
    spr_gen++;
}

void sprites_end(void)
{
    SPR_end();
    spr_gen++;
}

// Per-map VRAM layout (see stage.h): tileset, then parallax, then anim frames.
u16 stage_layout_bg(u8 map)
{
    return TILE_USER_INDEX + stage_def->tile_sets[map]->numTile;
}

u16 stage_layout_anim(u8 map)
{
    return stage_layout_bg(map) +
           (stage_def->bg_pal[map] ? stage_def->bg_tiles_n[map] : 0);
}

// The current map's terrain slice, cached at stage_load: this function is the
// hottest in the frame (every physics probe, enemy, object and projectile goes
// through it), and the generic form pays a 32-bit multiply for the map offset
// on every call.
static const u8 *terrain_cur;

u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY)
{
    if (map >= stage_def->map_count) return TERRAIN_SOLID;
    if (tileX < 0 || tileX >= stage_def->map_w) return TERRAIN_SOLID;
    if (tileY < 0 || tileY >= stage_def->map_h) return TERRAIN_UNBLOCKED;
    const u8 *base = (map == stage_current_map && terrain_cur)
                         ? terrain_cur
                         : stage_def->terrain
                               + (u16)map * stage_def->map_w * stage_def->map_h;
    return base[tileY * stage_def->map_w + tileX];
}

// Parallax background on BG_B from the decomposed art (tools/img2tiles.py):
// the unique tiles + a 64-wide tilemap replace the old full-image draw, so
// VRAM holds ~232 tiles instead of 768 (bg0) and ~26 instead of 896 (bg1) —
// bg1's tiles used to run into the animated-tile block.
static void apply_bg_color(u8 map_index);   // PC map_data.background_color

// A tile whose pixels are all palette index 0: on a plane that is transparent,
// so whatever is behind (the backdrop colour = PC background_color) shows.
static const u32 zero_tile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

u16 stage_sky_tile(void)
{
    return SPR_CFG_BLACK_TILE;
}

static void stage_sky_tile_load(void)
{
    VDP_loadTileData(zero_tile, SPR_CFG_BLACK_TILE, 1, DMA_QUEUE);
}

static void bg_upload_tiles_maps(u8 map_index)
{
    if (!stage_def->bg_pal[map_index]) return;
    stage_sky_tile_load();
    pal_load(BG_PAL, stage_def->bg_pal[map_index]->data);
    VDP_loadTileData(stage_def->bg_tiles[map_index], stage_bg_tile_base,
                     stage_def->bg_tiles_n[map_index], DMA);

    const u16 *m = stage_def->bg_maps[map_index];
    u16 row[64];
    // Rows the background art does not reach are NOT the art's first tile: the
    // PC clears the screen with map_data.background_color and draws the image
    // over it. A blank (transparent) tile lets that colour - the MD backdrop,
    // set by apply_bg_color() - show through.
    const u16 sky = TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                   stage_sky_tile());
    for (u16 y = 0; y < 32; y++) {
        for (u16 x = 0; x < 64; x++) {
            row[x] = (y < stage_def->bg_map_h[map_index])
                         ? TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                          stage_bg_tile_base + m[y * 64 + x])
                         : sky;
        }
        VDP_setTileMapData(VDP_BG_B, row, y * 64, 64, 2, CPU);
    }
}

// BG_B's current scroll, mirrored here: the tooltip backs its box with a fill
// tile on that plane (BG index 0 is transparent, BUG-042), so it must know
// where a screen cell actually lands on the scrolled parallax plane.
s16 stage_bg_hscroll;
s16 stage_bg_vscroll;

// Draw the parallax background of the given map on BG_B.
// PC E_BG_SCROLL_MODE (defines.h:1332): the background can move on its own,
// independent of the camera. The PC accumulates bg_scroll per frame; here the
// offset is derived from the frames since the map loaded, which cannot drift
// and is also valid in the middle of a room transition.
static u16 bg_tick;
static s16 bg_base_vscroll;

static s16 bg_auto_hscroll(u8 map)
{
    const StageBgInfo *i = &stage_def->bg_info[map];
    if (i->auto_scroll != 3 && i->auto_scroll != 4 && i->auto_scroll != 5)
        return 0;
    const s8 spd = i->speed ? i->speed : 1;
    s32 d = ((s32)bg_tick * spd) / 10;
    d %= 512;
    return (i->auto_scroll == 4) ? (s16)d : (s16)-d;   // 3 LEFT, 5 DIAGONAAL
}

static s16 bg_auto_vscroll(u8 map)
{
    const StageBgInfo *i = &stage_def->bg_info[map];
    if (i->auto_scroll != 1 && i->auto_scroll != 2 && i->auto_scroll != 5)
        return 0;
    const s8 spd = i->speed ? i->speed : 1;
    s32 d = ((s32)bg_tick * spd) / 10;
    d %= 256;                      // the plane is 32 rows tall
    return (i->auto_scroll == 2) ? (s16)d : (s16)-d;   // 1 UP, 2 DOWN
}

s16 stage_bg_auto_hscroll(u8 map) { return bg_auto_hscroll(map); }
s16 stage_bg_auto_vscroll(u8 map) { return bg_auto_vscroll(map); }
void stage_bg_tick(void) { bg_tick++; }

static void bg_load(u8 map_index)
{
    VDP_clearPlane(BG_B, TRUE);
    stage_bg_hscroll = 0;
    bg_tick = 0;
    VDP_setHorizontalScroll(BG_B, 0);

    // The blank sky tile is shared with the config screen (SPR_CFG_BLACK_TILE,
    // see stage.h): config loads its own opaque fill there, so it must be put
    // back on EVERY map load - not only when this map has parallax art, and not
    // only when the map is entered from the transition (BUG-044).
    stage_sky_tile_load();

    const StageBgInfo *info = &stage_def->bg_info[map_index];
    if (!info->present || !stage_def->bg_pal[map_index]) return;
    bg_upload_tiles_maps(map_index);
}

static void clamp_camera(void)
{
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > (stage_def->map_w * stage_def->tile) - 320) camera.x = (stage_def->map_w * stage_def->tile) - 320;
    if (camera.y > (stage_def->map_h * stage_def->tile) - 224) camera.y = (stage_def->map_h * stage_def->tile) - 224;
}

// ---- PC scroll lock (classmap.cpp:459/475) ------------------------------
static bool column_locked(s16 tile_x)
{
    if (tile_x < 0 || tile_x >= stage_def->map_w) return FALSE;
    const u8 *cols = stage_def->scroll_lock
                      + (u16)stage_current_map * stage_def->scroll_lock_max;
    const u8 n = stage_def->scroll_lock_n[stage_current_map];
    for (u8 i = 0; i < n; i++)
        if (cols[i] == tile_x) return TRUE;

    // PC classmap.cpp:171-191: a boss door locks the columns it occupies even
    // when the map tiles under it are empty. Use the live door state: once it
    // has opened, those columns must no longer hold the camera back.
    if (objects_boss_door_locked(tile_x)) return TRUE;
    return FALSE;
}

static void scroll_lock_camera(s16 prev_x)
{
    if (camera.x == prev_x) return;

    if (camera.x > prev_x) {
        s16 c0 = (prev_x + 320 - 16 + 2) >> 4;
        s16 c1 = (camera.x + 320 - 16 + 2) >> 4;
        if (column_locked(c0)) {
            camera.x = prev_x;
        } else {
            for (s16 c = c0 + 1; c <= c1; c++) {
                if (column_locked(c)) {
                    camera.x = (c << 4) - (320 - 16 + 2) - 1;
                    break;
                }
            }
        }
    } else {
        s16 c0 = (prev_x + 16 - 2) >> 4;
        s16 c1 = (camera.x + 16 - 2) >> 4;
        if (column_locked(c0)) {
            camera.x = prev_x;
        } else {
            for (s16 c = c0 - 1; c >= c1; c--) {
                if (column_locked(c)) {
                    camera.x = (c << 4) + 2;
                    break;
                }
            }
        }
    }
}

static void scroll_lock_span(s16 focus_x)
{
    // The camera must stay >= (c<<4)+2 for the nearest locked column c on the
    // player's left and <= (c<<4)-307 for the nearest one on his right (the
    // runtime scroll_lock_camera() limits). The old version looked only at the
    // columns strictly inside the view it started from, so a lock a few pixels
    // outside it (or one that only became relevant after the first push) let
    // the arrival camera sit beyond the lock - tools/check_arrival_locks.py.
    for (u8 pass = 0; pass < 4; pass++) {
        const s16 old = camera.x;
        const s16 c0 = (camera.x >> 4) - 2, c1 = ((camera.x + 320) >> 4) + 2;
        for (s16 c = c0; c <= c1; c++) {
            if (!column_locked(c)) continue;
            if (focus_x >= ((s16)c << 4) + 8) {
                if (camera.x < ((c << 4) + 2) && ((c << 4) + 2) <= focus_x)
                    camera.x = (c << 4) + 2;
            } else {
                if (camera.x > ((c << 4) - 307)) camera.x = (c << 4) - 307;
            }
        }
        if (camera.x == old) break;
    }
}

// The room around world X `x`: the nearest scroll-locked columns (room walls /
// closed boss doors) on each side, exclusive bounds in tile columns. A room can
// be narrower than the 320px view, so enemies of the room next door are on
// screen yet walled off from the player; they must not act (or shoot) across
// the wall. Computed once per frame by enemies_update, not per enemy.
void stage_room_span(s16 x, s16 *lo, s16 *hi)
{
    const s16 col = x >> 4;
    s16 l = -1, h = (s16)stage_def->map_w;
    for (s16 c = col - 1; c >= 0; c--)
        if (column_locked(c)) { l = c; break; }
    for (s16 c = col + 1; c < h; c++)
        if (column_locked(c)) { h = c; break; }
    *lo = l;
    *hi = h;
}

// ---- animated tiles + above-player tile3 ------------------------------
// Animations are compacted PER MAP (only the visible map needs VRAM): the
// converter emits one slot list per map plus a per-cell slot map (0 = none,
// else slot+1). stage_load loads the current map's slots at stage_anim_tile_base.
static void stage_map_patch(Map *map, u16 *buf, u16 x, u16 y,
                            MapUpdateType type, u16 size)
{
    const u16 mw = stage_def->map_w, mh = stage_def->map_h;
    const u8 *cell_slot = stage_def->anim_cell_slot + stage_current_map * mw * mh;
    for (u16 i = 0; i < size; i++) {
        u16 tx = (type == COLUMN_UPDATE) ? x : x + i;
        u16 ty = (type == COLUMN_UPDATE) ? y + i : y;
        u16 cx = tx >> 1, cy = ty >> 1;
        if (cx >= mw || cy >= mh) continue;
        u16 n = cy * mw + cx;
        const u16 moff = stage_current_map * STAGE_MASK_BYTES + (n >> 3);
        const u16 mbit = 1 << (n & 7);
        bool t3 = stage_def->tile3_mask[moff] & mbit;
        bool p3 = stage_def->pal3_mask && (stage_def->pal3_mask[moff] & mbit);
        u8 sv = cell_slot[n];
        if (sv) {
            u16 attr = (map->baseTile & TILE_ATTR_PALETTE_MASK) |
                       (t3 ? TILE_ATTR_PRIORITY_MASK : 0);
            u16 quad = (tx & 1) | ((ty & 1) << 1);
            buf[i] = attr | (stage_anim_tile_base
                             + (u16)(sv - 1) * TILE_ANIM_STRIDE + quad);
        } else if (t3) {
            buf[i] |= TILE_ATTR_PRIORITY_MASK;
        }
        // Colourcycled cell: its art carries PAL3 indices, so switch the
        // palette line here (the priority bit set above is preserved).
        if (p3)
            buf[i] = (buf[i] & ~TILE_ATTR_PALETTE_MASK) |
                     ((u16)STAGE_PAL_CYCLE << TILE_ATTR_PALETTE_SFT);
    }
}

// ---- colourcycle --------------------------------------------------------
// The pool lives in the PAL3 entries the decomposed parallax leaves free, so
// it is written after bg_load() has put the parallax palette in place.
static void stage_cycle_palette_load(void)
{
    if (!stage_def->cycle_n) return;
    const u16 cram = TILE_CYCLE_CRAM;
    PAL_setColor(cram + TILE_CYCLE_STABLE, 0x0000);
    for (u8 i = 0; i < stage_def->cycle_n; i++)
        PAL_setColor(cram + stage_def->cycle_base + i,
                     stage_def->cycle_colors[i]);
    for (u8 i = 0; i < stage_def->cycle_n; i++)
        PAL_setColor(cram + stage_def->cycle_base + i,
                     stage_def->cycle_colors[i]);
}

// MAP_create() paints the whole plane with the base palette (PAL1), so the
// cells whose art is PAL3-relative need their palette bits fixed once after
// the load's full draw. 20x14 metatiles is the NTSC screen.
static void stage_cycle_pal3_apply(void)
{
    if (!stage_def->pal3_mask || !stage_def->cycle_n) return;
    const u8 *mask = stage_def->pal3_mask + stage_current_map * STAGE_MASK_BYTES;
    const u16 mw = stage_def->map_w;
    const u16 pal3 = (u16)STAGE_PAL_CYCLE << TILE_ATTR_PALETTE_SFT;
    u16 row[40 * 2];        // 20x1 metatiles = 40 cells x 2 rows
    for (u16 my = 0; my < 14; my++) {
        MAP_getTilemapRect(stage_map, 0, my, 20, 1, FALSE, row);
        bool changed = FALSE;
        for (u16 mx = 0; mx < 20; mx++) {
            const u16 n = my * mw + mx;
            if (!(mask[n >> 3] & (1 << (n & 7)))) continue;
            changed = TRUE;
            for (u16 k = 0; k < 4; k++) {
                u16 *e = &row[mx * 2 + (k & 1) + ((k >> 1) * 40)];
                *e = (*e & ~TILE_ATTR_PALETTE_MASK) | pal3;
            }
        }
        if (changed)
            VDP_setTileMapDataRect(STAGE_PLANE, row, 0, my * 2, 40, 2, 40,
                                   DMA_QUEUE);
    }
}

static u16 cycle_timer;
static u8 cycle_step;
static u16 cycle_pending[STAGE_CYCLE_MAX];
static bool cycle_dirty;

// The rotation is written from the vblank callback: a *queued* CRAM DMA does
// not land on this setup (see the PAL0 note in stage_load), and a CPU write
// during active display would show a horizontal seam wherever the change
// took effect.
static void stage_cycle_vblank(void)
{
    if (!cycle_dirty) return;
    cycle_dirty = FALSE;
    for (u8 i = 0; i < stage_def->cycle_n; i++)
        PAL_setColor(TILE_CYCLE_CRAM + stage_def->cycle_base + i,
                     cycle_pending[i]);
}

static void stage_cycle_reset(void)
{
    cycle_timer = 0;
    cycle_step = 0;
    cycle_dirty = FALSE;
    SYS_setVBlankCallback(stage_cycle_vblank);
    stage_cycle_palette_load();
}

void stage_cycle_update(void)
{
    if (!stage_def->cycle_n || !stage_def->cycle_frames) return;
    // The tooltip borrows PAL3 for its art (tooltip.c); writing the rotating
    // colours under it would repaint its icon and its white ink.
    if (tooltip_owns_pal3()) return;
    if (++cycle_timer < stage_def->cycle_frames) return;
    cycle_timer = 0;
    cycle_step = (u8)((cycle_step + 1) % stage_def->cycle_n);
    for (u8 i = 0; i < stage_def->cycle_n; i++)
        cycle_pending[i] =
            stage_def->cycle_colors[(cycle_step + i) % stage_def->cycle_n];
    cycle_dirty = TRUE;
}

// Rewrite the visible map cells of animated slot `s` to use `base` (the
// current frame's four tiles). The converted map initially points at the
// static art in the tileset, so this must also run for frame 0; otherwise
// Daisie's palm trees all show the same tile until the first animation tick.
static void stage_anim_reset(void)
{
    u8 n = stage_def->anim_slot_n[stage_current_map];
    for (u8 s = 0; s < n; s++) {
        u16 idx = stage_current_map * stage_def->anim_slot_max + s;
        anim_frame[s] = 0;
        anim_timer[s] = stage_def->anim_slots[idx].delay[0];
        u16 base = stage_anim_tile_base + s * TILE_ANIM_STRIDE;
        VDP_loadTileData((const u32 *)stage_def->anim_frames[idx][0],
                         base, 4, DMA_QUEUE);
    }
}

void stage_anim_update(void)
{
    u8 n = stage_def->anim_slot_n[stage_current_map];
    for (u8 s = 0; s < n; s++) {
        u16 idx = stage_current_map * stage_def->anim_slot_max + s;
        const StageAnimSlot *slot = &stage_def->anim_slots[idx];
        if (slot->frames < 2) continue;
        if (anim_timer[s] > 1) { anim_timer[s]--; continue; }
        anim_frame[s]++;
        if (anim_frame[s] >= slot->frames) anim_frame[s] = 0;
        anim_timer[s] = slot->delay[anim_frame[s]];
        // The slot's cells always point at its own 4 tiles, so animating is a
        // single 4-tile DMA into that block - no tilemap writes.
        u16 base = stage_anim_tile_base + s * TILE_ANIM_STRIDE;
        VDP_loadTileData((const u32 *)stage_def->anim_frames[idx][anim_frame[s]],
                         base, 4, DMA_QUEUE);
    }
}

bool stage_map_ready(void)
{
    return stage_map != NULL;
}

void stage_load(u8 map_index)
{
    if (map_index >= stage_def->map_count) map_index = 0;
    stage_current_map = map_index;
    terrain_cur = stage_def->terrain
                  + (u16)map_index * stage_def->map_w * stage_def->map_h;

    bool wasEnabled = VDP_isEnable();
    const bool in_place = stage_load_in_place;
    stage_load_in_place = FALSE;
    if (!in_place) {
        VDP_setEnable(FALSE);
        VDP_clearPlane(BG_A, TRUE);
        VDP_clearPlane(BG_B, TRUE);
    }

    // Load tileset and palettes. CRAM writes are only reliable with the
    // display disabled (or during vblank): PAL1/PAL3 use CPU writes here
    // and work; PAL0 used a queued DMA that does not land in this window
    // (the CRAM readback showed the enemies/objects/HUD line still holding
    // another, one-step-darker palette).
    if (in_place)
        PAL_setPalette(STAGE_PAL, stage_def->palettes[map_index]->data,
                       DMA_QUEUE);
    else
        pal_load(STAGE_PAL, stage_def->palettes[map_index]->data);
    // PAL0 = shared font + enemy/object/HUD palette. Loaded from the
    // rescomp PALETTE resource (same mechanism as the working PAL1/PAL3
    // lines) instead of the C array, which did not stick: the CRAM readback
    // showed PAL0 holding another palette while PAL1 decoded exactly.
    if (in_place)
        PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);
    else
        pal_load(PAL0, pal_ui.data);
    apply_bg_color(map_index);

    // Flush anything the previous map queued, then drop it BEFORE the tileset
    // load. A compressed tileset allocated its unpack buffer from the heap,
    // and with the old map still resident that allocation could fail — the
    // library then left the PREVIOUS tileset in VRAM and the new map rendered
    // with garbage tiles after a room transition. Stage tilesets are stored
    // uncompressed now (res/resources.res NONE), and freeing first also gives
    // MAP_create the largest contiguous block.
    // In-place: keep the transition's last frame displayed. Do not flush the
    // queued palette/tileset now; they flush together at the end, after the
    // real map tilemap is written (otherwise T2B shows one white frame).
    if (!in_place) {
        SYS_doVBlankProcess();
    } else {
        // Flush the queued palette while the transition's last frame is still
        // shown, then load the base tileset synchronously. MAP_scrollToEx
        // below writes tilemap entries that reference these base tiles; if the
        // tileset is only queued, that first real-map frame shows blank/white
        // tiles (the T2B stair flash).
        SYS_doVBlankProcess();
    }
    stage_release_map();
    VDP_loadTileSet(stage_def->tile_sets[map_index], TILE_USER_INDEX, DMA);
    stage_bg_tile_base = stage_layout_bg(map_index);
    stage_anim_tile_base = stage_layout_anim(map_index);

    stage_map = MAP_create(stage_def->map_defs[map_index], STAGE_PLANE,
                           TILE_ATTR_FULL(STAGE_PAL, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    if (!stage_map) {
        // Out of heap: bail with the display restored (leaving it disabled
        // here would black the screen silently). BUG-040: the caller must not
        // keep going with a NULL map (drawing it crashed in setMapRow).
        kprintf("stage_load: MAP_create failed (free=%u largest=%u)\n",
                (unsigned)MEM_getFree(),
                (unsigned)MEM_getLargestFreeBlock());
        VDP_setEnable(wasEnabled);
        return;
    }
    MAP_setDataPatchCallback(stage_map, stage_map_patch);

    camera.x = (stage_pending_camera_x >= 0) ? stage_pending_camera_x : 0;
    camera.y = 0;
    stage_pending_camera_x = -1;
    MAP_scrollToEx(stage_map, camera.x, camera.y, TRUE);   // full draw on first load

    bg_load(map_index);
    stage_cycle_pal3_apply();                // PAL3 art: fix the palette bits
    stage_cycle_reset();

    // enemies/shots/objects are per-map: release the old ones, spawn this map's
    shots_reset();
    eshots_reset();
    explosion_reset();
    effects_reset();
    tooltip_hide();
    enemies_reset();
    objects_reset();

    // animated-tile frame 0 must be in VRAM before the first full draw
    stage_anim_reset();

    // Flush any pending transfers and restore display
    SYS_doVBlankProcess();
    if (!in_place) VDP_setEnable(wasEnabled);


    // BG vertical scroll (PC draws the bg at screen y = adjust_y, classmap
    // .cpp:614). VSRAM = the plane row at screen top, so pushing the image
    // DOWN by adjust_y means vsram = -adjust_y. Must be written with the
    // VDP enabled — the bg_load write landed inside the disabled window
    // and was lost (art sat at y=0: moon at top, skyline mid-screen).
    // BG vertical scroll (PC draws the bg at screen y = adjust_y, classmap
    // .cpp:614). VSRAM = the plane row at screen top, so pushing the image
    // DOWN by adjust_y means vsram = -adjust_y. Must be written with the
    // VDP enabled — the bg_load write landed inside the disabled window
    // and was lost (art sat at y=0: moon at top, skyline mid-screen).
    // The -16 corrects for the cropped top PC row (240 -> 224 screen).
    if (stage_def->bg_info[map_index].present) {
        bg_base_vscroll =
            (s16)(-((s16)stage_def->bg_info[map_index].adjust_y - 16));
        stage_bg_vscroll =
            (s16)(bg_base_vscroll + bg_auto_vscroll(map_index));
        VDP_setVerticalScroll(BG_B, stage_bg_vscroll);
    } else {
        stage_bg_vscroll = 0;
        bg_base_vscroll = 0;
        VDP_setVerticalScroll(BG_B, 0);
    }
}

// PC test_teleport/check_player_is_on_teleport (game.cpp:638-661, 771-835).
// Transition bands are in pixels: base lim3/lim4 = ty*16+8 / ty*16+16,
// widened per transition (PC game.cpp:793-798, float factors truncated).
// MD ty/py are the cropped-row equivalents of PC's, so the same constants
// land on the same windows (down: py > ty*16+16, up: py < ty*16+16 at
// ty=0 with lim3 = -19). Fires only while the player actually crosses the
// band (on-ladder iy 204..207 descending, -16..-6 ascending for stage 0).
#define LINK_TRANS_T2B 0
#define LINK_TRANS_B2T 1

static bool link_region(s16 px, s16 py, s16 py_centre, u8 tx, u8 ty,
                        u8 size, u8 trans)
{
    // PC check_player_is_on_teleport: lim2 = temp_x*TILESIZE + size*TILESIZE
    // (size is the band width in tiles; wide stairways use it).
    s16 lim1 = (s16)tx * stage_def->tile;
    s16 lim2 = lim1 + (s16)(size ? size : 1) * stage_def->tile;
    if (px < lim1 || px > lim2)
        return FALSE;
    s16 lim3 = (s16)ty * stage_def->tile + 8;
    s16 lim4 = (s16)ty * stage_def->tile + stage_def->tile;
    if (trans == LINK_TRANS_T2B) {
        lim3 += 8;      // PC lim3 += TILESIZE * 0.5
        lim4 += 24;     // PC lim4 += TILESIZE * 1.5
    } else {
        lim3 -= 27;     // PC lim3 -= 27.2 truncated (only ty=0 bands fire)
    }
    // Links on the last map row: the player is clamped or standing on the
    // bottom edge, so he can never satisfy py < lim4. Fire as soon as the
    // feet enter the link row (py = bottom - 8), which is where a ladder
    // releases him. The PC's T2B band shift would put lim3 below the map.
    // Bottom map row: judge by the BODY CENTRE, not the feet sample. With the
    // feet rule the band started 8 px above the row, so an arrival clamped to
    // the bottom landed inside it and had to be nudged a whole tile up the
    // ladder (BUG-005). Centre + row top keeps him at the bottom of the
    // screen and still fires as soon as he descends into the last row.
    if (ty >= (u8)(stage_def->map_h - 1))
        return py_centre >= (s16)ty * stage_def->tile;
    return py > lim3 && py < lim4;
}

// TRUE while the player box sits inside ANY link band of the current map,
// in either direction, with the same geometry stage_check_link uses but
// without the latch/suppressions. Used to nudge a link arrival out of the
// band it just came through (BUG-005: the player never left the link, so the
// return link stayed latched and never fired).
bool stage_link_band_hit(s16 left, s16 top, s16 right, s16 bottom)
{
    const s16 px = (left + right) >> 1;
    const s16 py = bottom - 8;

    for (u8 i = 0; i < stage_def->link_count; i++) {
        const StageLink *lk = &stage_def->links[i];
        u8 tx, ty, trans;

        if (lk->map_from == stage_current_map) {
            tx = lk->tile_x;
            ty = lk->tile_y;
            trans = (lk->tile_y > lk->dst_y) ? LINK_TRANS_T2B : LINK_TRANS_B2T;
        } else if (lk->map_to == stage_current_map && lk->bidir) {
            tx = lk->dst_x;
            ty = lk->dst_y;
            trans = (lk->tile_y < lk->dst_y) ? LINK_TRANS_T2B : LINK_TRANS_B2T;
        } else {
            continue;
        }
        if (link_region(px, py, (top + bottom) >> 1, tx, ty, lk->size, trans))
            return TRUE;
    }
    return FALSE;
}

s16 stage_link_band_off;   // player centre minus trigger band start (px)
u8 stage_link_type;        // PC LINK_TYPES of the link that just fired (>=2: teleporter)

bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y, u8 *to_trans)
{
    // Same link may not re-fire until the player leaves its band
    // (PC set_teleporter/get_teleporter, game.cpp:802-832). Survives
    // stage_load: on arrival the band still hits and would ping-pong.
    static s8 link_latch = -1;

    s16 px = (left + right) >> 1;   // PC px = x + width * 0.5 (game.cpp:778)
    s16 py = bottom - 8;            // PC py = y + h * 0.75 = y + 21 (game.cpp:779)

    for (u8 i = 0; i < stage_def->link_count; i++) {
        const StageLink *lk = &stage_def->links[i];
        u8 tx, ty, dst_map, dst_x, dst_y;
        u8 trans;
        bool hit;

        if (lk->map_from == stage_current_map) {
            // forward (game.cpp:638-649)
            tx = lk->tile_x;  ty = lk->tile_y;
            dst_map = lk->map_to;  dst_x = lk->dst_x;  dst_y = lk->dst_y;
            trans = (lk->tile_y > lk->dst_y) ? LINK_TRANS_T2B : LINK_TRANS_B2T;
        } else if (lk->map_to == stage_current_map && lk->bidir) {
            // reverse (game.cpp:650-661): only when the link is bidirectional
            tx = lk->dst_x;  ty = lk->dst_y;
            dst_map = lk->map_from;  dst_x = lk->tile_x;  dst_y = lk->tile_y;
            trans = (lk->tile_y < lk->dst_y) ? LINK_TRANS_T2B : LINK_TRANS_B2T;
        } else {
            continue;
        }

        hit = link_region(px, py, (top + bottom) >> 1, tx, ty, lk->size, trans);

        // A stair tile on the top row is the mouth of a ladder: the link is
        // only valid while the player is still climbing/hanging on it. Once
        // he steps off at the top, the PC band (lim3 -= 27) still covers the
        // floor below, which used to teleport him back down.
        if (hit && ty == 0 &&
            stage_terrain_at(stage_current_map, tx, ty) == TERRAIN_STAIR &&
            !player.on_ladder)
            hit = FALSE;

        // PC only fires a top/bottom transition when the player is partly
        // off-screen (game.cpp:801); teleporter links are exempt. A player
        // hanging on the ladder at the bottom/top is explicitly allowed:
        // the port clamps his feet flush with the map edge, so the old test
        // (bottom < 220) suppressed the bottom teleporter link entirely.
        const bool bottom_link = (ty >= (u8)(stage_def->map_h - 1));
        if (hit && lk->type < 2 && !player.on_ladder && !bottom_link &&
            top > 0 && bottom < 220) hit = FALSE;

        if (link_latch == (s8)i) {
            if (!hit) link_latch = -1;   // cleared on region miss only
            continue;
        }
        if (hit) {
            link_latch = (s8)i;
            // PC keeps the player's horizontal position on a scroll link and
            // moves the map instead (game.cpp:762). Save the offset inside
            // the trigger band so wide links (stage 3 uses sizes 4, 7, 8, 18)
            // land at the same relative place instead of snapping to the
            // destination tile centre.
            stage_link_band_off = px - (s16)tx * stage_def->tile;
            stage_link_type = lk->type;
            *to_map = dst_map;
            *to_x   = (s16)dst_x * stage_def->tile;
            *to_y   = (s16)dst_y * stage_def->tile;
            *to_trans = trans;
            return TRUE;
        }
    }
    return FALSE;
}

void stage_update_camera(s16 focusX, s16 focusY)
{
    const s16 cx = 160; // screen centre
    const s16 cy = 112;
    const s16 dzx = 12; // horizontal deadzone
    const s16 dzy = 8;  // vertical deadzone
    const s16 prev_x = camera.x;
    // PC QUAKE screen adjust: the player's pumpkin/quake weapon and any enemy
    // QUAKE projectile (APE BOT) both shake the planes by +-2px.
    const s16 shake = (s16)(shot_shake_x() + eshots_shake_x());

    if (focusX > camera.x + cx + dzx)       camera.x = focusX - cx - dzx;
    else if (focusX < camera.x + cx - dzx)  camera.x = focusX - cx + dzx;

    if (focusY > camera.y + cy + dzy)       camera.y = focusY - cy - dzy;
    else if (focusY < camera.y + cy - dzy)  camera.y = focusY - cy + dzy;

    clamp_camera();
    scroll_lock_camera(prev_x);
    if (stage_map) MAP_scrollTo(stage_map, camera.x + shake, camera.y);

    // Parallax: horizontal scroll derived DIRECTLY from camera.x at
    // speed/10, wrapped into (-512, 0]. PC accumulates camera_dx per frame
    // (classmap.cpp:449); deriving it from the absolute camera is equivalent
    // and cannot drift/jitter when the camera sits on a deadzone edge (the
    // accumulated version moved the bg a few px on its own).
    const StageBgInfo *info = &stage_def->bg_info[stage_current_map];
    if (info->present && (info->speed || info->auto_scroll)) {
        s16 scroll;
        if (info->auto_scroll == 3 || info->auto_scroll == 4 ||
            info->auto_scroll == 5) {
            // PC classmap.cpp:592: with an auto-scroll mode the background no
            // longer follows the camera, it moves by speed/10 per frame.
            scroll = bg_auto_hscroll(stage_current_map);
        } else {
            s32 v = -((s32)(camera.x + shake) * info->speed) / 10;
            v %= 512;
            if (v > 0) v -= 512;
            scroll = (s16)v;
        }
        stage_bg_hscroll = scroll;
        VDP_setHorizontalScroll(BG_B, scroll);
    }
    if (info->present && (info->auto_scroll == 1 ||
                          info->auto_scroll == 2 ||
                          info->auto_scroll == 5)) {
        stage_bg_vscroll = (s16)(bg_base_vscroll +
                                 bg_auto_vscroll(stage_current_map));
        VDP_setVerticalScroll(BG_B, stage_bg_vscroll);
    }
}

// PC game::horizontal_screen_move (game.cpp, called when a boss door has
// finished opening): shift the view by one screen with a short scroll. The
// door then closes behind the player, so it is one-way.
// PC game::horizontal_screen_move (game.cpp:1174), the door case: the view
// slides one screen while the player is pushed only ~2.5 tiles forward, so he
// ends up near the LEFT edge of the new screen (he used to be carried the full
// screen, which left him on the right).
//   TRANSITION_STEP 4 px, move_limit = RES_W/4 - TILESIZE/4 = 76 steps,
//   player_move_x = (TILESIZE*2.5)/move_limit per step (40 px total).
// The PC runs a step every ~2ms; on the MD two steps per frame keeps it smooth
// without being a single 320 px jump.
#define DOOR_SCROLL_STEP   4
#define DOOR_SCROLL_STEPS  ((320 / DOOR_SCROLL_STEP) - (16 / DOOR_SCROLL_STEP))
#define DOOR_PLAYER_MOVE_F 34      // 40 px over 76 steps, in 1/64 px (34/64 = 0.53)

void stage_horizontal_screen_move(s16 dir)
{
    const s16 max = stage_def->map_w * stage_def->tile - 320;
    const StageBgInfo *info = &stage_def->bg_info[stage_current_map];
    // 40 px over 76 steps, accumulated in 1/64 px so the player moves smoothly.
    fix16 player_acc = 0;
    for (u8 i = 0; i < DOOR_SCROLL_STEPS; i++) {
        camera.x += dir * DOOR_SCROLL_STEP;
        if (camera.x < 0) camera.x = 0;
        if (camera.x > max) camera.x = max;
        if (stage_map) MAP_scrollTo(stage_map, camera.x, camera.y);
        if (info->present && info->speed) {
            s32 scroll = -((s32)camera.x * info->speed) / 10;
            scroll %= 512;
            if (scroll > 0) scroll -= 512;
            stage_bg_hscroll = (s16)scroll;
            VDP_setHorizontalScroll(BG_B, (s16)scroll);
        }
        player_acc += DOOR_PLAYER_MOVE_F;
        player.x += F16_toInt(player_acc);
        player_acc &= 63;                     // keep the sub-pixel remainder
        player_sync_sprite();
        // The camera moved: doors/enemies/objects keep their WORLD position,
        // so their sprites must follow (the sub-boss door used to stay glued
        // to the screen while the stage slid under it).
        enemies_sync_sprites();
        objects_sync_sprites();
        if ((i & 1) == 0) {                   // 2 steps per frame
            SPR_update();
            SYS_doVBlankProcess();
        }
    }
    SPR_update();
    SYS_doVBlankProcess();
}

// Force a complete MAP redraw. Needed whenever the camera jumps a large
// distance in one frame (respawn, map link): the incremental scroll used by
// MAP_scrollTo only loads the newly exposed edge, so a jump leaves stale
// columns on screen (background "blinking").
void stage_force_redraw(void)
{
    // BUG-040: stage_load() can fail (no heap for MAP_create) and leaves
    // stage_map == NULL; redrawing then read a NULL map and crashed.
    if (!stage_map) return;
    clamp_camera();
    MAP_scrollToEx(stage_map, camera.x, camera.y, TRUE);
}

void stage_autoscroll(s8 dir)
{
    const s16 maxcam = stage_def->map_w * stage_def->tile - 320;
    s16 target = camera.x + dir * 320;
    if (target < 0) target = 0;
    if (target > maxcam) target = maxcam;
    s16 remain = target - camera.x;
    if (!remain) return;

    // Camera 8px/frame, player 1px/frame (PC: 4px + 0.5px over 76 frames at
    // 50Hz). Same 40-frame duration, player ends 40px (2.5 tiles) past the
    // lock, near the left/right edge of the incoming screen.
    const s16 cam_step = (remain > 0) ? 8 : -8;
    const s16 ply_step = (remain > 0) ? 1 : -1;
    u8 frames = (u8)((remain > 0 ? remain : -remain) / 8);
    for (u8 i = 0; i < frames; i++) {
        camera.x += cam_step;
        player.x += ply_step;
        if (stage_map) MAP_scrollTo(stage_map, camera.x, camera.y);
        player_sync_sprite();
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// Re-apply the colourcycle band after something else used PAL3 (the dialog
// panel loads its own palette into that line).
void stage_restore_cycle(void)
{
    stage_cycle_pal3_apply();
}

void stage_restore_palette(void)
{
    pal_load(STAGE_PAL, stage_def->palettes[stage_current_map]->data);
    apply_bg_color(stage_current_map);
}

// PC map_data.background_color: the backdrop colour behind both planes. CRAM 0
// belongs to PAL0 (UI/font), so it is written directly after that palette is
// loaded - the HUD and text use index 0 as "transparent" anyway.
static void apply_bg_color(u8 map_index)
{
    if (!stage_def->bg_color) return;
    const u8 *c = &stage_def->bg_color[map_index * 3];
    PAL_setColor(0, RGB3_3_3_TO_VDPCOLOR(c[0], c[1], c[2]));
}

void stage_redraw_bg(void)
{
    bg_load(stage_current_map);
}

// Reload the current map's animated tile frames. An overlay (the weapon
// menu) can overwrite the animation block with its own tiles; the BG reload
// does not cover it, so callers must restore it after the overlay closes.
void stage_redraw_anim(void)
{
    stage_anim_reset();
}

// Drop the stage map: its engine keeps deferred plane updates that flush on
// the next vblank and repaint BG_A (with the new tile contents) after an
// overlay/title took over the screen. stage_load() creates a fresh map.
void stage_release_map(void)
{
    if (stage_map) {
        MAP_release(stage_map);
        stage_map = NULL;
    }
}

// Link arrival: PC sets the new map scroll directly instead of walking the
// camera there (game.cpp:750 set_scrolling). Centre on the player, then
// keep any divider at a screen edge on the player's side — e.g. arriving at
// map0 (139,13) the view must not span the 137/138 divider.
void stage_arrival_camera(s16 focusX, s16 focusY)
{
    stage_arrival_camera_at(stage_arrival_camera_x(stage_current_map, focusX),
                            focusY);
}

void stage_arrival_camera_at(s16 cam_x, s16 focusY)
{
    const s16 old_x = camera.x;
    camera.x = cam_x;
    camera.y = focusY - 112;
    clamp_camera();
    // After stage_load, its first draw already used this exact camera. A
    // second full redraw here happens with the display enabled and is the
    // one-frame tilemap/TILESET flash seen after the Mummy stair.
    if (camera.x != old_x) stage_force_redraw();
}

// Same camera X, computed without touching the plane: used by the room
// transition, which needs it before the destination map is loaded (so
// column_locked() must see the destination map's scroll locks).
s16 stage_arrival_camera_x(u8 map, s16 focus_x)
{
    return stage_arrival_camera_keep(map, focus_x, 160);
}

// Same, but the camera is placed so `focus_x` lands at screen column
// `screen_x` instead of the middle. A room change keeps the player at the SAME
// screen X on both halves of the slide (PC game.cpp:716: new scroll = old scroll
// - teleporter_dist, i.e. the two link tiles line up); centring the destination
// instead left the halves up to 152px apart whenever the origin camera was
// clamped by a map edge or a scroll lock.
s16 stage_arrival_camera_keep(u8 map, s16 focus_x, s16 screen_x)
{
    const u8 save_map = stage_current_map;
    const s16 save_x = camera.x;
    const s16 save_y = camera.y;
    stage_current_map = map;
    camera.x = focus_x - screen_x;
    clamp_camera();
    scroll_lock_span(focus_x);
    // scroll_lock_span can push the camera negative: a locked column `c`
    // sets camera.x = (c<<4) - 307, which is -3 for c=19. The room
    // transition then loaded the destination screen with a negative camera
    // and drew garbage tiles (seen in ATLANTIDA, TRANS 0->1 cam-3).
    if (camera.x < 0) camera.x = 0;
    if (camera.x > (stage_def->map_w * stage_def->tile) - 320)
        camera.x = (stage_def->map_w * stage_def->tile) - 320;
    const s16 res = camera.x;
    camera.x = save_x;
    camera.y = save_y;
    stage_current_map = save_map;
    return res;
}

// PC get_first_lock_on_bottom (classmap.cpp:953): scanning up from the map
// bottom, the first column tile that has `above` clear tiles above it across
// `right` columns. Returns the free row just above the lock (PC i-1). The
// PC WATER/EASYBLOCK/HARDBLOCK exclusions match stage 0 (no water; the two
// difficulty pass-throughs never appear as column locks here).
static s16 first_lock_on_bottom(s16 tilex, s16 w, s16 h)
{
    s16 above = h / 16;
    if (above < 2) above = 2;
    s16 right = w / 16;
    if (right < 1) right = 1;

    for (s16 i = stage_def->map_h - 1; i >= above + 1; i--) {
        u16 t = stage_terrain_at(stage_current_map, tilex, i);
        // PC classmap.cpp:get_first_lock_on_bottom: WATER is not a lock, here or
        // in the tiles above. Treating it as one made the SEAHORSE boss
        // teleporter (a water column) find no ground and drop the player at
        // row 0, inside the ceiling.
        if (t == TERRAIN_UNBLOCKED || t == TERRAIN_WATER ||
            t == TERRAIN_EASYBLOCK || t == TERRAIN_HARDBLOCK)
            continue;
        bool bad = FALSE;
        for (s16 j = i - 1; j >= i - above && !bad; j--) {
            for (s16 k = 0; k < right; k++) {
                if (tilex + k >= stage_def->map_w) { bad = TRUE; break; }
                const u16 t2 = stage_terrain_at(stage_current_map, tilex + k, j);
                if (t2 != TERRAIN_UNBLOCKED && t2 != TERRAIN_WATER) {
                    bad = TRUE;
                    break;
                }
            }
        }
        if (!bad) return i - 1;
    }
    return 0;
}

// PC game::object_teleport_boss (game.cpp:2199) for OBJ_STAGE_BOSS_TELEPORTER
// (must_return=false: no return-teleport memory, no used-latch). Fade/sfx
// skipped; the destination scroll is the divider-safe arrival path (PC
// get_first_lock_on_left + set_scrolling). dest_ty is unused: PC drops the
// player on the dest_x column's ground, not at dest_y.
void stage_teleport_boss(u8 dest_map, u8 dest_tx, u8 dest_ty)
{
    (void)dest_ty;
    sfx_play(SFX_TELEPORT);
    stage_load(dest_map);

    s16 px = (s16)dest_tx * 16;
    s16 pos_y = first_lock_on_bottom(dest_tx,
                                     PLAYER_BOX_R - PLAYER_BOX_L, PLAYER_BOX_B);
    if (pos_y < 0 || pos_y > 14) pos_y = 0;    // PC: > RES_H/TILESIZE
    s16 py = (pos_y - 1) * 16;                 // PC pos_y--; set_position
    player_place(px, py);
    stage_arrival_camera(px + (PLAYER_BOX_L + PLAYER_BOX_R) / 2,
                         py + (PLAYER_BOX_T + PLAYER_BOX_B) / 2);
    // PC object.cpp:520 draws images/sprites/teleport_small.png at the
    // teleporter while it warps. The port warps in one frame, so the beam
    // plays at the destination instead — centred on the 29x29 player art,
    // and after the camera has moved to the arrival room.
    fx_teleport(px - 2, py - 2);
}
