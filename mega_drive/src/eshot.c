#include "eshot.h"

#include "stage.h"
#include "player.h"
#include "resources.h"
#include "stage_data.h"
#include "explosion.h"
#include "enemy.h"
#include "shot.h"
#include "sfx.h"
#include "sprite_guard.h"

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
    u8 owner;            // enemy index that fired it (0xFF = world/player)
    u8 w0;               // art width (LARGE_BEAM growth)
    u8 step_t;           // frame counter for the spawning trajectories
    s8 dir;
    u8 status;
    u16 effect_t;
    u16 age;
    bool on;
    bool reflected;     // PC projectile.is_reflected: harmless after shield
    bool boom;          // BOMB / FALL_BOMB: now the 56x56 explosion (the only damaging phase)
} EShot;

static EShot shots[ESHOT_MAX];

// PC CENTERED projectiles stay on their owner for 3600 ms (_effect_timer).
#define CIRCLE_FRAMES 216
u8 eshot_current_owner = 0xFF;
static u16 spr_seen;    // sprite-engine generation of the handles below

static u8 count_proj(u8 proj_id)
{
    u8 n = 0;
    for (u8 i = 0; i < ESHOT_MAX; i++)
        if (shots[i].on && shots[i].proj_id == proj_id) n++;
    return n;
}

// Spawn one projectile from an existing one (BOMB_RAIN children), ignoring the
// per-projectile max_shots gate, with a different trajectory.
static bool spawn_child(s16 x, s16 y, s8 dir, u8 proj_id, u8 traj);

// PC projectile::transform_into_explosion / FALL_BOMB landing: the bomb becomes
// a 56x56 animation (graphicsLib::draw_explosion, 100 ms per frame) that hurts
// for 3600 ms (BOMB) / 1600 ms (FALL_BOMB). Before that it deals no damage.
#define BOOM_FRAMES_BOMB 216
#define BOOM_FRAMES_FALL 96

static void start_boom(EShot *s, bool fall)
{
    if (s->spr) SPR_releaseSprite(s->spr);
    s->spr = SPR_addSprite(&bomb_explosion_sprite, 0, 0,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    spr_seen = spr_gen;
    if (s->spr) {
        SPR_setAutoAnimation(s->spr, FALSE);
        spr_set_frame(s->spr, 0);
    }
    s->x -= 28 - s->w / 2;                 // PC: x -= 28, y -= 28 (48 for FALL_BOMB)
    s->y -= (fall ? 48 : 28) - s->h / 2;
    s->w = s->h = 56;
    s->boom = TRUE;
    s->age = 0;
    sfx_play(SFX_EXPLOSION);
}

static void kill(EShot *s)
{
    if (s->spr) SPR_releaseSprite(s->spr);
    s->spr = NULL;
    s->on = FALSE;
}

static bool spawn_child(s16 x, s16 y, s8 dir, u8 proj_id, u8 traj)
{
    if (proj_id >= stage_def->enemy_proj_max) return FALSE;
    const EnemyProj *p = &stage_def->enemy_proj[proj_id];
    for (u8 i = 0; i < ESHOT_MAX; i++) {
        EShot *s = &shots[i];
        if (s->on) continue;
        s->spr = SPR_addSprite(p->spr, x - camera.x, y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        spr_seen = spr_gen;
        if (!s->spr) return FALSE;
        s->x = s->x0 = (s16)(x - p->w / 2);
        s->y = s->y0 = (s16)(y - p->h / 2);
        s->dx = (dir < 0) ? -(s16)p->speed : (s16)p->speed;
        s->dy = 0;
        s->speed = p->speed;
        s->dmg = p->dmg;
        s->w = p->w;
        s->h = p->h;
        s->w0 = p->w;
        s->proj_id = proj_id;
        s->traj = traj;
        s->owner = 0xFF;
        s->step_t = 0;
        s->dir = dir;
        s->status = 0;
        s->boom = FALSE;
        s->effect_t = 0;
        s->age = 0;
        s->reflected = FALSE;
        s->on = TRUE;
        return TRUE;
    }
    return FALSE;
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
    // Release, do not just forget the handle: dropping it left the projectile
    // sprites allocated and frozen on screen (BUG-012, visible during the 2 s
    // boss death sequence after shots_reset()/eshots_reset()).
    for (u8 i = 0; i < ESHOT_MAX; i++) {
        spr_release_guarded(&shots[i].spr, spr_seen);
        shots[i].on = FALSE;
    }
}

// PC projectile::show() mirrors the projectile art by its direction; the art
// faces right, so anything flying left must be flipped (GAMBIT's cards were
// always drawn facing right - BUG-021).
static void eshot_flip(EShot *s)
{
    if (!s->spr) return;
    // Missiles ship 7 DIRECTION rows (PC projectile::draw): RIGHT, UP, DOWN,
    // UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT. The converter lays them out as
    // 7 frames; the frame follows the flight direction instead of cycling.
    const SpriteDefinition *def = s->spr->definition;
    if (def && def->numAnimation == 1 && def->animations[0]->numFrame == 7) {
        const s16 ax = (s->dx < 0) ? -s->dx : s->dx;
        const s16 ay = (s->dy < 0) ? -s->dy : s->dy;
        u8 row;
        bool flip = FALSE;
        if (ay * 5 < ax * 2) {                       // flatter than ~22 degrees
            row = 0;
            flip = s->dx < 0;
        } else if (ax * 5 < ay * 2) {                // steeper than ~68 degrees
            row = (s->dy < 0) ? 1 : 2;
        } else if (s->dy < 0) {
            row = (s->dx < 0) ? 3 : 4;
        } else {
            row = (s->dx < 0) ? 5 : 6;
        }
        SPR_setAutoAnimation(s->spr, FALSE);
        spr_set_frame(s->spr, row);
        SPR_setHFlip(s->spr, flip);
        return;
    }
    SPR_setHFlip(s->spr, s->dx < 0);
}

static bool fire_impl(s16 x, s16 y, s16 dir, u8 proj_id, bool aim);

bool eshot_fire(s16 x, s16 y, s16 dir, u8 proj_id)
{
    const bool ok = fire_impl(x, y, dir, proj_id, FALSE);
    eshot_current_owner = 0xFF;     // AI frame scope ends with the call
    return ok;
}

// PC WALL_WALK_SHOOT: the shot is a TARGET_DIRECTION missile, i.e. it leaves
// toward the player instead of straight ahead. The MD art cannot rotate, so it
// stays upright, but the path is the PC's (report 2026-10-24).
bool eshot_fire_aimed(s16 x, s16 y, u8 proj_id)
{
    return fire_impl(x, y, 1, proj_id, TRUE);
}

static bool fire_impl(s16 x, s16 y, s16 dir, u8 proj_id, bool aim)
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
        spr_seen = spr_gen;
        if (!s->spr) return FALSE;
        if (p->traj == TRAJ_CHAIN) SPR_setAutoAnimation(s->spr, FALSE);   // frame = current length
        s->x = x - p->w / 2;
        s->y = y - p->h / 2;
        s->x0 = s->x;
        s->y0 = s->y;
        s->dx = (dir < 0) ? -(s16)p->speed : (s16)p->speed;
        s->dy = 0;
        s->speed = p->speed;
        s->dmg = p->dmg;
        s->w = (p->traj == TRAJ_CHAIN) ? p->speed : p->w;   // a chain grows from nothing
        s->h = p->h;
        s->proj_id = proj_id;
        s->owner = eshot_current_owner;
        s->w0 = p->w;
        s->step_t = 0;
        s->traj = p->traj;
        // PC DIAGONAL_UP/DOWN: one speed step on y per move, on top of the x step
        if (s->traj == TRAJ_DIAG_UP) s->dy = -(s16)p->speed;
        else if (s->traj == TRAJ_DIAG_DOWN) s->dy = (s16)p->speed;
        // PC TRAJECTORY_FREEZE fired by an enemy: freeze_weapon_effect =
        // FREEZE_EFFECT_PLAYER, the player cannot act for FREEZE_DURATION.
        if (s->traj == TRAJ_FREEZE) player_freeze();
        s->dir = (s8)dir;
        s->status = 0;
        s->boom = FALSE;
        s->effect_t = 0;
        s->age = 0;
        s->on = TRUE;
        s->reflected = FALSE;
        eshot_flip(s);          // art faces right: mirror when flying left

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
            eshot_flip(s);
        }
        if (p->traj == TRAJ_TARGET_EXACT) {
            // PC projectilelib.cpp:412: dx = speed toward the player,
            // dy = speed * (target.y + TILESIZE - y) / |target.x - x|, negated
            // when the shot travels left.
            s16 pcx, pcy;
            player_center(&pcx, &pcy);
            const s32 dist_x = (s32)pcx - (s->x + s->w / 2);
            const s32 dist_y = (s32)(pcy + 16) - (s->y + s->h / 2);
            const s32 adx = (dist_x < 0) ? -dist_x : dist_x;
            s->dx = (dist_x < 0) ? -(s16)p->speed : (s16)p->speed;
            if (adx > 0) {
                s32 vy = ((s32)p->speed * dist_y) / adx;
                if (vy > 32767) vy = 32767;
                if (vy < -32768) vy = -32768;
                s->dy = (s16)vy;
            } else {
                s->dy = 0;
            }
            s->traj = TRAJ_LINEAR;      // moves both axes in the default branch
            s->dir = (s->dx < 0) ? -1 : 1;
            eshot_flip(s);
        }
        if (aim) {
            // True 2-D aim: full speed on the dominant axis, proportional on
            // the other (the PC's TARGET_DIRECTION only knows 45 degrees).
            s16 pcx, pcy;
            player_center(&pcx, &pcy);
            s16 adx = (s16)(pcx - (s->x + s->w / 2));
            s16 ady = (s16)(pcy - (s->y + s->h / 2));
            s16 ax = (adx < 0) ? -adx : adx;
            s16 ay = (ady < 0) ? -ady : ady;
            const s16 m = (ax > ay) ? ax : ay;
            if (m > 0) {
                s->dx = (s16)((s32)adx * p->speed / m);
                s->dy = (s16)((s32)ady * p->speed / m);
            }
            s->traj = TRAJ_LINEAR;
            s->dir = (s->dx < 0) ? -1 : 1;
            eshot_flip(s);
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

// PC projectilelib.cpp:677: a live QUAKE projectile shakes the screen one
// QUAKE_SCREEN_MOVE (2px) every QUAKE_TIMER (20ms) and resets it when it
// expires. The PC's damage half (`ia_action_quake_attack`) has no caller in
// that build, so this is the whole observable effect - APE BOT's quake
// "doing nothing" was the port never shaking.
s16 eshots_shake_x(void)
{
    for (u8 i = 0; i < ESHOT_MAX; i++) {
        if (shots[i].on && shots[i].traj == TRAJ_QUAKE)
            return ((shots[i].age / 12) & 1) ? -2 : 2;   // QUAKE_SCREEN_MOVE
    }
    return 0;
}

static bool eshot_solid(s16 x, s16 y)
{
    return stage_terrain_at(stage_current_map, x >> 4, y >> 4) == TERRAIN_SOLID;
}

// 32-step sine, amplitude 48 (PC trajectory SIN uses 3 tiles).
static const s8 sin48[32] = {
    0, 9, 18, 27, 34, 41, 45, 47, 48, 47, 45, 41, 34, 27, 18, 9,
    0, -9, -18, -27, -34, -41, -45, -47, -48, -47, -45, -41, -34, -27, -18, -9,
};

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
        case TRAJ_SIN:
            s->x += s->dx;
            s->y = (s16)(s->y0 + sin48[(s->age >> 1) & 31]);
            break;
        case TRAJ_BOMB:
            if (!s->boom) {
                if (s->age > 60) start_boom(s, FALSE);
            } else {
                if (s->age >= BOOM_FRAMES_BOMB) { kill(s); continue; }
                if (s->spr) spr_set_frame(s->spr, (s->age / 6) % 7);
                if (s->age == 72 || s->age == 144) sfx_play(SFX_EXPLOSION);   // play_repeated_sfx x3
            }
            break;
        case TRAJ_FREEZE:
            // FREEZE: stationary fuse, then a puff and disappear.
            if (s->age > 60) {
                explosion_spawn(s->x, s->y);
                kill(s);
                continue;
            }
            break;
        case TRAJ_CENTERED:
            if (s->age < CIRCLE_FRAMES) {
                // PC projectilelib.cpp:700: held on its owner until the 3.6 s
                // effect timer runs out, then it flies off as LINEAR.
                s16 ox, oy, ow, oh;
                s8 odir;
                if (s->owner < 32 &&
                    enemies_get_box(s->owner, &ox, &oy, &ow, &oh, &odir)) {
                    s->x = (s16)(ox + ow / 2 - s->w / 2);
                    s->y = (s16)(oy + oh / 2 - s->h / 2);
                } else {
                    s->x = s->x0;
                    s->y = s->y0;
                }
            } else {
                s->x += s->dx;
                s->y += s->dy;
            }
            break;
        case TRAJ_ZIGZAG:
            s->x += s->dx;
            s->y += s->dy;
            if (eshot_solid((s->dx > 0) ? s->x + s->w : s->x,
                            s->y + s->h / 2) && s->status < 4) {
                s->dx = (s16)-s->dx;
                s->dir = (s8)-s->dir;
                eshot_flip(s);
                s16 vy = (s->dy < 0) ? -s->dy : s->dy;
                if (vy == 0) vy = (s16)s->speed;
                s->dy = (s->status & 1) ? -vy : vy;
                s->status++;
            }
            break;
        case TRAJ_BOUNCING: {
            // PC projectilelib.cpp:876: horizontal speed, gravity on Y, bounce
            // off the ground (and walls) until it expires.
            s->x += s->dx;
            s->dy = (s16)(s->dy + 1);
            s->y += s->dy;
            const bool wall = eshot_solid((s->dx > 0) ? s->x + s->w : s->x,
                                          s->y + s->h / 2);
            if (wall) {
                s->dx = (s16)-s->dx;
                s->dir = (s8)-s->dir;
                eshot_flip(s);
            }
            if (s->dy > 0 && eshot_solid(s->x + s->w / 2, s->y + s->h)) {
                s->y = (s16)(s->y - s->dy);
                s->dy = (s16)-(s->dy / 2 + 2);      // bounce, losing energy
            } else if (s->dy < 0 && eshot_solid(s->x + s->w / 2, s->y)) {
                s->dy = (s16)(-s->dy / 2);
            }
            if (s->age > 240) { kill(s); continue; }
            break;
        }
        case TRAJ_CHAIN: {
            // PC projectilelib.cpp:643: the chain is attached to its owner,
            // grows by `speed` per update until the art is fully out, then
            // retracts the same way. Only the visible slice hurts.
            if (s->status == 0) {
                s->w = (u8)(s->w + s->speed);
                if (s->w >= s->w0) { s->w = s->w0; s->status = 1; }
            } else {
                if (s->w <= s->speed) { kill(s); continue; }
                s->w = (u8)(s->w - s->speed);
            }
            s16 ox, oy, ow, oh;
            s8 odir;
            if (s->owner < 32 &&
                enemies_get_box(s->owner, &ox, &oy, &ow, &oh, &odir)) {
                s->dir = odir;
                s->x = (s->dir > 0) ? (s16)(ox + ow) : (s16)(ox - s->w);
                s->y = (s16)(oy + oh / 2 - s->h / 2);
            }
            if (s->spr) {
                // frame k = the spiked head plus 8k columns of chain (converter)
                s16 f = (s16)((s->w + 7) / 8) - 1;
                if (f < 0) f = 0;
                spr_set_frame(s->spr, f);
                SPR_setHFlip(s->spr, s->dir < 0);
            }
            break;
        }
        case TRAJ_BOMB_RAIN: {
            // PC projectilelib.cpp:927: the shooter drops BOMB_RAIN_N bombs,
            // spaced across the screen, one every BOMB_RAIN_DELAY (1600ms).
            if (s->owner >= 32) { kill(s); continue; }
            if (++s->step_t >= 96) {                    // 1600ms at 60 Hz
                s->step_t = 0;
                const s16 spacing = (320 - 32) / 4;     // BOMB_RAIN_N = 4
                const s16 nx = (s->dir > 0)
                                   ? (s16)(s->x0 + spacing * (s16)(s->status + 1))
                                   : (s16)(s->x0 - spacing * (s16)(s->status + 1));
                spawn_child(nx, (s16)(s->y0 - 96), s->dir, s->proj_id,
                            TRAJ_FALL_BOMB);
                if (++s->status >= 4) { kill(s); continue; }
            }
            break;
        }
        case TRAJ_LARGE_BEAM: {
            // PC projectilelib.cpp:162/239: 3-frame beam that grows one frame
            // width every LARGE_BEAM_DELAY (20ms, i.e. per frame here). The MD
            // art is a single 48x29 sheet, so the hitbox grows while the
            // sprite already shows the full width.
            if (s->step_t++ >= 1) {
                s->step_t = 0;
                s->status++;
                const s16 fw = (s16)(s->w0 / 3);
                if (s->dir < 0) s->x = (s16)(s->x - fw);
                s->w = (u8)(fw * (2 + s->status));
                if ((s16)s->status > (320 / (fw ? fw : 16))) {
                    kill(s);
                    continue;
                }
            }
            break;
        }
        case TRAJ_FALL_BOMB:
            if (!s->boom) {
                s->y += s->speed;
                if (eshot_solid(s->x + s->w / 2, s->y + s->h) || s->y > 240)
                    start_boom(s, TRUE);
            } else {
                if (s->age >= BOOM_FRAMES_FALL) { kill(s); continue; }
                if (s->spr) spr_set_frame(s->spr, (s->age / 6) % 7);
            }
            break;
        default:                    // LINEAR / DIAG_UP / DIAG_DOWN / fallback
            s->x += s->dx;
            s->y += s->dy;
            break;
        }

        s16 l = s->x, t = s->y, r = s->x + s->w, b = s->y + s->h;

        // Off the camera window only: the PC checks the map only for the
        // ZIGZAG / AHEAD_AND_BACK / bouncing trajectories, so LINEAR, DIAG,
        // ARC, FOLLOW and TARGET shots cross walls.
        if (r < camera.x - 32 || l > camera.x + 352) {
            kill(s);
            continue;
        }

        // PC: a reflected projectile is harmless and keeps flying.
        if (s->reflected) {
            if (s->spr) SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
            continue;
        }

        // bombs only hurt once they are exploding (PC get_damage: 0 before)
        const bool bomb_fuse = (s->traj == TRAJ_BOMB || s->traj == TRAJ_FALL_BOMB) && !s->boom;

        // player collision (PC projectile vs player)
        if (!bomb_fuse && pl < r && pr > l && pt < b && pb > t) {
            // PC classnpc.cpp:518: a shielded player reflects the shot (except
            // bombs/lightning/slash); otherwise it damages and vanishes.
            if (player_is_shielding() && s->traj != TRAJ_BOMB &&
                s->traj != 22 && s->traj != 27) {
                s->reflected = TRUE;
                s->dx = (s16)-s->dx;
                s16 vy = (s->dx < 0) ? -s->dx : s->dx;
                s->dy = (s->y & 1) ? -vy : vy;
                s->traj = (s->y & 1) ? TRAJ_DIAG_UP : TRAJ_DIAG_DOWN;
                continue;
            }
            if (shots_circle_absorb()) {        // star shield: both vanish
                kill(s);
                continue;
            }
            player_take_damage(s->dmg);
            if (s->boom) {                  // the explosion stays and keeps hurting
                if (s->spr) SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
                continue;
            }
            kill(s);
            continue;
        }

        if (s->spr) {
            s16 px = s->x;
            if (s->traj == TRAJ_CHAIN && s->dir < 0)
                px = (s16)(px - (((s16)s->w0 + 7 & ~7) - s->w));   // flipped slice hugs the cell's right edge
            SPR_setPosition(s->spr, px - camera.x, s->y - camera.y);
        }
    }
}

// PC character::is_using_circle_weapon(): the owner has exactly one projectile
// alive and it is a CENTERED one still held around him. While it is, the
// owner takes no damage from the player's shots (classplayer.cpp:738).
bool eshot_owner_has_circle(u8 owner)
{
    u8 n = 0;
    bool circle = FALSE;
    for (u8 i = 0; i < ESHOT_MAX; i++) {
        const EShot *s = &shots[i];
        if (!s->on || s->owner != owner) continue;
        n++;
        circle = (s->traj == TRAJ_CENTERED && s->age < CIRCLE_FRAMES);
    }
    return n == 1 && circle;
}
