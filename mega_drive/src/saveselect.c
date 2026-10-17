#include "saveselect.h"

#include "resources.h"
#include "saveselect_data.h"
#include "save.h"
#include "stage.h"
#include <string.h>

#define P_EN  PAL1
#define P_DIS PAL2
#define ROW0  2                 // first bar at tile row 2 (y=16)

static u16 sel;

static void u2str(u16 v, char *b)
{
    if (v >= 10) {
        b[0] = (char)('0' + v / 10);
        b[1] = (char)('0' + v % 10);
        b[2] = 0;
    } else {
        b[0] = (char)('0' + v);
        b[1] = 0;
    }
}

static u8 popcount16(u16 v)
{
    u8 n = 0;
    while (v) { n += v & 1; v >>= 1; }
    return n;
}

static void draw_rows(void)
{
    static u16 cells[SAVE_ROW_W * SAVE_ROW_H * SAVE_SLOT_COUNT];
    for (u8 s = 0; s < SAVE_SLOT_COUNT; s++) {
        u16 pal = (s == sel) ? P_EN : P_DIS;
        for (u16 r = 0; r < SAVE_ROW_H; r++) {
            for (u16 c = 0; c < SAVE_ROW_W; c++) {
                cells[(s * SAVE_ROW_H + r) * SAVE_ROW_W + c] =
                    (u16)(saveselect_map[r * SAVE_ROW_W + c] + SAVE_TILE_BASE)
                    | (u16)(pal << 13);
            }
        }
    }
    VDP_setTileMapDataRect(BG_A, cells, 0, ROW0, SAVE_ROW_W,
                           SAVE_ROW_H * SAVE_SLOT_COUNT, SAVE_ROW_W, CPU);
}

static void draw_labels(void)
{
    char buf[24];
    for (u8 s = 0; s < SAVE_SLOT_COUNT; s++) {
        u16 ty = ROW0 + s * SAVE_ROW_H + 2;
        buf[0] = 'S'; buf[1] = 'L'; buf[2] = 'O'; buf[3] = 'T';
        buf[4] = ' '; buf[5] = (char)('1' + s); buf[6] = 0;
        VDP_drawText(buf, 2, ty);
        if (save_slot_exists(s)) {
            u2str(popcount16(save_slot_stages(s)), buf);
            strcat(buf, " CLEARED");
        } else {
            strcpy(buf, "NO SAVE");
        }
        VDP_drawText(buf, 13, ty);
    }
}

s16 save_selector(bool is_new_game)
{
    stage_release_map();
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);

    VDP_loadTileData(saveselect_tiles_data, SAVE_TILE_BASE, SAVE_TILE_COUNT,
                     CPU);   // tiny (7 tiles); CPU avoids a stale first frame
    PAL_setPalette(P_EN, saveselect_pal_en, DMA_QUEUE);
    PAL_setPalette(P_DIS, saveselect_pal_dis, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);

    // black backdrop on both planes: the title logo still lives on BG_B
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    draw_rows();
    draw_labels();
    VDP_drawText(is_new_game ? "NEW GAME" : "LOAD GAME", 2, 0);
    VDP_drawText("START/A: SELECT   B: BACK", 2, 27);

    sel = 0;
    SYS_doVBlankProcess();      // flush queued DMA before input
    u16 prev = JOY_readJoypad(JOY_1);
    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);
        u16 pressed = pad & ~prev;
        prev = pad;
        bool redraw = FALSE;

        if (pressed & BUTTON_DOWN) {
            sel = (sel + 1) % SAVE_SLOT_COUNT;
            redraw = TRUE;
        } else if (pressed & BUTTON_UP) {
            sel = (sel + SAVE_SLOT_COUNT - 1) % SAVE_SLOT_COUNT;
            redraw = TRUE;
        } else if (pressed & (BUTTON_START | BUTTON_A | BUTTON_C)) {
            if (is_new_game || save_slot_exists(sel)) {
                return (s16)sel;
            }
            // no SFX here: menu PCM used to load the XGM2 Z80 driver before
            // any music, and BlastEm's Z80 core aborted on the driver. Audio
            // is a later milestone; menus stay silent.
        } else if (pressed & BUTTON_B) {
            return -1;
        }

        if (redraw) {
            draw_rows();
            draw_labels();
        }

        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}
