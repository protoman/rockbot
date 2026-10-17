#include "shot.h"
#include "sfx.h"

#include "stage.h"
#include "enemy.h"
#include "player.h"
#include "resources.h"

// game_projectile_list.dat record 0 "Normal": speed 4, damage 1, 6x6 art.
// Charged shots (records 11 semi / 10 full) both move at speed 3.
#define SHOT_SPEED    4
#define SHOT_CHARGE_SPEED 3
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 dx, dy;
    u8 dmg;
    u8 w, h;    // collision box (8x8 normal, 16x16 semi, 31x30 full charge)
    bool on;
} Shot;

static Shot shots[SHOT_MAX];
static u8 cooldown;

static void shot_kill(Shot *s)
{
    if (s->on) {
        SPR_releaseSprite(s->spr);
        s->spr = NULL;
        s->on = FALSE;
    }
}

void shots_reset(void)
{
    for (u8 i = 0; i < SHOT_MAX; i++) shot_kill(&shots[i]);
    cooldown = 0;
}

static Shot *shot_alloc_raw(void)
{
    for (u8 i = 0; i < SHOT_MAX; i++) {
        if (!shots[i].on) return &shots[i];
    }
    return NULL;
}

void shot_fire(s16 x, s16 y, s16 dir, s16 dy)
{
    if (cooldown) return;
    u8 live = 0;
    for (u8 i = 0; i < SHOT_MAX; i++) if (shots[i].on) live++;
    if (live >= player_max_shots()) return;      // PC player.max_shots

    // PC simultaneous_shots (BETABOT fires 2 per press): the extra shot
    // spawns one tile BEHIND the first and 5px lower (character.cpp:761-771).
    u8 n = player_simultaneous();
    for (u8 k = 0; k < n && live < player_max_shots(); k++) {
        s16 ox = x, oy = y;
        if (k == 1) {
            ox = (dir < 0) ? x + 16 : x - 16;
            oy = y + 5;
        }
        Shot *s = shot_alloc_raw();
        if (!s) break;
        s->spr = SPR_addSprite(&shot_sprite, ox - camera.x, oy - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!s->spr) break;
        s->x = ox;
        s->y = oy;
        s->dx = (dir < 0) ? -SHOT_SPEED : SHOT_SPEED;
        s->dy = dy;
        s->dmg = 1;   // game_projectile_list.dat record 0 "Normal"
        s->w = 8;
        s->h = 8;
        s->on = TRUE;
        live++;
    }
    cooldown = SHOT_COOLDOWN;
    sfx_play(SFX_SHOT);
}

void shot_charge_fire(s16 x, s16 y, s16 dir, u8 level)
{
    if (cooldown) return;
    u8 live = 0;
    for (u8 i = 0; i < SHOT_MAX; i++) if (shots[i].on) live++;
    if (live >= player_max_shots()) return;
    Shot *s = shot_alloc_raw();
    if (!s) return;
    // PC: semi charge = projectile 11 (charged_semi, 16x16, dmg 2, speed 3);
    // full charge = projectile 10 (charged_shot, 31x30, dmg 3, speed 3) —
    // both LINEAR with max_shots 1 (game_projectile_list.dat records 10/11).
    const SpriteDefinition *spr;
    if (level >= 2) {
        spr = &shot_charge_full_sprite;
        s->w = 31; s->h = 30;
        s->dmg = 3;
    } else {
        spr = &enemy_shot_semi_sprite;
        s->w = 16; s->h = 16;
        s->dmg = 2;
    }
    s->spr = SPR_addSprite(spr, x - camera.x, y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (!s->spr) return;
    SPR_setAnim(s->spr, 0);
    s->x = x;
    s->y = y;
    s->dx = (dir < 0) ? -SHOT_CHARGE_SPEED : SHOT_CHARGE_SPEED;
    s->on = TRUE;
    cooldown = SHOT_COOLDOWN;
    sfx_play(SFX_CHARGE);
}

void shots_set_visible(bool visible)
{
    for (u8 i = 0; i < SHOT_MAX; i++)
        if (shots[i].spr)
            SPR_setVisibility(shots[i].spr, visible ? VISIBLE : HIDDEN);
}

void shot_update(void)
{
    if (cooldown) cooldown--;

    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *s = &shots[i];
        if (!s->on) continue;

        s->x += s->dx;
        s->y += s->dy;

        // off the visible screen
        if (s->x + s->w < camera.x || s->x > camera.x + 320 ||
            s->y + s->h < camera.y || s->y > camera.y + 224) {
            shot_kill(s);
            continue;
        }

        // wall (test the leading edge)
        s16 cx = (s->dx > 0) ? s->x + s->w - 1 : s->x;
        s16 cy = s->y + s->h / 2;
        if (stage_terrain_at(stage_current_map, cx >> 4, cy >> 4) == TERRAIN_SOLID) {
            shot_kill(s);
            continue;
        }

        // enemy hit (shot dies on any hit; charged shots pierce: PC full
        // charge keeps flying — character.cpp:742 permanent projectiles)
        if (enemies_try_hit(s->x, s->y, s->x + s->w - 1, s->y + s->h - 1,
                            s->dmg)) {
            if (s->dmg < 3) {
                shot_kill(s);
                continue;
            }
        }

        SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
    }
}
