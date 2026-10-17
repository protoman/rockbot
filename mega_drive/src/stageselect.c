#include "stageselect.h"

#include "resources.h"
#include "ssel_data.h"
#include "stage.h"

#include <string.h>

// PC classic-style layout, 320x224 (top/bottom 8px of the 240 art cropped):
// faces at 64/40 spacing 80x64; cursor 8px up-left of the face. The
// converter bakes the static screen (bg + faces + player face) as tiles.
#define SSEL_PAL_BG     PAL3
#define SSEL_PAL_PLAYER PAL2
#define SSEL_PAL_CURSOR PAL1

#define CURSOR_X0   56          // face_x 64 - 8
#define CURSOR_Y0   24          // face_y 32 (PC 40 - crop 8) - 8
#define CURSOR_DX   80
#define CURSOR_DY   64

#define BAR_ROW0    24
#define BAR_ROWS    4

static Sprite *cur[8];          // 4 quadrants x {black, yellow}
static Sprite *eyes;            // player-face eyes, follow the cursor
static Sprite *face;            // player-face portrait (per player)
static const SpriteDefinition *const face_def[4] = {
    &ssel_face_p1_sprite, &ssel_face_p2_sprite,
    &ssel_face_p3_sprite, &ssel_face_p4_sprite,
};
static const SpriteDefinition *const eyes_def[4] = {
    &ssel_eyes_p1_sprite, &ssel_eyes_p2_sprite,
    &ssel_eyes_p3_sprite, &ssel_eyes_p4_sprite,
};
static u16 cells[SSEL_MAP_W * SSEL_MAP_H];
static s8 sx, sy;

static u8 complete_stage(void)
{
    u8 stage_n = sy * 3 + sx + 1;      // PC stage_n, 1..9
    return (stage_n > 5) ? stage_n - 1 : stage_n;
}

static void draw_names(void)
{
    // restore the bottom bar from the static map, then draw the two lines
    VDP_setTileMapDataRect(BG_A, &cells[BAR_ROW0 * SSEL_MAP_W], 0, BAR_ROW0,
                           SSEL_MAP_W, BAR_ROWS, SSEL_MAP_W, CPU);
    if (sx == 1 && sy == 1) {
        return;                     // centre slot: castle/player, no name
    }
    u8 st = complete_stage();
    char buf[40];
    strcpy(buf, "BOSS ");
    strcat(buf, ssel_boss_names[st]);
    VDP_drawText(buf, 2, BAR_ROW0 + 1);
    strcpy(buf, "STAGE ");
    strcat(buf, ssel_stage_names[st]);
    VDP_drawText(buf, 2, BAR_ROW0 + 2);
}

u8 stage_select_run(u8 player)
{
    SPR_initEx(504);
    stage_release_map();

    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);

    VDP_loadTileData(ssel_tiles_data, SSEL_TILE_BASE, SSEL_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(SSEL_PAL_BG, ssel_pal_bg, DMA_QUEUE);
    PAL_setPalette(SSEL_PAL_CURSOR, ssel_pal_cursor, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);

    for (u16 i = 0; i < SSEL_MAP_W * SSEL_MAP_H; i++) {
        cells[i] = ssel_map[i] + SSEL_TILE_BASE;   // palette bits preserved
    }
    VDP_setTileMapDataRect(BG_A, cells, 0, 0, SSEL_MAP_W, SSEL_MAP_H,
                           SSEL_MAP_W, CPU);

    // single-frame SPRITEs (the engine never auto-animates them)
    const SpriteDefinition *qblack[4] = {
        &ssel_cursor_tl_b_sprite, &ssel_cursor_tr_b_sprite,
        &ssel_cursor_bl_b_sprite, &ssel_cursor_br_b_sprite,
    };
    const SpriteDefinition *qyellow[4] = {
        &ssel_cursor_tl_y_sprite, &ssel_cursor_tr_y_sprite,
        &ssel_cursor_bl_y_sprite, &ssel_cursor_br_y_sprite,
    };
    for (u8 i = 0; i < 4; i++) {
        // SGDK/MD: lower sprite index = higher priority. Add the yellow
        // (blink) layer FIRST so it draws over the always-on black brackets.
        cur[4 + i] = SPR_addSprite(qyellow[i], 0, 0,
                                   TILE_ATTR(SSEL_PAL_CURSOR, FALSE, FALSE, FALSE));
        cur[i] = SPR_addSprite(qblack[i], 0, 0,
                               TILE_ATTR(SSEL_PAL_CURSOR, FALSE, FALSE, FALSE));
    }
    // Player face + eyes (per selected player), PAL2 loaded from the face
    // sheet. PC: face at (144,104), eyes at (154,122) nudged +-2px toward
    // the cursor cell (screen y 96/114 after the 8px crop).
    u8 pi = player & 3;
    PAL_setPalette(SSEL_PAL_PLAYER, face_def[pi]->palette->data, DMA_QUEUE);
    face = SPR_addSprite(face_def[pi], 144, 96,
                         TILE_ATTR(SSEL_PAL_PLAYER, FALSE, FALSE, FALSE));
    eyes = SPR_addSprite(eyes_def[pi], 154, 114,
                         TILE_ATTR(SSEL_PAL_PLAYER, FALSE, FALSE, FALSE));

    sx = 0;
    sy = 0;
    draw_names();
    SYS_doVBlankProcess();      // flush queued DMA before input

    u16 prev = JOY_readJoypad(JOY_1);
    u16 blink = 0;
    bool yellow = TRUE;

    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);
        u16 pressed = pad & ~prev;
        prev = pad;

        bool moved = FALSE;
        if (pressed & BUTTON_RIGHT) { sx = (sx + 1) % 3; moved = TRUE; }
        else if (pressed & BUTTON_LEFT) { sx = (sx + 2) % 3; moved = TRUE; }
        else if (pressed & BUTTON_DOWN) { sy = (sy + 1) % 3; moved = TRUE; }
        else if (pressed & BUTTON_UP) { sy = (sy + 2) % 3; moved = TRUE; }
        else if (pressed & (BUTTON_START | BUTTON_A | BUTTON_C)) {
            if (!(sx == 1 && sy == 1)) {    // centre = castle, locked
                u8 st = complete_stage();
                SPR_end();
                return st;
            }
        }
        if (moved) {
            draw_names();
        }

        // PC classic style pulses the yellow layer over the black brackets;
        // ~0.8s per phase (light / no light) at 60fps = 48 frames.
        if (++blink >= 48) {
            blink = 0;
            yellow = !yellow;
        }
        if (eyes) {
            s16 ex = 154 + ((sx == 0) ? -2 : (sx == 2) ? 2 : 0);
            s16 ey = 114 + ((sy == 0) ? -2 : (sy == 2) ? 2 : 0);
            SPR_setPosition(eyes, ex, ey);
        }
        s16 cx = CURSOR_X0 + sx * CURSOR_DX;
        s16 cy = CURSOR_Y0 + sy * CURSOR_DY;
        for (u8 i = 0; i < 4; i++) {
            SPR_setPosition(cur[i], cx + (i & 1 ? 24 : 0), cy + (i & 2 ? 24 : 0));
            SPR_setPosition(cur[4 + i], cx + (i & 1 ? 24 : 0), cy + (i & 2 ? 24 : 0));
            SPR_setVisibility(cur[4 + i], yellow ? VISIBLE : HIDDEN);
        }

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}
