#include "shot.h"

#include "stage.h"
#include "enemy.h"
#include "resources.h"

// game_projectile_list.dat record 0 "Normal": speed 4, damage 1, 6x6 art.
#define SHOT_SPEED    4
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)
#define SHOT_SIZE     8    // 6x6 art inside an 8x8 cell

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 dx;
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

void shot_fire(s16 x, s16 y, s16 dir)
{
    if (cooldown) return;
    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *s = &shots[i];
        if (s->on) continue;
        s->spr = SPR_addSprite(&shot_sprite, x - camera.x, y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!s->spr) return;
        s->x = x;
        s->y = y;
        s->dx = (dir < 0) ? -SHOT_SPEED : SHOT_SPEED;
        s->on = TRUE;
        cooldown = SHOT_COOLDOWN;
        return;
    }
}

void shot_update(void)
{
    if (cooldown) cooldown--;

    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *s = &shots[i];
        if (!s->on) continue;

        s->x += s->dx;

        // off the visible screen
        if (s->x + SHOT_SIZE < camera.x || s->x > camera.x + 320) {
            shot_kill(s);
            continue;
        }

        // wall
        s16 cx = s->x + SHOT_SIZE / 2;
        s16 cy = s->y + SHOT_SIZE / 2;
        if (stage_terrain_at(stage_current_map, cx >> 4, cy >> 4) == TERRAIN_SOLID) {
            shot_kill(s);
            continue;
        }

        // enemy hit (shot dies on any hit, damage applied inside)
        if (enemies_try_hit(s->x, s->y, s->x + SHOT_SIZE - 1, s->y + SHOT_SIZE - 1)) {
            shot_kill(s);
            continue;
        }

        SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
    }
}
