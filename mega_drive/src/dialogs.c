#include "dialogs.h"

#include <genesis.h>

#include "dialogs_data.h"
#include "stage.h"
#include "player.h"
#include "enemy.h"
#include "hud.h"
#include "object.h"
#include "config.h"
#include "resources.h"
#include "dlg_panel_tiles.h"
#include "dlg_panel_map.h"

// PC boss-room dialog (scenes/dialogs.cpp:249, called by
// game::map_present_boss when the boss appears in the arena). The PC also
// draws a dialog box image + the speaker's face; the port keeps the PC text
// (verbatim) in a black box on BG_A and restores the map afterwards.
//
// The stage-start half of the dialog files (lines 0..29) is dead data in the
// PC build: no code path reads it, and every stage's cutscene_pre/pos is -1.
// Only the boss block (30..59) is displayed here.

// PC graphicsLib::show_dialog: the 310x65 panel at x=5, y=3 (top side). The
// port draws it on BG_A with the camera's tile offset, in PAL0 with its own
// palette (see dlg_pal_save/restore below for why PAL0 and not PAL3).
// Below the sprite pool, next to the tooltip's block (src/stage.h owns the
// layout): a fixed index inside the pool overwrote live sprite tiles.
#define DLG_PANEL_TILE_BASE SPR_DLG_TILE_BASE
#define DLG_W 40
#define DLG_H 6

static const SpriteDefinition *const boss_face[9] = {
    &dlg_face_0_sprite, &dlg_face_1_sprite, &dlg_face_2_sprite,
    &dlg_face_3_sprite, &dlg_face_4_sprite, &dlg_face_5_sprite,
    &dlg_face_6_sprite, &dlg_face_7_sprite, &dlg_face_8_sprite,
};
static const Palette *const boss_face_pal[9] = {
    &dlg_face_0_pal, &dlg_face_1_pal, &dlg_face_2_pal,
    &dlg_face_3_pal, &dlg_face_4_pal, &dlg_face_5_pal,
    &dlg_face_6_pal, &dlg_face_7_pal, &dlg_face_8_pal,
};
static const SpriteDefinition *const player_face[4] = {
    &ssel_face_p1_sprite, &ssel_face_p2_sprite,
    &ssel_face_p3_sprite, &ssel_face_p4_sprite,
};
// The player's white eye sockets are palette index 0 (sprite-transparent) in
// the face art; the pupils live in this separate layer (the stage select
// draws it too). Without it the dialog portrait has empty eye sockets.
static const SpriteDefinition *const player_eyes[4] = {
    &ssel_eyes_p1_sprite, &ssel_eyes_p2_sprite,
    &ssel_eyes_p3_sprite, &ssel_eyes_p4_sprite,
};
// Eyes offset inside the 32x32 face (PC face (144,104), eyes (154,122)).
#define DLG_EYES_DX 10
#define DLG_EYES_DY 18

static void wait_button(void)
{
    u16 pad;
    // Ignore buttons already held when the dialog opens (in particular the
    // fire button used to collect the item/armor). Only a fresh JUMP press
    // advances the page, as in the PC dialog.
    do {
        SPR_update();
        SYS_doVBlankProcess();
        pad = JOY_readJoypad(JOY_1);
    } while (pad & config_jump_mask());
    do {
        SPR_update();
        SYS_doVBlankProcess();
        pad = JOY_readJoypad(JOY_1);
    } while (!(pad & config_jump_mask()));
}

// Draw the PC dialog panel (39x9 tiles) with the camera's plane offset.
// The panel/arrow/name palette: entry 0 of the source art is unused by the
// panel tiles, so it is repurposed as the panel fill colour. The font glyphs
// paint their whole 8x8 cell with palette index 0, so drawing the text in
// PAL3 gives teal text cells instead of black ones (the reported "dialog text
// background" bug). Entry 15 is the font's ink index, which the panel palette
// leaves black - force it white.
// The panel needs a whole palette line for its teal/grey shades. It used to
// take PAL3, which is the stage's PARALLAX line plus the colourcycle band, so
// the background behind the box changed colours for the length of the dialog
// (report 2026-10-24). PAL1 is the tileset and PAL2 holds the dialog faces, so
// the panel now borrows PAL0 - the line the font, enemies, objects and HUD use.
// Everything on that line is hidden for the duration of the dialog
// (dialog_show_boss / dialog_show_armor), and the line is saved and restored.
static u16 dlg_pal_saved[16];

#define DLG_CRAM 0

static void dlg_pal_save(void)
{
    for (u8 i = 0; i < 16; i++) dlg_pal_saved[i] = PAL_getColor(DLG_CRAM + i);
}

static void dlg_pal_restore(void)
{
    PAL_setColors(DLG_CRAM, dlg_pal_saved, 16, DMA_QUEUE);
}

static void dlg_palette_load(void)
{
    static u16 pal[16];
    for (u8 i = 0; i < 16; i++) pal[i] = dlg_panel_pal.data[i];
    pal[0] = dlg_panel_pal.data[2];     // panel fill (0,36,36)
    pal[15] = 0x0EEE;                  // font white
    PAL_setPalette(PAL0, pal, DMA_QUEUE);
}

static void dlg_panel_draw(u8 y0)
{
    const u16 base = (u16)((camera.x >> 3) & 63);
    const u8 top = (y0 > 0) ? (u8)(y0 - 1) : 0;
    dlg_palette_load();
    VDP_loadTileData(dlg_panel_tiles_data, DLG_PANEL_TILE_BASE,
                     DLG_PANEL_TILE_COUNT, DMA_QUEUE);
    for (u8 row = 0; row < DLG_PANEL_MAP_H; row++) {
        for (u8 col = 0; col < DLG_PANEL_MAP_W; col++) {
            u16 t = dlg_panel_map[row * DLG_PANEL_MAP_W + col];
            VDP_setTileMapXY(BG_A,
                             TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE,
                                            DLG_PANEL_TILE_BASE + t),
                             (u16)((base + col) & 63), (u16)(top + row));
        }
    }
}

// PC show_dialog_button: the A button icon inside the panel, bottom right.
// Drawn as a font glyph in the panel palette: the 12x12 art goes through
// img2tiles, which re-quantizes even an indexed source and put the arrow's
// white on index 1 - that index is PAL3's black border, so the icon rendered
// as a black box (the reported "A-button box"). "A" also matches what the PC
// icon says.
static void dlg_button_draw(u8 y0)
{
    const u16 base = (u16)((camera.x >> 3) & 63);
    const u8 top = (y0 > 0) ? (u8)(y0 - 1) : 0;
    VDP_setTextPalette(PAL0);       // white ink (15), panel-fill background (0)
    VDP_drawTextBG(BG_A, "A", (u16)((base + 36) & 63), (u16)(top + 6));
    VDP_setTextPalette(PAL0);
}

// One 3-line page. The panel is drawn at screen rows y0-1..y0+7 by writing
// plane cells with the camera's tile offset; per-cell writes also wrap at the
// 64-cell plane width.
static void show_page(const char *const *lines, u8 first, u8 y0,
                      const SpriteDefinition *face, const Palette *face_pal,
                      bool player_eyes_layer, u8 player_idx)
{
    const u16 base = (u16)((camera.x >> 3) & 63);
    dlg_panel_draw(y0);

    // Face on the left of the box (PC draw_dialog_bg: face at x+16, text at
    // x+52 -> the port uses 32px face + 1 tile gap, text from column 6).
    Sprite *fs = NULL;
    Sprite *es = NULL;
    if (face) {
        PAL_setPalette(PAL2, face_pal->data, DMA_QUEUE);
        // Eyes first: the sprite engine draws lower indices in front.
        if (player_eyes_layer) {
            es = SPR_addSprite(player_eyes[player_idx & 3],
                               8 + DLG_EYES_DX,
                               (s16)(y0 * 8 + 8 + DLG_EYES_DY),
                               TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
        }
        fs = SPR_addSprite(face, 8, (s16)(y0 * 8 + 8),
                           TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    }

    // The glyph cells carry palette index 0 as their background, so the text
    // must be written with the panel palette (PAL0): its entry 0 is the panel
    // fill, which paints the cell background in the panel colour and entry 15
    // (the ink) white. Drawn with the default PAL0 it painted black boxes.
    VDP_setTextPalette(PAL0);
    for (u8 i = 0; i < 3; i++) {
        const char *line = lines[first + i];
        if (!line[0]) continue;
        for (u8 c = 0; line[c]; c++) {
            char ch[2] = { line[c], '\0' };
            VDP_drawTextBG(BG_A, ch, (u16)((base + 6 + c) & 63),
                           (u16)(y0 + 1 + i));
        }
    }
    VDP_setTextPalette(PAL0);
    dlg_button_draw(y0);
    wait_button();
    if (fs) SPR_releaseSprite(fs);
    if (es) SPR_releaseSprite(es);
}

static const char *armor_shot_name(u8 id)
{
    switch (id) {
    case 11: return "SEMI-CHARGED";
    case 20: return "SUPER SHOT";
    case 21: return "LASER SHOT";
    case 22: return "HADOUKEN";
    default: return "SHOT";
    }
}

void dialog_show_armor(u8 kind)
{
    const u8 ability = player_armor_ability(kind);
    const char *lines[3];
    char shoot[40];

    if (kind == 0) {
        lines[0] = "IMPROVED ARMS ALLOWS YOU TO";
        lines[1] = "GIVE YOU THE ABILITY TO";
        strcpy(shoot, "SHOOT ");
        strcat(shoot, armor_shot_name(ability));
        strcat(shoot, ".");
        lines[2] = shoot;
    } else if (kind == 1) {
        lines[0] = "THIS FORTIFIED BODY WILL";
        lines[1] = "GIVE YOU THE ABILITY TO";
        if (ability == 0)      lines[2] = "BE INTANGIBLE MORE TIME";
        else if (ability == 1) lines[2] = "TAKE HALF DAMAGE";
        else if (ability == 2) lines[2] = "AVOID PUSH-BACK ON HIT";
        else                   lines[2] = "RESIST SPIKES";
    } else {
        lines[0] = "THOSE LIGHTER LEGS";
        lines[1] = "GIVE YOU THE ABILITY TO";
        if (ability == 0)      lines[2] = "DASH IN MIDDLE-AIR.";
        else if (ability == 1) lines[2] = "EXECUTE DOUBLE JUMP";
        else                   lines[2] = "SHOURUYKEN (UP+DASH)";
    }

    // The panel borrows PAL0 (font/enemies/objects/HUD): hide those users so
    // nothing on screen is left drawn with the panel's colours.
    enemies_set_visible(FALSE);
    objects_set_visible(FALSE);
    hud_set_visible(FALSE);
    dlg_pal_save();
    show_page(lines, 0, 1, &dlg_face_armor_sprite, &dlg_face_armor_pal,
              FALSE, 0);
    dlg_pal_restore();
    enemies_set_visible(TRUE);
    objects_set_visible(TRUE);
    hud_set_visible(TRUE);
    player_restore_palette();
    stage_force_redraw();
    player_sync_sprite();
    SYS_doVBlankProcess();
}

void dialog_show_boss(u8 stage, u8 player)
{
    if (stage >= DIALOG_STAGES) return;
    const char *const *d = stage_dialogs[stage];
    const u8 boss0 = 30;                  // 30..35 boss, 36+p*6 player
    const u8 ply0 = (u8)(36 + (player & 3) * 6);
    if (!d[boss0][0]) return;

    player_set_visible(FALSE);
    enemies_set_visible(FALSE);
    objects_set_visible(FALSE);   // shares PAL0 with the panel
    hud_set_visible(FALSE);

    // top side (all converted stages have dialog_top_side = true)
    const u8 y0 = 1;
    const SpriteDefinition *bf = (stage < 9) ? boss_face[stage] : NULL;
    const Palette *bp = (stage < 9) ? boss_face_pal[stage] : NULL;
    const SpriteDefinition *pf = player_face[player & 3];
    const Palette *pp = pf->palette;
    dlg_pal_save();               // the panel borrows the parallax line
    show_page(d, boss0, y0, bf, bp, FALSE, player);
    show_page(d, ply0, y0, pf, pp, TRUE, player);
    if (d[boss0 + 3][0])
        show_page(d, (u8)(boss0 + 3), y0, bf, bp, FALSE, player);
    if (d[ply0 + 3][0])
        show_page(d, (u8)(ply0 + 3), y0, pf, pp, TRUE, player);

    dlg_pal_restore();            // whole parallax line back (entries 0 and 15)
    player_restore_palette();  // PAL2 held the last face
    stage_restore_cycle();     // re-apply the colourcycle band on top
    stage_force_redraw();      // the panel overwrote map cells on BG_A
    player_set_visible(TRUE);
    enemies_set_visible(TRUE);
    objects_set_visible(TRUE);
    hud_set_visible(TRUE);
    player_sync_sprite();
    SYS_doVBlankProcess();
}
