#include "hud.h"

#include "enemy.h"
#include "player.h"
#include "resources.h"

// Classic HUD: vertical energy bars instead of the modern 5-ball rows.
// art/energy_bar_vertical.png is emitted by tools/convert_energy_bars.py as
// seven 8x8 frames; they are stacked and revealed from the bottom upward.
#define HUD_CHUNKS 7
#define HUD_BAR_HP_X   1
#define HUD_BAR_WPN_X  10
#define HUD_BAR_BOSS_X (320 - 15)
#define HUD_BAR_Y      2

static Sprite *hp_chunks[HUD_CHUNKS];
static Sprite *wpn_chunks[HUD_CHUNKS];
static Sprite *boss_chunks[HUD_CHUNKS];
static bool hp_on, wpn_on, boss_on;
static bool hud_on = TRUE;

static void set_bar(Sprite *const *chunks, bool show, s16 pct)
{
    s16 fill = (pct * 50) / 100;       // bar art is 51 px, use 0..50
    if (fill < 0) fill = 0;
    if (fill > 50) fill = 50;
    for (u8 i = 0; i < HUD_CHUNKS; i++) {
        if (!chunks[i]) continue;
        // i = 0 is the top chunk. Show the bottom `fill` pixels.
        bool visible = show && (fill > (s16)((HUD_CHUNKS - 1 - i) * 8));
        SPR_setVisibility(chunks[i], visible ? VISIBLE : HIDDEN);
    }
}

static Sprite *add_bar_chunk(const SpriteDefinition *def, s16 x, s16 y,
                             u16 pal)
{
    return SPR_addSprite(def, x, y, TILE_ATTR(pal, FALSE, FALSE, FALSE));
}

void hud_init(void)
{
    for (u8 i = 0; i < HUD_CHUNKS; i++) {
        hp_chunks[i] = add_bar_chunk(&hud_bar_hp_sprite,
                                     HUD_BAR_HP_X, HUD_BAR_Y + i * 8, PAL2);
        wpn_chunks[i] = add_bar_chunk(&hud_bar_wpn_sprite,
                                      HUD_BAR_WPN_X, HUD_BAR_Y + i * 8, PAL2);
        boss_chunks[i] = add_bar_chunk(&hud_bar_boss_sprite,
                                       HUD_BAR_BOSS_X, HUD_BAR_Y + i * 8, PAL0);
        if (hp_chunks[i]) {
            SPR_setFrame(hp_chunks[i], i);
            SPR_setDepth(hp_chunks[i], SPR_MIN_DEPTH);
        }
        if (wpn_chunks[i]) {
            SPR_setFrame(wpn_chunks[i], i);
            SPR_setDepth(wpn_chunks[i], SPR_MIN_DEPTH);
        }
        if (boss_chunks[i]) {
            SPR_setFrame(boss_chunks[i], i);
            SPR_setDepth(boss_chunks[i], SPR_MIN_DEPTH);
        }
    }
    hp_on = wpn_on = boss_on = FALSE;
    hud_on = TRUE;
}

void hud_weapon(u8 weapon, u8 ammo)
{
    if (!hud_on) return;
    bool show = (weapon != 0);
    s16 pct = (100 * ammo) / PLAYER_INITIAL_HP;
    set_bar(wpn_chunks, show, pct);
    wpn_on = show;
}

void hud_update(u8 hp)
{
    if (!hud_on) return;
    s16 pct = (100 * hp) / PLAYER_INITIAL_HP;
    set_bar(hp_chunks, TRUE, pct);
    hp_on = TRUE;
}

void hud_boss(s16 hp)
{
    if (!hud_on) return;
    bool show = (hp > 0);
    s16 pct = show ? (100 * hp) / BOSS_INITIAL_HP : 0;
    set_bar(boss_chunks, show, pct);
    boss_on = show;
}

void hud_set_visible(bool visible)
{
    hud_on = visible;
    for (u8 i = 0; i < HUD_CHUNKS; i++) {
        if (hp_chunks[i]) SPR_setVisibility(hp_chunks[i], visible && hp_on ? VISIBLE : HIDDEN);
        if (wpn_chunks[i]) SPR_setVisibility(wpn_chunks[i], visible && wpn_on ? VISIBLE : HIDDEN);
        if (boss_chunks[i]) SPR_setVisibility(boss_chunks[i], visible && boss_on ? VISIBLE : HIDDEN);
    }
}
