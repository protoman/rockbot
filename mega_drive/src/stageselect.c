#include "stageselect.h"

#include "music.h"
#include "pal_safe.h"

#include "resources.h"
#include "ssel_data.h"
#include "stage.h"
#include "stage_data.h"   // STAGE_DEFINED_COUNT
#include "sfx.h"

#include <string.h>

// PC classic-style layout, 320x224 (top/bottom 8px of the 240 art cropped):
// faces at 64/40 spacing 80x64; cursor 8px up-left of the face. The
// converter bakes the static screen (bg + faces + player face) as tiles.
#define SSEL_PAL_BG     PAL3
#define SSEL_PAL_PLAYER PAL2
#define SSEL_PAL_GRAY   PAL1
#define SSEL_PAL_CURSOR PAL0

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
static u16 sel_mask;            // beaten-stage bits for this save slot
static bool castle_unlocked;

// PC get_last_castle_stage(): the first unbeaten castle #1 stage (9..13).
static u8 castle_next_stage(void)
{
    u8 st = 9;
    while (st < 13 && (sel_mask & (1u << st))) st++;
    return st;
}

static u8 complete_stage(void)
{
    u8 stage_n = sy * 3 + sx + 1;      // PC stage_n, 1..9
    return (stage_n > 5) ? stage_n - 1 : stage_n;
}

// Boss faces sit at 64+80*col, 32+64*row (8x8 tiles); face 32x32 = 4x4.
static const u8 face_col[8] = { 0, 1, 2, 0, 2, 0, 1, 2 };
static const u8 face_row[8] = { 0, 0, 0, 1, 1, 2, 2, 2 };

// Switch the boss faces of beaten stages to PAL1 (the grey ramp). The tile
// pixels are unchanged; only the tilemap palette bits are rewritten.
static void grey_beaten_faces(void)
{
    for (u8 st = 1; st <= 8; st++) {
        if (!(sel_mask & (1u << st))) continue;
        const u16 tx0 = (u16)(8 + face_col[st - 1] * 10);
        const u16 ty0 = (u16)(4 + face_row[st - 1] * 8);
        for (u8 dy = 0; dy < 4; dy++) {
            for (u8 dx = 0; dx < 4; dx++) {
                u16 *e = &cells[(ty0 + dy) * SSEL_MAP_W + tx0 + dx];
                *e = (*e & ~TILE_ATTR_PALETTE_MASK) |
                     ((u16)SSEL_PAL_GRAY << TILE_ATTR_PALETTE_SFT);
            }
        }
    }
}

// Only STAGE_DEFINED_COUNT stages exist; stage_set_data() clamps anything else
// to stage 0, so an unimplemented slot would silently load CHAOS CITY.
static bool stage_ready(u8 st)
{
    return st < STAGE_DEFINED_COUNT;
}

static void draw_names(void)
{
    // The names get their own BLACK band instead of the PC bar art (BUG-014
    // kept coming back after a stage: the bar and the inherited palette line
    // depend on what the stage left in CRAM/video memory, so the glyph cells
    // could end up drawn in a stage colour). PAL0 = pal_ui (0 = black, 15 =
    // font ink) is written with the safe CRAM path so a pending DMA cannot
    // redirect it, and VDP_clearTextArea paints the band with font spaces in
    // palette 0 = solid black, whatever the bar tiles behind it look like.
    VDP_setTextPlane(BG_A);
    pal_load(PAL0, pal_ui.data);
    VDP_setTextPalette(PAL0);
    VDP_clearTextArea(0, BAR_ROW0 + 1, SSEL_MAP_W, BAR_ROWS - 1);

    if (sx == 1 && sy == 1 && !castle_unlocked) return;  // centre: player icon

    const bool centre = (sx == 1 && sy == 1);
    u8 st = centre ? castle_next_stage() : complete_stage();
    char buf[40];
    strcpy(buf, "BOSS ");
    strcat(buf, ssel_boss_names[st]);
    VDP_drawText(buf, 2, BAR_ROW0 + 1);
    strcpy(buf, "STAGE ");
    strcat(buf, ssel_stage_names[st]);
    VDP_drawText(buf, 2, BAR_ROW0 + 2);
    if (!centre && !stage_ready(st)) {
        VDP_drawText("NOT CONVERTED YET", 2, BAR_ROW0 + 3);
    }
}

u8 stage_select_run(u8 player, u16 stage_mask)
{
    sprites_begin(SPRITE_VRAM_TILES);
    stage_release_map();
    music_play(MUSIC_SELECTOR);

    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);

    // The font's VRAM block sits above every screen's tiles (TILE_FONT_INDEX
    // 1440); reload it before the first name line so a previous screen that
    // wrote past the user tile area cannot leave the glyphs garbled
    // (BUG-014 "stage select font garbage"). CPU transfer: the font resource
    // is compressed, so unpacking frees its buffer - unsafe for a queued DMA.
    VDP_loadDefaultFont(CPU);
    VDP_loadTileData(ssel_tiles_data, SSEL_TILE_BASE, SSEL_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(SSEL_PAL_BG, ssel_pal_bg, DMA_QUEUE);
    PAL_setPalette(SSEL_PAL_GRAY, ssel_pal_gray, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);

    sel_mask = stage_mask;
    for (u16 i = 0; i < SSEL_MAP_W * SSEL_MAP_H; i++) {
        cells[i] = ssel_map[i] + SSEL_TILE_BASE;   // palette bits preserved
    }
    grey_beaten_faces();
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
    castle_unlocked = (stage_mask & 0x01FF) == 0x01FF;
    // Player face + eyes (per selected player), PAL2 loaded from the face
    // sheet. PC: face at (144,104), eyes at (154,122) nudged +-2px toward
    // the cursor cell (screen y 96/114 after the 8px crop).
    // MD sprite priority: LOWER index draws in front, so the eyes must be
    // added BEFORE the face or the face art simply covers them (same rule the
    // cursor layers below rely on).
    u8 pi = player & 3;
    if (castle_unlocked) {
        // PC pick_stage_classic_style: once the castle is accessible, the
        // centre slot shows stage_select_castle.png instead of the player.
        PAL_setPalette(SSEL_PAL_PLAYER, ssel_castle_sprite.palette->data,
                       DMA_QUEUE);
        eyes = NULL;
        face = SPR_addSprite(&ssel_castle_sprite, 144, 96,
                             TILE_ATTR(SSEL_PAL_PLAYER, FALSE, FALSE, FALSE));
    } else {
        PAL_setPalette(SSEL_PAL_PLAYER, face_def[pi]->palette->data, DMA_QUEUE);
        eyes = SPR_addSprite(eyes_def[pi], 154, 114,
                             TILE_ATTR(SSEL_PAL_PLAYER, FALSE, FALSE, FALSE));
        face = SPR_addSprite(face_def[pi], 144, 96,
                             TILE_ATTR(SSEL_PAL_PLAYER, FALSE, FALSE, FALSE));
    }

    sx = 1;                 // default to the centre slot
    sy = 1;
    draw_names();               // borrows PAL1 for the lines, restores PAL0
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
            if (sx == 1 && sy == 1) {
                if (castle_unlocked) {
                    u8 st = castle_next_stage();
                    sfx_play(SFX_STAGE_SELECTED);
                    sprites_end();
                    return st;            // unbeaten castle #1 stage
                }
            } else {
                u8 st = complete_stage();
                if (stage_ready(st)) {
                    sfx_play(SFX_STAGE_SELECTED);
                    sprites_end();
                    return st;
                }
            }
        }
        if (moved) {
            sfx_play(SFX_CURSOR);
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
