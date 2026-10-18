#include "stage.h"
#include "sfx.h"

#include "resources.h"
#include "enemy.h"
#include "object.h"
#include "player.h"
#include "shot.h"
#include "eshot.h"


Map *stage_map;
Vect2D_s16 camera;
u8 stage_current_map;

#define STAGE_MASK_BYTES ((stage_def->map_w * stage_def->map_h + 7) / 8)

static u8 anim_frame[STAGE_ANIM_SLOT_MAX];
static u16 anim_timer[STAGE_ANIM_SLOT_MAX];

u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY)
{
    if (map >= stage_def->map_count) return TERRAIN_SOLID;
    if (tileX < 0 || tileX >= stage_def->map_w) return TERRAIN_SOLID;
    if (tileY < 0 || tileY >= stage_def->map_h) return TERRAIN_UNBLOCKED;
    return stage_def->terrain[map * stage_def->map_w * stage_def->map_h
                              + tileY * stage_def->map_w + tileX];
}

// Parallax background on BG_B from the decomposed art (tools/img2tiles.py):
// the unique tiles + a 64-wide tilemap replace the old full-image draw, so
// VRAM holds ~232 tiles instead of 768 (bg0) and ~26 instead of 896 (bg1) —
// bg1's tiles used to run into the animated-tile block.
static void bg_upload_tiles_maps(u8 map_index)
{
    if (!stage_def->bg_pal[map_index]) return;
    PAL_setPalette(BG_PAL, stage_def->bg_pal[map_index]->data, CPU);
    VDP_loadTileData(stage_def->bg_tiles[map_index], TILE_BG_INDEX,
                     stage_def->bg_tiles_n[map_index], DMA);

    const u16 *m = stage_def->bg_maps[map_index];
    u16 row[64];
    const u16 sky = TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                   TILE_BG_INDEX + m[0]);
    for (u16 y = 0; y < 32; y++) {
        for (u16 x = 0; x < 64; x++) {
            row[x] = (y < stage_def->bg_map_h[map_index])
                         ? TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE,
                                          TILE_BG_INDEX + m[y * 64 + x])
                         : sky;
        }
        VDP_setTileMapData(VDP_BG_B, row, y * 64, 64, 2, CPU);
    }
}

// Draw the parallax background of the given map on BG_B.
static void bg_load(u8 map_index)
{
    VDP_clearPlane(BG_B, TRUE);
    VDP_setHorizontalScroll(BG_B, 0);

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
    s16 cl = (camera.x + 14) >> 4;
    s16 cr = (camera.x + 306) >> 4;
    for (s16 c = cl + 1; c < cr; c++) {
        if (!column_locked(c)) continue;
        if (focus_x >= ((s16)c << 4) + 8) {
            if (camera.x < ((c << 4) + 2)) camera.x = (c << 4) + 2;
        } else {
            if (camera.x > ((c << 4) - 307)) camera.x = (c << 4) - 307;
        }
    }
}

// ---- animated tiles + above-player tile3 ------------------------------
// Animations are compacted PER MAP (only the visible map needs VRAM): the
// converter emits one slot list per map plus a per-cell slot map (0 = none,
// else slot+1). stage_load loads the current map's slots at TILE_ANIM_BASE.
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
        bool t3 = stage_def->tile3_mask[stage_current_map * STAGE_MASK_BYTES
                                        + (n >> 3)] & (1 << (n & 7));
        u8 sv = cell_slot[n];
        if (sv) {
            u16 attr = (map->baseTile & TILE_ATTR_PALETTE_MASK) |
                       (t3 ? TILE_ATTR_PRIORITY_MASK : 0);
            u16 quad = (tx & 1) | ((ty & 1) << 1);
            buf[i] = attr | (TILE_ANIM_BASE + anim_frame[sv - 1] * 4 + quad);
        } else if (t3) {
            buf[i] |= TILE_ATTR_PRIORITY_MASK;
        }
    }
}

static void stage_anim_reset(void)
{
    u8 n = stage_def->anim_slot_n[stage_current_map];
    for (u8 s = 0; s < n; s++) {
        u16 idx = stage_current_map * stage_def->anim_slot_max + s;
        anim_frame[s] = 0;
        anim_timer[s] = stage_def->anim_slots[idx].delay[0];
        VDP_loadTileData((const u32 *)stage_def->anim_frames[idx][0],
                         TILE_ANIM_BASE + s * TILE_ANIM_STRIDE, 4, DMA_QUEUE);
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
        u16 base = TILE_ANIM_BASE + s * TILE_ANIM_STRIDE + anim_frame[s] * 4;
        VDP_loadTileData((const u32 *)stage_def->anim_frames[idx][anim_frame[s]],
                         base, 4, DMA_QUEUE);

        // rewrite the visible cells of this slot
        const u8 *cell_slot = stage_def->anim_cell_slot
                              + stage_current_map * stage_def->map_w * stage_def->map_h;
        u16 pal = STAGE_PAL << TILE_ATTR_PALETTE_SFT;
        s16 cx0 = camera.x >> 4, cy0 = camera.y >> 4;
        for (s16 cy = cy0; cy <= cy0 + 14; cy++) {
            if (cy < 0 || cy >= stage_def->map_h) continue;
            for (s16 cx = cx0; cx <= cx0 + 20; cx++) {
                if (cx < 0 || cx >= stage_def->map_w) continue;
                u16 n2 = cy * stage_def->map_w + cx;
                if (cell_slot[n2] != s + 1) continue;
                s16 sx = cx * stage_def->tile - camera.x;
                s16 sy = cy * stage_def->tile - camera.y;
                if (sx < -stage_def->tile || sx >= 320 ||
                    sy < -stage_def->tile || sy >= 224) continue;
                bool t3 = stage_def->tile3_mask[stage_current_map * STAGE_MASK_BYTES
                                                + (n2 >> 3)] & (1 << (n2 & 7));
                u16 a = pal | (t3 ? TILE_ATTR_PRIORITY_MASK : 0);
                u16 tx = cx * 2, ty = cy * 2;
                VDP_setTileMapXY(STAGE_PLANE, a | (base + 0), tx,     ty);
                VDP_setTileMapXY(STAGE_PLANE, a | (base + 1), tx + 1, ty);
                VDP_setTileMapXY(STAGE_PLANE, a | (base + 2), tx,     ty + 1);
                VDP_setTileMapXY(STAGE_PLANE, a | (base + 3), tx + 1, ty + 1);
            }
        }
    }
}

void stage_load(u8 map_index)
{
    if (map_index >= stage_def->map_count) map_index = 0;
    stage_current_map = map_index;

    bool wasEnabled = VDP_isEnable();
    VDP_setEnable(FALSE);

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    // Load tileset and palettes. CRAM writes are only reliable with the
    // display disabled (or during vblank): PAL1/PAL3 use CPU writes here
    // and work; PAL0 used a queued DMA that does not land in this window
    // (the CRAM readback showed the enemies/objects/HUD line still holding
    // another, one-step-darker palette).
    PAL_setPalette(STAGE_PAL, stage_def->palettes[map_index]->data, CPU);
    // PAL0 = shared font + enemy/object/HUD palette. Loaded from the
    // rescomp PALETTE resource (same mechanism as the working PAL1/PAL3
    // lines) instead of the C array, which did not stick: the CRAM readback
    // showed PAL0 holding another palette while PAL1 decoded exactly.
    PAL_setPalette(PAL0, pal_ui.data, CPU);
    VDP_loadTileSet(stage_def->tile_sets[map_index], TILE_USER_INDEX, DMA);

    // Flush anything the previous map queued, then drop it: MAP_create()
    // allocates the Map plus its unpacked metatile/block/index buffers from
    // the heap, so without this every room change leaked ~1.5KB — repeated
    // up/down links filled the heap until the game corrupted itself and the
    // emulator died.
    SYS_doVBlankProcess();
    stage_release_map();

    stage_map = MAP_create(stage_def->map_defs[map_index], STAGE_PLANE,
                           TILE_ATTR_FULL(STAGE_PAL, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    if (!stage_map) {
        // Out of heap: bail with the display restored (leaving it disabled
        // here would black the screen silently).
        VDP_setEnable(wasEnabled);
        return;
    }
    MAP_setDataPatchCallback(stage_map, stage_map_patch);

    camera.x = 0;
    camera.y = 0;
    MAP_scrollToEx(stage_map, 0, 0, TRUE);   // full draw on first load

    bg_load(map_index);

    // enemies/shots/objects are per-map: release the old ones, spawn this map's
    shots_reset();
    eshots_reset();
    enemies_reset();
    objects_reset();

    // animated-tile frame 0 must be in VRAM before the first full draw
    stage_anim_reset();

    // Flush any pending transfers and restore display
    SYS_doVBlankProcess();
    VDP_setEnable(wasEnabled);


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
    if (stage_def->bg_info[map_index].present)
        VDP_setVerticalScroll(BG_B, -((s16)stage_def->bg_info[map_index].adjust_y - 16));
    else
        VDP_setVerticalScroll(BG_B, 0);
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

static bool link_region(s16 px, s16 py, u8 tx, u8 ty, u8 trans)
{
    if (px < (s16)tx * stage_def->tile || px > (s16)tx * stage_def->tile + stage_def->tile)
        return FALSE;
    s16 lim3 = (s16)ty * stage_def->tile + 8;
    s16 lim4 = (s16)ty * stage_def->tile + stage_def->tile;
    if (trans == LINK_TRANS_T2B) {
        lim3 += 8;      // PC lim3 += TILESIZE * 0.5
        lim4 += 24;     // PC lim4 += TILESIZE * 1.5
    } else {
        lim3 -= 27;     // PC lim3 -= 27.2 truncated (only ty=0 bands fire)
    }
    return py > lim3 && py < lim4;
}

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
        } else if (lk->map_to == stage_current_map) {
            // reverse: both stage 0 links are bidirectional (game.cpp:650-661)
            tx = lk->dst_x;  ty = lk->dst_y;
            dst_map = lk->map_from;  dst_x = lk->tile_x;  dst_y = lk->tile_y;
            trans = (lk->tile_y < lk->dst_y) ? LINK_TRANS_T2B : LINK_TRANS_B2T;
        } else {
            continue;
        }

        hit = link_region(px, py, tx, ty, trans);

        if (link_latch == (s8)i) {
            if (!hit) link_latch = -1;   // cleared on region miss only
            continue;
        }
        if (hit) {
            link_latch = (s8)i;
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

    if (focusX > camera.x + cx + dzx)       camera.x = focusX - cx - dzx;
    else if (focusX < camera.x + cx - dzx)  camera.x = focusX - cx + dzx;

    if (focusY > camera.y + cy + dzy)       camera.y = focusY - cy - dzy;
    else if (focusY < camera.y + cy - dzy)  camera.y = focusY - cy + dzy;

    clamp_camera();
    scroll_lock_camera(prev_x);
    MAP_scrollTo(stage_map, camera.x, camera.y);

    // Parallax: horizontal scroll derived DIRECTLY from camera.x at
    // speed/10, wrapped into (-512, 0]. PC accumulates camera_dx per frame
    // (classmap.cpp:449); deriving it from the absolute camera is equivalent
    // and cannot drift/jitter when the camera sits on a deadzone edge (the
    // accumulated version moved the bg a few px on its own).
    const StageBgInfo *info = &stage_def->bg_info[stage_current_map];
    if (info->present && info->speed) {
        s32 scroll = -((s32)camera.x * info->speed) / 10;
        scroll %= 512;
        if (scroll > 0) scroll -= 512;
        VDP_setHorizontalScroll(BG_B, (s16)scroll);
    }
}

// Force a complete MAP redraw. Needed whenever the camera jumps a large
// distance in one frame (respawn, map link): the incremental scroll used by
// MAP_scrollTo only loads the newly exposed edge, so a jump leaves stale
// columns on screen (background "blinking").
void stage_force_redraw(void)
{
    clamp_camera();
    MAP_scrollToEx(stage_map, camera.x, camera.y, TRUE);
}

void stage_restore_palette(void)
{
    PAL_setPalette(STAGE_PAL, stage_def->palettes[stage_current_map]->data, CPU);
}

void stage_redraw_bg(void)
{
    bg_load(stage_current_map);
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
    camera.x = stage_arrival_camera_x(stage_current_map, focusX);
    camera.y = focusY - 112;
    clamp_camera();
    stage_force_redraw();
}

// Same camera X, computed without touching the plane: used by the room
// transition, which needs it before the destination map is loaded (so
// column_locked() must see the destination map's scroll locks).
s16 stage_arrival_camera_x(u8 map, s16 focus_x)
{
    const u8 save_map = stage_current_map;
    const s16 save_x = camera.x;
    const s16 save_y = camera.y;
    stage_current_map = map;
    camera.x = focus_x - 160;
    clamp_camera();
    scroll_lock_span(focus_x);
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
        if (t == TERRAIN_UNBLOCKED || t == TERRAIN_EASYBLOCK || t == TERRAIN_HARDBLOCK)
            continue;
        bool bad = FALSE;
        for (s16 j = i - 1; j >= i - above && !bad; j--) {
            for (s16 k = 0; k < right; k++) {
                if (tilex + k >= stage_def->map_w) { bad = TRUE; break; }
                if (stage_terrain_at(stage_current_map, tilex + k, j) != TERRAIN_UNBLOCKED) {
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
}
