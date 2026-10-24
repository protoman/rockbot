#include "shot.h"
#include "sprite_guard.h"
#include "sfx.h"

#include "stage.h"
#include "stage_data.h"
#include "enemy.h"
#include "object.h"
#include "player.h"
#include "explosion.h"
#include "weapon_data.h"
#include "resources.h"

// game_projectile_list.dat: the player shot's sprite/speed/damage/size come
// from the projectile the SELECTED weapon fires (weapon_data[].proj), read out
// of stage_def->enemy_proj. Weapon 0 is the buster (projectile 0).
#define SHOT_CHARGE_SPEED 3
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)
#define SHOT_QUAKE_LIFE 60 // ~1s for the stationary QUAKE shot
#define SHOT_CHAIN_FRAMES 12   // chain out / chain back, ~0.2s each way

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 x0, y0;         // fire origin (ARC)
    s16 dx, dy;
    u8 dmg;
    u8 w, h;    // collision box (from the projectile table)
    u8 traj;
    u16 age;
    u16 effect_t;
    s8 dir;
    u8 status;
    bool on;
    bool pierce;        // full charged shot: PC vanishes_on_hit=false
    u8 weapon;          // weapon that fired it (PC projectile.get_weapon_id)
    bool reflected;     // PC projectile.is_reflected: no more hits
} Shot;

static Shot shots[SHOT_MAX];
static u16 spr_seen;   // sprite-engine generation of the handles below
static u8 cooldown;

static void shot_kill(Shot *s)
{
    if (s->on) {
        spr_release_guarded(&s->spr, spr_seen);
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

    // PC WEAPON_ITEM_COIL / WEAPON_ITEM_JET: no projectile, spawn an item.
    // COIL consumes one unit when placed; JET drains while being ridden.
    if (w == 9 || w == 10) {
        if (!objects_spawn_weapon_item(w == 10)) return;
        if (w == 9) player_use_weapon_ammo();
        sfx_play(SFX_SHOT);
        cooldown = SHOT_COOLDOWN;
        return;
    }

    const EnemyProj *pd = &stage_def->enemy_proj[weapon_data[w].proj];
    // Some ids have no art in the port (e.g. 7 SPIKED CHAIN is skipped by the
    // converter) — fall back to the buster so the weapon still shoots.
    if (!pd->spr || !pd->speed) pd = &stage_def->enemy_proj[0];
    const SpriteDefinition *spr = pd->spr ? pd->spr : &shot_sprite;

    // DYNAMITE FLASH is an instant screen-wide freeze, not a travelling
    // projectile: arm the effect and consume the shot without spawning one.
    if (pd->traj == TRAJ_FREEZE) {
        enemies_freeze(w);
        cooldown = SHOT_COOLDOWN;
        sfx_play(SFX_SHOT);
        if (w != 0) player_use_weapon_ammo();
        return;
    }

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
        spr_seen = spr_gen;
        // The projectile art faces RIGHT (the charged shot's tail is on the
        // left), so mirror it when firing left.
        SPR_setHFlip(s->spr, dir < 0);
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
        if (s->traj == TRAJ_FREEZE) enemies_freeze(w);
        s->age = 0;
        s->effect_t = 0;
        s->dir = (s8)dir;
        s->status = 0;
        s->on = TRUE;
        s->pierce = FALSE;
        s->reflected = FALSE;
        s->weapon = w;      // PC: damage uses the projectile's weapon id
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
    u8 speed = SHOT_CHARGE_SPEED;
    u8 cost = 1;
    s->traj = TRAJ_LINEAR;
    if (level >= 2) {
        // PC update_armor_properties(): ARMS armor replaces the full-charge
        // projectile (super shot / laser / hadouken). Projectile table ids
        // 20..22 are emitted by convert_enemies.py for every stage.
        u8 aid = player_armor_shot_id();
        const EnemyProj *ap = NULL;
        if (aid != 0xFF && aid < stage_def->enemy_proj_max)
            ap = &stage_def->enemy_proj[aid];
        if (ap && ap->spr) {
            spr = ap->spr;
            s->w = ap->w ? ap->w : 16;
            s->h = ap->h ? ap->h : 16;
            s->dmg = ap->dmg ? ap->dmg : 3;
            s->traj = ap->traj;
            speed = ap->speed ? ap->speed : SHOT_CHARGE_SPEED;
            cost = (s->traj == TRAJ_QUAKE) ? 4 : 1;
        } else {
            spr = &shot_charge_full_sprite;
            s->w = 31; s->h = 30;
            s->dmg = 3;
        }
    } else {
        spr = &enemy_shot_semi_sprite;
        s->w = 16; s->h = 16;
        s->dmg = 2;
    }
    s->spr = SPR_addSprite(spr, x - camera.x, y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (!s->spr) return;
    spr_seen = spr_gen;
    spr_set_anim(s->spr, 0);
    SPR_setHFlip(s->spr, dir < 0);   // art faces right (see shot_fire)
    s->x = x;
    s->y = y;
    s->x0 = x;
    s->y0 = y;
    s->dx = (dir < 0) ? -(s16)speed : (s16)speed;
    s->dy = 0;
    s->age = 0;
    s->effect_t = 0;
    s->dir = (s8)dir;
    s->status = 0;
    s->on = TRUE;
    // Only the full charged projectile is permanent on hit; normal weapon
    // projectiles (including damage-3 Desert Storm) vanish on contact.
    s->pierce = (level >= 2);
    s->reflected = FALSE;
    s->weapon = w;
    cooldown = SHOT_COOLDOWN;
    sfx_play(SFX_CHARGE);
    if (w != 0)
        for (u8 i = 0; i < cost; i++) player_use_weapon_ammo();
}

void shots_set_visible(bool visible)
{
    for (u8 i = 0; i < SHOT_MAX; i++)
        if (shots[i].spr)
            SPR_setVisibility(shots[i].spr, visible ? VISIBLE : HIDDEN);
}

static bool shot_solid(s16 x, s16 y)
{
    return stage_terrain_at(stage_current_map, x >> 4, y >> 4) == TERRAIN_SOLID;
}

s16 shot_shake_x(void)
{
    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *s = &shots[i];
        if (s->on && s->traj == TRAJ_QUAKE)
            return ((s->age / 12) & 1) ? -2 : 2;   // QUAKE_SCREEN_MOVE
    }
    return 0;
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
        case TRAJ_CENTERED:
            // MAGIC STAR: held around the player, then released as linear.
            if (s->age < 30) {
                s16 l, t, r, b;
                player_get_box(&l, &t, &r, &b);
                s->x = (s16)((l + r) / 2 - s->w / 2);
                s->y = (s16)((t + b) / 2 - s->h / 2);
            } else {
                s->x += s->dx;
                s->y += s->dy;
            }
            break;
        case TRAJ_FOLLOW: {
            // DESERT STORM homes on the nearest live enemy (PC target pos).
            s16 tx, ty;
            if (enemies_nearest(s->x, s->y, &tx, &ty)) {
                s16 sp = (s->dx < 0) ? -s->dx : s->dx;
                if (sp == 0) sp = 2;
                if (tx > s->x) s->x += sp;
                else if (tx < s->x) s->x -= sp;
                if (ty > s->y) s->y += sp;
                else if (ty < s->y) s->y -= sp;
            } else {
                s->x += s->dx;
                s->y += s->dy;
            }
            break;
        }
        case TRAJ_ZIGZAG:
            s->x += s->dx;
            s->y += s->dy;
            if (shot_solid((s->dx > 0) ? s->x + s->w : s->x, s->y + s->h / 2)) {
                // PC TRAJECTORY_ZIGZAG (SONAR WAVE): linear until a wall,
                // then reverse X only. It must not gain a vertical component.
                s->dx = -s->dx;
                s->dir = -s->dir;
                SPR_setHFlip(s->spr, s->dir < 0);
                s->status++;
            }
            break;
        case TRAJ_BOMB:
            // TIMED BOMB: stationary fuse, then explosion.
            if (s->age > 60) {
                explosion_spawn(s->x, s->y);
                enemies_try_hit(s->x - 20, s->y - 20, s->x + 20, s->y + 20,
                                s->dmg, 0, TRUE, player_weapon());
                shot_kill(s);
                continue;
            }
            break;
        case TRAJ_CHAIN:
            // PC projectilelib.cpp CHAIN (SPIKED CHAIN, weapon 7): the chain
            // shoots out, then retracts and the weapon is spent. It used to
            // fall into the default branch and fly on like a normal shot.
            if (s->age <= SHOT_CHAIN_FRAMES) {
                s->x += s->dx;
            } else if (s->age <= (SHOT_CHAIN_FRAMES * 2)) {
                s->x -= s->dx;
            } else {
                shot_kill(s);
                continue;
            }
            break;
        case TRAJ_FREEZE:
            // DYNAMITE: enemies_freeze() armed the 3.5s PC effect at spawn;
            // the projectile itself just sits there and expires with it.
            if (s->age > 210) { shot_kill(s); continue; }
            break;
        case TRAJ_LASER:
            // HADOUKEN/LASER: bounce off solid walls a few times.
            s->x += s->dx;
            s->y += s->dy;
            if (shot_solid((s->dx > 0) ? s->x + s->w : s->x, s->y + s->h / 2)
                && s->status < 4) {
                s->dx = -s->dx;
                s->dir = -s->dir;
                SPR_setHFlip(s->spr, s->dir < 0);
                s->status++;
            }
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

        // No map collision: the PC only calls check_map_collision() for the
        // ZIGZAG / AHEAD_AND_BACK / bouncing trajectories (projectilelib.cpp).
        // LINEAR, DIAGONAL, ARC, TARGET and FOLLOW simply fly on, so player
        // shots cross walls (user-reported Techno behavior).
        // PC execute_projectiles: once reflected the shot is harmless (it
        // skips both enemy and object collision) and just flies away.
        if (s->reflected) {
            SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
            continue;
        }
        // Bomb/freeze fuses do not collide before they explode.
        if (s->traj == TRAJ_BOMB || s->traj == TRAJ_FREEZE) {
            SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
            continue;
        }

        // destructible walls (OBJ_DESTRUCTIBLE_WALL) stop a non-charged shot
        if (objects_try_hit(s->x, s->y, s->x + s->w - 1, s->y + s->h - 1,
                            s->dmg) && !s->pierce) {
            shot_kill(s);
            continue;
        }

        // enemy hit (shot dies on any hit; full charged shots pierce:
        // PC character.cpp:742 permanent projectiles). Charged shots also
        // ignore shields: PC only reflects when vanishes_on_hit is true.
        u8 hit = enemies_try_hit(s->x, s->y, s->x + s->w - 1, s->y + s->h - 1,
                                 s->dmg, (s->dx < 0) ? -1 : 1, s->pierce,
                                 s->weapon);
        if (hit == 2) {
            // Shielded enemy: PC projectile::reflect() flips direction and
            // turns the shot into a diagonal (up/down by Y parity), then
            // is_reflected stops further collisions.
            s->reflected = TRUE;
            s->dx = -s->dx;
            s16 vy = (s->dx < 0) ? -s->dx : s->dx;
            s->dy = (s->y & 1) ? -vy : vy;
            s->traj = (s->y & 1) ? TRAJ_DIAG_UP : TRAJ_DIAG_DOWN;
            continue;
        }
        if (hit == 1) {
            if (!s->pierce) {
                shot_kill(s);
                continue;
            }
        }

        SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
    }
}
