#include "shot.h"

#include "stage.h"
#include "enemy.h"
#include "resources.h"

// game_projectile_list.dat record 0 "Normal": speed 4, damage 1, 6x6 art.
#define SHOT_SPEED    4
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 dx;
    u8 dmg;
    u8 size;    // 8 = normal, 16 = charged
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

static Shot *shot_alloc(void)
{
    if (cooldown) return NULL;
    for (u8 i = 0; i < SHOT_MAX; i++) {
        if (!shots[i].on) return &shots[i];
    }
    return NULL;
}

void shot_fire(s16 x, s16 y, s16 dir)
{
    Shot *s = shot_alloc();
    if (!s) return;
    s->spr = SPR_addSprite(&shot_sprite, x - camera.x, y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (!s->spr) return;
    s->x = x;
    s->y = y;
    s->dx = (dir < 0) ? -SHOT_SPEED : SHOT_SPEED;
    s->dmg = 1;
    s->size = 8;
    s->on = TRUE;
    cooldown = SHOT_COOLDOWN;
}

void shot_charge_fire(s16 x, s16 y, s16 dir, u8 level)
{
    Shot *s = shot_alloc();
    if (!s) return;
    s->spr = SPR_addSprite(&shot_charge_sprite, x - camera.x, y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (!s->spr) return;
    SPR_setAnim(s->spr, 0);
    s->x = x;
    s->y = y;
    s->dx = (dir < 0) ? -(SHOT_SPEED + level) : (SHOT_SPEED + level);
    s->dmg = (level >= 2) ? 3 : 2;
    s->size = 16;
    s->on = TRUE;
    cooldown = SHOT_COOLDOWN;
}

void shot_update(void)
{
    if (cooldown) cooldown--;

    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *s = &shots[i];
        if (!s->on) continue;

        s->x += s->dx;

        // off the visible screen
        if (s->x + s->size < camera.x || s->x > camera.x + 320) {
            shot_kill(s);
            continue;
        }

        // wall (test the leading edge)
        s16 cx = (s->dx > 0) ? s->x + s->size - 1 : s->x;
        s16 cy = s->y + s->size / 2;
        if (stage_terrain_at(stage_current_map, cx >> 4, cy >> 4) == TERRAIN_SOLID) {
            shot_kill(s);
            continue;
        }

        // enemy hit (shot dies on any hit; charged shots pierce: PC full
        // charge keeps flying — character.cpp:742 permanent projectiles)
        if (enemies_try_hit(s->x, s->y, s->x + s->size - 1, s->y + s->size - 1,
                            s->dmg)) {
            if (s->dmg < 3) {
                shot_kill(s);
                continue;
            }
        }

        SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
    }
}
