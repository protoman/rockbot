#include "transition.h"
#include "menu.h"
#include "pal_safe.h"
#include "menu_armor_data.h"
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
static u16 owned;            // owned weapons, bit n (PC has_weapon)

// PC find_next_weapon (class_config.cpp:419): the next owned weapon in `dir`,
// wrapping. 0..10 = buster, the eight bosses, COIL and JET. The PC moves four
// grid cells vertically; the port uses the same 2x6 layout but LEFT/RIGHT
// walk the linear weapon list (the grid is displayed for reference).
static u8 find_next_weapon(u8 cur, s8 dir)
{
    // PC class_config.cpp:419 verbatim: start at cur+move and scan LINEARLY
    // (by one, not by `move`) to the end wrapping to the start, and the other
    // way around for a negative move. The 4-column grid is only how the icons
    // are DRAWN. The port used to keep the search inside the cursor's row or
    // column, so an owned weapon whose grid neighbours were not owned could
    // never be selected - with only the buster (0) and the MAGE weapon (5), the
    // cursor stayed on the buster (report 2026-10-24).
    const s16 n = WEAPON_SWITCH_N;
    if (dir > 0) {
        for (s16 i = (s16)cur + dir; i < n; i++)
            if (owned & (1u << i)) return (u8)i;
        for (s16 i = 0; i < (s16)cur; i++)
            if (owned & (1u << i)) return (u8)i;
    } else {
        for (s16 i = (s16)cur + dir; i >= 0; i--)
            if (owned & (1u << i)) return (u8)i;
        for (s16 i = (s16)(n - 1); i > (s16)cur; i--)
            if (owned & (1u << i)) return (u8)i;
    }
    return cur;          // PC returns -1: the caller keeps the weapon
}

// PC 4-column weapon grid (draw_game_menu_weapon_bar: x = 25+col*76,
// y = 60+row*20; icon at x-10, y+1). Icons are the PC 8x8 icons_small art,
// converted to one shared 15-colour sprite palette.
static Sprite *wicon[WEAPON_SWITCH_N];
static Sprite *wicon_gray[WEAPON_SWITCH_N];
static Sprite *selected_icon;
// Cursor position: a weapon index (0..WEAPON_SWITCH_N-1) or one of the three
// reserve tanks appended after them. The PC puts the tanks in an extra row of
// the weapon grid (class_config.cpp:174/201/218): the cursor moves into them
// and BTN_ATTACK uses one. The port showed the counts but had no way to use a
// tank at all (player_use_tank had no caller).
// Entry WEAPON_SWITCH_N is an empty grid cell, so the three tanks start the
// last grid row (4 columns: entries 12..14).
#define MENU_ENTRY_ETANK (WEAPON_SWITCH_N + 1)
#define MENU_ENTRY_WTANK (WEAPON_SWITCH_N + 2)
#define MENU_ENTRY_STANK (WEAPON_SWITCH_N + 3)
#define MENU_ENTRY_N     16                     // 4x4 grid; cell 15 is empty
static u8 menu_entry;

// Grid: entry e sits at row e/4, col e%4 (weapons 0..10, then the three tanks).
// Rows are two tile rows apart and columns eight tiles apart: icon (8 px) +
// 6-tile bar + a small gap.
#define GRID_X0      38                       // icon x of column 0 (px)
#define GRID_DX      64
#define GRID_BAR_DX  10                       // bar starts this far right of the icon
#define GRID_TILE_Y0 6                        // tile row of grid row 0
#define GRID_ROWS    4
static Sprite *tank_icon[3];
static Sprite *tank_gray[3];

static inline s16 grid_icon_x(u8 e) { return (s16)(GRID_X0 + (e % 4) * GRID_DX); }
static inline u16 grid_tile_row(u8 e) { return (u16)(GRID_TILE_Y0 + (e / 4) * 2); }
static inline s16 grid_icon_y(u8 e) { return (s16)(grid_tile_row(e) * 8 + 1); }
static inline u16 grid_bar_col(u8 e) { return (u16)((GRID_X0 + GRID_BAR_DX) / 8 + (e % 4) * (GRID_DX / 8)); }

static void menu_restore_strip(u16 col, u16 row, u16 count);
static void tank_icons_select(void);

static u8 tank_count(u8 entry)
{
    if (entry == MENU_ENTRY_ETANK) return player.tanks_hp;
    if (entry == MENU_ENTRY_WTANK) return player.tanks_wp;
    return player.tanks_sp;
}

static bool entry_selectable(u8 entry)
{
    if (entry < WEAPON_SWITCH_N) return (owned & (1u << entry)) != 0;
    if (entry < MENU_ENTRY_ETANK || entry > MENU_ENTRY_STANK) return FALSE;   // empty cells
    return tank_count(entry) > 0;
}

// Move `dir` entries (1 for LEFT/RIGHT, 4 for UP/DOWN), skipping everything
// that cannot be selected, wrapping like the PC's linear find_next_weapon.
static u8 entry_step(u8 cur, s8 dir)
{
    u8 e = cur;
    for (u8 i = 0; i < MENU_ENTRY_N; i++) {
        e = (u8)(((s16)e + dir + MENU_ENTRY_N) % MENU_ENTRY_N);
        if (entry_selectable(e)) return e;
    }
    return cur;
}

// "ITEM:  E-TANK [2]   B:USE" line while the cursor is on a tank.
static void draw_tank_entry(void)
{
    static const char *const names[3] = { "E-TANK", "W-TANK", "S-TANK" };
    const u8 t = (u8)(menu_entry - MENU_ENTRY_ETANK);
    char buf[24];
    menu_restore_strip(13, 19, 27);
    menu_restore_strip(13, 20, 27);
    VDP_drawText("ITEM:", 13, 19);
    const u8 n = tank_count(menu_entry);
    buf[0] = names[t][0];
    buf[1] = names[t][1];
    buf[2] = names[t][2];
    buf[3] = names[t][3];
    buf[4] = names[t][4];
    buf[5] = names[t][5];
    buf[6] = ' ';
    buf[7] = '[';
    buf[8] = (char)('0' + (n % 10));
    buf[9] = ']';
    buf[10] = ' ';
    buf[11] = 'B';
    buf[12] = ':';
    buf[13] = 'U';
    buf[14] = 'S';
    buf[15] = 'E';
    buf[16] = 0;
    VDP_drawText(buf, 13, 20);
    if (selected_icon) {
        SPR_setVisibility(selected_icon, VISIBLE);
        spr_set_frame(selected_icon, 0);
    }
    tank_icons_select();
}

static void icons_release(void)
{
    for (u8 w = 0; w < WEAPON_SWITCH_N; w++) {
        if (wicon[w]) { SPR_releaseSprite(wicon[w]); wicon[w] = NULL; }
        if (wicon_gray[w]) { SPR_releaseSprite(wicon_gray[w]); wicon_gray[w] = NULL; }
    }
    for (u8 t = 0; t < 3; t++) {
        if (tank_icon[t]) { SPR_releaseSprite(tank_icon[t]); tank_icon[t] = NULL; }
        if (tank_gray[t]) { SPR_releaseSprite(tank_gray[t]); tank_gray[t] = NULL; }
    }
    if (selected_icon) {
        SPR_releaseSprite(selected_icon);
        selected_icon = NULL;
    }
}

// Tank count next to its icon, drawn in the bar slot ("x2").
static void draw_tank_counts(void)
{
    for (u8 t = 0; t < 3; t++) {
        const u8 e = (u8)(MENU_ENTRY_ETANK + t);
        char buf[4] = { 'x', (char)('0' + (tank_count(e) % 10)), 0, 0 };
        menu_restore_strip(grid_bar_col(e), grid_tile_row(e), 2);
        VDP_drawText(buf, grid_bar_col(e), grid_tile_row(e));
    }
}

// The tank under the cursor is coloured, the others grey (like the weapons).
static void tank_icons_select(void)
{
    for (u8 t = 0; t < 3; t++) {
        const bool sel = (menu_entry == MENU_ENTRY_ETANK + t);
        if (tank_icon[t]) SPR_setVisibility(tank_icon[t], sel ? VISIBLE : HIDDEN);
        if (tank_gray[t]) SPR_setVisibility(tank_gray[t], sel ? HIDDEN : VISIBLE);
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
        const s16 x = grid_icon_x(w);
        const s16 y = grid_icon_y(w);
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
    for (u8 t = 0; t < 3; t++) {
        const u8 e = (u8)(MENU_ENTRY_ETANK + t);
        tank_icon[t] = SPR_addSprite(&menu_tank_sprite, grid_icon_x(e), grid_icon_y(e),
                                     TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
        tank_gray[t] = SPR_addSprite(&menu_tank_gray_sprite, grid_icon_x(e), grid_icon_y(e),
                                     TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (tank_icon[t]) { SPR_setAutoAnimation(tank_icon[t], FALSE); spr_set_frame(tank_icon[t], t); }
        if (tank_gray[t]) { SPR_setAutoAnimation(tank_gray[t], FALSE); spr_set_frame(tank_gray[t], t); }
    }
    tank_icons_select();
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
        const u16 x = grid_bar_col(w);
        const u16 y = grid_tile_row(w);
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
    tank_icons_select();
    if (w != 0) {
        u8 a = player_weapon_ammo();
        char buf[4] = { '[', ' ', ' ', 0 };
        buf[2] = (char)('0' + (a % 10));
        if (a >= 10) buf[1] = (char)('0' + ((a / 10) % 10));
        VDP_drawText(buf, 31, 20);
    }
}

// PC weapon_menu_show_player (draw.cpp:1021-1040): the installed armor pieces
// are drawn as icons at the panel's right edge (x = RES_W-36, y = 157). The PC
// art is 4x4 tiles (28x29 padded), so it fits the dialog-panel tile block,
// which is unused while the menu is open.
static void draw_armor_icons(void)
{
    const u8 p = player_number() & 3;
    const u8 mask = (u8)(player.armor & 7);
    VDP_loadTileData(menu_armor_tiles[p][mask], SPR_DLG_TILE_BASE,
                     MENU_ARMOR_TILES, DMA_QUEUE);
    for (u8 r = 0; r < 4; r++) {
        for (u8 c = 0; c < 4; c++) {
            VDP_setTileMapXY(BG_A,
                             TILE_ATTR_FULL(PLAYER_PAL, FALSE, FALSE, FALSE,
                                            SPR_DLG_TILE_BASE + r * 4 + c),
                             (u16)(35 + c), (u16)(19 + r));
        }
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

// Draw (or redraw) the PC weapon panel + its texts on BG_A. The tiles and
// palette must already be loaded. Called again after the options screen,
// which uses the plane for its own text.
static void menu_draw_panel(void)
{
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    u16 *cells = screen_cells(MENU_MAP_W * MENU_MAP_H);
    if (cells) {
        for (u16 i = 0; i < MENU_MAP_W * MENU_MAP_H; i++) {
            cells[i] = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE,
                                      MENU_TILE_BASE + menu_map[i]);
        }
        // CPU transfer: a queued DMA would flush at the next vblank and wipe
        // the texts drawn right after it.
        VDP_setTileMapDataRect(BG_A, cells, 0, 0, MENU_MAP_W, MENU_MAP_H,
                               MENU_MAP_W, CPU);
    }
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
    draw_tank_counts();
    draw_armor_icons();
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
    pal_load(PAL0, menu_pal0);
    pal_load(PAL1, menu_pal.data);
    pal_load(PAL3, menu_icons_pal.data);
    VDP_setEnable(TRUE);

    // The font has transparent background pixels. BG_A holds the panel, but
    // its transparent pixels would otherwise reveal the stage parallax on
    // BG_B as coloured stripes behind every glyph. Blank BG_B and point the
    // backdrop at black while the menu is open; stage_redraw_bg() rebuilds
    // the parallax on close.
    VDP_setBackgroundColor(0);
    PAL_setColor(0, 0x0000);
    VDP_clearPlane(BG_B, TRUE);

    menu_entry = player_weapon();
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

    menu_entry = player_weapon();
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
        if ((pad & BUTTON_B) && menu_entry >= MENU_ENTRY_ETANK) {
            // PC class_config.cpp:218 use_tank(): BTN_ATTACK on a tank slot.
            if (player_use_tank(menu_entry == MENU_ENTRY_ETANK)) {
                sfx_play(SFX_PILL);   // PC SFX_GOT_ENERGY (closest PCM here)
                draw_tank_entry();
                draw_tank_counts();
            }
            continue;
        }
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
        s8 move = 0;
        if (pressed & BUTTON_RIGHT)      move = 1;
        else if (pressed & BUTTON_LEFT)  move = -1;
        else if (pressed & BUTTON_UP)    move = -4;   // PC matrix: one row
        else if (pressed & BUTTON_DOWN)  move = 4;
        if (move) {
            const u8 e = entry_step(menu_entry, move);
            if (e != menu_entry) {
                menu_entry = e;
                if (e < WEAPON_SWITCH_N) {
                    player_set_weapon(e);
                    tooltip_show(e);   // PC set_weapon(..., show_tooltip)
                    draw_weapon_line();
                } else {
                    draw_tank_entry();
                }
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
    pal_load(PAL0, old_pal0);
    pal_load(PAL1, old_pal1);
    pal_load(PAL3, old_pal3);

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
