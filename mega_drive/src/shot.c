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
#define SHOT_CHARGE_SPEED 5   // fallback; the table's value (5) is used
#define SHOT_COOLDOWN 10   // frames between shots (design choice, not in data)
#define SHOT_CIRCLE_FRAMES 216 // PC _effect_timer 3600 ms for CENTERED
#define SHOT_QUAKE_LIFE 60 // ~1s for the stationary QUAKE shot

typedef struct {
    Sprite *spr;
    s16 x, y;
    s16 x0, y0;         // fire origin (ARC)
    s16 dx, dy;
    u8 dmg;
    u8 w, h;    // collision box (from the projectile table)
    u8 w0;      // CHAIN: full length (w is the current, growing one)
    u8 traj;
    u16 age;
    u16 effect_t;
    s8 dir;
    s8 dir0;            // direction at fire time (SONAR WAVE returns to its origin)
    u8 status;
    bool on;
    bool pierce;        // continues through the target (no shot does today)
    bool charged;       // semi/full/armor charge: shields do not reflect it
    u8 weapon;          // weapon that fired it (PC projectile.get_weapon_id)
    bool reflected;     // PC projectile.is_reflected: no more hits
} Shot;

// game_projectile_list_v3: SONAR WAVE (trajectory ZIGZAG) has hp 3 = reversals before it ends.
#define SONAR_BOUNCES 3

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
    if (!pd->spr || (!pd->speed && pd->traj != TRAJ_SLASH)) pd = &stage_def->enemy_proj[0];
    // PC player normal_projectile_id: CANDYBOT fires the heart (26), KITTYBOT
    // the claw slash (35); the others keep the buster.
    if (w == 0) {
        const s8 nid = player_normal_proj_id();
        if (nid > 0 && nid < stage_def->enemy_proj_max) {
            const EnemyProj *np = &stage_def->enemy_proj[nid];
            if (np->spr && (np->speed || np->traj == TRAJ_SLASH)) pd = np;
        }
    }
    // PC classplayer.cpp:355 / inc_effect_weapon_status(): with a MAGIC STAR
    // held around the player, the next press releases it instead of making a
    // new one.
    if (pd->traj == TRAJ_CENTERED) {
        for (u8 i = 0; i < SHOT_MAX; i++) {
            Shot *h = &shots[i];
            if (h->on && h->traj == TRAJ_CENTERED && h->status == 0 &&
                h->age < SHOT_CIRCLE_FRAMES) {
                h->status = 1;
                h->dir = (s8)dir;
                h->dx = (dir < 0) ? -(s16)((h->dx < 0) ? -h->dx : h->dx)
                                  : (s16)((h->dx < 0) ? -h->dx : h->dx);
                cooldown = SHOT_COOLDOWN;
                sfx_play(SFX_SHOT);
                return;
            }
        }
    }
    // PC classplayer.cpp:348: while a FREEZE effect runs the weapon button
    // shoots the normal projectile (no ammo); once it ends the next press
    // starts the effect again.
    if (pd->traj == TRAJ_FREEZE && enemies_freeze_active()) {
        w = 0;
        pd = &stage_def->enemy_proj[0];
    }
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
    u8 n = (w == 0) ? player_simultaneous() : 1;   // PC: the 2nd shot only for the normal attack
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
        // QUAKE is an effect (ground damage + screen shake), not a body: the
        // slot only carries its timer, so it never shows art.
        if (pd->traj == TRAJ_QUAKE) SPR_setVisibility(s->spr, HIDDEN);
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
        s->w0 = s->w;
        if (s->traj == TRAJ_FREEZE) enemies_freeze(w);
        s->age = 0;
        s->effect_t = 0;
        s->dir = (s8)dir;
        s->dir0 = (s8)dir;
        s->status = 0;
        s->on = TRUE;
        s->pierce = FALSE;
        s->charged = FALSE;
        s->reflected = FALSE;
        s->weapon = w;      // PC: damage uses the projectile's weapon id
        if (s->traj == TRAJ_SLASH) {
            // PC TRAJECTORY_SLASH: no travel, follows the owner (x -/+ 20, y + 6)
            // and ends with its animation (a hit never consumes it).
            s->pierce = TRUE;
            s->dx = 0;
            SPR_setAutoAnimation(s->spr, TRUE);
        }
        if (s->traj == TRAJ_CHAIN) {
            // PC consume_projectile(): the chain is never consumed by a hit, it
            // only grows and retracts, attached to the player.
            s->pierce = TRUE;
            s->w = pd->speed;
            SPR_setAutoAnimation(s->spr, FALSE);
        }
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

// PC projectiles 10 (CHARGED) and 11 (SEMI-CHARGED): speed, hitbox and damage
// come from game_projectile_list_v3.dat (5 px/frame, 31x25 / 16x16, 3 / 2).
// The fallbacks are those same values, for a stage table that stops short.
#define CHARGE_FULL_PROJ 10
#define CHARGE_SEMI_PROJ 11
static void charge_proj(u8 id, Shot *s, u8 *speed, u8 w, u8 h, u8 dmg)
{
    const EnemyProj *p = (id < stage_def->enemy_proj_max)
                             ? &stage_def->enemy_proj[id] : NULL;
    s->w = (p && p->w) ? p->w : w;
    s->h = (p && p->h) ? p->h : h;
    s->dmg = (p && p->dmg) ? p->dmg : dmg;
    if (p && p->speed) *speed = p->speed;
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
        // PC _charged_shot_projectile_id: the ARMS armour's projectile replaces
        // the player's own full-charge one (KITTYBOT: 24, ROCKBOT: 10 = default).
        if (aid == 0xFF && player_full_proj_id() > 0 &&
            player_full_proj_id() != CHARGE_FULL_PROJ)
            aid = (u8)player_full_proj_id();
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
            charge_proj(CHARGE_FULL_PROJ, s, &speed, 31, 25, 3);
        }
    } else {
        spr = &enemy_shot_semi_sprite;
        charge_proj(CHARGE_SEMI_PROJ, s, &speed, 16, 16, 2);
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
    s->dir0 = (s8)dir;
    s->status = 0;
    s->on = TRUE;
    // Only the full charged projectile is permanent on hit; normal weapon
    // projectiles (including damage-3 Desert Storm) vanish on contact.
    // Report 2026-10-24: a charged shot must be CONSUMED by the enemy it
    // hits. It used to pierce for level >= 2, so a full charge flew through
    // the whole row.
    s->pierce = FALSE;
    s->charged = TRUE;      // vanishes on hit, but is never reflected by a shield
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
            SPR_setVisibility(shots[i].spr, (visible && shots[i].traj != TRAJ_QUAKE)
                                            ? VISIBLE : HIDDEN);
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
            enemies_quake_hit(s->weapon);
            break;
        case TRAJ_ARC:
            s->x += s->dx;
            s->y = s->y0 - arc_offset(s->x - s->x0);
            break;
        case TRAJ_CENTERED:
            // MAGIC STAR: held around the player until the next attack press
            // (status) or the 3.6 s effect timer, then released as linear
            // (PC projectilelib.cpp:700).
            if (s->age < SHOT_CIRCLE_FRAMES && s->status == 0) {
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
        case TRAJ_ZIGZAG: {
            s->x += s->dx;
            s->y += s->dy;
            // PC TRAJECTORY_ZIGZAG (SONAR WAVE): linear until a wall, then reverse
            // X only (no vertical component). Each reversal is inc_zigzag_status():
            // status++ and the wave ends once status > projectile hp. A wave fired
            // to the right also counts its return to the fire point.
            bool hit = shot_solid((s->dx > 0) ? s->x + s->w : s->x, s->y + s->h / 2);
            if (s->dir0 != s->dir && s->dir < 0 && s->x <= s->x0) hit = TRUE;
            if (hit) {
                if (++s->status > SONAR_BOUNCES) { shot_kill(s); continue; }
                s->dx = -s->dx;
                s->dir = -s->dir;
                SPR_setHFlip(s->spr, s->dir < 0);
            }
            break;
        }
        case TRAJ_BOMB:
            // TIMED BOMB: stationary fuse, then explosion.
            if (s->age > 60) {
                explosion_spawn(s->x, s->y);
                enemies_try_hit(s->x - 20, s->y - 20, s->x + 20, s->y + 20,
                                s->dmg, 0, TRUE, player_weapon());
                // PC: the bomb's explosion (effect_n == 1) is what breaks an
                // OBJ_DESTRUCTIBLE_WALL - no other player shot does.
                objects_try_hit(s->x - 20, s->y - 20, s->x + 20, s->y + 20,
                                s->dmg, TRUE);
                shot_kill(s);
                continue;
            }
            break;
        case TRAJ_CHAIN: {
            // PC projectilelib.cpp:643: attached to the player (x + 5, +16 when
            // facing right, y + 14), grows by `speed` per update until the art
            // is fully out, then retracts; a hit never consumes it. Only the
            // visible slice hurts (frame k = the spiked head + 8k columns of
            // chain, built by the converter, as for the enemy chain in eshot.c).
            const u8 spd = (u8)((s->dx < 0) ? -s->dx : s->dx);
            if (s->status == 0) {
                s->w = (u8)(s->w + spd);
                if (s->w >= s->w0) { s->w = s->w0; s->status = 1; }
            } else {
                if (s->w <= spd) { shot_kill(s); continue; }
                s->w = (u8)(s->w - spd);
            }
            s->dir = (s8)player.facing;
            const s16 anchor = player.x + ((s->dir > 0) ? PLAYER_BOX_R : PLAYER_BOX_L);
            s->x = (s->dir > 0) ? anchor : (s16)(anchor - s->w);
            s->y = player.y + 14;
            SPR_setHFlip(s->spr, s->dir < 0);
            spr_set_frame(s->spr, (s16)((s->w - 1) >> 3));
            break; }
        case TRAJ_PROGRESSIVE:
            // FLOWER (PC projectilelib.cpp:762): steps ahead only when the
            // animation restarts (every ~100 ms) and drops to the ground first.
            if ((s->age % 6) == 0) {
                s->x += s->dx;
                for (u16 i = 0; i < 240; i++) {
                    if (stage_terrain_at(stage_current_map, (s->x + s->w / 2) >> 4,
                                         (s->y + s->h) >> 4) != TERRAIN_UNBLOCKED)
                        break;
                    s->y++;
                }
            }
            break;
        case TRAJ_SLASH: {
            const s16 frames = s->spr->definition->animations[0]->numFrame;
            if (s->age > frames * 6) { shot_kill(s); continue; }    // PC: 100 ms per frame
            s->dir = (s8)player.facing;
            s->x = player.x + ((s->dir > 0) ? 20 : -20);
            s->y = player.y + 6;
            SPR_setHFlip(s->spr, s->dir < 0);
            break; }
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
        // Bomb/freeze fuses do not collide before they explode; QUAKE never
        // collides (PC `continue`s after damage_ground_npcs()): its damage is
        // dealt from the ground check in the TRAJ_QUAKE case above.
        if (s->traj == TRAJ_BOMB || s->traj == TRAJ_FREEZE ||
            s->traj == TRAJ_QUAKE) {
            SPR_setPosition(s->spr, s->x - camera.x, s->y - camera.y);
            continue;
        }

        // destructible walls (OBJ_DESTRUCTIBLE_WALL) stop a non-charged shot
        if (objects_try_hit(s->x, s->y, s->x + s->w - 1, s->y + s->h - 1,
                            s->dmg, FALSE) && !s->pierce) {
            shot_kill(s);
            continue;
        }

        // enemy hit (shot dies on any hit; full charged shots pierce:
        // PC character.cpp:742 permanent projectiles). Charged shots also
        // ignore shields: PC only reflects when vanishes_on_hit is true.
        u8 hit = enemies_try_hit(s->x, s->y, s->x + s->w - 1, s->y + s->h - 1,
                                 s->dmg, (s->dx < 0) ? -1 : 1,
                                 s->pierce || s->charged, s->weapon);
        if (hit == 2) {
            // Shielded enemy: PC projectile::reflect() flips direction and
            // turns the shot into a diagonal (up/down by Y parity), then
            // is_reflected stops further collisions.
            s->reflected = TRUE;
            sfx_play(SFX_REFLECT);          // PC projectilelib.cpp:1248
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

        s16 px = s->x;
        if (s->traj == TRAJ_CHAIN && s->dir < 0)   // the flipped slice hugs the right edge of its 8-aligned cell
            px = (s16)(px - (((s16)s->w0 + 7 & ~7) - s->w));
        SPR_setPosition(s->spr, px - camera.x, s->y - camera.y);
    }
}

// PC character::is_using_circle_weapon() for the player: a held MAGIC STAR eats
// one enemy projectile (both vanish, classnpc.cpp:534). TRUE = absorbed.
bool shots_circle_absorb(void)
{
    for (u8 i = 0; i < SHOT_MAX; i++) {
        Shot *h = &shots[i];
        if (h->on && h->traj == TRAJ_CENTERED && h->status == 0 &&
            h->age < SHOT_CIRCLE_FRAMES) {
            shot_kill(h);
            return TRUE;
        }
    }
    return FALSE;
}
