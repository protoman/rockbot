#include "enemy.h"
#include "sfx.h"

#include "eshot.h"
#include "stage.h"
#include "explosion.h"
#include "object.h"
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
#define AI_SHOT_INVERT 7
#define AI_AIR_WALK    8
#define AI_FALL        9
#define AI_TELEPORT   10
#define AI_DASH       11
#define AI_GRAB_WALL  12
#define AI_SPAWN_NPC  13
#define AI_CHANGE_MOVE 14
#define AI_REPLACE_NPC 15
#define AI_JUMP_ATTACK_UP 18

// PC WALK/FLY option parameters we see in stage 0 (defines.h:778/795)
#define AI_WALK_TURN      1
#define AI_WALK_FALLING   7   // HORIZONTAL_AHEAD_FALLING: may walk off ledges
#define AI_WALK_TO_PLAYER 2
#define AI_WALK_TO_SAVED  3
#define AI_WALK_RANDOM    5
#define AI_WALK_OPPOSITE  6
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

// PC projectile TRAJECTORY_FREEZE: global freeze_weapon_effect for 3500ms.
// Only enemies whose weakness for the freeze weapon is non-zero are frozen.
#define FREEZE_FRAMES 210
static u16 freeze_t;
static u8 freeze_wpn;

void enemies_freeze(u8 weapon)
{
    freeze_t = FREEZE_FRAMES;
    freeze_wpn = (weapon < 9) ? weapon : 0;
    eshots_reset();              // PC classnpc::execute clean_projectiles
}

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
    u16 respawn_t;    // PC respawn_delay countdown (frames)
    u8 frozen;        // hidden by the DYNAMITE freeze effect
    u8 dropping;      // PC map_present_boss drop from the top
    u8 drop_done;
    s16 drop_target;
    u8 always_ahead;  // PC always_move_ahead (AI state loops on itself)
    u8 auto_jump;     // stepping over a low wall (GOLD CART)
    // --- game_ai_list.dat reactions (PC check_ai_reaction) ----------------
    // react = 0 none, else reaction index+1; while set, ai_action_init/run
    // read the reaction slot instead of the state. dying = hp reached 0 and
    // a REACTION_DEAD is playing (the enemy is removed when it finishes).
    u8 react;
    u8 was_hit;       // damage taken since the last reaction check
    u8 dying;
} Enemy;

static Enemy *enemy_add(u8 def_i, s16 x, s16 y, s16 x_min, s16 x_max, u8 dir);
static Enemy enemies[ENEMY_MAX];
static u8 enemy_count;
static s16 boss_i = -1;      // index of the is_boss enemy, -1 = none
static bool boss_died;
static bool boss_screen;     // boss visible this frame (gates the HP row)

// PC character.cpp:1921-1923: a non-boss NPC may not leave water, and may not
// enter it either - the water enemies (ATLANTIDA fish) stayed on land in the
// port (BUG-018). Bosses are exempt, as on the PC.
static bool enemy_water_ok(const Enemy *e, s16 nx, s16 ny)
{
    const EnemyDef *d = e->def;
    if (d->is_boss) return TRUE;
    const s16 cx = nx + d->box_l + d->box_w / 2;
    const s16 cy = ny + d->box_t + d->box_h / 2;
    const bool now = stage_terrain_at(stage_current_map, cx >> 4, cy >> 4)
                     == TERRAIN_WATER;
    const bool was = stage_terrain_at(stage_current_map,
                                      (e->x + d->box_l + d->box_w / 2) >> 4,
                                      (e->y + d->box_t + d->box_h / 2) >> 4)
                     == TERRAIN_WATER;
    return now == was;
}

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
        u16 t = stage_terrain_at(stage_current_map, tx, row);
        // PC hit_ground (character.cpp:1399): stairs are floor for NPCs, so a
        // walker crossing a ladder column does not fall through it.
        if (t == TERRAIN_STAIR) return TRUE;
        if (t != TERRAIN_SOLID)
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

// ---- AI core (PC artificial_inteligence.cpp). Chance rolls and goto chains
// keep the PC data semantics: go_to 0 = re-roll chance, N = state N-1
// (1-based, defines.h:758).

// PC AI_REACTIONS order, as tested by check_ai_reaction
// (artificial_inteligence.cpp:117).
#define REACT_RANGE  0
#define REACT_HIT    1
#define REACT_DEAD   2
#define REACT_SAME_Y 3
#define REACT_CLOSE  4
#define REACT_SAME_X 5

// The action currently executing: a reaction slot while one is playing,
// otherwise the AI state.
static const EnemyAi *ai_state(const Enemy *e)
{
    if (e->react && stage_def->enemy_react)
        return &stage_def->enemy_react[e->def_i][e->react - 1];
    return &stage_def->enemy_ai[e->def_i][e->ai_chain];
}

static void ai_action_init(Enemy *e)
{
    const EnemyDef *d = e->def;
    const EnemyAi *st = ai_state(e);
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
        } else if (st->param == AI_WALK_TO_SAVED) {
            e->dest_x = e->spawn_x;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_WALK_OPPOSITE) {
            // PC TO_OPPOSITE_DIRECTION: move away from screen centre by
            // walk_range (artificial_inteligence.cpp:1097).
            e->dir = ((e->x - camera.x) > 160) ? -1 : 1;
            e->dest_x = e->x + e->dir * (s16)d->range;
        } else if (st->param == AI_WALK_RANDOM) {
            e->dir = (random() & 1) ? 1 : -1;
            e->dest_x = e->x + e->dir * (s16)d->range;
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

// PC check_can_move_to_point always_move_ahead branch: a walker facing a
// blocked tile with open space directly above it hops once.
static bool ai_wall_ahead(const Enemy *e)
{
    const EnemyDef *d = e->def;
    s16 lead = (e->dir > 0) ? e->x + d->box_l + d->box_w - 1
                            : e->x + d->box_l;
    s16 col = (lead + e->dir) >> 4;
    s16 foot = (e->y + d->box_t + d->box_h - 2) >> 4;
    u16 ahead = stage_terrain_at(stage_current_map, col, foot);
    u16 above = stage_terrain_at(stage_current_map, col, foot - 1);
    return ahead != TERRAIN_UNBLOCKED && ahead != TERRAIN_WATER &&
           (above == TERRAIN_UNBLOCKED || above == TERRAIN_WATER);
}

// Horizontal arc used by the automatic wall hop (same physics as AI_JUMP).
// Returns TRUE on landing so the walk action can continue.
static bool ai_auto_jump(Enemy *e)
{
    const EnemyDef *d = e->def;
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
                e->y = (row << 4) - bottom;
                e->vy = 0;
                e->jumping = 0;
                e->auto_jump = 0;
                e->moving = 0;
                return TRUE;
            }
        }
    }
    e->y = ny;
    return FALSE;
}

// PC remove_self + auto_respawn (classnpc): hide for respawn_delay or release.
static void enemy_die(Enemy *e)
{
    const EnemyDef *d = e->def;
    e->dying = 0;
    e->react = 0;
    if (d->respawn_delay && !d->is_boss && !d->is_sub_boss) {
        if (e->spr) SPR_setVisibility(e->spr, HIDDEN);
        e->respawn_t = (u16)(((u32)d->respawn_delay * 6) / 100);
    } else {
        if (e->spr) SPR_releaseSprite(e->spr);
        e->spr = NULL;
    }
    e->alive = FALSE;
}

// Find the def with this PC npc id (the converter puts it in the bundle for
// AI SPAWN_NPC / REPLACE_NPC / MORPH targets) and place it near the parent.
static Enemy *ai_spawn_npc(Enemy *e, u8 npc_id, bool at_self)
{
    for (u8 i = 0; i < stage_def->enemy_def_count; i++) {
        if (stage_def->enemy_defs[i].npc_id != npc_id) continue;
        return enemy_add(i, at_self ? e->x : e->x + e->dir * 24, e->y,
                         e->x_min, e->x_max, e->dir);
    }
    return NULL;
}

// PC check_ai_reaction: six slots, tested in this order. Only a slot with
// action != 0 can fire; the first match wins. `was_hit` is cleared every
// call, as the PC does at the end of check_ai_reaction.
static s8 ai_reaction_pick(Enemy *e)
{
    if (!stage_def->enemy_react) return -1;
    const EnemyAi *r = stage_def->enemy_react[e->def_i];
    const EnemyDef *d = e->def;
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);
    s32 dx = (s32)player.x - e->x;
    s32 dy = (s32)player.y - e->y;
    s32 dist2 = dx * dx + dy * dy;
    s32 range2 = (s32)d->range * d->range;
    s16 diff_y = (pb) - (e->y + d->box_t + d->box_h);
    if (diff_y < 0) diff_y = -diff_y;
    s16 diff_x = (pl <= e->x)
                 ? (s16)((pr) - e->x)
                 : (s16)(player.x - (e->x + d->box_w));
    if (diff_x < 0) diff_x = -diff_x;

    s8 type = -1;
    if (r[REACT_RANGE].action && dist2 < 64 * 64)           // TILESIZE*4
        type = REACT_RANGE;
    else if (e->hp == 0 && r[REACT_DEAD].action)
        type = REACT_DEAD;
    else if (e->was_hit && r[REACT_HIT].action)
        type = REACT_HIT;
    else if (r[REACT_SAME_Y].action && dist2 < range2 && diff_y < 2)
        type = REACT_SAME_Y;
    else if (r[REACT_SAME_X].action && dist2 < range2 && diff_x < 16)
        type = REACT_SAME_X;
    else if (r[REACT_CLOSE].action &&
             dist2 < (s32)(d->range / 4) * (d->range / 4))
        type = REACT_CLOSE;   // PC: dist < walk_range/4

    e->was_hit = FALSE;
    if (type < 0) return -1;

    e->react = (u8)(type + 1);
    e->act_t = 0;
    e->ai_phase = AI_RUN;
    ai_action_init(e);
    return type;
}

// The reaction action finished: PC follows reactions[type].go_to; 0 means
// "back to the chance roll", N jumps to state N-1 and its own go_to chain
// takes over. A death reaction removes the enemy.
static void ai_reaction_end(Enemy *e)
{
    const EnemyAi *r = ai_state(e);
    u8 go = r->go_to;
    u16 delay = r->delay;
    e->react = 0;

    if (e->dying) {
        enemy_die(e);
        return;
    }
    if (go > 0 && go <= 20) {
        e->ai_chain = go - 1;
        e->act_t = 0;
        ai_action_init(e);
        e->always_ahead = (stage_def->enemy_ai[e->def_i][e->ai_chain].go_to
                           == e->ai_chain + 1);
        e->ai_phase = AI_RUN;
    } else {
        e->ai_phase = AI_WAIT;
        e->ai_wait = ((u32)delay * 6) / 100;
    }
}

// Run one frame of the current action. TRUE = action finished.
static bool ai_action_run(Enemy *e)
{
    const EnemyDef *d = e->def;
    const EnemyAi *st = ai_state(e);

    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR)
            return TRUE;                                // instant (PC anim)
        if (e->auto_jump) {
            ai_auto_jump(e);
            return FALSE;               // continue walking after landing
        }
        if (e->always_ahead && !d->fly && on_ground(e, d) &&
            ai_wall_ahead(e)) {
            e->jumping = 1;
            e->auto_jump = 1;
            e->vy = ENEMY_JUMP_VY;
            e->moving = 1;
            return FALSE;
        }
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
        // PC execute_ai_action_trow_projectile: only a MOVING enemy turns to
        // face the player. A stationary shooter keeps the editor-placed
        // direction (the flipped_enemy bug).
        if (st->action == AI_SHOT_DIR && d->speed && e->act_t == 1)
            e->dir = (player.x + 14 > e->x + d->box_l + d->box_w / 2) ? 1 : -1;
        if (e->act_t == 1 + (dur * 3) / 10) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, e->dir, pid);
        }
        return e->act_t >= dur;
    }

    case AI_SHOT_INVERT: {
        // PC SHOT_PROJECTILE_INVERT_DIRECTION: same attack animation, fires
        // opposite to the current facing.
        u16 dur = AI_SHOT_FRAMES;
        if (d->attack_n) {
            dur = (u16)(((u32)d->attack_ms * 6) / 100);
            if (dur < AI_SHOT_FRAMES) dur = AI_SHOT_FRAMES;
            e->shooting = 1;
        }
        if (e->act_t == 1 + (dur * 3) / 10) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, -e->dir, pid);
        }
        return e->act_t >= dur;
    }

    case AI_AIR_WALK:
        e->moving = 1;
        if (st->param == 0) e->dir = -1;
        else if (st->param == 1) e->dir = 1;
        else e->dir = (player.x > e->x) ? 1 : -1;
        e->x += e->dir * (s16)d->speed;
        return on_ground(e, d);

    case AI_FALL:
        e->jumping = 1;                    // suppress gravity while descending
        e->y += (s16)d->speed;
        if (feet_solid(e, (e->y + d->box_t + d->box_h) >> 4)) {
            e->jumping = 0;
            return TRUE;
        }
        return e->y > (stage_def->map_h * stage_def->tile) + 32;

    case AI_JUMP_ATTACK_UP: {
        if (!e->jumping) {
            if (!on_ground(e, d)) return FALSE;
            e->vy = ENEMY_JUMP_VY;
            e->jumping = 1;
            e->moving = 1;
            return FALSE;
        }
        s16 lead = (e->dir > 0) ? e->x + d->box_l + d->box_w - 1
                                : e->x + d->box_l;
        if (!box_hits_solid(lead, e->y + d->box_t,
                            lead, e->y + d->box_t + d->box_h))
            e->x += e->dir * (s16)d->speed;
        if (e->act_t == 8 && st->param >= 0) {
            eshot_fire(e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, e->dir, (u8)st->param);
        }
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
                    return TRUE;
                }
            }
        }
        e->y = ny;
        return FALSE;
    }

    case AI_TELEPORT:
        // PC teleports around the player; keep it close but on the parent's
        // patrol span so it cannot end up inside a wall far away.
        if (e->act_t == 1) {
            s16 nx = player.x + ((random() & 1) ? 56 : -56);
            if (nx < e->x_min) nx = e->x_min;
            if (nx > e->x_max) nx = e->x_max;
            e->x = nx;
            e->y = player.y - 16;
            sfx_play(SFX_TELEPORT);
        }
        return e->act_t >= 20;

    case AI_DASH: {
        // Quick burst in the facing direction (PC dash action).
        if (e->act_t == 1) e->dest_x = e->x + e->dir * 64;
        e->moving = 1;
        s16 nx = e->x + e->dir * 3;
        s16 lead = (e->dir > 0) ? nx + d->box_l + d->box_w - 1 : nx + d->box_l;
        if (!box_hits_solid(lead, e->y + d->box_t,
                            lead, e->y + d->box_t + d->box_h))
            e->x = nx;
        return e->act_t >= 20;
    }

    case AI_GRAB_WALL:
        // Cling in place (no gravity) for the action's duration.
        e->jumping = 1;
        if (e->act_t >= 30) {
            e->jumping = 0;
            return TRUE;
        }
        return FALSE;

    case AI_SPAWN_NPC:
        if (e->act_t == 1) ai_spawn_npc(e, (u8)st->param, FALSE);
        return TRUE;

    case AI_REPLACE_NPC: {
        // PC AI_ACTION_REPLACE_NPC: spawn the target NPC and remove the
        // parent. DESTRIN TANK's REACTION_DEAD uses it to become DESTRIN SHIP.
        if (e->act_t == 1) {
            Enemy *child = ai_spawn_npc(e, (u8)st->param, TRUE);
            // PC artificial_inteligence.cpp:2294: a stage boss's replacement
            // inherits the boss status (HP bar + stage clear) with full HP.
            if (child && (s16)(e - enemies) == boss_i) {
                child->hp = BOSS_INITIAL_HP;
                boss_i = (s16)(child - enemies);
                boss_died = FALSE;
            }
        }
        enemy_die(e);
        return TRUE;
    }

    case AI_CHANGE_MOVE:
        return TRUE;                    // animation-only action on PC

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
    // PC always_move_ahead(): a state whose go_to points back to itself
    // keeps walking; while doing so a low wall ahead triggers a one-shot jump.
    e->always_ahead = (tbl[e->ai_chain].go_to == e->ai_chain + 1);
    e->ai_phase = AI_RUN;
}

static void ai_tick(Enemy *e)
{
    // a reaction interrupts whatever the state machine was doing
    if (e->react) {
        e->act_t++;
        if (ai_action_run(e) && e->react) ai_reaction_end(e);
        return;
    }
    // PC checks reactions before the action timer, every frame.
    if (ai_reaction_pick(e) >= 0) return;
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
        const EnemyAi *st = ai_state(e);
        e->ai_phase = AI_WAIT;
        e->ai_wait = ((u32)st->delay * 6) / 100;        // PC ms -> 60fps frames
    }
}

static Enemy *enemy_add(u8 def_i, s16 x, s16 y, s16 x_min, s16 x_max, u8 dir)
{
    if (enemy_count >= ENEMY_MAX) return NULL;
    const EnemyDef *d = &stage_def->enemy_defs[def_i];
    u8 idx = enemy_count;
    Enemy *e = &enemies[enemy_count++];
    e->def = d;
    e->def_i = def_i;
    e->x = x;
    e->y = y;
    e->spawn_x = x;
    e->spawn_y = y;
    e->x_min = x_min;
    e->x_max = x_max;
    e->dir = (dir == 0) ? -1 : 1;
    e->vy = 0;
    snap_out_of_floor(e);
    e->hp = d->hp;
    e->hurt_timer = 0;
    e->ai_phase = AI_FIRST;
    e->ai_chain = 0;
    e->ai_wait = 0;
    e->react = 0;
    e->was_hit = 0;
    e->dying = 0;
    e->act_t = 0;
    e->dest_x = x;
    e->dest_y = y;
    e->jumping = 0;
    e->moving = 0;
    e->move_acc = 0;
    e->respawn_t = 0;
    e->frozen = 0;
    e->dropping = 0;
    e->drop_done = (d->is_boss == 0);
    e->drop_target = e->y;
    e->always_ahead = 0;
    e->auto_jump = 0;
    if (d->is_boss && boss_i < 0) boss_i = idx;
    e->spr = SPR_addSprite(d->spr, e->x - camera.x, e->y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    e->alive = (e->spr != NULL);
    if (e->spr) {
        SPR_setVisibility(e->spr, HIDDEN);
        SPR_setAnim(e->spr, d->walk_n > 0 ? 1 : 0);
        SPR_setHFlip(e->spr, e->dir < 0);
    }
    return e;
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
        enemy_add(sp->def, sp->x, sp->y, sp->x_min, sp->x_max, sp->dir);
    }
}

void enemies_update(void)
{
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);

    if (freeze_t) freeze_t--;
    const bool freeze_on = (freeze_t != 0);

    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        const s16 pre_x = e->x;
        const s16 pre_y = e->y;
        if (!e->alive) {
            if (e->respawn_t && --e->respawn_t == 0) {
                const EnemyDef *rd = e->def;
                e->x = e->spawn_x;
                e->y = e->spawn_y;
                e->hp = rd->hp;
                e->hurt_timer = 0;
                e->ai_phase = AI_FIRST;
                e->ai_chain = 0;
                e->ai_wait = 0;
                e->act_t = 0;
                e->react = 0;
                e->was_hit = 0;
                e->dying = 0;
                e->jumping = 0;
                e->moving = 0;
                e->shooting = 0;
                e->move_acc = 0;
                e->vy = 0;
                e->frozen = 0;
                e->always_ahead = 0;
                e->auto_jump = 0;
                e->alive = (e->spr != NULL);
                if (e->spr) SPR_setVisibility(e->spr, VISIBLE);
            }
            continue;
        }
        const EnemyDef *d = e->def;

        // PC classnpc::execute: a weak enemy skips its AI and is not drawn
        // while freeze_weapon_effect is active.
        if (freeze_on && d->weak[freeze_wpn]) {
            // Freeze means exactly that: keep the enemy visible but skip its
            // AI/animation/gravity. Hiding it made DYNAMITE FLASH look like
            // every enemy had vanished.
            if (!e->frozen) {
                e->frozen = 1;
                e->moving = 0;
                e->shooting = 0;
            }
            if (e->spr) {
                const s16 fx = e->x - camera.x;
                const s16 fy = e->y - camera.y;
                SPR_setVisibility(e->spr,
                    (fx > -40 && fx < 336 && fy > -40 && fy < 248)
                    ? VISIBLE : HIDDEN);
                SPR_setPosition(e->spr, fx, fy);
            }
            continue;
        }
        if (e->frozen) {
            e->frozen = 0;
            if (e->spr) SPR_setVisibility(e->spr, VISIBLE);
        }

        // PC map_present_boss: when the stage boss first reaches the screen,
        // blink and drop him from the top (the dialog waits for drop_done).
        if (d->is_boss) {
            s16 bsx = e->x - camera.x;
            if (!e->drop_done && !e->dropping &&
                bsx >= -40 && bsx < 336) {
                e->dropping = 1;
                e->y = e->drop_target - 160;
                e->vy = 0;
            }
            if (e->dropping) {
                e->y += 6;
                if (e->y >= e->drop_target) {
                    e->y = e->drop_target;
                    e->dropping = 0;
                    e->drop_done = 1;
                }
                if (e->spr) {
                    s16 dsx = e->x - camera.x;
                    s16 dsy = e->y - camera.y;
                    SPR_setVisibility(e->spr,
                        (dsx >= -40 && dsx < 336 && dsy >= -40 && dsy < 248)
                        ? VISIBLE : HIDDEN);
                    SPR_setPosition(e->spr, dsx, dsy);
                }
                boss_screen = FALSE;
                continue;
            }
        }

        if (e->hurt_timer) e->hurt_timer--;

        // ---- visibility (the AI only runs on-screen, like the PC move
        // early-exit; keeps off-screen chasers parked until seen) ---------
        s16 sx = e->x - camera.x;
        s16 sy = e->y - camera.y;
        bool vis = (sx >= -40 && sx < 336 && sy >= -40 && sy < 248);

        if (vis) {
            ai_tick(e);
            if (!e->alive) continue;    // AI_REPLACE_NPC removed it
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

        // PC character.cpp:1921: a non-boss NPC never crosses the water line.
        if ((e->x != pre_x || e->y != pre_y) &&
            !enemy_water_ok(e, e->x, e->y)) {
            e->x = pre_x;
            e->y = pre_y;
            e->moving = 0;
            if (e->spr) SPR_setPosition(e->spr, e->x - camera.x, e->y - camera.y);
        }

        // ---- contact damage ------------------------------------------------
        if (e->dying) continue;     // already dead, playing the reaction
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
u8 enemies_try_hit(s16 l, s16 t, s16 r, s16 b, u8 damage, s8 shot_dir,
                   bool charged, u8 weapon)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive || e->dying) continue;
        const EnemyDef *d = e->def;
        // A replacement NPC can become the stage boss, so the live index (not
        // the def flag) decides.
        const bool stage_boss = ((s16)i == boss_i);
        if (stage_boss && e->dropping) return 1;
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (l >= el + d->box_w || r <= el || t >= et + d->box_h || b <= et)
            continue;

        // shielded enemy: a vanishing shot is reflected, no damage. Charged
        // shots (vanishes_on_hit = false) pass through the shield and damage
        // the enemy, as in PC classplayer.cpp:708.
        if (!charged && enemy_is_shielded(e, shot_dir)) return 2;

        // Boss intangible window (PC BOSS_HIT_DURATION): the shot is
        // consumed but deals no damage.
        if (e->hurt_timer) return 1;

        // PC classplayer.cpp:747 uses weakness[weapon].damage_multiplier;
        // 0 = immune to that weapon. The PC then `continue`s, so the shot is
        // NOT consumed and keeps travelling. Item weapons (9/10) have no
        // weakness slot and use the buster's entry.
        u8 mul = d->weak[(weapon < 9) ? weapon : 0];
        if (mul == 0) return 0;
        u8 dmg = damage * mul;

        e->hp = (dmg >= e->hp) ? 0 : e->hp - dmg;
        if (stage_boss) e->hurt_timer = BOSS_HURT_FRAMES;
        e->was_hit = 1;                 // PC _was_hit -> AI_REACTION_HIT
        sfx_play(SFX_NPC_HIT);
        if (e->hp == 0) {
            // PC REACTION_DEAD (artificial_inteligence.cpp:184): if the
            // enemy has a death reaction (DESTRIN TANK -> DESTRIN SHIP), play
            // it first; the reaction removes the enemy when it finishes and a
            // stage boss's replacement inherits the boss status, so the fight
            // continues.
            const bool dead_react = (stage_def->enemy_react &&
                stage_def->enemy_react[e->def_i][REACT_DEAD].action);
            sfx_play(stage_boss ? SFX_EXPLOSION : SFX_NPC_KILLED);
            if (stage_boss) {
                if (!dead_react) boss_died = TRUE;
            } else {
                // PC classmap.cpp:2232: a regular enemy's death adds the 32x32
                // explosion puff at its position (the stage boss has its own
                // death sequence). Dead enemies also drop an item, but only
                // while on screen (classMap::drop_item).
                s16 cx = e->x + d->box_l + d->box_w / 2;
                s16 cy = e->y + d->box_t + d->box_h / 2;
                s16 sx = cx - camera.x;
                s16 sy = cy - camera.y;
                if (sx >= 0 && sx < 320 && sy >= 0 && sy < 224)
                    objects_drop_item(cx, cy, d->is_sub_boss != 0);
                explosion_spawn(e->x, e->y);
            }
            if (dead_react) {
                e->dying = 1;
                e->was_hit = 0;
                e->react = REACT_DEAD + 1;
                e->act_t = 0;
                e->ai_phase = AI_RUN;
                ai_action_init(e);
            } else {
                enemy_die(e);
            }
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
    return boss_i >= 0 && enemies[boss_i].alive && boss_screen &&
           enemies[boss_i].drop_done;
}

void enemies_boss_death_pos(s16 *x, s16 *y)
{
    *x = 0;
    *y = 0;
    if (boss_i < 0) return;
    const EnemyDef *d = enemies[boss_i].def;
    *x = enemies[boss_i].x + d->box_l + d->box_w / 2;
    *y = enemies[boss_i].y + d->box_t + d->box_h / 2;
}

bool enemies_nearest(s16 x, s16 y, s16 *tx, s16 *ty)
{
    s32 best = 0x7FFFFFFF;
    bool found = FALSE;
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;
        s16 ex = e->x + d->box_l + d->box_w / 2;
        s16 ey = e->y + d->box_t + d->box_h / 2;
        s16 sx = ex - camera.x;
        if (sx < -40 || sx > 360) continue;
        s32 dx = ex - x, dy = ey - y;
        s32 dist = dx * dx + dy * dy;
        if (dist < best) {
            best = dist;
            *tx = ex;
            *ty = ey;
            found = TRUE;
        }
    }
    return found;
}

bool enemies_subboss_alive_left(s16 tile_x)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;
        if (!d->is_boss && !d->is_sub_boss) continue;
        s16 ex = e->x;
        if (ex >= (s16)((tile_x - 20) * 16) && ex <= (s16)(tile_x * 16))
            return TRUE;
    }
    return FALSE;
}

bool enemies_boss_died(void)
{
    return boss_died;
}
