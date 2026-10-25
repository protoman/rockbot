#include "bossintro.h"
#include "pal_safe.h"
#include "sprite_guard.h"

#include "music.h"

#include "resources.h"
#include "stage_data.h"
#include "stage.h"
#include "ssel_data.h"   // ssel_boss_names[stage]
#include "boss_intro_bg_tiles.h"
#include "boss_intro_bg_map.h"
#include "castle64_map.h"
#include "castle64_tiles.h"
#include "castle64_pal.h"

#include <string.h>

#define BOSS_INTRO_PAL       PAL1
#define BOSS_INTRO_TILE_BASE TILE_USER_INDEX
#define BOSS_INTRO_FRAMES    384      // 6.4s @60fps (PC wait_and_update_screen)
#define CASTLE_FRAMES        390      // PC classic_style_castle_intro: 6500ms

// PC draws the boss at y = BOSS_INTRO_BG_POS_Y - h/2 = 96 - h/2 (240px),
// name centred at y=200; both shifted up 16 for the MD 224 crop.
#define BOSS_INTRO_BOSS_Y    80
#define BOSS_INTRO_NAME_ROW  23

// PC stage_select::classic_style_castle_intro (stage_select.cpp:600), shown
// before a castle #1 stage. Full-screen art through the four-line pipeline
// (tools/title64.py), same drawing path as the title.
static void castle_screen(void)
{
    VDP_loadTileData(castle64_tiles_data, TILE_USER_INDEX,
                     CASTLE64_TILE_COUNT, DMA);
    for (u8 L = 0; L < 4; L++)
        pal_load((u16)(PAL0 + L), castle64_pal[L]);
    VDP_setBackgroundColor(0);
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 64, 32);
    static u16 cells[CASTLE64_W * CASTLE64_H];
    for (u16 i = 0; i < CASTLE64_W * CASTLE64_H; i++) {
        u16 e = castle64_map[i];
        cells[i] = (u16)((e & ~TILE_INDEX_MASK) |
                         (TILE_USER_INDEX + (e & TILE_INDEX_MASK)));
    }
    VDP_setTileMapDataRect(BG_A, cells, 0, 0, CASTLE64_W, CASTLE64_H,
                           CASTLE64_W, CPU);
    music_play(MUSIC_CASTLE_INTRO);
    for (u16 f = 0; f < CASTLE_FRAMES; f++) {
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

void boss_intro_run(u8 stage)
{
    sprites_end();
    sprites_begin(768);
    stage_release_map();

    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_clearPlane(BG_B, TRUE);

    if (stage >= 9) castle_screen();

    music_play(MUSIC_BOSS_INTRO);
    VDP_loadTileData(boss_intro_bg_tiles_data, BOSS_INTRO_TILE_BASE,
                     BOSS_INTRO_BG_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(BOSS_INTRO_PAL, boss_intro_bg_pal.data, DMA_QUEUE);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);

    static u16 cells[BOSS_INTRO_BG_MAP_W * BOSS_INTRO_BG_MAP_H];
    for (u16 i = 0; i < BOSS_INTRO_BG_MAP_W * BOSS_INTRO_BG_MAP_H; i++) {
        cells[i] = TILE_ATTR_FULL(BOSS_INTRO_PAL, FALSE, FALSE, FALSE,
                                  BOSS_INTRO_TILE_BASE + boss_intro_bg_map[i]);
    }
    VDP_setTileMapDataRect(BG_A, cells, 0, 0, BOSS_INTRO_BG_MAP_W,
                           BOSS_INTRO_BG_MAP_H, BOSS_INTRO_BG_MAP_W, CPU);

    // The stage's boss def carries the sprite sheet (STAND row, frame 0).
    const EnemyDef *boss = NULL;
    for (u8 i = 0; i < stage_def->enemy_def_count; i++) {
        if (stage_def->enemy_defs[i].is_boss) {
            boss = &stage_def->enemy_defs[i];
            break;
        }
    }
    Sprite *spr = NULL;
    if (boss && boss->spr) {
        spr = SPR_addSprite(boss->spr,
                            160 - boss->spr->w / 2,
                            BOSS_INTRO_BOSS_Y - boss->spr->h / 2,
                            TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (spr) spr_set_anim(spr, 0);
    }

    // Boss name, centred (PC draw_centered_text(200, botname)). The text
    // palette is GLOBAL state: the stage select borrows PAL1 and the dialogs
    // PAL3, so a screen that forgot to restore left this name drawn with the
    // boss-intro BACKGROUND palette - a solid bar of glyph cells (black ink on
    // black, or inverted). Force PAL0 (pal_ui: 0 = black, 15 = font white).
    VDP_setTextPlane(BG_A);
    VDP_setTextPalette(PAL0);
    if (stage < SSEL_STAGE_SLOTS && ssel_boss_names[stage][0]) {
        const char *n = ssel_boss_names[stage];
        u8 len = strlen(n);
        VDP_drawText(n, (len < 40) ? (40 - len) / 2 : 0, BOSS_INTRO_NAME_ROW);
    }

    // Flush the queued DMA NOW so the first displayed frame already has its
    // palette and background tiles.
    SYS_doVBlankProcess();
    VDP_setReg(17, 0);
    VDP_setReg(18, 0);

    // PC `graphicsLib::wait_and_update_screen(6400)` reads NO input: the boss
    // intro is a fixed 6.4 s hold and cannot be skipped. The port used to skip
    // on any pad button, which also made it disappear instantly when a button
    // from the stage select was still held (BUG-015).
    for (u16 f = 0; f < BOSS_INTRO_FRAMES; f++) {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }

    if (spr) SPR_releaseSprite(spr);
    sprites_end();   // the caller (runStage) re-inits the sprite engine
}
