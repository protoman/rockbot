#include "enemy.h"
#include "sfx.h"

#include "eshot.h"
#include "stage.h"
#include "explosion.h"
#include "player.h"
#include "resources.h"

// Gravity for walkers (PC GRAVITY_*); AI actions below drive flyers and
// scripted movement from game_ai_list.dat (see stage_data.h EnemyAi).
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
#define AI_WALK_FALLING   7   // HORIZONTAL_AHEAD_FALLING: may walk off ledges
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
    u8 shooting;      // attack anim while a SHOT action runs
    s16 move_acc;     // 50Hz movement accumulator (PC AI step throttle)
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

// Floor probe for landing/standing, inset 2px per side. A column solid
// across the whole box height is a SIDE WALL (map divider, right map edge),
// not floor: the boss used to "land" on the divider mid-air and re-launch,
// stair-stepping up out of the screen jump by jump.
// PC spawn data can place a big enemy with its box already inside the floor
// (a 36x33 boss box on a walkway whose top is above the spawn y). The gravity
// loop only scans DOWN, so such an enemy fell through and was released — it
// never appeared. Lift it until the box is clear, resting on the first solid
// row it overlapped (PC collision resolution pushes the same way).
static void snap_out_of_floor(Enemy *e)
{
    const EnemyDef *d = e->def;
    s16 l = e->x + d->box_l, r = l + d->box_w;
    s16 t = e->y + d->box_t, b = t + d->box_h;
    s16 top_solid = -1;
    for (s16 row = t >> 4; row <= (b - 1) >> 4; row++) {
        for (s16 col = l >> 4; col <= (r - 1) >> 4; col++) {
            if (stage_terrain_at(stage_current_map, col, row) == TERRAIN_SOLID) {
                if (top_solid < 0) top_solid = row;
                break;
            }
        }
    }
    if (top_solid < 0) return;
    e->y = (s16)(top_solid * 16) - (d->box_t + d->box_h);
}

static bool feet_solid(const Enemy *e, s16 row)
{
    const EnemyDef *d = e->def;
    s16 top = (e->y + d->box_t) >> 4;
    s16 l = e->x + d->box_l + 2;
    s16 r = e->x + d->box_l + d->box_w - 3;
    for (s16 tx = l >> 4; tx <= r >> 4; tx++) {
        if (stage_terrain_at(stage_current_map, tx, row) != TERRAIN_SOLID)
            continue;
        // side wall? solid for the full box height (some open row = floor)
        bool wall = TRUE;
        for (s16 ry = top; ry <= row; ry++) {
            if (stage_terrain_at(stage_current_map, tx, ry) != TERRAIN_SOLID) {
                wall = FALSE;
                break;
            }
        }
        if (wall) continue;
        return TRUE;
    }
    return FALSE;
}

static bool on_ground(const Enemy *e, const EnemyDef *d)
{
    return feet_solid(e, (e->y + d->box_t + d->box_h) >> 4);
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
    const EnemyAi *st = &stage_def->enemy_ai[e->def_i][e->ai_chain];
    s16 half = (d->box_l + d->box_w) / 2 + (s16)d->range;

    e->moving = 0;
    e->shooting = 0;
    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR) {
            if (st->param == AI_WALK_TURN) {
                e->dir = -e->dir;
            } else {
                // PC TURN_TO_PLAYER (artificial_inteligence.cpp:1121-1129):
                // raw position compare, facing set instantly
                e->dir = (player.x > e->x) ? 1 : -1;
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

// Move one axis toward dest; TRUE when reached or blocked. The PC runs the
// AI step every 20ms (50Hz, artificial_inteligence.cpp:276) so one "speed"
// pixel lands every 20ms — moving speed px per 60fps frame would be 20%
// faster than the PC. The accumulator reproduces speed*50 px/s exactly.
static bool step_axis(Enemy *e, s16 *pos, s16 dest, bool horizontal, bool can_fall)
{
    const EnemyDef *d = e->def;
    e->move_acc += (s16)d->speed * 5;
    s16 step = e->move_acc / 6;
    e->move_acc %= 6;
    if (*pos == dest) return TRUE;
    if (step == 0) return FALSE;
    s16 dir = sign16(dest - *pos);
    s16 n = *pos + dir * step;
    if ((dir > 0 && n > dest) || (dir < 0 && n < dest)) n = dest;

    if (horizontal) {
        s16 lead = (dir > 0) ? n + d->box_l + d->box_w - 1 : n + d->box_l;
        if (box_hits_solid(lead, e->y + d->box_t,
                           lead, e->y + d->box_t + d->box_h))
            return TRUE;
        // PC check_can_move_to_point (artificial_inteligence.cpp:2052): a
        // walker samples the tile BELOW the leading edge; a hole there stops
        // it (turn around) unless the option is HORIZONTAL_AHEAD_FALLING.
        // FLYERS are exempt - no ground to fall off (FLEE BEE parked on a
        // stair because its WALK action was treated as a walker).
        if (!can_fall && !d->fly) {
            s16 fcol = (dir > 0) ? ((n + d->box_l + d->box_w) >> 4)
                                 : ((n + d->box_l - 1) >> 4);
            s16 frow = (e->y + d->box_t + d->box_h + 3) >> 4;
            s16 ccol = (e->x + d->box_l + d->box_w / 2) >> 4;
            u16 ahead = stage_terrain_at(stage_current_map, fcol, frow);
            u16 cur = stage_terrain_at(stage_current_map, ccol, frow);
            if ((ahead == TERRAIN_UNBLOCKED || ahead == TERRAIN_WATER)
                && ahead != cur) {
                // PC check_can_move_to_point (artificial_inteligence.cpp:2052)
                // returns "can't move" here - a walker STOPS at the edge and
                // lets the AI decide. The port used to turn around, which
                // fought the AI's own turn state (LARVA's states alternate
                // walk/turn): the enemy flipped left-right in place forever.
                return TRUE;    // action finished, no movement
            }
        }
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
    const EnemyAi *st = &stage_def->enemy_ai[e->def_i][e->ai_chain];

    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR)
            return TRUE;                                // instant (PC anim)
        e->moving = 1;
        return step_axis(e, &e->x, e->dest_x, TRUE,
                             st->param == AI_WALK_FALLING);

    case AI_FLY: {
        if (st->param == AI_FLY_TURN) return TRUE;
        e->moving = 1;
        if (st->param == AI_FLY_ZIGZAG)
            e->dest_y = e->spawn_y + (((e->act_t >> 5) & 1) ? 12 : -12);
        bool dx = step_axis(e, &e->x, e->dest_x, TRUE, TRUE);
        bool dy = step_axis(e, &e->y, e->dest_y, FALSE, TRUE);
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
                if (feet_solid(e, row)) {
                    ny = (row << 4) - bottom;
                    e->vy = 0;
                    e->jumping = 0;
                    e->moving = 0;
                    return TRUE;                    // landed
                }
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
    case AI_SHOT_DIR: {
        // The PC plays ANIM_TYPE_ATTACK while firing (metool's 6-frame
        // shooting motion, the boss's arm swing); the shot leaves inside the
        // animation. Approximated: fire at ~30% of the attack animation,
        // which is its long "shot" frame in the data.
        u16 dur = AI_SHOT_FRAMES;
        if (d->attack_n) {
            dur = (u16)(((u32)d->attack_ms * 6) / 100);
            if (dur < AI_SHOT_FRAMES) dur = AI_SHOT_FRAMES;
            e->shooting = 1;
        }
        if (st->action == AI_SHOT_DIR && e->act_t == 1)
            e->dir = (player.x + 14 > e->x + d->box_l + d->box_w / 2) ? 1 : -1;
        if (e->act_t == 1 + (dur * 3) / 10) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, e->dir, pid);
        }
        return e->act_t >= dur;
    }

    default:
        return TRUE;                                    // unsupported: skip
    }
}

static void ai_pick(Enemy *e, bool initial)
{
    const EnemyAi *tbl = stage_def->enemy_ai[e->def_i];
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
    const EnemyAi *st = &stage_def->enemy_ai[e->def_i][e->ai_chain];

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

    for (u8 i = 0; i < stage_def->enemy_spawn_count && enemy_count < ENEMY_MAX; i++) {
        const EnemySpawn *sp = &stage_def->enemy_spawns[i];
        if (sp->map != stage_current_map) continue;

        const EnemyDef *d = &stage_def->enemy_defs[sp->def];
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
        snap_out_of_floor(e);
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
        e->move_acc = 0;
        if (d->is_boss && boss_i < 0) boss_i = idx;
        e->spr = SPR_addSprite(d->spr, e->x - camera.x, e->y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        e->alive = (e->spr != NULL);
        if (e->spr) {
            // Visibility is managed manually every frame in enemies_update:
            // a screen_x > 1023 gets masked to 10 bits in the VDP sprite
            // table (off-right enemies reappeared on the left, sliding at
            // camera speed); AUTO_FAST masked before its test and never hid.
            // Spawn HIDDEN: stage_load() spawns them while the camera is
            // still at 0, so their "screen" x is their world x (up to 3200)
            // — VISIBLE here drew every enemy of the room wrapped onto the
            // screen for the frame before the first enemies_update().
            SPR_setVisibility(e->spr, HIDDEN);
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
                s16 bottom = d->box_t + d->box_h;
                for (s16 row = (e->y + bottom) >> 4;
                     row <= ((ny + bottom) >> 4); row++) {
                    if (feet_solid(e, row)) {
                        ny = (row << 4) - bottom;
                        e->vy = 0;
                        break;
                    }
                }
                e->y = ny;
                if (e->y > (stage_def->map_h * stage_def->tile) + 32) {   // fell out of the level
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
            if (d->attack_n > 0 && e->shooting)
                SPR_setAnim(e->spr, 2);          // ANIM_ATTACK row
            else if (d->walk_n > 0)
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

void enemies_set_visible(bool visible)
{
    for (u8 i = 0; i < enemy_count; i++)
        if (enemies[i].spr)
            SPR_setVisibility(enemies[i].spr, visible ? VISIBLE : HIDDEN);
}

// Room transition (see transition.c): the outgoing screen is a frozen copy of
// what is on screen, so its enemies slide with it. Visibility is set
// explicitly (clearing SGDK's auto modes) before the position write, which
// would otherwise recompute it.
void enemies_shift_y(s16 dy)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->spr || !e->alive) continue;
        s16 sx = e->x - camera.x;
        s16 sy = e->y - camera.y + dy;
        SPR_setVisibility(e->spr, (sx >= -40 && sx < 336 &&
                                   sy >= -40 && sy < 248) ? VISIBLE : HIDDEN);
        SPR_setPosition(e->spr, sx, sy);
    }
}

// PC character::is_shielded (character.cpp:3357) for the enemy side. The port
// has no separate anim type: `moving` is the walk anim, !moving is STAND.
// shot_dir is the shot's travel direction (1 = right, -1 = left), the PC's
// projectile_direction. SHIELD_FRONT blocks shots coming at the enemy's front
// (PC: projectile_direction != state.direction).
static bool enemy_is_shielded(const Enemy *e, s8 shot_dir)
{
    // The port's displayed anim row is attack (2) while shooting, walk (1)
    // while moving, else stand (0) — the PC's ANIM_TYPE_STAND/WALK/ATTACK.
    bool standing = !e->moving && !e->shooting;
    bool walking = e->moving;

    switch (e->def->shield_type) {
    case SHIELD_FULL:
        return TRUE;
    case SHIELD_FRONT:
    case SHIELD_STAND_AND_WALK:
        return shot_dir != e->dir && (standing || walking);
    case SHIELD_STAND:
        return standing;
    case SHIELD_STAND_FRONT:
        return shot_dir != e->dir && standing;
    case SHIELD_BACK:
        return shot_dir == e->dir;
    default:
        // SHIELD_NO / SHIELD_DISGUISE / SHIELD_USING_SHIELD: enemies in this
        // port have no disguise or shield animation, so nothing to block.
        return FALSE;
    }
}

// Returns: 0 = miss, 1 = hit (damage applied), 2 = blocked by a shield
// (the caller reflects the shot, PC classplayer.cpp:708).
u8 enemies_try_hit(s16 l, s16 t, s16 r, s16 b, u8 damage, s8 shot_dir)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (l >= el + d->box_w || r <= el || t >= et + d->box_h || b <= et)
            continue;

        // shielded enemy: the shot is reflected, no damage
        if (enemy_is_shielded(e, shot_dir)) return 2;

        // Boss intangible window (PC BOSS_HIT_DURATION): the shot is
        // consumed but deals no damage.
        if (e->hurt_timer) return 1;

        // weakness[0].damage_multiplier (PC classplayer.cpp:747
        // damage x multiplier); 0 = immune to the normal weapon.
        if (d->dmg_mul == 0) return 1;
        u8 dmg = damage * d->dmg_mul;

        e->hp = (dmg >= e->hp) ? 0 : e->hp - dmg;
        if (d->is_boss) e->hurt_timer = BOSS_HURT_FRAMES;
        sfx_play(SFX_NPC_HIT);
        if (e->hp == 0) {
            sfx_play(d->is_boss ? SFX_EXPLOSION : SFX_NPC_KILLED);
            if (d->is_boss) {
                boss_died = TRUE;
            } else {
                // PC classmap.cpp:2232: a regular enemy's death adds the 32x32
                // explosion puff at its position (the stage boss has its own
                // death sequence).
                explosion_spawn(e->x, e->y);
            }
            if (e->spr) SPR_releaseSprite(e->spr);
            e->spr = NULL;
            e->alive = FALSE;
        }
        return 1;
    }
    return 0;
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
