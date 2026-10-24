#include "menu.h"
#include "sprite_guard.h"
#include "sfx.h"

#include "player.h"
#include "stage.h"
#include "tooltip.h"
#include "hud.h"
#include "enemy.h"
#include "shot.h"
#include "eshot.h"
#include "object.h"
#include "resources.h"
#include "menu_map.h"
#include "menu_tiles.h"
#include "weapon_data.h"
#include "config.h"

// PC class_config::execute_ingame_menu (class_config.cpp:290) with the
// panel from draw::draw_in_game_menu (draw.cpp:855): full-screen
// weapon_menu.png, "GAME MENU" at (20,22), "CONFIG [R]" right aligned,
// weapon grid at x=25+col*76 / y=60+row*20, portrait at (6,157).
// The MD draws the PC panel from its decomposed tiles (tools/img2tiles.py:
// 41 unique pieces instead of a 320x240 bitmap) and overlays the texts.
// PC 320x240 coordinates are shifted by the usual 16px top crop.
//
// The panel tiles load over the parallax block (stage_bg_tile_base);
// stage_redraw_bg() restores the parallax art on close. VRAM has no free
// block left: tiles 0..1439 = stage 16..143, bg 144..911, anim 912..936,
// sprite engine 936..1439, font 1440..1535 (TILE_FONT_INDEX is 1440, not
// 1952 as older comments claimed).
#define MENU_TILE_BASE stage_bg_tile_base
#define MENU_BAR_TILE  (MENU_TILE_BASE + MENU_TILE_COUNT)
#define MENU_BAR_TILES 11
#define MENU_GRAY_BAR_TILE (MENU_BAR_TILE + MENU_BAR_TILES)

static Sprite *portrait;
static u16 cells[MENU_MAP_W * MENU_MAP_H];
static u16 owned;            // owned weapons, bit n (PC has_weapon)

// PC find_next_weapon (class_config.cpp:419): the next owned weapon in `dir`,
// wrapping. 0..10 = buster, the eight bosses, COIL and JET. The PC moves four
// grid cells vertically; the port uses the same 2x6 layout but LEFT/RIGHT
// walk the linear weapon list (the grid is displayed for reference).
static u8 find_next_weapon(u8 cur, s8 dir)
{
    // PC class_config.cpp:419: walk the LINEAR weapon list `cur+move` upward
    // (or downward and wrapping) and stop at the first owned slot. The
    // 4-column grid is only how the icons are DRAWN - the port used to keep
    // the search inside the current row/column, so an owned weapon whose
    // neighbours were not owned could never be selected: with only buster (0)
    // and the MAGE weapon (5) the cursor never left the buster (report
    // 2026-10-24). UP/DOWN pass +-4, LEFT/RIGHT +-1, as on the PC.
    for (u8 i = 1; i <= WEAPON_SWITCH_N; i++) {
        s16 w = (s16)((s16)cur + (s16)dir * (s16)i);
        while (w >= WEAPON_SWITCH_N) w = (s16)(w - WEAPON_SWITCH_N);
        while (w < 0) w = (s16)(w + WEAPON_SWITCH_N);
        if (owned & (1u << w)) return (u8)w;
    }
    return cur;
}

// PC 4-column weapon grid (draw_game_menu_weapon_bar: x = 25+col*76,
// y = 60+row*20; icon at x-10, y+1). Icons are the PC 8x8 icons_small art,
// converted to one shared 15-colour sprite palette.
static Sprite *wicon[WEAPON_SWITCH_N];
static Sprite *wicon_gray[WEAPON_SWITCH_N];
static Sprite *selected_icon;

static void icons_release(void)
{
    for (u8 w = 0; w < WEAPON_SWITCH_N; w++) {
        if (wicon[w]) { SPR_releaseSprite(wicon[w]); wicon[w] = NULL; }
        if (wicon_gray[w]) { SPR_releaseSprite(wicon_gray[w]); wicon_gray[w] = NULL; }
    }
    if (selected_icon) {
        SPR_releaseSprite(selected_icon);
        selected_icon = NULL;
    }
}

static void menu_icons_select(void)
{
    for (u8 w = 0; w < WEAPON_SWITCH_N; w++) {
        bool sel = (w == player_weapon());
        if (wicon[w]) SPR_setVisibility(wicon[w], sel ? VISIBLE : HIDDEN);
        if (wicon_gray[w]) SPR_setVisibility(wicon_gray[w], sel ? HIDDEN : VISIBLE);
    }
}

static void icons_create(void)
{
    icons_release();
    PAL_setPalette(PAL3, menu_icons_pal.data, DMA_QUEUE);
    for (u8 w = 0; w < WEAPON_SWITCH_N; w++) {
        if (!(owned & (1u << w))) continue;
        u8 row = (u8)(w / 4);
        u8 col = (u8)(w % 4);
        // Keep the icons on the same 80x24 pixel grid as the BG tile bars:
        // the panel draws the bars in 8px tiles, so a PC-style 76x20 grid
        // drifts 4px per column (bar ends up far right of its icon) and the
        // 20px rows cannot line up with 8px tile rows either.
        s16 x = (s16)(14 + col * 80);
        s16 y = (s16)(41 + row * 24);
        Sprite *sp = SPR_addSprite(&menu_icon_sprite, x, y,
                                   TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
        Sprite *gr = SPR_addSprite(&menu_icon_gray_sprite, x, y,
                                   TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!sp && !gr) break;
        if (sp) {
            SPR_setAutoAnimation(sp, FALSE);
            spr_set_frame(sp, w);
            wicon[w] = sp;
        }
        if (gr) {
            SPR_setAutoAnimation(gr, FALSE);
            spr_set_frame(gr, w);
            wicon_gray[w] = gr;
        }
    }
    menu_icons_select();
    // Selected-weapon indicator at the bottom of the panel: icon + name.
    selected_icon = SPR_addSprite(&menu_icon_sprite, 96, 152,
                                  TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
    if (selected_icon) {
        SPR_setAutoAnimation(selected_icon, FALSE);
        spr_set_frame(selected_icon, player_weapon());
    }
}

// Restore a strip of the panel before drawing text over it. VDP_drawText's
// space glyph is transparent, so "                    " does not erase the
// previous weapon/tank text (it left ghost characters behind).
static void menu_restore_strip(u16 col, u16 row, u16 count)
{
    // Solid panel colour behind every text row. The font's background pixel
    // is index 0, so a strip of the panel's flat tile keeps glyphs readable
    // even when the underlying panel has bars/patterns.
    // Tile 16 is the classic panel's flat dark fill (tile 4 is a border
    // pattern and produced the striped text backgrounds).
    const u16 e = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE,
                                 MENU_TILE_BASE + 16);
    for (u16 i = 0; i < count && col + i < MENU_MAP_W; i++)
        VDP_setTileMapXY(BG_A, e, col + i, row);
}

// One tile per weapon so each bar can use a different PAL0 index. Palette
// entries 1..11 are filled with the selected player's weapon colors; the
// font itself only uses indices 0 and 15.
static u32 menu_bar_tiles[MENU_BAR_TILES + 1][8];

static void make_bar_tiles(void)
{
    for (u8 w = 0; w < MENU_BAR_TILES; w++) {
        // Classic bar texture: alternate the weapon colour and black, like
        // the horizontal PC bar art. Index 1 is black and is also the font's
        // background pixel; slots 2..12 hold the weapon colours.
        u32 row = 0;
        for (u8 x = 0; x < 8; x++) {
            const u8 idx = (x & 1) ? (u8)(w + 2) : 1;
            row |= (u32)idx << (28 - x * 4);
        }
        for (u8 y = 0; y < 8; y++) menu_bar_tiles[w][y] = row;
    }
    // Last tile: grey unselected bars use PAL0 index 13.
    u32 grey = 0;
    for (u8 x = 0; x < 8; x++)
        grey |= (u32)((x & 1) ? 13 : 1) << (28 - x * 4);
    for (u8 y = 0; y < 8; y++) menu_bar_tiles[MENU_BAR_TILES][y] = grey;
}

// The old ASCII gauge used the font palette and disappeared against the
// panel. Draw a plain white tile strip instead; it is intentionally simple
// until the PC menu art is matched exactly.
static void draw_weapon_bar(u8 value, u8 max, u16 col, u16 row,
                           u8 n, u8 weapon, bool selected)
{
    if (max == 0) max = 1;
    u8 filled = (u8)(((u16)value * n) / max);
    if (filled > n) filled = n;
    menu_restore_strip(col, row, n);
    const s16 tile = selected
        ? (s16)(MENU_BAR_TILE + ((weapon < MENU_BAR_TILES) ? weapon : 0))
        : (s16)MENU_GRAY_BAR_TILE;
    for (u8 i = 0; i < filled; i++) {
        VDP_setTileMapXY(BG_A,
                         TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, tile),
                         (u16)(col + i), row);
    }
}

// Bars belong under the PC grid icons (x=25+col*76, y=60+row*20), not on the
// selected-weapon text line.
static void draw_grid_bars(void)
{
    for (u8 w = 0; w < WEAPON_SWITCH_N; w++) {
        if (!(owned & (1u << w))) continue;
        u8 row = (u8)(w / 4);
        u8 col = (u8)(w % 4);
        u16 x = (u16)(3 + col * 10);
        // PC bar y=60+row*20; after the 16px crop it shares the icon line
        // (icon y=41+row*24). Tile-row granularity: rows 5/8/11 keep the
        // grid evenly spaced around the PC 44/64/84 positions.
        static const u8 bar_row[3] = { 5, 8, 11 };
        u16 y = bar_row[row];
        u8 value = (w == 0) ? PLAYER_INITIAL_HP : player.ammo[w];
        draw_weapon_bar(value, PLAYER_INITIAL_HP, x, y, 6, w,
                        w == player_weapon());
    }
}

// Selected weapon name, at the PC lower-left position (draw.cpp:984).
static void draw_weapon_line(void)
{
    u8 w = player_weapon();
    menu_restore_strip(13, 19, 27);
    menu_restore_strip(13, 20, 27);
    VDP_drawText("WEAPON/ITEM:", 13, 19);
    VDP_drawText(weapon_data[w].name, 13, 20);
    if (selected_icon) {
        spr_set_frame(selected_icon, w);
        SPR_setVisibility(selected_icon, VISIBLE);
    }
    menu_icons_select();
    if (w != 0) {
        u8 a = player_weapon_ammo();
        char buf[4] = { '[', ' ', ' ', 0 };
        buf[2] = (char)('0' + (a % 10));
        if (a >= 10) buf[1] = (char)('0' + ((a / 10) % 10));
        VDP_drawText(buf, 31, 20);
    }
}

// Stored tanks (OBJ_ENERGY_TANK / OBJ_WEAPON_TANK), PC game_save.items.
static void draw_tank_line(void)
{
    menu_restore_strip(5, 22, 35);
    char buf[40] = "ITEMS  E-TANK:0  W-TANK:0  S-TANK:0";
    buf[14] = (char)('0' + player.tanks_hp);
    buf[24] = (char)('0' + player.tanks_wp);
    buf[34] = (char)('0' + player.tanks_sp);
    VDP_drawText(buf, 5, 22);
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

// Draw (or redraw) the PC weapon panel + its texts on BG_A. The tiles and
// palette must already be loaded. Called again after the options screen,
// which uses the plane for its own text.
static void menu_draw_panel(void)
{
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
    VDP_setTextPalette(PAL0);

    // texts PC draws over the panel (menu_map.h names from img2tiles.py)
    menu_restore_strip(0, 0, 40);
    VDP_drawText("GAME MENU", 2, 0);
    VDP_drawText("CONFIG [B]", 28, 0);
    draw_weapon_line();
    icons_create();
    draw_grid_bars();
    menu_restore_strip(2, 25, 20);
    VDP_drawText("B: CONFIG", 2, 25);
    menu_restore_strip(2, 26, 20);
    VDP_drawText("A: LEAVE STAGE", 2, 26);
    draw_tank_line();
    // PC weapon_menu_show_player (draw.cpp:1021): lives next to the portrait
    {
        char buf[6] = { 'x', '[', '0', ']', 0, 0 };
        buf[2] = (char)('0' + (player_lives() % 10));
        menu_restore_strip(5, 20, 5);
        VDP_drawText(buf, 5, 20);
    }
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
    make_bar_tiles();
    for (u8 w = 0; w < MENU_BAR_TILES + 1; w++)
        VDP_loadTileData(menu_bar_tiles[w], (u16)(MENU_BAR_TILE + w), 1,
                         DMA_QUEUE);
    // Build one PAL0 palette with the UI colours and the selected player's
    // weapon colours in entries 1..11. The text uses PAL0: the font glyph is
    // colour 15 (light) and its background pixel is 0 (transparent).
    static u16 menu_pal0[16];
    for (u8 i = 0; i < 16; i++) menu_pal0[i] = pal_ui.data[i];
    menu_pal0[0] = menu_pal.data[3];
    for (u8 w = 0; w < MENU_BAR_TILES; w++)
        menu_pal0[2 + w] = player_weapon_color(w);
    menu_pal0[13] = pal_ui.data[8];       // grey for unselected icons/bars
    // Write CRAM with the display disabled: the queued variants were racing
    // with the panel/tile DMA and left the font palette at the stage colours.
    // PAL0 drives the UI/font and PAL3 the icons; both also belong to the
    // stage (enemy/object and parallax palettes), so keep the full lines.
    u16 old_pal0[16], old_pal1[16], old_pal3[16];
    for (u8 i = 0; i < 16; i++) {
        old_pal0[i] = PAL_getColor(i);
        old_pal1[i] = PAL_getColor((PAL1 << 4) + i);
        old_pal3[i] = PAL_getColor((PAL3 << 4) + i);
    }
    const u8 old_bg_index = VDP_getBackgroundColor();
    VDP_setEnable(FALSE);
    PAL_setPalette(PAL0, menu_pal0, CPU);
    PAL_setPalette(PAL1, menu_pal.data, CPU);
    PAL_setPalette(PAL3, menu_icons_pal.data, CPU);
    VDP_setEnable(TRUE);

    // The font has transparent background pixels. BG_A holds the panel, but
    // its transparent pixels would otherwise reveal the stage parallax on
    // BG_B as coloured stripes behind every glyph. Blank BG_B and point the
    // backdrop at black while the menu is open; stage_redraw_bg() rebuilds
    // the parallax on close.
    VDP_setBackgroundColor(0);
    PAL_setColor(0, 0x0000);
    VDP_clearPlane(BG_B, TRUE);

    stage_sprites_visible(FALSE);
    SYS_doVBlankProcess();     // apply PAL0/PAL1 before drawing text
    menu_draw_panel();

    portrait = SPR_addSprite(player_definition(), 6, 141,
                             TILE_ATTR(PLAYER_PAL, FALSE, FALSE, FALSE));
    if (portrait) spr_set_anim(portrait, ANIM_ATTACK);

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
        if (pad & BUTTON_B) {                         // options (PC BTN_R)
            if (portrait) {
                SPR_releaseSprite(portrait);
                portrait = NULL;
            }
            bool cfg_leave = config_run(true);
            if (cfg_leave) { leave = TRUE; break; }
            portrait = SPR_addSprite(player_definition(), 6, 141,
                                     TILE_ATTR(PLAYER_PAL, FALSE, FALSE, FALSE));
            if (portrait) spr_set_anim(portrait, ANIM_ATTACK);
            menu_draw_panel();
            continue;
        }
        // weapon switching (PC find_next_weapon / the weapon grid)
        u16 pressed = pad & ~prev;
        prev = pad;
        if (pressed & BUTTON_RIGHT) {
            u8 w = find_next_weapon(player_weapon(), 1);
            if (w != player_weapon()) {
                player_set_weapon(w);
                tooltip_show(w);       // PC set_weapon(..., show_tooltip)
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        } else if (pressed & BUTTON_LEFT) {
            u8 w = find_next_weapon(player_weapon(), -1);
            if (w != player_weapon()) {
                player_set_weapon(w);
                tooltip_show(w);       // PC set_weapon(..., show_tooltip)
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        } else if (pressed & BUTTON_UP) {
            // PC matrix navigation: up/down move one row (4 columns).
            u8 w = find_next_weapon(player_weapon(), -4);
            if (w != player_weapon()) {
                player_set_weapon(w);
                tooltip_show(w);       // PC set_weapon(..., show_tooltip)
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        } else if (pressed & BUTTON_DOWN) {
            u8 w = find_next_weapon(player_weapon(), 4);
            if (w != player_weapon()) {
                player_set_weapon(w);
                tooltip_show(w);       // PC set_weapon(..., show_tooltip)
                draw_weapon_line();
                sfx_play(SFX_CURSOR);
            }
        }
    }

    if (portrait) {
        SPR_releaseSprite(portrait);
        portrait = NULL;
    }
    icons_release();

    VDP_setBackgroundColor(old_bg_index);
    PAL_setPalette(PAL0, old_pal0, CPU);
    PAL_setPalette(PAL1, old_pal1, CPU);
    PAL_setPalette(PAL3, old_pal3, CPU);

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
    stage_redraw_anim();    // ...and possibly the animated-tile block
    stage_force_redraw();
    return FALSE;
}
