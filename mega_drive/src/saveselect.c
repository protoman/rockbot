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

// ---- save-slot icons (PC scenesLib::draw_save_details, sceneslib.cpp:916) --
// One 16x16 icon per stage/weapon (frame 0 = not owned, 1 = owned) plus the
// selected player's icon, at the PC's spacing: x = (i+1)*18-14, 12px below the
// row top. The port models neither the tanks nor the difficulty, so the rest
// of the PC's row (11/12/13 + [M]) is not drawn.
#define ICON_N    10                       // 9 stages/weapons + the player
#define ICON_DX   18
#define ICON_X0   4
#define ICON_DY   12

static Sprite *icons[SAVE_SLOT_COUNT][ICON_N];

static const SpriteDefinition *const icon_def[9] = {
    &sicon_00_sprite, &sicon_01_sprite, &sicon_02_sprite, &sicon_03_sprite,
    &sicon_04_sprite, &sicon_05_sprite, &sicon_06_sprite, &sicon_07_sprite,
    &sicon_08_sprite,
};
static const SpriteDefinition *const picon_def[4] = {
    &sicon_14_sprite, &sicon_15_sprite, &sicon_16_sprite, &sicon_17_sprite,
};

static void icons_clear(void)
{
    for (u8 s = 0; s < SAVE_SLOT_COUNT; s++)
        for (u8 i = 0; i < ICON_N; i++) {
            if (icons[s][i]) {
                SPR_releaseSprite(icons[s][i]);
                icons[s][i] = NULL;
            }
        }
}

static void icons_draw(void)
{
    icons_clear();
    for (u8 s = 0; s < SAVE_SLOT_COUNT; s++) {
        if (!save_slot_exists(s)) continue;
        const u16 mask = save_slot_stages(s);
        const s16 y = (s16)(ROW0 * 8 + s * SAVE_ROW_H * 8 + ICON_DY);
        for (u8 i = 0; i < 9; i++) {
            Sprite *sp = SPR_addSprite(icon_def[i], ICON_X0 + i * ICON_DX, y,
                                       TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
            if (!sp) break;
            // frame 0 = disabled art, frame 1 = enabled (see the converter)
            SPR_setAutoAnimation(sp, FALSE);
            SPR_setFrame(sp, (mask & (1u << i)) ? 1 : 0);
            icons[s][i] = sp;
        }
        Sprite *ps = SPR_addSprite(picon_def[save_slot_player(s) & 3],
                                   ICON_X0 + 9 * ICON_DX, y,
                                   TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
        if (ps) {
            SPR_setAutoAnimation(ps, FALSE);
            SPR_setFrame(ps, 1);
            icons[s][9] = ps;
        }
    }
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
        // An occupied slot shows its icons (sprites, icons_draw) instead of
        // the old "N CLEARED" text; empty slots keep the PC's "NO SAVE".
        if (!save_slot_exists(s)) {
            strcpy(buf, "NO SAVE");
            VDP_drawText(buf, 13, ty);
        }
    }
}

s16 save_selector(bool is_new_game)
{
    stage_release_map();
    // The title calls SPR_end(): without this the icon sprites below are all
    // NULL (SPR_addSprite fails with the engine ended), which is exactly what
    // happened — the slot rows came up empty. The stage select re-inits the
    // same way.
    SPR_initEx(504);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);

    VDP_loadTileData(saveselect_tiles_data, SAVE_TILE_BASE, SAVE_TILE_COUNT,
                     CPU);   // tiny (7 tiles); CPU avoids a stale first frame
    PAL_setPalette(P_EN, saveselect_pal_en, DMA_QUEUE);
    PAL_setPalette(P_DIS, saveselect_pal_dis, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);
    // icons live on PAL3 (PAL0 font, PAL1/PAL2 the bars)
    PAL_setPalette(PAL3, sicon_pal.data, DMA_QUEUE);

    // black backdrop on both planes: the title logo still lives on BG_B
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    draw_rows();
    draw_labels();
    icons_draw();
    VDP_drawText(is_new_game ? "NEW GAME" : "LOAD GAME", 2, 0);
    VDP_drawText("START/A: SELECT   B: BACK", 2, 27);

    sel = 0;
    SPR_update();               // the icons are sprites: write the SAT
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
                icons_clear();
                SPR_end();
                return (s16)sel;
            }
            // no SFX here: menu PCM used to load the XGM2 Z80 driver before
            // any music, and BlastEm's Z80 core aborted on the driver. Audio
            // is a later milestone; menus stay silent.
        } else if (pressed & BUTTON_B) {
            icons_clear();
            SPR_end();
            return -1;
        }

        if (redraw) {
            draw_rows();
            draw_labels();
            icons_draw();
        }

        SPR_update();           // sprites (save icons) need the SAT written
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}
