#include "shot.h"
#include "sfx.h"

#include "stage.h"
#include "stage_data.h"
#include "enemy.h"
#include "player.h"
#include "weapon_data.h"
#include "resources.h"

// game_projectile_list.dat: the player shot's sprite/speed/damage/size come
// from the projectile the SELECTED weapon fires (weapon_data[].proj), read out
// of stage_def->enemy_proj. Weapon 0 is the buster (projectile 0).
#define SHOT_CHARGE_SPEED 3
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)
#define SHOT_QUAKE_LIFE 60 // ~1s for the stationary QUAKE shot

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 x0, y0;         // fire origin (ARC)
    s16 dx, dy;
    u8 dmg;
    u8 w, h;    // collision box (from the projectile table)
    u8 traj;
    u16 age;
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

// Same PC trajectory_parabola shape eshot.c uses (ARC weapons: PETALS
// RAINBOW). ARC_DIST is defined in eshot.c; keep the formula local here.
static s16 arc_offset(s16 n)
{
    if (n < 0) n = -n;
    if (n > 300) n = 300;
    s32 y = (s32)53 * n - ((s32)n * n + n) / 2;
    return (s16)((y * 64) / 1404);
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

    // PC classplayer.cpp:336: only the DEFAULT weapon is unlimited; any other
    // weapon needs gauge left (refilled at each stage start).
    u8 w = player_weapon();
    if (w != 0 && player_weapon_ammo() == 0) return;

    const EnemyProj *pd = &stage_def->enemy_proj[weapon_data[w].proj];
    // Some ids have no art in the port (e.g. 7 SPIKED CHAIN is skipped by the
    // converter) — fall back to the buster so the weapon still shoots.
    if (!pd->spr || !pd->speed) pd = &stage_def->enemy_proj[0];
    const SpriteDefinition *spr = pd->spr ? pd->spr : &shot_sprite;

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
        s->spr = SPR_addSprite(spr, ox - camera.x, oy - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!s->spr) break;
        s->x = ox;
        s->y = oy;
        s->x0 = ox;
        s->y0 = oy;
        s->dx = (dir < 0) ? -(s16)pd->speed : (s16)pd->speed;
        s->dy = dy;
        s->dmg = pd->dmg;
        s->w = pd->w ? pd->w : 8;
        s->h = pd->h ? pd->h : 8;
        s->traj = pd->traj;
        s->age = 0;
        s->on = TRUE;
        live++;
    }
    cooldown = SHOT_COOLDOWN;
    sfx_play(SFX_SHOT);

    // PC ammo cost (classplayer.cpp:466): QUAKE 4, anything else 1; the
    // default weapon never consumes.
    if (w != 0) {
        u8 cost = (pd->traj == TRAJ_QUAKE) ? 4 : 1;
        for (u8 i = 0; i < cost; i++) player_use_weapon_ammo();
    }
}

void shot_charge_fire(s16 x, s16 y, s16 dir, u8 level)
{
    if (cooldown) return;
    u8 w = player_weapon();
    if (w != 0 && player_weapon_ammo() == 0) return;
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
    s->x0 = x;
    s->y0 = y;
    s->dx = (dir < 0) ? -SHOT_CHARGE_SPEED : SHOT_CHARGE_SPEED;
    s->dy = 0;
    s->traj = TRAJ_LINEAR;
    s->age = 0;
    s->on = TRUE;
    cooldown = SHOT_COOLDOWN;
    sfx_play(SFX_CHARGE);
    if (w != 0) player_use_weapon_ammo();
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
        s->age++;

        switch (s->traj) {
        case TRAJ_QUAKE:
            // PC shakes the screen; the projectile itself stays put (~1s).
            if (s->age > SHOT_QUAKE_LIFE) { shot_kill(s); continue; }
            break;
        case TRAJ_ARC:
            s->x += s->dx;
            s->y = s->y0 - arc_offset(s->x - s->x0);
            break;
        default:                    // LINEAR / DIAG_* / not ported -> straight
            s->x += s->dx;
            s->y += s->dy;
            break;
        }

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
