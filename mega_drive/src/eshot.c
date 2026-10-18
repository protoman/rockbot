#include "eshot.h"

#include "stage.h"
#include "player.h"
#include "resources.h"
#include "stage_data.h"

// PROJECTILE_TRAJECTORIES (PC defines.h) live in stage_data.h (shared with the
// player shots in shot.c).
#define ARC_DIST   (320 / 3)     // PC: trajectory_parabola(RES_W/3)
#define QUAKE_LIFE 60            // ~1s (PC: 50 * QUAKE_TIMER 20ms)

typedef struct {
    Sprite *spr;
    s16 x, y;            // top-left
    s16 x0, y0;          // fire origin (ARC)
    s16 dx, dy;          // per-frame velocity
    u8 speed;
    u8 dmg, w, h;
    u8 traj;
    u8 proj_id;
    u16 age;
    bool on;
} EShot;

static EShot shots[ESHOT_MAX];

static u8 count_proj(u8 proj_id)
{
    u8 n = 0;
    for (u8 i = 0; i < ESHOT_MAX; i++)
        if (shots[i].on && shots[i].proj_id == proj_id) n++;
    return n;
}

// Same rule as the player shot: PC projectile::check_map_collision — any tile
// except UNBLOCKED/WATER stops the projectile (stage_terrain_at covers
// out-of-range as solid).
static bool hits_solid(s16 l, s16 t, s16 r, s16 b)
{
    for (s16 ty = t >> 4; ty <= (b - 1) >> 4; ty++)
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++)
            if (terrain_blocks_projectile(stage_terrain_at(stage_current_map,
                                                           tx, ty)))
                return TRUE;
    return FALSE;
}

static void kill(EShot *s)
{
    if (s->spr) SPR_releaseSprite(s->spr);
    s->spr = NULL;
    s->on = FALSE;
}

// PC trajectory_parabola for a RES_W/3 arc: y = (v*n - (n^2+n)/2)/rf with
// v ~ 53 and rf ~ 1404/64, integer-approximated.
static s16 arc_offset(s16 n)
{
    if (n < 0) n = -n;
    if (n > 300) n = 300;
    s32 y = (s32)53 * n - ((s32)n * n + n) / 2;
    return (s16)((y * 64) / 1404);
}

static void player_center(s16 *cx, s16 *cy)
{
    s16 l, t, r, b;
    player_get_box(&l, &t, &r, &b);
    *cx = (l + r) / 2;
    *cy = (t + b) / 2;
}

// PC position_to_ground: drop until the tile just below is not walk-through.
static void snap_to_ground(EShot *s)
{
    for (u16 i = 0; i < 240; i++) {
        s16 tx = (s->x + s->w / 2) >> 4;
        s16 ty = (s->y + s->h) >> 4;
        u8 t = stage_terrain_at(stage_current_map, tx, ty);
        if (t != TERRAIN_UNBLOCKED) return;
        s->y++;
    }
}

void eshots_reset(void)
{
    for (u8 i = 0; i < ESHOT_MAX; i++) {
        shots[i].spr = NULL;
        shots[i].on = FALSE;
    }
}

bool eshot_fire(s16 x, s16 y, s16 dir, u8 proj_id)
{
    if (proj_id >= stage_def->enemy_proj_max) proj_id = 0;
    const EnemyProj *p = &stage_def->enemy_proj[proj_id];
    if (!p->spr) return FALSE;

    // PC throw_projectile: max_shots from the projectile record (0 = no cap)
    if (p->max_shots && count_proj(proj_id) >= p->max_shots) return FALSE;

    for (u8 i = 0; i < ESHOT_MAX; i++) {
        EShot *s = &shots[i];
        if (s->on) continue;
        s->spr = SPR_addSprite(p->spr, x - camera.x, y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (!s->spr) return FALSE;
        s->x = x - p->w / 2;
        s->y = y - p->h / 2;
        s->x0 = s->x;
        s->y0 = s->y;
        s->dx = (dir < 0) ? -(s16)p->speed : (s16)p->speed;
        s->dy = 0;
        s->speed = p->speed;
        s->dmg = p->dmg;
        s->w = p->w;
        s->h = p->h;
        s->proj_id = proj_id;
        s->traj = p->traj;
        s->age = 0;
        s->on = TRUE;

        // Trajectories that resolve at fire time (PC set_target_position).
        if (p->traj == TRAJ_TARGET_DIRECTION) {
            s16 pcx, pcy;
            player_center(&pcx, &pcy);
            // PC set_target_position: TILESIZE*2 above/below -> diagonal
            if (pcy < s->y - 32) {
                s->traj = TRAJ_DIAG_UP;
                s->dy = -(s16)p->speed;
            } else if (pcy > s->y + 32) {
                s->traj = TRAJ_DIAG_DOWN;
                s->dy = (s16)p->speed;
            } else {
                s->traj = TRAJ_LINEAR;
            }
            s->dx = (pcx < s->x) ? -(s16)p->speed : (s16)p->speed;
        }
        return TRUE;
    }
    return FALSE;
}

void eshots_set_visible(bool visible)
{
    for (u8 i = 0; i < ESHOT_MAX; i++)
        if (shots[i].spr)
            SPR_setVisibility(shots[i].spr, visible ? VISIBLE : HIDDEN);
}

void eshots_update(void)
{
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);
    s16 pcx = (pl + pr) / 2;
    s16 pcy = (pt + pb) / 2;

    for (u8 i = 0; i < ESHOT_MAX; i++) {
        EShot *s = &shots[i];
        if (!s->on) continue;
        s->age++;

        switch (s->traj) {
        case TRAJ_ARC:
            s->x += s->dx;
            s->y = s->y0 - arc_offset(s->x - s->x0);
            if (s->y > 240) { kill(s); continue; }
            break;
        case TRAJ_FOLLOW:
            if (pcx > s->x) s->x += s->speed;
            else if (pcx < s->x) s->x -= s->speed;
            if (pcy > s->y) s->y += s->speed;
            else if (pcy < s->y) s->y -= s->speed;
            break;
        case TRAJ_PROGRESSIVE:
            // PC moves it only on the animation frame reset (~100ms)
            if ((s->age % 6) == 0) {
                s->x += s->dx;
                snap_to_ground(s);
            }
            break;
        case TRAJ_QUAKE:
            // PC shakes the screen and the projectile stays put (no shake
            // ported yet); it expires after ~1s.
            if (s->age > QUAKE_LIFE) { kill(s); continue; }
            break;
        default:                    // LINEAR / DIAG_UP / DIAG_DOWN / fallback
            s->x += s->dx;
            s->y += s->dy;
            break;
        }

        s16 l = s->x, t = s->y, r = s->x + s->w, b = s->y + s->h;

        // off the camera window (2 tiles of margin) or into a wall.
        // The wall probe is a thin band on the MUZZLE line: testing the whole
        // box killed any projectile taller than the gap under the muzzle
        // (MAGE BOT's MAGIC STAR is 56x56 and its box reaches into the floor).
        if (r < camera.x - 32 || l > camera.x + 352 ||
            hits_solid(l, s->y0 + 2, r, s->y0 + 3)) {
            kill(s);
            continue;
        }

        // player collision (PC projectile vs player)
        if (pl < r && pr > l && pt < b && pb > t) {
            // PC classnpc.cpp:532: a shielded player blocks the shot instead of
            // taking damage. The player's shield_type is SHIELD_FRONT, but the
            // player branch of is_shielded() (character.cpp:3357) ignores the
            // shot direction, so any side blocks.
            if (!player_is_shielding()) player_take_damage(s->dmg);
            kill(s);
            continue;
        }

        if (s->spr) {
            SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
        }
    }
}
