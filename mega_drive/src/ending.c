#include "ending.h"

#include <genesis.h>
#include <string.h>

#include "resources.h"
#include "player.h"
#include "stage_data.h"
#include "ssel_data.h"
#include "credits_data.h"
#include "end_l1_tiles.h"
#include "end_l1_map.h"
#include "end_l2_tiles.h"
#include "end_l2_map.h"
#include "end_l3_tiles.h"
#include "end_l3_map.h"

// PC scenesLib::show_player_walking_ending, show_enemies_ending and
// draw_lib::show_credits. The three walking layers use BG_B for the back
// layer and BG_A for the two front layers; the front layers scroll at
// different speeds through the per-line horizontal scroll table (the MD has
// only two planes but one scroll value per line).

#define END_BASE3 TILE_USER_INDEX
#define END_BASE2 (END_BASE3 + END_L3_TILE_COUNT)
#define END_BASE1 (END_BASE2 + END_L2_TILE_COUNT)

#define END_PAL3 PAL0
#define END_PAL2 PAL1
#define END_PAL1 PAL3

#define END_L3_ROW 1
#define END_L2_ROW 8
#define END_L1_ROW 15

#define END_WALK_X 336
#define END_WALK_Y 91
#define END_WALK_FRAMES 400

#define BLANK_TILE TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX)

static void black_screen(void)
{
    u16 blank = BLANK_TILE;
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);
    VDP_fillTileMapRect(BG_A, blank, 0, 0, 64, 32);
    VDP_fillTileMapRect(BG_B, blank, 0, 0, 64, 32);
}

static void center_text(const char *s, u16 row)
{
    u16 len = (u16)strlen(s);
    if (len > 40) len = 40;
    VDP_drawText(s, (40 - len) / 2, row);
}

static bool wait_frames(u16 n)
{
    while (n--) {
        if (JOY_readJoypad(JOY_1) & BUTTON_START) return FALSE;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
    return TRUE;
}

// Upload one repeated map into a plane. Every row repeats the source map
// horizontally so a 40-tile (320px) image can scroll across the 64-tile plane.
static void layer_upload(u16 plane, const u16 *map, u16 w, u16 h,
                         u16 base, u16 pal, u16 row)
{
    // One row at a time: this buffer used to be 64x20 (2.5 KB of BSS), which
    // shrank the heap enough that MAP_create could fail on some stages (black
    // screen, then a crash in stage_update_camera).
    static u16 cells[64];
    for (u16 y = 0; y < h; y++) {
        for (u16 x = 0; x < 64; x++) {
            u16 idx = map[y * w + (x % w)];
            cells[x] = TILE_ATTR_FULL(pal, FALSE, FALSE, FALSE, base + idx);
        }
        VDP_setTileMapDataRect(plane, cells, 0, row + y, 64, 1, 64, CPU);
    }
}

static void walking_parade(void)
{
    SPR_end();
    SPR_initEx(504);

    black_screen();

    VDP_loadTileData(end_l3_tiles_data, END_BASE3,
                     END_L3_TILE_COUNT, DMA_QUEUE);
    VDP_loadTileData(end_l2_tiles_data, END_BASE2,
                     END_L2_TILE_COUNT, DMA_QUEUE);
    VDP_loadTileData(end_l1_tiles_data, END_BASE1,
                     END_L1_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(END_PAL3, end_l3_pal.data, DMA_QUEUE);
    PAL_setPalette(END_PAL2, end_l2_pal.data, DMA_QUEUE);
    PAL_setPalette(END_PAL1, end_l1_pal.data, DMA_QUEUE);
    PAL_setPalette(PAL2, player_palette_def()->data, DMA_QUEUE);

    layer_upload(BG_B, end_l3_map, END_L3_MAP_W, END_L3_MAP_H,
                 END_BASE3, END_PAL3, END_L3_ROW);
    layer_upload(BG_A, end_l2_map, END_L2_MAP_W, END_L2_MAP_H,
                 END_BASE2, END_PAL2, END_L2_ROW);
    layer_upload(BG_A, end_l1_map, END_L1_MAP_W, END_L1_MAP_H,
                 END_BASE1, END_PAL1, END_L1_ROW);

    Sprite *ply = SPR_addSprite(player_definition(), END_WALK_X, END_WALK_Y,
                                TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    if (ply) SPR_setAnim(ply, ANIM_WALK);

    VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
    static s16 hs_a[28];
    static s16 hs_b[28];

    u16 frame = 0;
    for (;;) {
        s16 px = (s16)(END_WALK_X - frame);
        if (ply) SPR_setPosition(ply, px, END_WALK_Y);

        s16 o1 = (s16)-(s16)(frame % 320);
        s16 o2 = (s16)-(s16)((frame * 2) % 320);
        s16 o3 = (s16)-(s16)((frame * 4) % 320);
        for (u16 y = 0; y < 28; y++) {
            hs_b[y] = o3;
            if (y >= END_L2_ROW && y < END_L2_ROW + END_L2_MAP_H)
                hs_a[y] = o2;
            else if (y >= END_L1_ROW && y < END_L1_ROW + END_L1_MAP_H)
                hs_a[y] = o1;
            else
                hs_a[y] = 0;
        }
        VDP_setHorizontalScrollLine(BG_B, 0, hs_b, 28, DMA_QUEUE);
        VDP_setHorizontalScrollLine(BG_A, 0, hs_a, 28, DMA_QUEUE);

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);

        if (JOY_readJoypad(JOY_1) & BUTTON_START) break;
        if (frame >= END_WALK_FRAMES) break;
        frame++;
    }

    if (ply) SPR_releaseSprite(ply);
    SPR_update();
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
}

// PC show_enemies_ending is a long line of normal enemies; the port shows the
// bosses, which are the sprites every stage already carries. One boss per
// stage, name under it, START skips.
static void boss_parade(void)
{
    black_screen();
    PAL_setPalette(PAL0, pal_ui.data, CPU);

    for (u8 stage = 0; stage < STAGE_DEFINED_COUNT; stage++) {
        stage_set_data(stage);

        const EnemyDef *boss = NULL;
        for (u8 i = 0; i < stage_def->enemy_def_count; i++) {
            if (stage_def->enemy_defs[i].is_boss) {
                boss = &stage_def->enemy_defs[i];
                break;
            }
        }
        if (!boss) continue;

        Sprite *spr = NULL;
        if (boss->spr)
            spr = SPR_addSprite(boss->spr,
                                160 - boss->spr->w / 2,
                                104 - boss->spr->h / 2,
                                TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (spr) SPR_setAnim(spr, 0);

        VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, 0, 64, 32);
        if (stage < SSEL_STAGE_SLOTS) {
            center_text(ssel_stage_names[stage], 1);
            if (ssel_boss_names[stage][0])
                center_text(ssel_boss_names[stage], 23);
        }

        if (!wait_frames(100)) {
            if (spr) SPR_releaseSprite(spr);
            break;
        }
        if (spr) SPR_releaseSprite(spr);
        SPR_update();
    }
}

static void credits_scroll(void)
{
    black_screen();
    PAL_setPalette(PAL0, pal_ui.data, CPU);

    u16 top = 0;
    while (top < CREDITS_LINES) {
        VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, 0, 64, 32);
        for (u16 row = 0; row < 28; row++) {
            u16 i = (u16)(top + row);
            if (i < CREDITS_LINES && credits_lines[i][0])
                center_text(credits_lines[i], row);
        }
        if (!wait_frames(2)) break;
        top++;
    }

    // Hold the last page for a moment (PC wait_and_update_screen).
    if (!(JOY_readJoypad(JOY_1) & BUTTON_START)) wait_frames(120);
}

void ending_run(void)
{
    walking_parade();
    boss_parade();
    credits_scroll();

    SPR_end();
    black_screen();
}

void ending_credits_run(void)
{
    SPR_end();
    SPR_initEx(504);
    credits_scroll();
    SPR_end();
    black_screen();
}
