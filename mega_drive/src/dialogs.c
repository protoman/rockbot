#include "dialogs.h"

#include <genesis.h>

#include "dialogs_data.h"
#include "stage.h"
#include "player.h"
#include "enemy.h"
#include "hud.h"
#include "config.h"
#include "resources.h"

// PC boss-room dialog (scenes/dialogs.cpp:249, called by
// game::map_present_boss when the boss appears in the arena). The PC also
// draws a dialog box image + the speaker's face; the port keeps the PC text
// (verbatim) in a black box on BG_A and restores the map afterwards.
//
// The stage-start half of the dialog files (lines 0..29) is dead data in the
// PC build: no code path reads it, and every stage's cutscene_pre/pos is -1.
// Only the boss block (30..59) is displayed here.

#define DLG_BLACK_TILE 15       // system blank tile (935 is inside the anim slots)
#define DLG_W 40
#define DLG_H 6

static const u32 dlg_black[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
};

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

// One 3-line page. The box is drawn at screen rows y0..y0+5 by writing plane
// cells with the camera's tile offset; per-cell writes also wrap at the
// 64-cell plane width.
static void show_page(const char *const *lines, u8 first, u8 y0,
                      const SpriteDefinition *face, const Palette *face_pal)
{
    const u16 base = (u16)((camera.x >> 3) & 63);
    const u16 entry = TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE,
                                     DLG_BLACK_TILE);
    for (u8 row = 0; row < DLG_H; row++)
        for (u8 col = 0; col < DLG_W; col++)
            VDP_setTileMapXY(BG_A, entry, (u16)((base + col) & 63),
                             (u16)(y0 + row));

    // Face on the left of the box (PC draw_dialog_bg: face at x+16, text at
    // x+52 -> the port uses 32px face + 1 tile gap, text from column 6).
    Sprite *fs = NULL;
    if (face) {
        PAL_setPalette(PAL2, face_pal->data, DMA_QUEUE);
        fs = SPR_addSprite(face, 8, (s16)(y0 * 8 + 8),
                           TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    }

    for (u8 i = 0; i < 3; i++) {
        const char *line = lines[first + i];
        if (!line[0]) continue;
        for (u8 c = 0; line[c]; c++) {
            char ch[2] = { line[c], '\0' };
            VDP_drawTextBG(BG_A, ch, (u16)((base + 6 + c) & 63),
                           (u16)(y0 + 1 + i));
        }
    }
    VDP_drawTextBG(BG_A, "JUMP: NEXT", (u16)((base + 30) & 63),
                   (u16)(y0 + DLG_H - 1));
    wait_button();
    if (fs) SPR_releaseSprite(fs);
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

    show_page(lines, 0, 1, &dlg_face_armor_sprite, &dlg_face_armor_pal);
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
    hud_set_visible(FALSE);

    VDP_loadTileData(dlg_black, DLG_BLACK_TILE, 1, DMA);
    // top side (all converted stages have dialog_top_side = true)
    const u8 y0 = 1;
    const SpriteDefinition *bf = (stage < 9) ? boss_face[stage] : NULL;
    const Palette *bp = (stage < 9) ? boss_face_pal[stage] : NULL;
    const SpriteDefinition *pf = player_face[player & 3];
    const Palette *pp = pf->palette;
    show_page(d, boss0, y0, bf, bp);
    show_page(d, ply0, y0, pf, pp);
    if (d[boss0 + 3][0]) show_page(d, (u8)(boss0 + 3), y0, bf, bp);
    if (d[ply0 + 3][0]) show_page(d, (u8)(ply0 + 3), y0, pf, pp);

    player_restore_palette();  // PAL2 held the last face
    stage_force_redraw();     // the box overwrote map cells on BG_A
    player_set_visible(TRUE);
    enemies_set_visible(TRUE);
    hud_set_visible(TRUE);
    player_sync_sprite();
    SYS_doVBlankProcess();
}
