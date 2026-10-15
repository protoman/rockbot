#include "stage.h"

#include "resources.h"
#include "stage0_enemies.h"
#include "enemy.h"
#include "shot.h"

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

static s32 bg_scroll_fx;   // 16.16 fixed parallax scroll, wraps at plane width
static s16 bg_last_cam_x;

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
    VDP_setVerticalScroll(BG_B, 0);

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
    u16 row[64];
    const u16 t = TILE_ATTR_FULL(BG_PAL, FALSE, FALSE, FALSE, TILE_BG_INDEX);
    for (u16 i = 0; i < 64; i++) row[i] = t;
    const u16 rows = img->tileset->numTile / 64;   // image is 64 tiles wide
    for (u16 y = rows; y < 32; y++)
        VDP_setTileMapData(VDP_BG_B, row, y * 64, 64, 2, CPU);

    VDP_setVerticalScroll(BG_B, info->adjust_y);
}

static void clamp_camera(void)
{
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > STAGE0_PIXEL_W - 320) camera.x = STAGE0_PIXEL_W - 320;
    if (camera.y > STAGE0_PIXEL_H - 224) camera.y = STAGE0_PIXEL_H - 224;
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

    camera.x = 0;
    camera.y = 0;
    MAP_scrollToEx(stage_map, 0, 0, TRUE);   // full draw on first load

    bg_load(map_index);

    // PAL0 = shared font + enemy palette (tools/convert_enemies.py):
    // index 0 transparent, index 15 must stay font white (0x0EEE) so
    // VDP_drawText keeps working, indices 1..14 hold the enemy colors.
    PAL_setPalette(PAL0, palette_ui_enemies, DMA_QUEUE);

    // enemies/shots are per-map: release the old ones, spawn this map's
    shots_reset();
    enemies_reset();

    // Flush any pending transfers and restore display
    SYS_doVBlankProcess();
    VDP_setEnable(wasEnabled);
}

bool stage_check_link(s16 left, s16 top, s16 right, s16 bottom,
                      u8 *to_map, s16 *to_x, s16 *to_y)
{
    for (u8 i = 0; i < STAGE0_LINK_COUNT; i++) {
        const StageLink *lk = &stage0_links[i];

        if (lk->map_from != stage_current_map) continue;

        s16 tx0 = (s16)lk->tile_x * STAGE0_TILE;
        s16 ty0 = (s16)lk->tile_y * STAGE0_TILE;

        // box overlaps the origin tile
        if (left <= tx0 + STAGE0_TILE - 1 && right >= tx0 &&
            top  <= ty0 + STAGE0_TILE - 1 && bottom >= ty0) {
            *to_map = lk->map_to;
            *to_x   = (s16)lk->dst_x * STAGE0_TILE;
            *to_y   = (s16)lk->dst_y * STAGE0_TILE;
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

    if (focusX > camera.x + cx + dzx)       camera.x = focusX - cx - dzx;
    else if (focusX < camera.x + cx - dzx)  camera.x = focusX - cx + dzx;

    if (focusY > camera.y + cy + dzy)       camera.y = focusY - cy - dzy;
    else if (focusY < camera.y + cy - dzy)  camera.y = focusY - cy + dzy;

    clamp_camera();
    MAP_scrollTo(stage_map, camera.x, camera.y);

    // Parallax: PC does bg_scroll.x -= camera_dx * speed/10 (classmap.cpp:462)
    // and draws the art at that offset (negative = plane shifted left).
    // 16.16 accumulator, wrapped at the plane width (512px).
    const s16 dx = camera.x - bg_last_cam_x;
    bg_last_cam_x = camera.x;
    const StageBgInfo *info = &stage0_bg_info[stage_current_map];
    if (info->present && info->speed) {
        bg_scroll_fx -= ((s32)dx * info->speed * 65536) / 10;
        while (bg_scroll_fx <= -((s32)512 << 16)) bg_scroll_fx += (s32)512 << 16;
        while (bg_scroll_fx > 0) bg_scroll_fx += (s32)512 << 16;
        VDP_setHorizontalScroll(BG_B, (s16)(-(bg_scroll_fx >> 16)));
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
