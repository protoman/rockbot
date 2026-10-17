#include "menu.h"
#include "sfx.h"

#include "player.h"
#include "stage.h"
#include "hud.h"
#include "enemy.h"
#include "shot.h"
#include "eshot.h"
#include "object.h"
#include "resources.h"
#include "menu_map.h"
#include "menu_tiles.h"

// PC class_config::execute_ingame_menu (class_config.cpp:290) with the
// panel from draw::draw_in_game_menu (draw.cpp:855): full-screen
// weapon_menu.png, "GAME MENU" at (20,22), "CONFIG [R]" right aligned,
// weapon grid at x=25+col*76 / y=60+row*20, portrait at (6,157).
// The MD draws the PC panel from its decomposed tiles (tools/img2tiles.py:
// 41 unique pieces instead of a 320x240 bitmap) and overlays the texts.
// PC 320x240 coordinates are shifted by the usual 16px top crop.
//
// The panel tiles load over the BACKGROUND tile block (TILE_BG_INDEX);
// stage_redraw_bg() restores the parallax art on close. VRAM has no free
// block left: tiles 0..1439 = stage 16..143, bg 144..911, anim 912..936,
// sprite engine 936..1439, font 1440..1535 (TILE_FONT_INDEX is 1440, not
// 1952 as older comments claimed).
#define MENU_TILE_BASE TILE_BG_INDEX

static Sprite *portrait;
static u16 cells[MENU_MAP_W * MENU_MAP_H];

static void stage_sprites_visible(bool visible)
{
    player_set_visible(visible);
    enemies_set_visible(visible);
    shots_set_visible(visible);
    eshots_set_visible(visible);
    objects_set_visible(visible);
    hud_set_visible(visible);
}

bool menu_open(void)
{
    sfx_play(SFX_CURSOR);
    // freeze the view on the panel: both planes unscrolled, stage palette
    // line borrowed by the panel palette (restored on close)
    // tiles come from the generated 4bpp array (not a rescomp TILESET,
    // which dedups/reorders and would break the tilemap indices)
    VDP_loadTileData(menu_tiles_data, MENU_TILE_BASE, MENU_TILE_COUNT, DMA_QUEUE);
    // DMA_QUEUE: a CPU CRAM write during active display corrupts on
    // accurate emulators (see debugging.md / the SGDK boot notes)
    PAL_setPalette(PAL1, menu_pal.data, DMA_QUEUE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    for (u16 i = 0; i < MENU_MAP_W * MENU_MAP_H; i++) {
        cells[i] = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE,
                                  MENU_TILE_BASE + menu_map[i]);
    }
    // CPU transfer: a queued DMA would flush at the next vblank and wipe
    // the texts drawn right after it.
    VDP_setTileMapDataRect(BG_A, cells, 0, 0, MENU_MAP_W, MENU_MAP_H,
                           MENU_MAP_W, CPU);

    stage_sprites_visible(FALSE);

    // texts PC draws over the panel (menu_map.h names from img2tiles.py)
    VDP_drawText("GAME MENU", 2, 0);
    VDP_drawText("CONFIG [C]", 28, 0);
    VDP_drawText("BUSTER", 5, 5);          // weapon 0 slot (PC x25/y60)
    VDP_drawText(">", 3, 5);
    VDP_drawText("A: LEAVE STAGE", 2, 26);

    portrait = SPR_addSprite(player_definition(), 6, 141,
                             TILE_ATTR(PLAYER_PAL, FALSE, FALSE, FALSE));
    if (portrait) SPR_setAnim(portrait, ANIM_ATTACK);

    // wait for the buttons to be released first (PC input.clean)
    u16 pad;
    do {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
        pad = JOY_readJoypad(JOY_1);
    } while (pad & (BUTTON_START | BUTTON_A | BUTTON_C));

    bool leave = FALSE;
    while (TRUE) {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
        pad = JOY_readJoypad(JOY_1);
        if (pad & (BUTTON_START | BUTTON_C)) break;   // resume (PC START/JUMP)
        if (pad & BUTTON_A) { leave = TRUE; break; }  // PC BTN_DASH
    }

    if (portrait) {
        SPR_releaseSprite(portrait);
        portrait = NULL;
    }

    if (leave) {
        // The stage is over: do NOT restore it (a map redraw would leave the
        // MAP engine with a deferred plane update that flushes at the next
        // vblank, repainting the title's planes). Just drop the map; the
        // title sets up its own planes.
        stage_release_map();
        return TRUE;
    }

    stage_sprites_visible(TRUE);
    stage_restore_palette();
    stage_redraw_bg();      // the panel overwrote the parallax tiles
    stage_force_redraw();
    return FALSE;
}
