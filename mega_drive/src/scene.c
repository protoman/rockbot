#include "dbgprint.h"
#include <genesis.h>

#include "scene.h"
#include "walkdiag.h"
#include "pal_safe.h"
#include "scenes_data.h"
#include "music.h"
#include "stage.h"

// PC scene player (scenes/sceneshow.cpp) for the intro.
//
// Planes: BG_B carries the background / scene images, BG_A is the text
// overlay. Outside a text box BG_A is filled with the blank tile (pixels 0 =
// transparent), so the art below shows through; text boxes are solid black.
// The intro's parallax uses BG_A for its front layer and BG_B for the back.
//
// VRAM: the scene runs before any stage is loaded, so the whole user tile
// area is free. Tile 16 is a solid black tile, the layers start at 17.
// TILE_FONT_INDEX (1440) is the SGDK font and stays untouched.
//
// Not implemented here: show-image, viewpoint and animation ops (their art
// needs the colour-reduction pass first), and audio (stubbed).

#define SCENE_BLACK_TILE TILE_USER_INDEX
#define SCENE_TILE_BASE  (TILE_USER_INDEX + 1)

#define SCENE_BG_PAL0 PAL1
#define SCENE_BG_PAL1 PAL2
#define SCENE_IMG_PAL0 PAL2
#define SCENE_IMG_PAL1 PAL3
#define SCREEN_TILE_W 40

// Image/background allocation, reset by CLEAR_SCREEN and CLEAR_AREA.
static u16 scene_tile_next;
static u8 scene_img_line;
static bool scene_bg_set;
static u16 scene_bg_entry;

// 8x8 tile filled with palette index 1 (index 0 would be transparent and let
// the other plane show through). 0x11111111 = all 8 pixels of the row are
// bitplane 0, i.e. value 1; the old 0x11111111 set all four planes, so only
// pixels 3 and 7 lit up as index 0xF - two white stripes per tile (BUG-044).
static const u32 black_tile[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
};

static bool wait_frames(u16 n)
{
    while (n--) {
        if (JOY_readJoypad(JOY_1) & BUTTON_START) return FALSE;
        SYS_doVBlankProcess();
    }
    return TRUE;
}

static bool wait_ms(u16 ms)
{
    return wait_frames((u16)(((u32)ms * 60 + 999) / 1000));
}

static void clear_screen(void)
{
    const u16 black = TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE,
                                     SCENE_BLACK_TILE);
    // BG_A: blank tile (all pixels 0 -> transparent, shows BG_B)
    const u16 blank = TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, 0);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_fillTileMapRect(BG_A, blank, 0, 0, 64, 32);
    VDP_fillTileMapRect(BG_B, black, 0, 0, 64, 32);
    VDP_setBackgroundColor(0);
    scene_tile_next = TILE_USER_INDEX + 1;   // tile 16 = black
    scene_img_line = 0;
    scene_bg_entry = black;
    scene_bg_set = FALSE;
}

// Fill one plane with a layer's 64x32 map (rows past the art reuse its first
// tile, as stage.c does for the sky).
static void bg_upload_plane(const SceneLayer *l, u16 plane_addr, u16 pal_line,
                            u16 tile_base)
{
    pal_load(pal_line, l->pal->data);   // CRAM DMA does not land
    VDP_loadTileData(l->tiles, tile_base, l->tiles_n, DMA);

    u16 row[64];
    const u16 first = TILE_ATTR_FULL(pal_line, FALSE, FALSE, FALSE,
                                     tile_base + l->map[0]);
    for (u16 y = 0; y < 32; y++) {
        for (u16 x = 0; x < 64; x++) {
            row[x] = (y < l->map_h)
                         ? TILE_ATTR_FULL(pal_line, FALSE, FALSE, FALSE,
                                          tile_base + l->map[y * 64 + x])
                         : first;
        }
        VDP_setTileMapData(plane_addr, row, y * 64, 64, 2, CPU);
    }
}

static bool parallax_play(const SceneParallax *p)
{
    clear_screen();

    u16 base = SCENE_TILE_BASE;
    u32 pos[2] = { 0, 0 };
    VDPPlane plane[2];

    for (u8 i = 0; i < p->count; i++) {
        const SceneLayer *l = &p->layers[i];
        // draw order: PC draws layer 0 first, the last layer ends up on top
        plane[i] = (i + 1 == p->count) ? BG_A : BG_B;
        bg_upload_plane(l, (plane[i] == BG_A) ? VDP_BG_A : VDP_BG_B,
                        (i == 0) ? SCENE_BG_PAL0 : SCENE_BG_PAL1, base);
        base += l->tiles_n;
        VDP_setVerticalScroll(plane[i], 16 - l->adjust_y);
    }
    SYS_doVBlankProcess();   // land the queued tile DMA before scrolling

    // PC run_parallax_scene: x += speed every frame_delay ms, wrapping at the
    // art width. The port advances the same speed at 60 Hz (speed px per
    // 10 ms = speed*100 px/s -> speed*426 in 8.8 fixed point per frame).
    // The wrap uses the layer's BAKED period (tools/convert_scenes.py bakes
    // the art at its own repeat width), not the 512 px plane: a 384 px layer
    // whose art repeats at 256 px would otherwise jump 128 px every cycle.
    const u32 frames = (u32)p->duration * 60 / 1000;
    for (u32 f = 0; f < frames; f++) {
        for (u8 i = 0; i < p->count; i++) {
            pos[i] += (u32)p->layers[i].speed * 426;
            const u16 per = p->layers[i].period ? p->layers[i].period : 512;
            VDP_setHorizontalScroll(plane[i], -(s16)((pos[i] >> 8) % per));
        }
        if (!wait_frames(1)) return FALSE;
    }
    return TRUE;
}

static bool text_play(u8 rec)
{
    const SceneTextPos *p = &scene_text_pos[rec];
    const char *const *lines = scene_text[rec];

    // PC run_text counts up to the LAST non-empty line (gaps are skipped when
    // drawing but still occupy a row in the block).
    u8 rows = 0;
    for (u8 i = 0; i < SCENE_TEXT_LINES; i++)
        if (lines[i][0]) rows = i + 1;
    if (!rows) return TRUE;

    // PC run_text positions (dialog types are in 240-line PC pixels, the
    // MD screen drops the top 16 lines; PC centers on y=120 -> 104 here).
    // Lines are packed 8 px apart (the user allowed tighter spacing than the
    // PC's 12).
    s16 y0;
    if (p->position == SCENE_TEXT_CENTERED || p->position == SCENE_TEXT_CENTER_Y)
        y0 = 104 - (rows * 8) / 2;
    else if (p->position == SCENE_TEXT_DIALOG_TOP)
        y0 = 0;                        // PC y=10 is above the cropped screen
    else if (p->position == SCENE_TEXT_DIALOG_BOTTOM)
        y0 = 160 - 16;                 // PC SCENES_TEXT_BOTTOM_POSY
    else
        y0 = p->y;
    const s16 tx = (p->position == SCENE_TEXT_USER) ? (p->x >> 3) : 1;
    const u8 yt = (u8)(y0 >> 3);

    // Width of the widest line, in tiles (centered lines are placed per line).
    u8 min_x = 40, max_x = 0;
    for (u8 i = 0; i < SCENE_TEXT_LINES; i++) {
        u8 len = strlen(lines[i]);
        if (!len) continue;
        u8 x = (s16)tx;
        if (p->position == SCENE_TEXT_CENTERED)
            x = (SCREEN_TILE_W - len) / 2;
        if (x < min_x) min_x = x;
        if (x + len > max_x) max_x = x + len;
    }

    // PC clears the text area black before typing. The area has to cover what
    // the PREVIOUS page drew as well: its lines can be wider or sit on other
    // rows than this page's, and only clearing this page's rectangle left
    // stray letters at the edges (clear_text_001/002.png).
    static u8 prev_x0 = 40, prev_x1 = 0, prev_y0 = 0, prev_y1 = 0;
    u8 cx0 = min_x, cx1 = max_x, cy0 = yt, cy1 = (u8)(yt + rows);
    if (prev_x1 > prev_x0) {
        if (prev_x0 < cx0) cx0 = prev_x0;
        if (prev_x1 > cx1) cx1 = prev_x1;
        if (prev_y0 < cy0) cy0 = prev_y0;
        if (prev_y1 > cy1) cy1 = prev_y1;
    }
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE,
                                       SCENE_BLACK_TILE),
                        cx0, cy0, cx1 - cx0, cy1 - cy0);
    prev_x0 = min_x; prev_x1 = max_x; prev_y0 = yt; prev_y1 = (u8)(yt + rows);

    for (u8 i = 0; i < SCENE_TEXT_LINES; i++) {
        const char *line = lines[i];
        if (!line[0]) continue;
        u8 x = (p->position == SCENE_TEXT_CENTERED)
                   ? (SCREEN_TILE_W - strlen(line)) / 2
                   : tx;
        for (u8 c = 0; line[c]; c++) {
            char ch[2] = { line[c], '\0' };
            VDP_drawText(ch, x + c, yt + i);
            // PC draw_progressive_text: 30 ms per character (~2 frames)
            if (!wait_frames(2)) return FALSE;
        }
    }
    return TRUE;
}

// MOVE_IMAGE (PC sceneShow::show_image): draw the portrait on BG_B. The art
// is a fully opaque 68x68 square (background pre-painted in the clear-area
// colour), so only the 9x9 map is written; padding cells (tile 0) keep the
// background tile.
static bool show_image(u8 rec)
{
    const SceneImage *im = &scene_images[rec];
    if (!im->tiles || !im->map_w || !im->map_h) return TRUE;

    u16 line = (scene_img_line == 0) ? SCENE_IMG_PAL0 : SCENE_IMG_PAL1;
    pal_load(line, im->pal->data);   // CRAM DMA does not land here
    VDP_loadTileData(im->tiles, scene_tile_next, im->tiles_n, DMA);
    u16 base = scene_tile_next;
    scene_tile_next += im->tiles_n;
    if (scene_img_line < 2) scene_img_line++;

    const u16 bg = scene_bg_entry;
    static u16 cells[16 * 16];   // portraits are 9x9 cells
    s16 x = im->x, y = im->y;
    for (;;) {
        u16 n = 0;
        for (u8 cy = 0; cy < im->map_h; cy++) {
            for (u8 cx = 0; cx < im->map_w; cx++) {
                u16 t = im->map[cy * im->map_w + cx];
                cells[n++] = t ? TILE_ATTR_FULL(line, FALSE, FALSE, FALSE,
                                                base + t) : bg;
            }
        }
        VDP_setTileMapDataRect(BG_B, cells, (u16)(x >> 3), (u16)(y >> 3),
                               im->map_w, im->map_h, im->map_w, CPU);

        if (im->dest_x == im->x && im->dest_y == im->y) break;   // static
        if (!wait_ms(im->move_delay)) return FALSE;              // START aborts

        // Move one step toward the destination (PC sceneShow::show_image:
        // the longer axis advances 1px, the other axis by the ratio) and
        // erase the previous rect with the background tile. Endings are all
        // static, so this path exists for the lab scene / future scenes.
        s16 dx = im->dest_x - x, dy = im->dest_y - y;
        s16 ax = (dx < 0) ? -dx : dx, ay = (dy < 0) ? -dy : dy;
        if (!ax && !ay) break;
        s16 sx8 = 0, sy8 = 0;
        if (ax >= ay) {
            sx8 = (dx > 0) ? 256 : -256;
            sy8 = ax ? (dy * 256) / ax : 0;
        } else {
            sy8 = (dy > 0) ? 256 : -256;
            sx8 = ay ? (dx * 256) / ay : 0;
        }
        s16 nx = x + (sx8 / 256), ny = y + (sy8 / 256);
        if (nx == x && ny == y) {
            // sub-pixel step: force one pixel on the major axis
            if (ax >= ay) nx = x + ((dx > 0) ? 1 : -1);
            else ny = y + ((dy > 0) ? 1 : -1);
        }
        u16 oldx = (u16)(x >> 3), oldy = (u16)(y >> 3);
        for (u8 cy = 0; cy < im->map_h; cy++)
            for (u8 cx = 0; cx < im->map_w; cx++)
                VDP_setTileMapXY(BG_B, bg, oldx + cx, oldy + cy);
        x = nx;
        y = ny;
    }
    return TRUE;
}

static void clear_area_play(u8 rec)
{
    const SceneClear *c = &scene_clear[rec];
    // Fill colour lives on PAL1 entry 1 (PAL_setColor takes the global CRAM
    // index: PAL1 entry 1 = 16 + 1). Text stays on PAL0.
    pal_color((SCENE_BG_PAL0 << 4) + 1, c->color);
    const u16 fill = TILE_ATTR_FULL(SCENE_BG_PAL0, FALSE, FALSE, FALSE,
                                    SCENE_BLACK_TILE);
    VDP_fillTileMapRect(BG_B, fill,
                        c->x >> 3, (c->y - 16) >> 3, c->w >> 3, c->h >> 3);
    // BG_A stays transparent over the fill
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, 0),
                        c->x >> 3, (c->y - 16) >> 3, c->w >> 3, c->h >> 3);
    scene_bg_entry = fill;
    scene_bg_set = TRUE;
    scene_tile_next = TILE_USER_INDEX + 1;
    scene_img_line = 0;
}

void scene_play(u8 index)
{
    if (index >= 6) return;

    sprites_end();                 // the intro logos used the sprite pool

    VDP_setTextPlane(BG_A);
    pal_color(0, 0x0000);   // backdrop / font background
    pal_color(1, 0x0000);   // black tile
    pal_color(15, 0x0EEE);  // font white
    VDP_setBackgroundColor(0);
    VDP_loadTileData(black_tile, SCENE_BLACK_TILE, 1, DMA);
    SYS_doVBlankProcess();

    const SceneDef *s = &scene_defs[index];
    for (u8 i = 0; i < s->count; i++) {
        const SceneOp *op = &s->ops[i];
        switch (op->type) {
            case SCENE_CLEAR_SCREEN:
                clear_screen();
                break;
            case SCENE_CLEAR_AREA:
                clear_area_play(op->seek);
                break;
            case SCENE_SHOW_TEXT:
                if (!text_play(op->seek)) return;
                break;
            case SCENE_MOVE_IMAGE:
                if (!show_image(op->seek)) return;
                break;
            case SCENE_PARALLAX:
                if (!parallax_play(&scene_parallax[op->seek])) return;
                break;
            case SCENE_PLAY_MUSIC:
                // PC sceneShow::play_music -> playmusic_list[n]; the converter
                // maps each record's filename to a MUSIC_* id (music.c owns the
                // id -> resource table), so a cutscene can change the track.
                wd_ck("scene PLAY_MUSIC before");
                if (op->seek < SCENE_MUSIC_COUNT &&
                    scene_music_ids[op->seek] != MUSIC_NONE)
                    music_play(scene_music_ids[op->seek]);
                wd_ckq("scene PLAY_MUSIC after");
                break;
            case SCENE_STOP_MUSIC:
                wd_ck("scene STOP_MUSIC before");
                music_stop();
                wd_ck("scene STOP_MUSIC after");
                break;
            case SCENE_PLAY_SFX:
                // No converted scene uses this op; scenes_play_sfx.dat has two
                // records but no scene references them (checked 2026-10-23).
                break;
            case SCENE_MOVE_VIEWPOINT:
            case SCENE_SHOW_ANIMATION:
            case SCENE_SUBSCENE:
                // Declared by the PC scene format, never used by a scene the
                // port plays (the intro is scene 0, the endings are 2..5; the
                // LAB opening, scene 1, is the only user of SHOW_ANIMATION).
                // tools/convert_scenes.py refuses to emit a *played* scene with
                // one of these, so reaching here means a data export went
                // around the check - say so instead of skipping silently.
                dbg_print("scene %d: op %d not implemented, skipped\n",
                        index, op->type);
                break;
            default:
                break;
        }
        if (!wait_ms(op->delay)) return;
    }
    wd_ckq("scene end");
}
