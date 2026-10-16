#include "enemy.h"

#include "eshot.h"
#include "stage.h"
#include "player.h"
#include "resources.h"

// Gravity for walkers (PC GRAVITY_*); AI actions below drive flyers and
// scripted movement from game_ai_list.dat (see stage0_enemies.h EnemyAi).
#define ENEMY_GRAVITY FIX16(0.25)
#define ENEMY_MAX_FALL FIX16(6)
// Boss intangibility between hits: PC BOSS_HIT_DURATION = 1500ms
// (defines.h:959, enforced in character.cpp:3395), 1500ms = 90 frames @60fps.
#define BOSS_HURT_FRAMES 90
// PC ia_action_jump_ahead arcs to a point; simplified to an impulse hop.
#define ENEMY_JUMP_VY FIX16(-4)
// Stand-in for the attack animation PC waits before firing.
#define AI_SHOT_FRAMES 20

// AI actions used by stage 0 data (PC AI_ACTION_LIST, defines.h:702)
#define AI_WALK        0
#define AI_FLY         1
#define AI_JUMP        2
#define AI_WAIT_RANGE  3
#define AI_SHOT_AHEAD  5
#define AI_SHOT_DIR    6

// PC WALK/FLY option parameters we see in stage 0 (defines.h:778/795)
#define AI_WALK_TURN      1
#define AI_WALK_TO_PLAYER 2
#define AI_WALK_TURN_PLYR 4
#define AI_FLY_TURN       1
#define AI_FLY_UP         2
#define AI_FLY_TO_PLAYER  4
#define AI_FLY_TO_SAVED   5
#define AI_FLY_FALL       7
#define AI_FLY_ZIGZAG     15

#define AI_FIRST 0
#define AI_RUN   1
#define AI_WAIT  2

typedef struct {
    const EnemyDef *def;
    Sprite *spr;
    s16 x, y;         // cell origin (matches PC position = art top-left)
    s16 x_min, x_max; // precomputed patrol limits (converter)
    s16 dir;          // -1 / +1 (data 0 = left, 1 = right)
    fix16 vy;
    u8 hp;
    u16 hurt_timer;   // frames of intangibility after a hit (boss only)
    bool alive;
    // --- game_ai_list.dat state machine -----------------------------------
    u8 def_i;         // index into enemy_defs / enemy_ai
    s16 spawn_x, spawn_y;   // FLY TO_SAVED_POINT target
    u8 ai_phase;      // AI_FIRST / AI_RUN / AI_WAIT
    u8 ai_chain;      // current state (0..19)
    u16 ai_wait;      // frames left in AI_WAIT (state go_to_delay)
    u16 act_t;        // frames inside the current action
    s16 dest_x, dest_y;
    u8 jumping;       // JUMP action in the air
    u8 moving;        // walk anim while the action moves the enemy
} Enemy;

static Enemy enemies[ENEMY_MAX];
static u8 enemy_count;
static s16 boss_i = -1;      // index of the is_boss enemy, -1 = none
static bool boss_died;
static bool boss_screen;     // boss visible this frame (gates the HP row)

// TRUE if any tile of the box [l..r) x [t..b) is solid.
static bool box_hits_solid(s16 l, s16 t, s16 r, s16 b)
{
    for (s16 ty = t >> 4; ty <= (b - 1) >> 4; ty++) {
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
            if (stage_terrain_at(stage_current_map, tx, ty) == TERRAIN_SOLID)
                return TRUE;
        }
    }
    return FALSE;
}

static bool on_ground(const Enemy *e, const EnemyDef *d)
{
    s16 b = e->y + d->box_t + d->box_h;
    return box_hits_solid(e->x + d->box_l, b, e->x + d->box_l + d->box_w, b + 1);
}

static s16 sign16(s16 v)
{
    return (v > 0) - (v < 0);
}

// ---- AI core (PC artificial_inteligence.cpp, states only; stage 0 has no
// reactions). Chance rolls and goto chains keep the PC data semantics:
// go_to 0 = re-roll chance, N = state N-1 (1-based, defines.h:758).

static void ai_action_init(Enemy *e)
{
    const EnemyDef *d = e->def;
    const EnemyAi *st = &enemy_ai[e->def_i][e->ai_chain];
    s16 half = (d->box_l + d->box_w) / 2 + (s16)d->range;

    e->moving = 0;
    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR) {
            if (st->param == AI_WALK_TURN) {
                e->dir = -e->dir;
            } else if (player.x + 14 > e->x + half) {
                e->dir = 1;
            } else {
                e->dir = -1;
            }
        } else if (st->param == AI_WALK_TO_PLAYER) {
            e->dest_x = player.x;
            e->dir = sign16(e->dest_x - e->x);
        } else {
            e->dest_x = e->x + e->dir * half;
            if (e->dest_x < e->x_min) e->dest_x = e->x_min;
            if (e->dest_x > e->x_max) e->dest_x = e->x_max;
        }
        break;
    case AI_FLY:
        if (st->param == AI_FLY_TURN) {
            e->dir = -e->dir;
        } else if (st->param == AI_FLY_TO_PLAYER) {
            e->dest_x = player.x;
            e->dest_y = player.y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_TO_SAVED) {
            e->dest_x = e->spawn_x;
            e->dest_y = e->spawn_y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_UP) {
            e->dest_y = e->y - (s16)d->range;
        } else if (st->param == AI_FLY_FALL) {
            e->dest_y = e->y + (s16)d->range;
        } else {                                        // ahead (+ zig-zag)
            e->dest_x = e->x + e->dir * (half);
            e->dest_y = e->spawn_y;
        }
        break;
    default:
        break;
    }
}

// Move one axis toward dest at speed; TRUE when reached or blocked.
static bool step_axis(Enemy *e, s16 *pos, s16 dest, bool horizontal)
{
    const EnemyDef *d = e->def;
    s16 step = (s16)d->speed;
    if (*pos == dest) return TRUE;
    s16 dir = sign16(dest - *pos);
    s16 n = *pos + dir * step;
    if ((dir > 0 && n > dest) || (dir < 0 && n < dest)) n = dest;

    if (horizontal) {
        s16 lead = (dir > 0) ? n + d->box_l + d->box_w - 1 : n + d->box_l;
        if (box_hits_solid(lead, e->y + d->box_t,
                           lead, e->y + d->box_t + d->box_h))
            return TRUE;
        e->dir = dir;
    } else {
        // simple vertical block: stop just at the solid tile
        s16 b = n + d->box_t + d->box_h;
        if (box_hits_solid(e->x + d->box_l, b - (dir > 0 ? 0 : d->box_h),
                           e->x + d->box_l + d->box_w, b))
            return TRUE;
    }
    *pos = n;
    return *pos == dest;
}

// Run one frame of the current action. TRUE = action finished.
static bool ai_action_run(Enemy *e)
{
    const EnemyDef *d = e->def;
    const EnemyAi *st = &enemy_ai[e->def_i][e->ai_chain];

    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR)
            return TRUE;                                // instant (PC anim)
        e->moving = 1;
        return step_axis(e, &e->x, e->dest_x, TRUE);

    case AI_FLY: {
        if (st->param == AI_FLY_TURN) return TRUE;
        e->moving = 1;
        if (st->param == AI_FLY_ZIGZAG)
            e->dest_y = e->spawn_y + (((e->act_t >> 5) & 1) ? 12 : -12);
        bool dx = step_axis(e, &e->x, e->dest_x, TRUE);
        bool dy = step_axis(e, &e->y, e->dest_y, FALSE);
        return dx && dy;
    }

    case AI_JUMP: {
        if (!e->jumping) {
            if (!on_ground(e, d)) return FALSE;         // wait for the floor
            e->vy = ENEMY_JUMP_VY;
            e->jumping = 1;
            e->moving = 1;
            return FALSE;
        }
        // airborne: hop forward at walk speed while the arc plays
        s16 lead = (e->dir > 0) ? e->x + d->box_l + d->box_w - 1
                                : e->x + d->box_l;
        if (!box_hits_solid(lead, e->y + d->box_t,
                            lead, e->y + d->box_t + d->box_h))
            e->x += e->dir * (s16)d->speed;

        e->vy += ENEMY_GRAVITY;
        s16 dy = F16_toInt(e->vy);
        s16 bottom = d->box_t + d->box_h;
        s16 ny = e->y + dy;
        if (dy > 0) {
            for (s16 row = (e->y + bottom) >> 4; row <= ((ny + bottom) >> 4);
                 row++) {
                bool blocked = FALSE;
                for (s16 tx = (e->x + d->box_l) >> 4;
                     tx <= (e->x + d->box_l + d->box_w - 1) >> 4; tx++) {
                    if (stage_terrain_at(stage_current_map, tx, row) ==
                        TERRAIN_SOLID) {
                        ny = (row << 4) - bottom;
                        e->vy = 0;
                        e->jumping = 0;
                        e->moving = 0;
                        return TRUE;                    // landed
                    }
                }
                (void)blocked;
            }
        }
        e->y = ny;
        return FALSE;
    }

    case AI_WAIT_RANGE: {
        s16 dx = player.x - e->x;
        if (dx < 0) dx = -dx;
        return dx <= (s16)d->range;
    }

    case AI_SHOT_AHEAD:
        if (e->act_t == 1) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, e->dir, pid);
        }
        return e->act_t >= AI_SHOT_FRAMES;

    case AI_SHOT_DIR:
        if (e->act_t == 1) {
            e->dir = (player.x + 14 > e->x + d->box_l + d->box_w / 2) ? 1 : -1;
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, e->dir, pid);
        }
        return e->act_t >= AI_SHOT_FRAMES;

    default:
        return TRUE;                                    // unsupported: skip
    }
}

static void ai_pick(Enemy *e, bool initial)
{
    const EnemyAi *tbl = enemy_ai[e->def_i];
    u8 go = tbl[e->ai_chain].go_to;

    if (initial || go == 0) {
        // PC define_ai_next_step: cumulative chance over the 20 states
        s16 r = (s16)(random() % 100);
        s16 sum = 0;
        u8 i;
        for (i = 0; i < 20; i++) {
            sum += tbl[i].chance;
            if (r < sum) break;
        }
        e->ai_chain = (i < 20) ? i : 0;
    } else {
        e->ai_chain = (go <= 20) ? go - 1 : 0;          // 1-based (PC go_to-1)
    }

    e->act_t = 0;
    ai_action_init(e);
    e->ai_phase = AI_RUN;
}

static void ai_tick(Enemy *e)
{
    const EnemyAi *st = &enemy_ai[e->def_i][e->ai_chain];

    if (e->ai_phase == AI_FIRST) {
        ai_pick(e, TRUE);
        return;
    }
    if (e->ai_phase == AI_WAIT) {
        if (e->ai_wait > 0) e->ai_wait--;
        if (e->ai_wait == 0) ai_pick(e, FALSE);
        return;
    }
    // AI_RUN
    e->act_t++;
    if (ai_action_run(e)) {
        e->ai_phase = AI_WAIT;
        e->ai_wait = ((u32)st->delay * 6) / 100;        // PC ms -> 60fps frames
    }
}

void enemies_reset(void)
{
    for (u8 i = 0; i < enemy_count; i++) {
        if (enemies[i].spr) SPR_releaseSprite(enemies[i].spr);
        enemies[i].spr = NULL;
        enemies[i].alive = FALSE;
    }
    enemy_count = 0;
    boss_i = -1;
    boss_died = FALSE;
    boss_screen = FALSE;

    for (u8 i = 0; i < ENEMY_SPAWN_COUNT && enemy_count < ENEMY_MAX; i++) {
        const EnemySpawn *sp = &stage0_enemy_spawns[i];
        if (sp->map != stage_current_map) continue;

        const EnemyDef *d = &enemy_defs[sp->def];
        u8 idx = enemy_count;
        Enemy *e = &enemies[enemy_count++];
        e->def = d;
        e->def_i = sp->def;
        e->x = sp->x;
        e->y = sp->y;
        e->spawn_x = sp->x;
        e->spawn_y = sp->y;
        e->x_min = sp->x_min;
        e->x_max = sp->x_max;
        e->dir = (sp->dir == 0) ? -1 : 1;
        e->vy = 0;
        e->hp = d->hp;
        e->hurt_timer = 0;
        e->ai_phase = AI_FIRST;
        e->ai_chain = 0;
        e->ai_wait = 0;
        e->act_t = 0;
        e->dest_x = sp->x;
        e->dest_y = sp->y;
        e->jumping = 0;
        e->moving = 0;
        if (d->is_boss && boss_i < 0) boss_i = idx;
        e->spr = SPR_addSprite(d->spr, e->x - camera.x, e->y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        e->alive = (e->spr != NULL);
        if (e->spr) {
            // visibility is managed manually every frame in enemies_update:
            // a screen_x > 1023 gets masked to 10 bits in the VDP sprite
            // table (off-right enemies reappeared on the left, sliding at
            // camera speed); AUTO_FAST masked before its test and never hid.
            SPR_setVisibility(e->spr, VISIBLE);
            // rows: 0 = ANIM_STAND, 1 = ANIM_WALK (converter contract)
            SPR_setAnim(e->spr, d->walk_n > 0 ? 1 : 0);
            SPR_setHFlip(e->spr, e->dir < 0);
        }
    }
}

void enemies_update(void)
{
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);

    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;

        if (e->hurt_timer) e->hurt_timer--;

        // ---- visibility (the AI only runs on-screen, like the PC move
        // early-exit; keeps off-screen chasers parked until seen) ---------
        s16 sx = e->x - camera.x;
        s16 sy = e->y - camera.y;
        bool vis = (sx >= -40 && sx < 336 && sy >= -40 && sy < 248);

        if (vis) {
            ai_tick(e);
            // boss hit blink (PC shows a white flash via palette; the boss
            // shares PAL0 with the HUD, so blink instead).
            if (d->is_boss) boss_screen = TRUE;
        } else if (d->is_boss) {
            boss_screen = FALSE;
        }

        // ---- gravity (walkers only; AI JUMP manages its own arc) ---------
        if (!d->fly && !e->jumping) {
            if (e->vy < ENEMY_MAX_FALL) e->vy += ENEMY_GRAVITY;
            s16 dy = F16_toInt(e->vy);
            if (dy > 0) {
                s16 ny = e->y + dy;
                s16 l = e->x + d->box_l;
                s16 r = e->x + d->box_l + d->box_w;
                s16 bottom = d->box_t + d->box_h;
                for (s16 row = (e->y + bottom) >> 4;
                     row <= ((ny + bottom) >> 4); row++) {
                    bool blocked = FALSE;
                    for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
                        if (stage_terrain_at(stage_current_map, tx, row) == TERRAIN_SOLID) {
                            ny = (row << 4) - bottom;
                            e->vy = 0;
                            blocked = TRUE;
                            break;
                        }
                    }
                    if (blocked) break;
                }
                e->y = ny;
                if (e->y > STAGE0_PIXEL_H + 32) {   // fell out of the level
                    SPR_releaseSprite(e->spr);
                    e->spr = NULL;
                    e->alive = FALSE;
                    continue;
                }
            }
        }
        if (e->x < e->x_min) { e->x = e->x_min; }
        if (e->x > e->x_max) { e->x = e->x_max; }

        // ---- draw ----------------------------------------------------------
        if (e->spr) {
            bool vis2 = (sx >= -40 && sx < 336 && sy >= -40 && sy < 248);
            // Boss hit blink (PC shows a white flash via palette; the boss
            // shares PAL0 with the HUD, so blink instead).
            if (e->hurt_timer && ((e->hurt_timer >> 3) & 1)) vis2 = FALSE;
            SPR_setVisibility(e->spr, vis2 ? VISIBLE : HIDDEN);
            SPR_setHFlip(e->spr, e->dir < 0);
            if (d->walk_n > 0)
                SPR_setAnim(e->spr, e->moving ? 1 : 0);
            SPR_setPosition(e->spr, e->x - camera.x, e->y - camera.y);
        }

        // ---- contact damage ------------------------------------------------
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (pl < el + d->box_w && pr > el && pt < et + d->box_h && pb > et)
            player_take_damage(TOUCH_DAMAGE_SMALL);
    }
}

bool enemies_try_hit(s16 l, s16 t, s16 r, s16 b, u8 damage)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (l >= el + d->box_w || r <= el || t >= et + d->box_h || b <= et)
            continue;

        // Boss intangible window (PC BOSS_HIT_DURATION): the shot is
        // consumed but deals no damage.
        if (e->hurt_timer) return TRUE;

        // weakness[0].damage_multiplier (PC classplayer.cpp:747
        // damage x multiplier); 0 = immune to the normal weapon.
        if (d->dmg_mul == 0) return TRUE;
        u8 dmg = damage * d->dmg_mul;

        e->hp = (dmg >= e->hp) ? 0 : e->hp - dmg;
        if (d->is_boss) e->hurt_timer = BOSS_HURT_FRAMES;
        if (e->hp == 0) {
            if (d->is_boss) boss_died = TRUE;
            if (e->spr) SPR_releaseSprite(e->spr);
            e->spr = NULL;
            e->alive = FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

u8 enemies_boss_hp(void)
{
    if (boss_i < 0 || !enemies[boss_i].alive) return 0;
    return enemies[boss_i].hp;
}

bool enemies_boss_on_screen(void)
{
    return boss_i >= 0 && enemies[boss_i].alive && boss_screen;
}

bool enemies_boss_died(void)
{
    return boss_died;
}
