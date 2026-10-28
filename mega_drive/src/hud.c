#include "hud.h"
#include "sprite_guard.h"

#include "enemy.h"
#include "player.h"
#include "resources.h"

// Classic HUD: vertical energy bars from art/energy_bar_vertical.png. Each
// bar is three stacked sprites (24 + 24 + 8 px) and has 51 frames, one per
// percent pixel, so the gauge changes every point instead of every 8px.
#define HUD_PARTS 3
#define HUD_BAR_HP_X   1
#define HUD_BAR_WPN_X  10
#define HUD_BAR_BOSS_X (320 - 15)
#define HUD_BAR_Y      2
// PC draw.cpp:1328: the icon is drawn at y=54 with the 50px bar ending at 53,
// i.e. a 1px gap, on top of a black box the size of the icon. The icon art's
// black ring is sprite-transparent (colour 0), so the box needs its own
// sprite; the bar's frame 0 is a solid black 8x8 we can reuse.
#define HUD_ICON_Y     (HUD_BAR_Y + 52)

// The bottom slice overlaps the middle one (source rows 43..47 are
// drawn twice, identical), so the bar is 51 rows tall like the art.
static const u16 part_y[HUD_PARTS] = { 0, 24, 43 };

static Sprite *hp_parts[HUD_PARTS];
static Sprite *wpn_parts[HUD_PARTS];
static Sprite *wpn_back[HUD_PARTS];   // black backdrop, the wpn bar is cut out
static Sprite *boss_parts[HUD_PARTS];
static Sprite *hp_icon;
static Sprite *wpn_icon;
static Sprite *hp_box;
static Sprite *wpn_box;
static bool hp_on, wpn_on, boss_on;
static bool hud_on = TRUE;

static Sprite *add_part(const SpriteDefinition *def, s16 x, s16 y, u16 pal)
{
    Sprite *sp = SPR_addSprite(def, x, y, TILE_ATTR(pal, FALSE, FALSE, FALSE));
    if (sp) SPR_setDepth(sp, SPR_MIN_DEPTH);
    return sp;
}

// Frames in res/hud_bar_*_*.png (tools/convert_energy_bars.py): 51 fill steps,
// one per percent, ordered EMPTY..FULL (frame 0 = no fill, frame 50 = full).
// The old HUD_BALL_FRAMES 5 mapped 100% HP to frame 0, so the bars showed their
// empty art all the time.
#define HUD_BAR_FRAMES 51
// Lowest frame that still shows one whole segment. Frame 0 is the empty art and
// frame 1 is a sliver of the first segment, so a nearly-dead player used to see
// no gauge at all (report 2026-10-24: "HUD showed zero lines and I was still
// alive"). tools/check_hud_bars.py asserts this mapping.
#define HUD_BAR_MIN_FRAME 3

// The icon and its black box follow the bar, so no caller can show a bar
// without its icon (the weapon icon used to stay hidden until the menu forced
// hud_set_visible).
// The bars only touch the sprite engine when what they show changed: both calls
// ran every frame and cost ~16 sprite-engine calls (~50 subticks).
static s16 hp_last = -2, boss_last = -2;

static void set_bar(Sprite *const *parts, bool show, s16 pct,
                    Sprite *icon, Sprite *box)
{
    s16 frame = (pct * (HUD_BAR_FRAMES - 1)) / 100;
    if (frame < 0) frame = 0;
    if (frame > HUD_BAR_FRAMES - 1) frame = HUD_BAR_FRAMES - 1;
    if (frame > 0 && frame < HUD_BAR_MIN_FRAME) frame = HUD_BAR_MIN_FRAME;
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (!parts[i]) continue;
        SPR_setVisibility(parts[i], show ? VISIBLE : HIDDEN);
        if (show) spr_set_frame(parts[i], frame);
    }
    if (icon) SPR_setVisibility(icon, show ? VISIBLE : HIDDEN);
    if (box) SPR_setVisibility(box, show ? VISIBLE : HIDDEN);
}

void hud_init(void)
{
    hp_last = boss_last = -2;
    const SpriteDefinition *hp[3] = {
        &hud_bar_hp_top_sprite, &hud_bar_hp_mid_sprite, &hud_bar_hp_bot_sprite
    };
    const SpriteDefinition *wpn[3] = {
        &hud_bar_wpn_top_sprite, &hud_bar_wpn_mid_sprite, &hud_bar_wpn_bot_sprite
    };
    const SpriteDefinition *boss[3] = {
        &hud_bar_boss_top_sprite, &hud_bar_boss_mid_sprite, &hud_bar_boss_bot_sprite
    };
    for (u8 i = 0; i < HUD_PARTS; i++) {
        // HP bar: PAL2 2/3/4 = the selected player's own colours (player.c
        // writes them; tools/convert_player.py keeps the art off those
        // entries). It used to sit on 8/9/12, which the sheets really use -
        // BETABOT's whole black body is index 8, so the bar turned him red.
        hp_parts[i] = add_part(hp[i], HUD_BAR_HP_X, HUD_BAR_Y + part_y[i], PAL2);
        // Weapon bar: PAL2 13/14 are the sheet's weapon-colour placeholders and
        // its empty part is transparent (index 0), so it needs its own black
        // backdrop behind it (added first = furthest back).
        wpn_back[i] = SPR_addSprite(boss[i], HUD_BAR_WPN_X,
                                    HUD_BAR_Y + part_y[i],
                                    TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (wpn_back[i]) {
            spr_set_frame(wpn_back[i], 0);       // frame 0 = solid black
            SPR_setDepth(wpn_back[i], SPR_MAX_DEPTH);
        }
        wpn_parts[i] = add_part(wpn[i], HUD_BAR_WPN_X, HUD_BAR_Y + part_y[i], PAL2);
        boss_parts[i] = add_part(boss[i], HUD_BAR_BOSS_X,
                                 HUD_BAR_Y + part_y[i], PAL0);
        if (hp_parts[i]) spr_set_frame(hp_parts[i], 0);
        if (wpn_parts[i]) spr_set_frame(wpn_parts[i], 0);
        if (boss_parts[i]) spr_set_frame(boss_parts[i], 0);
    }
    // Icon box first so it sits behind the icon sprite (SPR_MIN_DEPTH is the
    // front-most depth).
    hp_box = SPR_addSprite(&hud_bar_hp_bot_sprite, HUD_BAR_HP_X, HUD_ICON_Y,
                           TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    wpn_box = SPR_addSprite(&hud_bar_hp_bot_sprite, HUD_BAR_WPN_X, HUD_ICON_Y,
                            TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    if (hp_box) {
        spr_set_frame(hp_box, 0);          // bar frame 0 = solid black 8x8
        SPR_setDepth(hp_box, SPR_MAX_DEPTH);
    }
    if (wpn_box) {
        spr_set_frame(wpn_box, 0);
        SPR_setDepth(wpn_box, SPR_MAX_DEPTH);
    }
    hp_icon = SPR_addSprite(&hud_icon_sprite, HUD_BAR_HP_X, HUD_ICON_Y,
                             TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    wpn_icon = SPR_addSprite(&hud_icon_sprite, HUD_BAR_WPN_X, HUD_ICON_Y,
                             TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (hp_icon) {
        spr_set_frame(hp_icon, 0);
        SPR_setDepth(hp_icon, SPR_MIN_DEPTH);
    }
    if (wpn_icon) {
        spr_set_frame(wpn_icon, 0);
        SPR_setDepth(wpn_icon, SPR_MIN_DEPTH);
    }
    // Nothing is shown until the first update: the bar sprites are added
    // visible with frame 0 (solid black), which used to draw black HUD boxes
    // over the stage during "MISSION STARTED".
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (hp_parts[i]) SPR_setVisibility(hp_parts[i], HIDDEN);
        if (wpn_parts[i]) SPR_setVisibility(wpn_parts[i], HIDDEN);
        if (wpn_back[i]) SPR_setVisibility(wpn_back[i], HIDDEN);
        if (boss_parts[i]) SPR_setVisibility(boss_parts[i], HIDDEN);
    }
    if (hp_icon) SPR_setVisibility(hp_icon, HIDDEN);
    if (wpn_icon) SPR_setVisibility(wpn_icon, HIDDEN);
    if (hp_box) SPR_setVisibility(hp_box, HIDDEN);
    if (wpn_box) SPR_setVisibility(wpn_box, HIDDEN);
    hp_on = wpn_on = boss_on = FALSE;
    hud_on = TRUE;
}

void hud_weapon(u8 weapon, u8 ammo)
{
    if (!hud_on) return;
    bool show = (weapon != 0);
    s16 pct = (100 * ammo) / PLAYER_INITIAL_HP;
    set_bar(wpn_parts, show, pct, wpn_icon, wpn_box);
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (wpn_back[i]) SPR_setVisibility(wpn_back[i], show ? VISIBLE : HIDDEN);
    }
    wpn_on = show;
    if (show && wpn_icon) spr_set_frame(wpn_icon, weapon);
}

void hud_update(u8 hp)
{
    if (!hud_on) return;
    s16 pct = (100 * hp) / PLAYER_INITIAL_HP;
    if (pct == hp_last && hp_on) return;
    hp_last = pct;
    set_bar(hp_parts, TRUE, pct, hp_icon, hp_box);
    hp_on = TRUE;
}

void hud_boss(s16 hp)
{
    if (!hud_on) return;
    bool show = (hp > 0);
    s16 pct = show ? (100 * hp) / BOSS_INITIAL_HP : 0;
    const s16 key = show ? pct : -1;
    if (key == boss_last) return;
    boss_last = key;
    set_bar(boss_parts, show, pct, NULL, NULL);
    boss_on = show;
}

void hud_set_visible(bool visible)
{
    hud_on = visible;
    hp_last = boss_last = -2;      // force the next update to re-send
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (hp_parts[i]) SPR_setVisibility(hp_parts[i], visible && hp_on ? VISIBLE : HIDDEN);
        if (wpn_parts[i]) SPR_setVisibility(wpn_parts[i], visible && wpn_on ? VISIBLE : HIDDEN);
        if (wpn_back[i]) SPR_setVisibility(wpn_back[i], visible && wpn_on ? VISIBLE : HIDDEN);
        if (boss_parts[i]) SPR_setVisibility(boss_parts[i], visible && boss_on ? VISIBLE : HIDDEN);
        if (hp_icon) SPR_setVisibility(hp_icon, visible && hp_on ? VISIBLE : HIDDEN);
        if (wpn_icon) SPR_setVisibility(wpn_icon, visible && wpn_on ? VISIBLE : HIDDEN);
        if (hp_box) SPR_setVisibility(hp_box, visible && hp_on ? VISIBLE : HIDDEN);
        if (wpn_box) SPR_setVisibility(wpn_box, visible && wpn_on ? VISIBLE : HIDDEN);
    }
}
