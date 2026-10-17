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
#include "weapon_data.h"

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
static u16 owned;            // owned weapons, bit n (PC has_weapon)

// PC find_next_weapon (class_config.cpp:419): the next owned weapon in `dir`,
// wrapping; only 0..WEAPON_PLAYER_N-1 are switchable (items are not modelled).
static u8 find_next_weapon(u8 cur, s8 dir)
{
    for (u8 i = 1; i <= WEAPON_PLAYER_N; i++) {
        s8 w = (s8)(cur + dir * (s8)i);
        if (w < 0) w += WEAPON_PLAYER_N;
        if (w >= WEAPON_PLAYER_N) w -= WEAPON_PLAYER_N;
        if (owned & (1u << (u8)w)) return (u8)w;
    }
    return cur;
}

// Weapon name + gauge, drawn over the panel's first weapon slot (PC x25/y60).
static void draw_weapon_line(void)
{
    u8 w = player_weapon();
    VDP_drawText("                    ", 5, 5);
    VDP_drawText(weapon_data[w].name, 5, 5);
    if (w != 0) {
        u8 a = player_weapon_ammo();
        char buf[4] = { ' ', ' ', ' ', 0 };
        buf[2] = (char)('0' + (a % 10));
        if (a >= 10) buf[1] = (char)('0' + ((a / 10) % 10));
        VDP_drawText(buf, 27, 5);
    }
}

static void stage_sprites_visible(bool visible)
{
    player_set_visible(visible);
    enemies_set_visible(visible);
    shots_set_visible(visible);
    eshots_set_visible(visible);
    objects_set_visible(visible);
    hud_set_visible(visible);
}

bool menu_open(u16 weapon_mask)
{
    owned = weapon_mask;
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
    VDP_drawText(">", 3, 5);
    draw_weapon_line();
    VDP_drawText("A: LEAVE STAGE", 2, 26);
    VDP_drawText("LEFT/RIGHT: WEAPON", 2, 25);

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
    u16 prev = pad;
    while (TRUE) {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
        pad = JOY_readJoypad(JOY_1);
        if (pad & (BUTTON_START | BUTTON_C)) break;   // resume (PC START/JUMP)
        if (pad & BUTTON_A) { leave = TRUE; break; }  // PC BTN_DASH
        // weapon switching (PC find_next_weapon / the weapon grid)
        u16 pressed = pad & ~prev;
        prev = pad;
        if (pressed & BUTTON_RIGHT) {
            u8 w = find_next_weapon(player_weapon(), 1);
            if (w != player_weapon()) {
                player_set_weapon(w);
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        } else if (pressed & BUTTON_LEFT) {
            u8 w = find_next_weapon(player_weapon(), -1);
            if (w != player_weapon()) {
                player_set_weapon(w);
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        }
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
