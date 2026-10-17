#include "bossintro.h"

#include "resources.h"
#include "stage_data.h"
#include "stage.h"
#include "ssel_data.h"   // ssel_boss_names[stage]
#include "boss_intro_bg_tiles.h"
#include "boss_intro_bg_map.h"

#include <string.h>

#define BOSS_INTRO_PAL       PAL1
#define BOSS_INTRO_TILE_BASE TILE_USER_INDEX
#define BOSS_INTRO_FRAMES    384      // 6.4s @60fps (PC wait_and_update_screen)

// PC draws the boss at y = BOSS_INTRO_BG_POS_Y - h/2 = 96 - h/2 (240px),
// name centred at y=200; both shifted up 16 for the MD 224 crop.
#define BOSS_INTRO_BOSS_Y    80
#define BOSS_INTRO_NAME_ROW  23

void boss_intro_run(u8 stage)
{
    SPR_end();
    SPR_initEx(504);
    stage_release_map();

    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_clearPlane(BG_B, TRUE);

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
        if (spr) SPR_setAnim(spr, 0);
    }

    // Boss name, centred (PC draw_centered_text(200, botname)).
    if (stage < 9 && ssel_boss_names[stage][0]) {
        const char *n = ssel_boss_names[stage];
        u8 len = strlen(n);
        VDP_drawText(n, (len < 40) ? (40 - len) / 2 : 0, BOSS_INTRO_NAME_ROW);
    }

    // Flush the queued DMA NOW: if the player skips on the first frame the
    // pending palette/tile transfers would otherwise land after the caller's
    // (CPU) stage palette writes and clobber them.
    SYS_doVBlankProcess();
    VDP_setReg(17, 0);
    VDP_setReg(18, 0);

    // PC waits 6400ms; allow skipping with a button.
    for (u16 f = 0; f < BOSS_INTRO_FRAMES; f++) {
        u16 pad = JOY_readJoypad(JOY_1);
        if (pad & (BUTTON_START | BUTTON_A | BUTTON_B | BUTTON_C)) break;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }

    if (spr) SPR_releaseSprite(spr);
    SPR_end();   // the caller (runStage) re-inits the sprite engine
}
