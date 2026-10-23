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
// Sprite-engine generation these handles belong to (stage.h): after a pool
// destroy/create they are stale and must be forgotten, not released.
static u16 spr_seen;

void explosion_reset(void)
{
    for (u8 i = 0; i < EXPLOSION_MAX; i++) {
        spr_release_guarded(&puffs[i].spr, spr_seen);
        puffs[i].age = -1;
    }
    explosion_ring_reset();
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
        spr_seen = spr_gen;
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

// ---------------------------------------------------------------------------
// Boss death ring (PC game::classic_style_draw_boss_death, game.cpp:1798):
// eight explosions (right/left/up/down + the four 45-degree diagonals) leave
// the boss centre and travel outward until the radius crosses the screen. The
// PC blits the 24x24 art onto the game screen; on the MD they are sprites:
// 8 x 9 tiles = 72 tiles of the 768-tile pool, which fits the tightest stage
// layout (check_vram: stage2 map2 leaves 99 tiles free).
// ---------------------------------------------------------------------------
#define RING_SPRITES 8
// PC game.cpp:1798 advances the radius 1px per step with a 5 ms step, i.e.
// ~200 px/s. At 60 fps that is 3 px per frame, and the ring is gone after
// RING_STEPS frames (radius past 320).
#define RING_STEPS   108
#define RING_STEP_PX 3
#define RING_ART_FRAMES 5
#define RING_ART_TICKS  3         // 5 ms steps at 60 fps
// cos(45) in 8.8 fixed point (the diagonals are at +-distance*181/256).
#define RING_DIAG 181

static struct {
    Sprite *spr[RING_SPRITES];
    s16 cx, cy;                   // world centre
    u16 step;
    u8 art_t;
    s8 art_j;
    s8 art_dir;
    bool on;
} ring;

void explosion_ring_reset(void)
{
    for (u8 i = 0; i < RING_SPRITES; i++)
        spr_release_guarded(&ring.spr[i], spr_seen);
    ring.on = FALSE;
}

void explosion_ring_start(s16 cx, s16 cy)
{
    explosion_ring_reset();
    ring.cx = cx;
    ring.cy = cy;
    ring.step = 0;
    ring.art_t = 0;
    ring.art_j = 0;
    ring.art_dir = 1;
    spr_seen = spr_gen;
    for (u8 i = 0; i < RING_SPRITES; i++) {
        ring.spr[i] = SPR_addSprite(&death_explosion_sprite,
                                    cx - 12 - camera.x, cy - 12 - camera.y,
                                    TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (ring.spr[i]) {
            SPR_setAutoAnimation(ring.spr[i], FALSE);
            SPR_setFrame(ring.spr[i], 0);
        }
    }
    ring.on = TRUE;
}

// TRUE while the ring is still running.
bool explosion_ring_update(void)
{
    if (!ring.on) return FALSE;

    s16 d = (s16)(ring.step * RING_STEP_PX);
    s16 dx8 = (s16)((d * RING_DIAG) >> 8);
    // axis points then the four diagonals (PC order: right, left, up, down)
    s16 rx[RING_SPRITES] = { (s16)(ring.cx + d), (s16)(ring.cx - d), ring.cx,
                             ring.cx,
                             (s16)(ring.cx + dx8), (s16)(ring.cx - dx8),
                             (s16)(ring.cx + dx8), (s16)(ring.cx - dx8) };
    s16 ry[RING_SPRITES] = { ring.cy, ring.cy, (s16)(ring.cy - d),
                             (s16)(ring.cy + d),
                             (s16)(ring.cy - dx8), (s16)(ring.cy - dx8),
                             (s16)(ring.cy + dx8), (s16)(ring.cy + dx8) };

    // art frame ping-pongs 0..4 like the PC explosion_status
    if (++ring.art_t >= RING_ART_TICKS) {
        ring.art_t = 0;
        ring.art_j = (s8)(ring.art_j + ring.art_dir);
        if (ring.art_j >= RING_ART_FRAMES - 1) ring.art_dir = -1;
        else if (ring.art_j <= 0) ring.art_dir = 1;
    }

    for (u8 i = 0; i < RING_SPRITES; i++) {
        Sprite *s = ring.spr[i];
        if (!s) continue;
        SPR_setPosition(s, (s16)(rx[i] - 12 - camera.x),
                        (s16)(ry[i] - 12 - camera.y));
        SPR_setFrame(s, (s16)ring.art_j);
        SPR_setVisibility(s, VISIBLE);
    }

    if (++ring.step > RING_STEPS) {
        explosion_ring_reset();
        return FALSE;
    }
    return TRUE;
}
