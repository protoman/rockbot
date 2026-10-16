#include "stage.h"

#include "resources.h"
#include "stage0_enemies.h"
#include "enemy.h"
#include "object.h"
#include "player.h"
#include "shot.h"
#include "eshot.h"


Map *stage_map;
Vect2D_s16 camera;
u8 stage_current_map;

static const MapDefinition *const map_defs[STAGE0_MAP_COUNT] = {
    &stage0_map0, &stage0_map1, &stage0_map2
};
static const TileSet *const tile_sets[STAGE0_MAP_COUNT] = {
    &stage0_tiles0, &stage0_tiles1, &stage0_tiles2
};
static const Palette *const palettes[STAGE0_MAP_COUNT] = {
    &stage0_pal0, &stage0_pal1, &stage0_pal2
};
static const Image *const bg_images[STAGE0_MAP_COUNT] = {
    &stage0_bg0, &stage0_bg1, NULL
};

static s32 bg_scroll_fx;   // 16.16 parallax scroll, wraps at the baked width
static s16 bg_last_cam_x;
static u8 anim_frame[STAGE0_ANIM_SLOT_COUNT];
static u16 anim_timer[STAGE0_ANIM_SLOT_COUNT];

// Draw the parallax background of the given map on BG_B. Art is baked 512px
// wide (= plane width) by the converter, so the plane wrap is seamless with
// no runtime tile surgery. Vertical position = adjust_y from the PC data
// (classmap.cpp:614: bg drawn at bg_scroll.y + adjust_y).
static void bg_load(u8 map_index)
{
    bg_scroll_fx = 0;
    bg_last_cam_x = 0;

    VDP_clearPlane(BG_B, TRUE);
    VDP_setHorizontalScroll(BG_B, 0);

    const StageBgInfo *info = &stage0_bg_info[map_index];
    const Image *img = bg_images[map_index];
    if (!info->present || !img) return;

    PAL_setPalette(BG_PAL, img->palette->data, CPU);
    SYS_doVBlankProcess();   // sync before the image DMA (see main.c showTitle)
    SYS_disableInts();
    VDP_drawImageEx(BG_B, img,
                    TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE, TILE_BG_INDEX),
                    0, 0, TRUE, DMA);
    SYS_enableInts();

    // Rows below the image: fill with the image's top-left tile (dark sky).
    // A plain clear would expose system tile 0 with garbage colors.
    // NOTE: VDP_setTileMapData wants the VRAM address (VDP_BG_B = bgb_addr),
    // NOT the VDPPlane enum (BG_B == 1) — passing 1 writes over tile data.
    // The plane is 64x32 cells: with VSCROLL_PLANE the sky (y < adjust_y)
    // wraps to rows 28..31, so fill through row 31 — never beyond, rows 32+
    // belong to the WINDOW plane at 0xD000.
    // Start from the IMAGE height, not tileset->numTile / 64: rescomp
    // dedups identical tiles (the city art is ~94% black), so numTile/64
    // computed 1 and the fill erased the whole image except row 0.
    u16 row[64];
    const u16 t = TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE, TILE_BG_INDEX);
    for (u16 i = 0; i < 64; i++) row[i] = t;
    const u16 rows = img->tilemap->h;   // image height in 8px rows (96px -> 12)
    for (u16 y = rows; y < 32; y++)
        VDP_setTileMapData(VDP_BG_B, row, y * 64, 64, 2, CPU);
}

static void clamp_camera(void)
{
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > STAGE0_PIXEL_W - 320) camera.x = STAGE0_PIXEL_W - 320;
    if (camera.y > STAGE0_PIXEL_H - 224) camera.y = STAGE0_PIXEL_H - 224;
}

// ---- PC scroll lock (classmap.cpp:459/475) ------------------------------
// A full-wall column (map divider; list precomputed by convert_stage.py)
// may not sit at the screen edge the camera is moving toward, so the view
// never shows what is behind the wall. Like PC, the check is directional
// (it only guards the advancing edge) and a jump spanning several columns
// is clamped to the first wall it would cross.
static bool column_locked(s16 tile_x)
{
    if (tile_x < 0 || tile_x >= STAGE0_MAP_W) return FALSE;
    const u8 *cols = stage0_scroll_lock[stage_current_map];
    const u8 n = stage0_scroll_lock_n[stage_current_map];
    for (u8 i = 0; i < n; i++)
        if (cols[i] == tile_x) return TRUE;
    return FALSE;
}

static void scroll_lock_camera(s16 prev_x)
{
    if (camera.x == prev_x) return;

    if (camera.x > prev_x) {
        // right-edge tile: scroll.x + RES_W - TILESIZE + 2 (classmap.cpp:459)
        s16 c0 = (prev_x + 320 - 16 + 2) >> 4;
        s16 c1 = (camera.x + 320 - 16 + 2) >> 4;
        if (column_locked(c0)) {
            camera.x = prev_x;                      // edge already on a wall
        } else {
            for (s16 c = c0 + 1; c <= c1; c++) {
                if (column_locked(c)) {
                    camera.x = (c << 4) - (320 - 16 + 2) - 1;
                    break;
                }
            }
        }
    } else {
        // left-edge tile: scroll.x + TILESIZE - 2 (classmap.cpp:475)
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

// After a camera jump (link arrival) the view can span a divider that
// incremental walking could never cross. Push the camera so the divider
// sits at a screen edge, on the focus's side of it.
static void scroll_lock_span(s16 focus_x)
{
    s16 cl = (camera.x + 14) >> 4;
    s16 cr = (camera.x + 306) >> 4;
    for (s16 c = cl + 1; c < cr; c++) {
        if (!column_locked(c)) continue;
        if (focus_x >= ((s16)c << 4) + 8) {
            if (camera.x < ((c << 4) + 2)) camera.x = (c << 4) + 2;   // wall to left edge
        } else {
            if (camera.x > ((c << 4) - 307)) camera.x = (c << 4) - 307; // wall to right edge
        }
    }
}

// ---- animated tiles + above-player tile3 ------------------------------
// tile3 cells carry TILE_ATTR_PRIORITY so the VDP draws them above sprites
// (the player, enemies, shots). Anim cells point at a fixed VRAM slot per
// animation; the SGDK map patch callback rewrites every tilemap buffer
// (full draw and scroll columns) so stale entries never reach the VDP.
static void stage_map_patch(Map *map, u16 *buf, u16 x, u16 y,
                            MapUpdateType type, u16 size)
{
    for (u16 i = 0; i < size; i++) {
        u16 tx = (type == COLUMN_UPDATE) ? x : x + i;
        u16 ty = (type == COLUMN_UPDATE) ? y + i : y;
        u16 cx = tx >> 1, cy = ty >> 1;    // 16px cells
        if (cx >= STAGE0_MAP_W || cy >= STAGE0_MAP_H) continue;
        u16 n = cy * STAGE0_MAP_W + cx;
        bool t3 = stage0_tile3_mask[stage_current_map][n >> 3] & (1 << (n & 7));
        bool an = stage0_anim_mask[stage_current_map][n >> 3] & (1 << (n & 7));
        if (an) {
            // one anim slot per stage today; multi-slot needs a per-cell slot map
            u16 attr = (map->baseTile & TILE_ATTR_PALETTE_MASK) |
                       (t3 ? TILE_ATTR_PRIORITY_MASK : 0);
            u16 quad = (tx & 1) | ((ty & 1) << 1);
            buf[i] = attr | (TILE_ANIM_BASE + anim_frame[0] * 4 + quad);
        } else if (t3) {
            buf[i] |= TILE_ATTR_PRIORITY_MASK;
        }
    }
}

static void stage_anim_reset(void)
{
    for (u8 s = 0; s < STAGE0_ANIM_SLOT_COUNT; s++) {
        anim_frame[s] = 0;
        anim_timer[s] = stage0_anim_slots[s].delay[0];
        VDP_loadTileData((const u32 *)stage0_anim_frames[s][0],
                         TILE_ANIM_BASE + s * TILE_ANIM_STRIDE, 4, DMA_QUEUE);
    }
}

void stage_anim_update(void)
{
    for (u8 s = 0; s < STAGE0_ANIM_SLOT_COUNT; s++) {
        const StageAnimSlot *slot = &stage0_anim_slots[s];
        if (slot->frames < 2) continue;
        if (anim_timer[s] > 1) { anim_timer[s]--; continue; }
        anim_frame[s]++;
        if (anim_frame[s] >= slot->frames) anim_frame[s] = 0;
        anim_timer[s] = slot->delay[anim_frame[s]];
        u16 base = TILE_ANIM_BASE + s * TILE_ANIM_STRIDE + anim_frame[s] * 4;
        VDP_loadTileData((const u32 *)stage0_anim_frames[s][anim_frame[s]],
                         base, 4, DMA_QUEUE);
        // rewrite visible cells of this slot; scroll columns are kept
        // correct by stage_map_patch
        const StageAnimCell *cells = stage0_anim_cells[stage_current_map];
        u16 n = stage0_anim_cell_n[stage_current_map];
        u16 pal = STAGE_PAL << TILE_ATTR_PALETTE_SFT;
        for (u16 i = 0; i < n; i++) {
            if (cells[i].slot != s) continue;
            s16 sx = (s16)cells[i].x * STAGE0_TILE - camera.x;
            s16 sy = (s16)cells[i].y * STAGE0_TILE - camera.y;
            if (sx < -STAGE0_TILE || sx >= 320 ||
                sy < -STAGE0_TILE || sy >= 224) continue;
            u16 a = pal | (cells[i].tile3 ? TILE_ATTR_PRIORITY_MASK : 0);
            u16 tx = cells[i].x * 2, ty = cells[i].y * 2;
            VDP_setTileMapXY(STAGE_PLANE, a | (base + 0), tx,     ty);
            VDP_setTileMapXY(STAGE_PLANE, a | (base + 1), tx + 1, ty);
            VDP_setTileMapXY(STAGE_PLANE, a | (base + 2), tx,     ty + 1);
            VDP_setTileMapXY(STAGE_PLANE, a | (base + 3), tx + 1, ty + 1);
        }
    }
}

void stage_load(u8 map_index)
{
    if (map_index >= STAGE0_MAP_COUNT) map_index = 0;
    stage_current_map = map_index;

    bool wasEnabled = VDP_isEnable();
    VDP_setEnable(FALSE);

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    // Load tileset and palette
    PAL_setPalette(STAGE_PAL, palettes[map_index]->data, CPU);
    VDP_loadTileSet(tile_sets[map_index], TILE_USER_INDEX, DMA);

    stage_map = MAP_create(map_defs[map_index], STAGE_PLANE,
                           TILE_ATTR_FULL(STAGE_PAL, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    MAP_setDataPatchCallback(stage_map, stage_map_patch);

    camera.x = 0;
    camera.y = 0;
    MAP_scrollToEx(stage_map, 0, 0, TRUE);   // full draw on first load

    bg_load(map_index);

    // PAL0 = shared font + enemy palette (tools/convert_enemies.py):
    // index 0 transparent, index 15 must stay font white (0x0EEE) so
    // VDP_drawText keeps working, indices 1..14 hold the enemy colors.
    PAL_setPalette(PAL0, palette_ui_enemies, DMA_QUEUE);

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
    if (stage0_bg_info[map_index].present)
        VDP_setVerticalScroll(BG_B, -((s16)stage0_bg_info[map_index].adjust_y - 16));
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
    if (px < (s16)tx * STAGE0_TILE || px > (s16)tx * STAGE0_TILE + STAGE0_TILE)
        return FALSE;
    s16 lim3 = (s16)ty * STAGE0_TILE + 8;
    s16 lim4 = (s16)ty * STAGE0_TILE + STAGE0_TILE;
    if (trans == LINK_TRANS_T2B) {
        lim3 += 8;      // PC lim3 += TILESIZE * 0.5
        lim4 += 24;     // PC lim4 += TILESIZE * 1.5
    } else {
        lim3 -= 27;     // PC lim3 -= 27.2 truncated (only ty=0 bands fire)
    }
    return py > lim3 && py < lim4;
}

bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y)
{
    // Same link may not re-fire until the player leaves its band
    // (PC set_teleporter/get_teleporter, game.cpp:802-832). Survives
    // stage_load: on arrival the band still hits and would ping-pong.
    static s8 link_latch = -1;

    s16 px = (left + right) >> 1;   // PC px = x + width * 0.5 (game.cpp:778)
    s16 py = bottom - 8;            // PC py = y + h * 0.75 = y + 21 (game.cpp:779)

    for (u8 i = 0; i < STAGE0_LINK_COUNT; i++) {
        const StageLink *lk = &stage0_links[i];
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
            *to_x   = (s16)dst_x * STAGE0_TILE;
            *to_y   = (s16)dst_y * STAGE0_TILE;
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

    // Parallax city scene (requested look): horizontal scroll tied to the
    // camera at bg speed/10, wrapped with an overflow-safe modulo into
    // (-512px, 0]. PC marks these maps auto_scroll=NONE; keeping the effect
    // is an intentional port deviation (user request).
    const s16 dx = camera.x - bg_last_cam_x;
    bg_last_cam_x = camera.x;
    const StageBgInfo *info = &stage0_bg_info[stage_current_map];
    if (info->present && info->speed) {
        const s32 W = (s32)512 << 16;
        bg_scroll_fx -= ((s32)dx * info->speed * 65536) / 10;
        bg_scroll_fx %= W;
        if (bg_scroll_fx > 0) bg_scroll_fx -= W;
        VDP_setHorizontalScroll(BG_B, (s16)(bg_scroll_fx >> 16));
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
    bg_last_cam_x = camera.x;   // no parallax delta across camera jumps
}

// Link arrival: PC sets the new map scroll directly instead of walking the
// camera there (game.cpp:750 set_scrolling). Centre on the player, then
// keep any divider at a screen edge on the player's side — e.g. arriving at
// map0 (139,13) the view must not span the 137/138 divider.
void stage_arrival_camera(s16 focusX, s16 focusY)
{
    camera.x = focusX - 160;
    camera.y = focusY - 112;
    clamp_camera();
    scroll_lock_span(focusX);
    stage_force_redraw();
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

    for (s16 i = STAGE0_MAP_H - 1; i >= above + 1; i--) {
        u16 t = stage_terrain_at(stage_current_map, tilex, i);
        if (t == TERRAIN_UNBLOCKED || t == TERRAIN_EASYBLOCK || t == TERRAIN_HARDBLOCK)
            continue;
        bool bad = FALSE;
        for (s16 j = i - 1; j >= i - above && !bad; j--) {
            for (s16 k = 0; k < right; k++) {
                if (tilex + k >= STAGE0_MAP_W) { bad = TRUE; break; }
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
