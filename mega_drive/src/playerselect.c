#include "playerselect.h"

#include "resources.h"
#include "stage.h"
#include "psel_bg_tiles.h"
#include "psel_bg_map.h"
#include "player_data.h"
#include "psel_p1_tiles.h"
#include "psel_p1_map.h"
#include "psel_p2_tiles.h"
#include "psel_p2_map.h"
#include "psel_p3_tiles.h"
#include "psel_p3_map.h"
#include "psel_p4_tiles.h"
#include "psel_p4_map.h"

#include <string.h>

#define PSEL_PAL_BG    PAL1
#define PSEL_PAL_PORT  PAL2
#define PSEL_BG_BASE   TILE_USER_INDEX
#define PSEL_PORT_BASE (PSEL_BG_BASE + PSEL_BG_TILE_COUNT)
#define PSEL_ROW0      4          // PC portrait y=50 -> (50-16)/8 = 4 (y32)
#define PSEL_MAX       4

static const u32 *const p_tiles[PSEL_MAX] = {
    psel_p1_tiles_data, psel_p2_tiles_data, psel_p3_tiles_data, psel_p4_tiles_data
};
static const u16 p_tile_n[PSEL_MAX] = {
    PSEL_P1_TILE_COUNT, PSEL_P2_TILE_COUNT, PSEL_P3_TILE_COUNT, PSEL_P4_TILE_COUNT
};
static const u16 *const p_maps[PSEL_MAX] = {
    psel_p1_map, psel_p2_map, psel_p3_map, psel_p4_map
};
static const u16 p_map_h[PSEL_MAX] = {
    PSEL_P1_MAP_H, PSEL_P2_MAP_H, PSEL_P3_MAP_H, PSEL_P4_MAP_H
};
static const Palette *const p_pal[PSEL_MAX] = {
    &psel_p1_pal, &psel_p2_pal, &psel_p3_pal, &psel_p4_pal
};

static u8 sel;

static void draw_centered(const char *t, u16 row)
{
    u8 len = strlen(t);
    VDP_drawText(t, (len < 40) ? (40 - len) / 2 : 0, row);
}

static void draw_portrait(void)
{
    VDP_loadTileData(p_tiles[sel], PSEL_PORT_BASE, p_tile_n[sel], DMA_QUEUE);
    PAL_setPalette(PSEL_PAL_PORT, p_pal[sel]->data, DMA_QUEUE);
    u16 w = PSEL_P1_MAP_W;
    static u16 cells[PSEL_P1_MAP_W * PSEL_P1_MAP_H];
    for (u16 y = 0; y < p_map_h[sel]; y++) {
        for (u16 x = 0; x < w; x++) {
            cells[y * w + x] = TILE_ATTR_FULL(PSEL_PAL_PORT, FALSE, FALSE, FALSE,
                                              PSEL_PORT_BASE + p_maps[sel][y * w + x]);
        }
    }
    VDP_setTileMapDataRect(BG_A, cells, 0, PSEL_ROW0, w, p_map_h[sel], w, CPU);
}

static void draw_name(void)
{
    // clear the name line, then draw
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PSEL_PAL_BG, FALSE, FALSE, FALSE,
                                       PSEL_BG_BASE + psel_bg_map[20 * 40]),
                        0, 20, 40, 1);
    draw_centered(player_data[sel].name, 20);
}

u8 player_select_run(void)
{
    SPR_end();
    stage_release_map();
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_clearPlane(BG_B, TRUE);

    VDP_loadTileData(psel_bg_tiles_data, PSEL_BG_BASE, PSEL_BG_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(PSEL_PAL_BG, psel_bg_pal.data, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);

    static u16 bgcells[PSEL_BG_MAP_W * PSEL_BG_MAP_H];
    for (u16 i = 0; i < PSEL_BG_MAP_W * PSEL_BG_MAP_H; i++) {
        bgcells[i] = TILE_ATTR_FULL(PSEL_PAL_BG, FALSE, FALSE, FALSE,
                                    PSEL_BG_BASE + psel_bg_map[i]);
    }
    VDP_setTileMapDataRect(BG_A, bgcells, 0, 0, PSEL_BG_MAP_W, PSEL_BG_MAP_H,
                           PSEL_BG_MAP_W, CPU);

    sel = 0;
    draw_portrait();
    draw_centered("SELECT PLAYER", 1);
    draw_centered("PRESS START TO SELECT", 25);
    draw_name();

    // flush queued DMA before accepting input (see bossintro.c)
    SYS_doVBlankProcess();
    VDP_setReg(17, 0);
    VDP_setReg(18, 0);

    u16 prev = JOY_readJoypad(JOY_1);
    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);
        u16 pressed = pad & ~prev;
        prev = pad;

        if (pressed & BUTTON_RIGHT) {
            sel = (sel + 1) % PSEL_MAX;
            draw_portrait();
            draw_name();
        } else if (pressed & BUTTON_LEFT) {
            sel = (sel + PSEL_MAX - 1) % PSEL_MAX;
            draw_portrait();
            draw_name();
        } else if (pressed & (BUTTON_START | BUTTON_A | BUTTON_C)) {
            return sel;
        }

        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}
