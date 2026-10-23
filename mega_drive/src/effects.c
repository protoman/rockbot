// Transient sprite effects — see effects.h. Same shape as explosion.c: a small
// pool of sprites, each with its own art timing, released when it ends.
#include "effects.h"

#include "resources.h"
#include "stage.h"

// PC timings in frames at 60 fps.
#define DUST_TICKS     10          // 160 ms (character.cpp:1681)
#define DUST_FRAMES     3          // 24x16 art / 8x8 frames
#define DUST_ROUNDS     1          // PC repeat_times 0: plays once

#define SPLASH_TICKS    5          // 80 ms (projectilelib.cpp:129)
#define SPLASH_FRAMES   4          // 128x23 art / 32x23 frames
#define SPLASH_ROUNDS   2          // PC repeat_times 2
// PC draws the art 8 px above its reference, which is the water FLOOR
// (projectilelib.cpp:129). A splash from the player entering the water bursts
// from the SURFACE instead: the painted droplets end two rows above the art's
// 24 px bottom (the 23 px art is padded to 24), so lifting the art by 22 puts
// that base exactly on the surface.
#define SPLASH_Y_ADJUST 22

#define TP_TICKS        6          // 100 ms (draw.cpp:429)
#define TP_FRAMES       2          // 64x32 art / 32x32 frames
#define TP_ROUNDS       3          // ~0.6 s, the PC's warp window

#define BUBBLE_RISE     2          // classmap.cpp:378 pos.y -= 2
#define BUBBLE_LIFE   180          // 3000 ms, the PC's cap
#define BUBBLE_GAP    180          // add_bubble_animation: 3000 ms cooldown
#define BUBBLE_WOBBLE   8          // half pixels: the PC goes 0 -> -4 px

#define FX_MAX 6

enum { FX_FREE = 0, FX_ON };

typedef struct {
    Sprite *spr;
    u16 seen;                      // sprite-pool generation (stage.h)
    s16 x, y;                      // world position of the art's top-left
    u16 age, life;
    u16 ticks;                     // frames per art frame
    u8 frames;
} Fx;

// Handles carry the sprite-pool generation they were created in: after
// SPR_end/SPR_initEx they are stale and must be forgotten, not released
// (releasing into the new pool corrupts it — see stage.h).
static Fx fx[FX_MAX];

static Sprite *bubble_spr;
static s16 bubble_x, bubble_y;
static s16 bubble_wob;             // half pixels, BUBBLE_WOBBLE..0
static s8 bubble_wdir;
static u16 bubble_life, bubble_cd;
static u16 bubble_seen;

static Sprite *add(const SpriteDefinition *def, s16 x, s16 y, bool flip)
{
    // TILE_ATTR(pal, prio, flipV, flipH): the dust mirrors by the facing.
    Sprite *s = SPR_addSprite(def, x - camera.x, y - camera.y,
                              TILE_ATTR(PAL0, FALSE, FALSE, flip));
    if (s) {
        SPR_setAutoAnimation(s, FALSE);
        SPR_setFrame(s, 0);
    }
    return s;
}

static void spawn(const SpriteDefinition *def, s16 x, s16 y,
                  u8 frames, u16 ticks, u8 rounds, bool flip)
{
    for (u8 i = 0; i < FX_MAX; i++) {
        Fx *f = &fx[i];
        if (f->spr) continue;
        f->spr = add(def, x, y, flip);
        if (!f->spr) return;       // pool/VRAM full: the effect is cosmetic
        f->seen = spr_gen;
        f->x = x;
        f->y = y;
        f->age = 0;
        f->life = (u16)(frames * ticks * rounds);
        f->ticks = ticks;
        f->frames = frames;
        return;
    }
}

void fx_dust(s16 x, s16 y, s8 dir)
{
    spawn(&fx_dust_sprite, x, y, DUST_FRAMES, DUST_TICKS, DUST_ROUNDS,
          dir < 0);
}

void fx_splash(s16 x, s16 y, s8 dir)
{
    spawn(&fx_splash_sprite, x, y - SPLASH_Y_ADJUST,
          SPLASH_FRAMES, SPLASH_TICKS, SPLASH_ROUNDS, dir < 0);
}

void fx_teleport(s16 x, s16 y)
{
    spawn(&fx_teleport_sprite, x, y, TP_FRAMES, TP_TICKS, TP_ROUNDS, FALSE);
}

static void bubble_kill(void)
{
    spr_release_guarded(&bubble_spr, bubble_seen);
    bubble_life = 0;
}

void fx_water_bubble(s16 x, s16 y)
{
    if (bubble_cd) return;
    bubble_cd = BUBBLE_GAP;
    bubble_kill();                 // the PC keeps a single bubble
    bubble_x = x;
    bubble_y = y;
    bubble_wob = 0;
    bubble_wdir = -1;
    bubble_life = BUBBLE_LIFE;
    bubble_spr = add(&fx_bubble_sprite, x, y, FALSE);
    bubble_seen = spr_gen;
}

static void bubble_update(void)
{
    if (bubble_cd) bubble_cd--;
    if (!bubble_spr) return;
    if (bubble_seen != spr_gen) {  // pool recreated: handle is stale
        bubble_spr = NULL;
        return;
    }

    if (--bubble_life == 0) {
        bubble_kill();
        return;
    }
    bubble_y -= BUBBLE_RISE;
    bubble_wob += bubble_wdir;
    if (bubble_wob <= -BUBBLE_WOBBLE) bubble_wdir = 1;
    else if (bubble_wob >= 0) bubble_wdir = -1;

    s16 sx = (s16)(bubble_x + (bubble_wob / 2));
    s16 sy = bubble_y;
    // PC: dies as soon as it leaves the water (or the 3 s cap).
    bool water = stage_terrain_at(stage_current_map, (sx + 2) >> 4, sy >> 4)
                 == TERRAIN_WATER;
    if (!water || sy < -8) {
        bubble_kill();
        return;
    }
    SPR_setPosition(bubble_spr, sx - camera.x, sy - camera.y);
}

void effects_update(void)
{
    for (u8 i = 0; i < FX_MAX; i++) {
        Fx *f = &fx[i];
        if (!f->spr) continue;
        if (f->seen != spr_gen) {  // pool recreated: handle is stale
            f->spr = NULL;
            continue;
        }
        if (++f->age >= f->life) {
            spr_release_guarded(&f->spr, f->seen);
            continue;
        }
        SPR_setFrame(f->spr, (s16)((f->age / f->ticks) % f->frames));
        SPR_setPosition(f->spr, f->x - camera.x, f->y - camera.y);
    }
    bubble_update();
}

void effects_reset(void)
{
    for (u8 i = 0; i < FX_MAX; i++) {
        spr_release_guarded(&fx[i].spr, fx[i].seen);
        fx[i].age = fx[i].life = 0;
    }
    spr_release_guarded(&bubble_spr, bubble_seen);
    bubble_life = 0;
    bubble_cd = 0;
}
