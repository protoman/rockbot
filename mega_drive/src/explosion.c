// Enemy death puff — see explosion.h. The PC adds a 32x32 explosion animation
// at the dead NPC's position (classmap.cpp:2232), 80ms per frame, repeated
// twice. On the MD each puff is one sprite; the PC's 80ms is 5 frames at 60fps
// and the sheet has 3 frames, so a puff lives 2 * 3 * 5 = 30 frames.
#include "explosion.h"

#include "resources.h"
#include "stage.h"

#define FRAME_TIME 5                 // 80ms at 60fps (PC add_animation)
#define REPEATS    2                 // PC repeat_times
#define LIFE       (FRAME_TIME * 3 * REPEATS)

typedef struct {
    Sprite *spr;
    s16 x, y;                        // world position (the enemy's)
    s16 age;                         // frames lived; -1 = free
} Puff;

static Puff puffs[EXPLOSION_MAX];

void explosion_reset(void)
{
    for (u8 i = 0; i < EXPLOSION_MAX; i++) {
        if (puffs[i].spr) SPR_releaseSprite(puffs[i].spr);
        puffs[i].spr = NULL;
        puffs[i].age = -1;
    }
}

void explosion_spawn(s16 x, s16 y)
{
    // PC: don't add the death explosion when the enemy dies out of screen.
    if (y >= 224) return;
    for (u8 i = 0; i < EXPLOSION_MAX; i++) {
        if (puffs[i].age >= 0) continue;
        // PC adjust_pos (-8,-8): the 32x32 puff is centred on the enemy.
        Sprite *s = SPR_addSprite(&explosion_sprite, x - 8 - camera.x,
                                  y - 8 - camera.y,
                                  TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!s) return;
        puffs[i].spr = s;
        puffs[i].x = x - 8;
        puffs[i].y = y - 8;
        puffs[i].age = 0;
        // manual frames: the sheet's own timing is only used as a fallback
        SPR_setAutoAnimation(s, FALSE);
        SPR_setFrame(s, 0);
        return;
    }
}

void explosion_update(void)
{
    for (u8 i = 0; i < EXPLOSION_MAX; i++) {
        Puff *p = &puffs[i];
        if (p->age < 0) continue;
        if (++p->age >= LIFE) {
            SPR_releaseSprite(p->spr);
            p->spr = NULL;
            p->age = -1;
            continue;
        }
        SPR_setFrame(p->spr, (p->age / FRAME_TIME) % 3);
        SPR_setPosition(p->spr, p->x - camera.x, p->y - camera.y);
    }
}
