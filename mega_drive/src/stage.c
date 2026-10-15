#include "stage.h"

#include "resources.h"

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
