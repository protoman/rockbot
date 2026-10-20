#include "hud.h"

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

static const u16 part_y[HUD_PARTS] = { 0, 24, 48 };

static Sprite *hp_parts[HUD_PARTS];
static Sprite *wpn_parts[HUD_PARTS];
static Sprite *boss_parts[HUD_PARTS];
static bool hp_on, wpn_on, boss_on;
static bool hud_on = TRUE;

static Sprite *add_part(const SpriteDefinition *def, s16 x, s16 y, u16 pal)
{
    Sprite *sp = SPR_addSprite(def, x, y, TILE_ATTR(pal, FALSE, FALSE, FALSE));
    if (sp) SPR_setDepth(sp, SPR_MIN_DEPTH);
    return sp;
}

static void set_bar(Sprite *const *parts, bool show, s16 pct)
{
    s16 frame = (pct * 50) / 100;
    if (frame < 0) frame = 0;
    if (frame > 50) frame = 50;
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (!parts[i]) continue;
        SPR_setVisibility(parts[i], show ? VISIBLE : HIDDEN);
        if (show) SPR_setFrame(parts[i], frame);
    }
}

void hud_init(void)
{
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
        hp_parts[i] = add_part(hp[i], HUD_BAR_HP_X, HUD_BAR_Y + part_y[i], PAL2);
        wpn_parts[i] = add_part(wpn[i], HUD_BAR_WPN_X, HUD_BAR_Y + part_y[i], PAL2);
        boss_parts[i] = add_part(boss[i], HUD_BAR_BOSS_X,
                                 HUD_BAR_Y + part_y[i], PAL0);
        if (hp_parts[i]) SPR_setFrame(hp_parts[i], 0);
        if (wpn_parts[i]) SPR_setFrame(wpn_parts[i], 0);
        if (boss_parts[i]) SPR_setFrame(boss_parts[i], 0);
    }
    hp_on = wpn_on = boss_on = FALSE;
    hud_on = TRUE;
}

void hud_weapon(u8 weapon, u8 ammo)
{
    if (!hud_on) return;
    bool show = (weapon != 0);
    s16 pct = (100 * ammo) / PLAYER_INITIAL_HP;
    set_bar(wpn_parts, show, pct);
    wpn_on = show;
}

void hud_update(u8 hp)
{
    if (!hud_on) return;
    s16 pct = (100 * hp) / PLAYER_INITIAL_HP;
    set_bar(hp_parts, TRUE, pct);
    hp_on = TRUE;
}

void hud_boss(s16 hp)
{
    if (!hud_on) return;
    bool show = (hp > 0);
    s16 pct = show ? (100 * hp) / BOSS_INITIAL_HP : 0;
    set_bar(boss_parts, show, pct);
    boss_on = show;
}

void hud_set_visible(bool visible)
{
    hud_on = visible;
    for (u8 i = 0; i < HUD_PARTS; i++) {
        if (hp_parts[i]) SPR_setVisibility(hp_parts[i], visible && hp_on ? VISIBLE : HIDDEN);
        if (wpn_parts[i]) SPR_setVisibility(wpn_parts[i], visible && wpn_on ? VISIBLE : HIDDEN);
        if (boss_parts[i]) SPR_setVisibility(boss_parts[i], visible && boss_on ? VISIBLE : HIDDEN);
    }
}
