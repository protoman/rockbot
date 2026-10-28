#include "enemy.h"
#include "sprite_guard.h"
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
// PC character::damage(): a hit only lands if the last one is older than the
// NPC's hit_duration - HIT_BLINK_ANIMATION_LAPSE (80ms) for every NPC and
// sub-boss, BOSS_HIT_DURATION (1500ms) for the stage boss (classnpc.cpp:41/707).
// Without it a piercing charged shot hit a normal enemy on every frame.
#define NPC_HURT_FRAMES 5
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
// The rest of the PC's AI_ACTION_LIST (defines.h): none of them is used by the
// AI data of stages 0..13 today, but they exist in the editor's action list,
// so a re-exported stage can start using them at any time.
#define AI_SAVE_POINT       4    // marker: nothing to do (state bookkeeping)
#define AI_CIRCLE_PLAYER   16
#define AI_CHANGE_MOVE_REV 17    // animation-only, like CHANGE_MOVE
#define AI_JUMP_ATTACK_AHEAD 19
#define AI_WAIT_RANDOM     20
#define AI_MORPH           21    // become another NPC in place
#define AI_PLAY_SFX        22    // param = SFX id
#define AI_WALL_WALK       23    // cease gravity and walk along the ceiling
#define AI_WALL_WALK_SHOOT 24    // same, firing periodically
#define AI_SHOT_MULTIPLE   25    // param = projectile id, 3-shot spread
#define AI_EXPLODE         26    // explode and die
#define AI_THROW_ITEM      27    // drop a pickup at the enemy position

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
#define AI_FLY_DOWN       3
#define AI_FLY_TO_PLAYER  4
#define AI_FLY_TO_SAVED   5
#define AI_FLY_FALL       7
#define AI_FLY_ZIGZAG     15
#define AI_FLY_TO_RANDOM  6
#define AI_FLY_DRILL_DOWN 8
#define AI_FLY_PLAYER_DIR 10
#define AI_FLY_OPPOSITE_WALL  11
#define AI_FLY_OPPOSITE_SHOOT 12
#define AI_FLY_RANDOM_X   16
#define AI_FLY_RANDOM_Y   17
// RockDroid2's tables use these three (audit_data.py --game RockDroid2):
// TO_PLAYER_X/Y track one axis, DASH_TO_PLAYER homes in at double speed.
#define AI_FLY_TO_PLAYER_X  13
#define AI_FLY_TO_PLAYER_Y  14
#define AI_FLY_DASH         19

// PC AI_ACTION_JUMP_OPTION_LIST (defines.h:833)
// PC AI_ACTION_TELEPORT_OPTION_LIST (defines.h)
#define AI_TELEPORT_LEFT        0
#define AI_TELEPORT_RIGHT       1
#define AI_TELEPORT_TO_PLAYER   2
#define AI_TELEPORT_RANDOM_X    3
#define AI_TELEPORT_RANDOM_Y    4
#define AI_TELEPORT_RANDOM_POINT 5
#define AI_TELEPORT_AHEAD       6

// PC AI_ACTION_DASH_OPTION_LIST
#define AI_DASH_LEFT        0
#define AI_DASH_RIGHT       1
#define AI_DASH_TO_PLAYER   2
#define AI_DASH_OPPOSITE    3
#define AI_DASH_AHEAD       4
#define AI_DASH_NEAR_PLAYER 5

#define AI_JUMP_AHEAD          0
#define AI_JUMP_RANDOM         1
#define AI_JUMP_UP             2
#define AI_JUMP_TO_PLAYER      3
#define AI_JUMP_TO_ROOF        4
#define AI_JUMP_TO_SAVED       5
#define AI_JUMP_TO_PLAYER_DIR  6
#define AI_JUMP_TELEPORT_IN    7
#define AI_JUMP_TELEPORT_OUT   8
#define AI_JUMP_LONG_AHEAD     9
#define AI_JUMP_ONCE          10

#define AI_FIRST 0
#define AI_RUN   1
#define AI_WAIT  2

// PC projectile TRAJECTORY_FREEZE: global freeze_weapon_effect for 3500ms.
// Only enemies whose weakness for the freeze weapon is non-zero are frozen.
#define FREEZE_FRAMES 210
static u16 freeze_t;
static u8 freeze_wpn;

bool enemies_freeze_active(void) { return freeze_t != 0; }

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
    u16 hurt_timer;   // frames of intangibility after a hit (80ms NPC / 1500ms stage boss)
    bool alive;
    // --- game_ai_list.dat state machine -----------------------------------
    u8 def_i;         // index into enemy_defs / enemy_ai
    s16 spawn_x, spawn_y;   // FLY TO_SAVED_POINT target
    s16 base_y;             // FLY AHEAD / ZIGZAG: y at the start of the action
    s16 origin_x;           // FLY OPPOSITE_SHOOT: x where the action began
    u8 shot_n;              // FLY OPPOSITE_SHOOT: volleys fired so far
    u8 ai_phase;      // AI_FIRST / AI_RUN / AI_WAIT
    u8 ai_chain;      // current state (0..19)
    u16 ai_wait;      // frames left in AI_WAIT (state go_to_delay)
    u16 act_t;        // frames inside the current action
    s16 dest_x, dest_y;
    u8 jumping;       // JUMP action in the air
    u8 teleporting;   // TELEPORT action (teleport animation row)
    u16 intro_t;      // frames of ANIM_TYPE_INTRO left (boss landing)
    u8 moving;        // walk anim while the action moves the enemy
    u8 shooting;      // attack anim while a SHOT action runs
    u8 leaping;       // GRAB_WALL flight (JUMP pose)
    u8 clinging;      // hanging on the wall after GRAB_WALL (WALK_AIR pose)
    u8 dashing;       // DASH action (SLIDE pose)
    u8 jatk;          // jump attack action (JUMP_ATTACK pose)
    s16 move_acc;     // 50Hz movement accumulator (PC AI step throttle)
    u16 respawn_t;    // PC respawn_delay countdown (frames)
    u8 frozen;        // hidden by the DYNAMITE freeze effect
    u8 dropping;      // PC map_present_boss drop from the top
    u8 drop_done;
    s16 drop_target;
    u8 always_ahead;  // PC always_move_ahead (AI state loops on itself)
    u8 auto_jump;     // stepping over a low wall (GOLD CART)
    u16 turn_t;       // frames of ANIM_TYPE_TURN left (AI TURN options)
    s8 turn_dir;      // facing applied when the turn animation ends
    u8 jump_opt;      // AI_JUMP option of the current action
    u8 no_gravity;    // PC _ignore_gravity: TO_ROOF / WALL_WALK keep it set
    u8 ghost;         // DRILL_DOWN passes through walls at 1/4 speed
    u8 dash_ai;       // FLY DASH_TO_PLAYER: moves at 2x the enemy speed
    u16 rand_t;       // WAIT_RANDOM_TIME duration / WALL_WALK_SHOOT timer
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

// What an NPC cannot walk through. PC check_map_collision_point blocks NPCs on
// every terrain except UNBLOCKED / STAIR / the difficulty blocks (water has its
// own rule): conveyors, ice, sand and spikes are FLOORS for them. The port only
// knew TERRAIN_SOLID, so a LARVA spawned on a conveyor row (FLOWER FIELDS map 1)
// sat inside the belt art.
#define ENEMY_SOLID(t) ((t) == TERRAIN_SOLID || (t) == TERRAIN_ICE || \
                        (t) == TERRAIN_MOVE_LEFT || (t) == TERRAIN_MOVE_RIGHT || \
                        (t) == TERRAIN_QUICKSAND || (t) == TERRAIN_SPIKE)

// PC character.cpp:1921-1923: a non-boss NPC may not leave water, and may not
// enter it either - the water enemies (SEAHORSE fish) stayed on land in the
// port (BUG-018). Bosses are exempt, as on the PC.
// `ox, oy` is where the enemy was before this frame's AI: the old code read
// e->x/e->y for the "was" probe too, but that was already the new position, so
// the comparison was always true and the rule never fired.
// TRUE when any probe of the body at (x, y) stands in water: the centre for a
// flyer / swimmer, the two feet corners and the centre of the feet line for a
// walker (SPINNASTER used to wade in up to its centre before being stopped).
static bool enemy_in_water(const EnemyDef *d, s16 x, s16 y)
{
    const s16 cx = x + d->box_l + d->box_w / 2;
    if (d->fly) {
        return stage_terrain_at(stage_current_map, cx >> 4,
                                (y + d->box_t + d->box_h / 2) >> 4)
               == TERRAIN_WATER;
    }
    const s16 fy = (y + d->box_t + d->box_h - 1) >> 4;
    const s16 l = (x + d->box_l) >> 4;
    const s16 r = (x + d->box_l + d->box_w - 1) >> 4;
    return stage_terrain_at(stage_current_map, l, fy) == TERRAIN_WATER ||
           stage_terrain_at(stage_current_map, cx >> 4, fy) == TERRAIN_WATER ||
           stage_terrain_at(stage_current_map, r, fy) == TERRAIN_WATER;
}

static bool enemy_water_ok(const Enemy *e, s16 ox, s16 oy,
                           s16 nx, s16 ny)
{
    const EnemyDef *d = e->def;
    if (d->is_boss) return TRUE;
    return enemy_in_water(d, nx, ny) == enemy_in_water(d, ox, oy);
}

// TRUE if any tile of the box [l..r) x [t..b) is solid.
// PC throw_projectile: an enemy that cannot turn (move_speed 0) does not fire a
// targeted projectile (TARGET_DIRECTION / ARC_TO_TARGET / TARGET_EXACT) while
// the player is behind it - SIDERSHOOTER shot backwards.
static bool eshot_fire_e(const Enemy *e, const EnemyDef *d, s16 x, s16 y, s16 dir, u8 pid)
{
    if (d->speed == 0 && pid < stage_def->enemy_proj_max) {
        const u8 t = stage_def->enemy_proj[pid].traj;
        if (t == TRAJ_TARGET_DIRECTION || t == 14 || t == TRAJ_TARGET_EXACT) {
            const s16 ex = e->x + d->box_l + d->box_w / 2;
            const bool right = player.x + 14 >= ex;
            if (right ? e->dir < 0 : e->dir > 0) return FALSE;
        }
    }
    return eshot_fire(x, y, dir, pid);
}

static bool box_hits_solid(s16 l, s16 t, s16 r, s16 b)
{
    for (s16 ty = t >> 4; ty <= (b - 1) >> 4; ty++) {
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
            if (ENEMY_SOLID(stage_terrain_at(stage_current_map, tx, ty)))
                return TRUE;
        }
    }
    return FALSE;
}

// True when the player is on the side a wall walker may shoot at.
static bool wall_shot_ahead(const Enemy *e, const EnemyDef *d)
{
    const s16 cy = e->y + d->box_t + d->box_h / 2;
    const s16 top = e->y + d->box_t, bot = top + d->box_h;
    const s16 l = e->x + d->box_l, r = l + d->box_w - 1;
    if (box_hits_solid(l, top - 1, r, top))     // on a ceiling: shoots down
        return player.y + 14 > cy;
    if (box_hits_solid(l, bot, r, bot + 1))     // on a floor: shoots up
        return player.y + 14 < cy;
    return (player.x > e->x) ? e->dir > 0 : e->dir < 0;
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
            if (ENEMY_SOLID(stage_terrain_at(stage_current_map, col, row))) {
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
        if (!ENEMY_SOLID(t))
            continue;
        // side wall? solid for the full box height (some open row = floor)
        bool wall = TRUE;
        for (s16 ry = top; ry <= row; ry++) {
            if (!ENEMY_SOLID(stage_terrain_at(stage_current_map, tx, ry))) {
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

// PC create_rand_X_point/create_rand_Y_point: a point up to walk_range away.
static s16 ai_rand_off(s16 range)
{
    if (range <= 0) return 0;
    return (s16)((s16)(random() % (u16)(2 * range + 1)) - range);
}

// AI_JUMP TO_ROOF: the enemy `y` at which the box top rests against the first
// solid row above it (PC ia_action_jump_to_roof scans the same way). 0 when
// there is no ceiling, which makes the jump end at the top of the map.
static s16 ai_roof_y(const Enemy *e)
{
    const EnemyDef *d = e->def;
    s16 l = e->x + d->box_l + 2;
    s16 r = e->x + d->box_l + d->box_w - 3;
    s16 top = e->y + d->box_t;
    for (s16 row = (top >> 4) - 1; row >= 0; row--) {
        for (s16 tx = l >> 4; tx <= r >> 4; tx++) {
            if (ENEMY_SOLID(stage_terrain_at(stage_current_map, tx, row)))
                return (s16)((row + 1) * 16 - d->box_t);
        }
    }
    return 0;
}

static void ai_action_init(Enemy *e)
{
    const EnemyDef *d = e->def;
    const EnemyAi *st = ai_state(e);
    s16 half = (d->box_l + d->box_w) / 2 + (s16)d->range;

    e->moving = 0;
    e->shooting = 0;
    e->leaping = 0;
    e->clinging = 0;
    e->dashing = 0;
    e->jatk = 0;
    e->ghost = 0;
    e->dash_ai = 0;
    e->rand_t = 0;
    e->turn_t = 0;
    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR) {
            // PC artificial_inteligence.cpp:1072 + 1462: the turn plays the
            // art's ANIM_TYPE_TURN row and the facing flips when it ends (the
            // flying shielder rotates its shield). Enemies without that row
            // flip instantly, as before.
            s16 nd = (st->param == AI_WALK_TURN) ? (s16)-e->dir
                     : ((player.x > e->x) ? 1 : -1);
            // TURN_TO_PLAYER flips at once on PC (:1099); only HORIZONTAL_TURN
            // waits for the animation.
            if (st->param == AI_WALK_TURN && d->turn_anim != 255 && d->turn_ms) {
                e->turn_dir = (s8)nd;
                e->turn_t = (u16)(((u32)d->turn_ms * 6) / 100);
            } else {
                e->dir = nd;
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
    case AI_JUMP:
        // PC AI_ACTION_JUMP_OPTION_LIST: the option decides where the hop goes.
        // It used to be ignored, so TO_ROOF (APE BOT) became a plain hop and
        // his ceiling walk never happened.
        e->jump_opt = (u8)st->param;
        // Destinations follow PC artificial_inteligence.cpp (jump_to_player /
        // jump_once / jump_ahead / jump_long_ahead / jump_to_random).
        switch (e->jump_opt) {
        case AI_JUMP_TO_PLAYER: {
            // Latched once: the player's centre, skipped under 3 tiles away.
            const s16 mid = e->x + d->box_l + d->box_w / 2;
            const s16 pc = player.x + 14;
            s16 dist = pc - mid;
            if (dist < 0) dist = -dist;
            e->dest_x = (dist < 48) ? e->x : (s16)(e->x + (pc - mid));
            break;
        }
        case AI_JUMP_TO_SAVED:
            e->dest_x = e->spawn_x;
            break;
        case AI_JUMP_RANDOM: {
            // A point on the visible screen at least 4 tiles away.
            s16 rx = e->x;
            for (u8 n = 0; n < 100; n++) {
                rx = (s16)(camera.x + (random() % 320));
                s16 dd = rx - e->x;
                if (dd < 0) dd = -dd;
                if (dd >= 64) break;
            }
            e->dest_x = rx;
            break;
        }
        case AI_JUMP_TO_PLAYER_DIR:
            // jump_once: face the player, then 4 tiles that way.
            e->dir = (player.x > e->x) ? 1 : -1;
            e->dest_x = e->x + e->dir * 64;
            break;
        case AI_JUMP_ONCE:
        case AI_JUMP_TELEPORT_IN:
        case AI_JUMP_TELEPORT_OUT:
            e->dest_x = e->x + e->dir * 64;
            break;
        case AI_JUMP_LONG_AHEAD:
            e->dest_x = e->x + e->dir * 16 * 7;     // LONG_JUMP_DIST tiles
            break;
        case AI_JUMP_AHEAD:
            e->dest_x = e->x + e->dir * 16 * (d->speed ? d->speed : 1);
            break;
        default:
            e->dest_x = e->x;                       // UP / TO_ROOF: no drift
        }
        break;
    case AI_FLY:
        e->dash_ai = (st->param == AI_FLY_DASH) ? 1 : 0;
        if (st->param == AI_FLY_TURN) {
            if (d->turn_anim != 255 && d->turn_ms) {
                e->turn_dir = (s8)-e->dir;
                e->turn_t = (u16)(((u32)d->turn_ms * 6) / 100);
            } else {
                e->dir = -e->dir;
            }
        } else if (st->param == AI_FLY_TO_PLAYER) {
            e->dest_x = player.x;
            e->dest_y = player.y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_TO_PLAYER_X ||
                   st->param == AI_FLY_TO_PLAYER_Y) {
            // PC: one axis only, the other keeps its current value
            // (artificial_inteligence.cpp:1413/1425).
            if (st->param == AI_FLY_TO_PLAYER_X) {
                e->dest_x = player.x;
                e->dest_y = e->y;
                e->dir = sign16(player.x - e->x);
            } else {
                e->dest_x = e->x;
                e->dest_y = player.y;
            }
        } else if (st->param == AI_FLY_DASH) {
            // PC DASH_TO_PLAYER: home in on the player at 2x move_speed.
            e->dest_x = player.x;
            e->dest_y = player.y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_TO_SAVED) {
            e->dest_x = e->spawn_x;
            e->dest_y = e->spawn_y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_UP) {
            e->dest_x = e->x;                           // speed_x 0 on PC
            e->dest_y = e->y - (s16)d->range;
        } else if (st->param == AI_FLY_DOWN) {
            // PC FLY_OPTION_DOWN: dest_y = y + walk_range
            // (artificial_inteligence.cpp:1372). It had no case here, so the 8
            // stages that use it flew horizontally ahead instead.
            e->dest_x = e->x;
            e->dest_y = e->y + (s16)d->range;
        } else if (st->param == AI_FLY_FALL) {
            e->dest_x = e->x;
            // PC FLY_OPTION_FALL: "fall until out of screen or hitting ground
            // (ignores distance)": dest_y = RES_H + height + 2 (:1375); the
            // stepper stops at solid ground on the way.
            e->dest_y = (s16)(stage_def->map_h * stage_def->tile + d->box_h + 2);
        } else if (st->param == AI_FLY_TO_RANDOM) {
            // PC create_rand_X/Y_point within walk_range of the current spot.
            e->dest_x = e->x + ai_rand_off((s16)d->range);
            e->dest_y = e->y + ai_rand_off((s16)d->range);
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_RANDOM_X) {
            e->dest_x = e->x + ai_rand_off((s16)d->range);
            e->dest_y = e->y;
            e->dir = sign16(e->dest_x - e->x);
        } else if (st->param == AI_FLY_RANDOM_Y) {
            e->dest_x = e->x;
            e->dest_y = e->y + ai_rand_off((s16)d->range);
        } else if (st->param == AI_FLY_DRILL_DOWN) {
            // PC: ghostly descent to below the screen at 1/4 speed, walls
            // ignored (it is a drill).
            e->ghost = 1;
            e->dest_x = e->x + ai_rand_off((s16)d->range);
            e->dest_y = (s16)((stage_def->map_h * stage_def->tile) + 64);
        } else if (st->param == AI_FLY_PLAYER_DIR) {
            // PC PLAYER_DIRECTION: face the player and travel walk_range that
            // way (i.e. toward him).
            e->dir = (player.x < e->x) ? -1 : 1;
            e->dest_x = e->x + e->dir * (s16)d->range;
            e->dest_y = e->y;
        } else if (st->param == AI_FLY_OPPOSITE_WALL ||
                   st->param == AI_FLY_OPPOSITE_SHOOT) {
            // PC OPOSITE_WALL/OPOSITE_SHOOT: head for the other half of the
            // screen; the SHOOT variant fires when it arrives.
            e->dir = ((e->x - camera.x) > 160) ? -1 : 1;
            e->dest_x = e->x + e->dir * (s16)d->range;
            e->dest_y = e->y;
            e->shooting = (st->param == AI_FLY_OPPOSITE_SHOOT);
            e->origin_x = e->x;
            e->shot_n = 0;
        } else {                                        // ahead (+ zig-zag)
            // PC HORIZONTAL_AHEAD keeps the CURRENT y (move_to_point speed_y
            // 0); ZIGZAG oscillates around the y it started at. It used the
            // spawn y, pulling a flyer that had moved up/down back to it.
            e->dest_x = e->x + e->dir * (half);
            e->dest_y = e->y;
            e->base_y = e->y;
        }
        break;
    case AI_GRAB_WALL: {
        // PC execute_ai_step_jump_to_wall: face the player, find the wall on
        // that side and leap to it, landing at mid-screen height (RES_H/2)
        // with gravity off. No wall: the action ends at once.
        e->dir = (player.x > e->x) ? 1 : -1;
        s16 wall = -1;
        const s16 row0 = (e->y + d->box_t) >> 4;
        const s16 row1 = (e->y + d->box_t + d->box_h - 1) >> 4;
        for (s16 col = (e->x + d->box_l + d->box_w / 2) >> 4;
             col >= 0 && col < stage_def->map_w; col += e->dir) {
            bool hit = FALSE;
            for (s16 r = row0; r <= row1; r++)
                if (ENEMY_SOLID(stage_terrain_at(stage_current_map, col, r)))
                    hit = TRUE;
            if (hit) { wall = col; break; }
        }
        if (wall < 0) {
            e->dest_x = e->x;
            e->dest_y = e->y;
        } else {
            e->dest_x = (e->dir > 0) ? (s16)(wall * 16 - d->box_l - d->box_w)
                                     : (s16)((wall + 1) * 16 - d->box_l);
            e->dest_y = (s16)(112 - d->box_h / 2 - d->box_t);
        }
        e->dash_ai = 1;                        // PC leaps at move_speed*2
        break;
    }
    case AI_DASH: {
        e->dashing = 1;
        // PC execute_ai_step_dash: a run at 2x speed toward a point
        // walk_range away from the frame centre; the option picks the side.
        const s16 mid = d->box_l + d->box_w / 2;
        s16 side = e->dir;
        switch ((s8)st->param) {
        case AI_DASH_LEFT:      side = -1; break;
        case AI_DASH_RIGHT:     side = 1;  break;
        case AI_DASH_TO_PLAYER:
        case AI_DASH_NEAR_PLAYER:
            side = (player.x > e->x + mid) ? 1 : -1;
            break;
        case AI_DASH_OPPOSITE:  side = ((e->x - camera.x) > 160) ? -1 : 1; break;
        default: {              // AHEAD: turn when the way ahead is shut
            const s16 probe = e->x + e->dir * 8;
            const s16 lead = (e->dir > 0) ? probe + d->box_l + d->box_w - 1
                                          : probe + d->box_l;
            if (box_hits_solid(lead, e->y + d->box_t, lead,
                               e->y + d->box_t + d->box_h))
                side = -e->dir;
        }
        }
        e->dir = (s8)side;
        e->dest_x = e->x + mid + side * (s16)d->range;
        if ((s8)st->param == AI_DASH_OPPOSITE)
            e->dest_x = e->x + side * (s16)d->range;
        e->dash_ai = 1;
        break;
    }
    default:
        break;
    }
}

// Movement scale. The port multiplied the PC's 50 Hz AI step by 5/6, which
// reproduces speed*50 px/s exactly - but the enemies still read as faster than
// the PC in play (report 2026-10-24), so every enemy movement runs at 70% of
// that rate. One place, so steps, wall-walkers and the auto-hop stay in sync.
#define ENEMY_SPEED_NUM 7
#define ENEMY_SPEED_DEN 10

static s16 enemy_step(u8 speed)
{
    s16 v = (s16)(((u16)speed * ENEMY_SPEED_NUM) / ENEMY_SPEED_DEN);
    return v ? v : 1;
}

// Move one axis toward dest; TRUE when reached or blocked. The PC runs the
// AI step every 20ms (50Hz, artificial_inteligence.cpp:276) so one "speed"
// pixel lands every 20ms — moving speed px per 60fps frame would be 20%
// faster than the PC. The accumulator reproduces speed*50 px/s exactly.
// Frame (60 Hz) of a shot action at which the projectile spawns. The PC fires
// when the attack animation reaches the enemy's attack_frame (else its last
// frame): artificial_inteligence.cpp:1185. The data carries that as the time to
// reach it (attack_shot_ms); an enemy with no attack art keeps 30% of the flat
// action. The old flat 30% made every HELMUT (attack_frame 3 = 800ms of 1200)
// shoot ~440ms before the PC does.
static u16 ai_shot_frame(const EnemyDef *d, u16 dur)
{
    if (!d->attack_n) return (u16)((dur * 3) / 10);
    u16 f = (u16)(((u32)d->attack_shot_ms * 6) / 100);
    return (f < dur) ? f : (u16)(dur - 1);
}

static bool step_axis(Enemy *e, s16 *pos, s16 dest, bool horizontal, bool can_fall)
{
    const EnemyDef *d = e->def;
    // DASH_TO_PLAYER runs at twice the enemy's speed (artificial_inteligence
    // .cpp:1517 passes move_speed*2 to move_to_point).
    e->move_acc += enemy_step(d->speed) * 5 * (e->dash_ai ? 2 : 1);
    s16 step = e->move_acc / 6;
    e->move_acc %= 6;
    if (*pos == dest) return TRUE;
    if (step == 0) return FALSE;
    s16 dir = sign16(dest - *pos);
    s16 n = *pos + dir * step;
    if ((dir > 0 && n > dest) || (dir < 0 && n < dest)) n = dest;

    if (e->ghost) {                     // DRILL_DOWN: pass through walls
        *pos = n;
        return *pos == dest;
    }
    if (horizontal) {
        // A walker's patrol limit is a wall for the AI (the state ends there)
        // instead of being enforced after the fact.
        if (!d->fly && !e->always_ahead && (n < e->x_min || n > e->x_max))
            return TRUE;
        s16 lead = (dir > 0) ? n + d->box_l + d->box_w - 1 : n + d->box_l;
        if (box_hits_solid(lead, e->y + d->box_t,
                           lead, e->y + d->box_t + d->box_h) ||
            // solid map objects block NPCs exactly like walls (PC collision
            // loop runs for every character). WOOD SHIELDER used to walk
            // through the destructible block (report 2026-10-24).
            objects_block_box(lead, e->y + d->box_t,
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
        // Only the LEADING edge can block a vertical move: the old test took
        // the whole box at the new position when going up, so a bat resting
        // beside/on a pillar top was "blocked" while moving AWAY from it and
        // its return flight ended on the spot (BAT BALL, DARK MINE).
        s16 edge = (dir > 0) ? n + d->box_t + d->box_h : n + d->box_t;
        if (box_hits_solid(e->x + d->box_l, edge,
                           e->x + d->box_l + d->box_w, edge + 1))
            return TRUE;
    }
    // The water line is a wall for non-boss NPCs (PC character.cpp:1921). Seen
    // from the AI it must be a BLOCK, like a solid tile: reverting the move
    // afterwards left the state unfinished forever (fish chasing a player above
    // the surface stood still).
    {
        const s16 nx = horizontal ? n : e->x;
        const s16 ny = horizontal ? e->y : n;
        if (!enemy_water_ok(e, e->x, e->y, nx, ny)) return TRUE;
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

// Rising enemies (hops, jump attacks) used to pass through ceilings: only the
// landing scan existed. Clip the move so the box top rests under the first
// solid row it would enter.
static bool ceiling_clip(const Enemy *e, s16 *ny)
{
    const EnemyDef *d = e->def;
    if (*ny >= e->y) return FALSE;
    const s16 l = (e->x + d->box_l) >> 4;
    const s16 r = (e->x + d->box_l + d->box_w - 1) >> 4;
    for (s16 row = ((e->y + d->box_t) >> 4) - 1;
         row >= ((*ny + d->box_t) >> 4); row--) {
        for (s16 tx = l; tx <= r; tx++) {
            if (ENEMY_SOLID(stage_terrain_at(stage_current_map, tx, row))) {
                *ny = (s16)(((row + 1) << 4) - d->box_t);
                return TRUE;
            }
        }
    }
    return FALSE;
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
    if (dy < 0 && ceiling_clip(e, &ny)) e->vy = 0;
    e->y = ny;
    return FALSE;
}

static u16 spr_seen;   // sprite-engine generation of the handles below

// PC remove_self + auto_respawn (classnpc): hide for respawn_delay or release.
static void enemy_die(Enemy *e)
{
    const EnemyDef *d = e->def;
    e->dying = 0;
    e->react = 0;
    if (!d->is_boss && (d->respawn_delay || d->is_sub_boss)) {
        // The sub-boss keeps its sprite: PC classmap.cpp:2201 revives it once
        // its body is off screen, so the door it guards never stays free.
        if (e->spr) SPR_setVisibility(e->spr, HIDDEN);
        const u16 t = d->respawn_delay
                          ? (u16)(((u32)d->respawn_delay * 6) / 100) : 1;
        e->respawn_t = t ? t : 1;
    } else {
        spr_release_guarded(&e->spr, spr_seen);
    }
    e->alive = FALSE;
}

// Find the def with this PC npc id (the converter puts it in the bundle for
// AI SPAWN_NPC / REPLACE_NPC / MORPH targets) and place it near the parent.
// PC classMap::get_first_lock_in_direction: walk `dir` from the enemy's side
// along its own row and stop before the first solid tile, at most `limit` px
// (default: the enemy's walk_range). Y is untouched, which is what keeps a
// teleporting enemy on its floor.
static s16 ai_first_wall_x(const Enemy *e, s8 dir, s16 limit)
{
    const EnemyDef *d = e->def;
    if (limit <= 0) limit = d->range ? (s16)d->range : 32;
    const s16 y = (s16)(e->y + d->box_t + d->box_h / 2);
    s16 x = e->x;
    const s16 step = (dir < 0) ? -4 : 4;
    for (s16 moved = 0; moved < limit; moved += 4) {
        const s16 probe = (dir < 0) ? (s16)(x - 4 + d->box_l)
                                    : (s16)(x + d->box_w + 4);
        if (stage_terrain_at(stage_current_map, probe >> 4, y >> 4)
            != TERRAIN_UNBLOCKED &&
            stage_terrain_at(stage_current_map, probe >> 4, y >> 4)
            != TERRAIN_WATER)
            break;
        x = (s16)(x + step);
    }
    return x;
}

static Enemy *ai_spawn_npc(Enemy *e, u8 npc_id, bool at_self)
{
    for (u8 i = 0; i < stage_def->enemy_def_count; i++) {
        const EnemyDef *cd = &stage_def->enemy_defs[i];
        if (cd->npc_id != npc_id) continue;
        const s16 cx = at_self ? e->x : (s16)(e->x + e->dir * 24);
        // The child is a FRESH NPC on the PC (classMap::spawn_map_npc builds a
        // classnpc from the map), so it must not inherit the spawner's patrol
        // span: TOP HAT's span pinned the ANGRY BUNNY it spawns to a few pixels,
        // and every AHEAD move clamped back -> it hopped straight up in place
        // (report 2026-10-24).
        const s16 half = cd->range ? (s16)cd->range : 48;
        const s16 max_x = (s16)(stage_def->map_w * stage_def->tile) - cd->box_w;
        s16 lo = (s16)(cx - half);
        s16 hi = (s16)(cx + half);
        if (lo < 0) lo = 0;
        if (hi > max_x) hi = max_x;
        if (lo > hi) lo = hi;
        // enemy_add() takes the DATA direction byte (0 = left, 1 = right),
        // not the signed runtime dir: passing -1 became 255, i.e. "right", so
        // every child of a left-facing parent spawned facing right.
        return enemy_add(i, cx, e->y, lo, hi, (e->dir < 0) ? 0 : 1);
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
    eshot_current_owner = (u8)(e - enemies);   // CHAIN/BOMB_RAIN owner
    const EnemyDef *d = e->def;
    const EnemyAi *st = ai_state(e);

    switch (st->action) {
    case AI_WALK:
        if (st->param == AI_WALK_TURN || st->param == AI_WALK_TURN_PLYR) {
            if (!e->turn_t) return TRUE;                // no art: instant
            if (--e->turn_t == 0) e->dir = e->turn_dir;
            return FALSE;
        }
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
        // PC check_can_move_to_point skips the hole test entirely while the
        // state loops on itself (_always_move_ahead): such a walker marches on
        // over gaps and falls (PAMPA METALEMA).
        return step_axis(e, &e->x, e->dest_x, TRUE,
                         st->param == AI_WALK_FALLING || e->always_ahead);

    case AI_FLY: {
        if (st->param == AI_FLY_TURN) {
            if (!e->turn_t) return TRUE;
            if (--e->turn_t == 0) e->dir = e->turn_dir;
            return FALSE;
        }
        e->moving = 1;
        if (st->param == AI_FLY_ZIGZAG)
            e->dest_y = e->base_y + (((e->act_t >> 5) & 1) ? 12 : -12);
        else if (st->param == AI_FLY_TO_PLAYER_X) {
            // PC re-reads the player every step and finishes within half a
            // tile (artificial_inteligence.cpp:1569).
            s16 ddx = player.x - e->x;
            if (ddx < 0) ddx = -ddx;
            if (ddx < 8) return TRUE;
            e->dest_x = player.x;
            e->dir = sign16(player.x - e->x);
        } else if (st->param == AI_FLY_TO_PLAYER_Y) {
            e->dest_y = player.y;
        }
        // PC OPOSITE_SHOOT_1: a shot every 120 px travelled from where the
        // action began (the first one right away), not one at the end.
        if (st->param == AI_FLY_OPPOSITE_SHOOT) {
            s16 dist = e->x - e->origin_x;
            if (dist < 0) dist = -dist;
            if (dist > 120 * (s16)e->shot_n) {
                e->shot_n++;
                eshot_fire_e(e, d, e->x + d->box_l + d->box_w / 2,
                           e->y + d->box_t + d->box_h / 2, e->dir, d->shot);
            }
        }
        bool dx = step_axis(e, &e->x, e->dest_x, TRUE, TRUE);
        bool dy = step_axis(e, &e->y, e->dest_y, FALSE, TRUE);
        if (st->param == AI_FLY_OPPOSITE_SHOOT && dx && dy) e->shooting = 0;
        // PC OPOSITE_WALL: turn around on arrival so he does not face the wall.
        if (st->param == AI_FLY_OPPOSITE_WALL && dx && dy) e->dir = -e->dir;
        return dx && dy;
    }

    case AI_JUMP: {
        if (!e->jumping) {
            if (!on_ground(e, d)) return FALSE;         // wait for the floor
            // PC jump_to_player: under 3 tiles away the action is skipped.
            if (e->jump_opt == AI_JUMP_TO_PLAYER && e->dest_x == e->x)
                return TRUE;
            e->jumping = 1;
            e->moving = 1;
            if (e->jump_opt == AI_JUMP_TO_ROOF) {
                // PC ia_action_jump_to_roof(): rise to the ceiling with gravity
                // disabled and KEEP it disabled, which is what lets the next
                // WALK state move him along the roof (APE BOT).
                e->dest_y = ai_roof_y(e);
                e->vy = 0;
                e->no_gravity = 1;
                return FALSE;
            }
            e->vy = ENEMY_JUMP_VY;
            return FALSE;
        }
        if (e->jump_opt == AI_JUMP_TO_ROOF) {
            s16 ny = (s16)(e->y - (s16)(d->speed ? d->speed * 2 : 2));
            if (ny <= e->dest_y) {
                e->y = e->dest_y;
                e->jumping = 0;
                e->moving = 0;              // stays up there: no_gravity
                return TRUE;
            }
            e->y = ny;
            return FALSE;
        }
        // airborne: hop toward the option's destination at walk speed. Uses
        // the shared 50 Hz stepper, so an airborne enemy moves at the same
        // rate as a walking one (the old per-frame `d->speed` was 20% fast)
        // and still stops at walls and map objects.
        if (e->jump_opt != AI_JUMP_UP) {
            e->dir = sign16(e->dest_x - e->x);
            if (e->dir == 0) e->dir = (player.x > e->x) ? 1 : -1;
        }
        step_axis(e, &e->x, e->dest_x, TRUE, TRUE);   // TRUE = arrived


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
        if (dy < 0 && ceiling_clip(e, &ny)) e->vy = 0;
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
            e->shooting = 2;        // 2 = one-shot attack (holds its last frame)
        }
        // PC execute_ai_action_trow_projectile: only a MOVING enemy turns to
        // face the player. A stationary shooter keeps the editor-placed
        // direction (the flipped_enemy bug).
        if (st->action == AI_SHOT_DIR && d->speed && e->act_t == 1)
            e->dir = (player.x + 14 > e->x + d->box_l + d->box_w / 2) ? 1 : -1;
        if (e->act_t == 1 + ai_shot_frame(d, dur)) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire_e(e, d, e->x + d->box_l + d->box_w / 2,
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
            e->shooting = 2;
        }
        if (e->act_t == 1 + ai_shot_frame(d, dur)) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            eshot_fire_e(e, d, e->x + d->box_l + d->box_w / 2,
                       e->y + d->box_t + d->box_h / 2, -e->dir, pid);
        }
        return e->act_t >= dur;
    }

    case AI_AIR_WALK:
        // PC ia_action_air_walk (artificial_inteligence.cpp:826): latch the
        // player's X once, walk there and finish on ARRIVAL. The port waited
        // for `on_ground`, which never happens while the enemy hangs under the
        // ceiling after AI_JUMP TO_ROOF - the APE BOT parked on the roof for
        // the rest of the stage and could not be reached or killed (report
        // 2026-10-24).
        if (e->act_t == 1) {
            e->dest_x = player.x;
            e->dir = sign16(e->dest_x - e->x);
            if (e->dir == 0) e->dir = (player.x > e->x) ? 1 : -1;
        }
        e->moving = 1;
        return step_axis(e, &e->x, e->dest_x, TRUE, TRUE);

    case AI_FALL:
        e->no_gravity = 0;                 // coming down off the ceiling
        e->jumping = 1;                    // suppress gravity while descending
        e->y += (s16)d->speed;
        if (feet_solid(e, (e->y + d->box_t + d->box_h) >> 4)) {
            e->jumping = 0;
            return TRUE;
        }
        return e->y > (stage_def->map_h * stage_def->tile) + 32;

    case AI_JUMP_ATTACK_UP: {
        e->jatk = 1;
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
            e->x += e->dir * enemy_step(d->speed);
        if (e->act_t == 8 && st->param >= 0) {
            eshot_fire_e(e, d, e->x + d->box_l + d->box_w / 2,
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
        if (dy < 0 && ceiling_clip(e, &ny)) e->vy = 0;
        e->y = ny;
        return FALSE;
    }

    case AI_TELEPORT:
        // PC artificial_inteligence.cpp:2457 ia_action_teleport: the option
        // decides the destination and Y NEVER CHANGES for the horizontal ones
        // (the PC reuses the same row, stopping at the first wall within
        // walk_range). The port teleported next to the player at
        // `player.y - 16`, which put GAMBIT inside the floor and dropped it
        // through the ground after two shots (report 2026-10-24).
        if (e->act_t == 1) {
            switch ((s8)st->param) {
            case AI_TELEPORT_LEFT:
                e->x = ai_first_wall_x(e, -1, 0);
                break;
            case AI_TELEPORT_RIGHT:
                e->x = ai_first_wall_x(e, 1, 0);
                break;
            case AI_TELEPORT_AHEAD:
                e->x = ai_first_wall_x(e, e->dir, 0);
                break;
            case AI_TELEPORT_TO_PLAYER:
                e->x = (s16)(player.x + PLAYER_BOX_L + PLAYER_BOX_R) / 2
                       - (s16)(d->box_l + d->box_w / 2);
                break;
            case AI_TELEPORT_RANDOM_X: {
                const s16 off = ai_rand_off((s16)d->range);
                e->x = ai_first_wall_x(e, (off < 0) ? -1 : 1, off);
                break;
            }
            default:
                // RANDOM_Y / RANDOM_POINT: pick a nearby point, then slide the
                // blocked axis back to the first wall.
                e->x = ai_first_wall_x(e, (random() & 1) ? 1 : -1,
                                       (s16)(d->range ? d->range : 32));
                break;
            }
            if (e->x < e->x_min) e->x = e->x_min;
            if (e->x > e->x_max) e->x = e->x_max;
            // PC teleport lands facing the player; without this the boss kept
            // the old facing after reappearing behind the player (BUG-010).
            e->dir = (player.x >= e->x) ? 1 : -1;
            e->teleporting = 1;
            sfx_play(SFX_TELEPORT);
        }
        if (e->act_t >= 20) {
            e->teleporting = 0;     // back to the normal pose (it stayed on the dissolve frame)
            return TRUE;
        }
        return FALSE;

    case AI_DASH: {
        // Ends on arrival or within a tile of the target (PC :1782).
        e->moving = 1;
        s16 left = e->dest_x - e->x;
        if (left < 0) left = -left;
        if (left < 16) return TRUE;
        return step_axis(e, &e->x, e->dest_x, TRUE, TRUE);
    }

    case AI_GRAB_WALL: {
        e->moving = 1;
        e->leaping = 1;
        e->no_gravity = 1;                      // PC _ignore_gravity
        const bool gx = step_axis(e, &e->x, e->dest_x, TRUE, TRUE);
        const bool gy = step_axis(e, &e->y, e->dest_y, FALSE, TRUE);
        if (gx && gy) {
            e->dir = -e->dir;                   // PC: face away from the wall
            e->moving = 0;
            e->leaping = 0;
            e->clinging = 1;                    // PC sets ANIM_TYPE_WALK_AIR here
            return TRUE;
        }
        return FALSE;
    }

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
    case AI_CHANGE_MOVE_REV:
        return TRUE;                    // animation-only action on PC

    case AI_SAVE_POINT:
        return TRUE;                    // bookkeeping marker, nothing to do

    case AI_CIRCLE_PLAYER: {
        // PC CIRCLE_PLAYER: orbit the player at walk_range. One step every
        // ~10 frames around a 16-point circle (the PC uses a sin/cos point
        // each AI step; the visual is the same).
        static const s8 c16[16] = {
            16, 15, 11, 6, 0, -6, -11, -15,
            -16, -15, -11, -6, 0, 6, 11, 15
        };
        s16 r = (s16)d->range;
        u8 a = (u8)(((e->act_t / 10) + (e->def_i & 7)) & 15);
        e->dest_x = (s16)(player.x + (c16[a] * r) / 16);
        e->dest_y = (s16)(player.y + (c16[(a + 4) & 15] * r) / 16);
        e->moving = 1;
        e->dir = sign16(e->dest_x - e->x);
        bool dx = step_axis(e, &e->x, e->dest_x, TRUE, TRUE);
        bool dy = step_axis(e, &e->y, e->dest_y, FALSE, TRUE);
        return dx && dy;
    }

    case AI_WAIT_RANDOM:
        // PC WAIT_RANDOM_TIME: hold for a random 0..range*2 frames.
        if (!e->rand_t)
            e->rand_t = (u16)(10 + (random() % (u16)(2 * (d->range ? d->range : 30) + 1)));
        return e->act_t >= e->rand_t;

    case AI_PLAY_SFX:
        if (e->act_t == 1 && st->param >= 0 && st->param < SFX_COUNT)
            sfx_play((u8)st->param);
        return TRUE;

    case AI_MORPH: {
        // PC MORPH_INTO_NPC: same spot, becomes the target NPC.
        if (e->act_t == 1) ai_spawn_npc(e, (u8)st->param, TRUE);
        enemy_die(e);
        return TRUE;
    }

    case AI_JUMP_ATTACK_AHEAD:
    case AI_WALL_WALK:
    case AI_WALL_WALK_SHOOT: {
        // PC WALL_WALK runs along the ceiling with gravity off (the same
        // _ignore_gravity the roof jump sets). JUMP_ATTACK_AHEAD is the
        // upward jump-attack variant that fires while airborne.
        e->jatk = (st->action != AI_WALL_WALK && st->action != AI_WALL_WALK_SHOOT);
        if (st->action == AI_JUMP_ATTACK_AHEAD && !e->jumping) {
            if (!on_ground(e, d)) return FALSE;
            e->vy = ENEMY_JUMP_VY;
            e->jumping = 1;
            e->moving = 1;
            return FALSE;
        }
        if (st->action != AI_JUMP_ATTACK_AHEAD) e->no_gravity = 1;
        e->moving = 1;
        s16 nx = (s16)(e->x + e->dir * enemy_step(d->speed));
        s16 lead = (e->dir > 0) ? nx + d->box_l + d->box_w - 1 : nx + d->box_l;
        if (box_hits_solid(lead, e->y + d->box_t,
                           lead, e->y + d->box_t + d->box_h))
            e->dir = -e->dir;                       // bounce off the end
        else
            e->x = nx;
        // PC WALL_WALK_SHOOT fires a TARGET_DIRECTION missile: it has to
        // head for the player, not straight out of the wall.
        // PC fires along shoot_direction: facing side, or down from a ceiling
        // ([TOP]) / up from a floor ([BOTTOM]) - never behind the walker.
        if (st->action == AI_WALL_WALK_SHOOT && (e->act_t % 40) == 0 &&
            wall_shot_ahead(e, d))
            eshot_fire_aimed(e->x + d->box_l + d->box_w / 2,
                             e->y + d->box_t + d->box_h / 2, d->shot);
        if (st->action == AI_JUMP_ATTACK_AHEAD) {
            if (e->act_t == 8 && st->param >= 0)
                eshot_fire_e(e, d, e->x + d->box_l + d->box_w / 2,
                           e->y + d->box_t + d->box_h / 2, e->dir, (u8)st->param);
            e->vy += ENEMY_GRAVITY;
            s16 dy = F16_toInt(e->vy);
            s16 bottom = d->box_t + d->box_h;
            s16 ny = e->y + dy;
            if (dy > 0) {
                for (s16 row = (e->y + bottom) >> 4;
                     row <= ((ny + bottom) >> 4); row++) {
                    if (feet_solid(e, row)) {
                        e->y = (row << 4) - bottom;
                        e->vy = 0;
                        e->jumping = 0;
                        e->moving = 0;
                        return TRUE;
                    }
                }
            }
            if (dy < 0 && ceiling_clip(e, &ny)) e->vy = 0;
            e->y = ny;
            return FALSE;
        }
        return FALSE;                   // wall walk lasts until the state ends
    }

    case AI_SHOT_MULTIPLE: {
        // PC SHOT_MULTIPLE_PROJECTILE: three shots in a small vertical spread,
        // spawned at the attack frame like the single shots (ai.cpp:1214).
        u16 dur = AI_SHOT_FRAMES;
        if (d->attack_n) {
            dur = (u16)(((u32)d->attack_ms * 6) / 100);
            if (dur < AI_SHOT_FRAMES) dur = AI_SHOT_FRAMES;
        }
        e->shooting = 2;
        if (e->act_t == 1 + ai_shot_frame(d, dur)) {
            u8 pid = (st->param >= 0) ? (u8)st->param : d->shot;
            s16 cx = (s16)(e->x + d->box_l + d->box_w / 2);
            s16 cy = (s16)(e->y + d->box_t + d->box_h / 2);
            eshot_fire_e(e, d, cx, cy, e->dir, pid);
            eshot_fire_e(e, d, cx, (s16)(cy - 16), e->dir, pid);
            eshot_fire_e(e, d, cx, (s16)(cy + 16), e->dir, pid);
        }
        return e->act_t >= dur;
    }

    case AI_EXPLODE:
        if (e->act_t == 1) {
            explosion_spawn((s16)(e->x + d->box_l + d->box_w / 2),
                            (s16)(e->y + d->box_t + d->box_h / 2));
            enemy_die(e);
        }
        return TRUE;

    case AI_THROW_ITEM: {
        // PC THROW_ITEM: lob a pickup; the port reuses the enemy drop table at
        // the enemy's position (the PC item object art is not converted).
        if (e->act_t == 1)
            objects_drop_item((s16)(e->x + d->box_l + d->box_w / 2),
                              (s16)(e->y + d->box_t + d->box_h / 2), FALSE);
        return TRUE;
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
    // Guard against a chain that leaves the direction at 0 (a spawner with no
    // facing passed one): AI_JUMP AHEAD and AI_WALK AHEAD would then move by
    // dir*range = 0, the enemy hopping straight up in place (report
    // 2026-10-24, ANGRY BUNNY).
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
    e->no_gravity = 0;
    if (d->is_boss && boss_i < 0) boss_i = idx;
    spr_seen = spr_gen;
    e->spr = SPR_addSprite(d->spr, e->x - camera.x, e->y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    e->alive = (e->spr != NULL);
    if (e->spr) {
        SPR_setVisibility(e->spr, HIDDEN);
        spr_set_anim(e->spr, d->walk_n > 0 ? 1 : 0);
        SPR_setHFlip(e->spr, e->dir < 0);
    }
    return e;
}

void enemies_reset(void)
{
    for (u8 i = 0; i < enemy_count; i++) {
        spr_release_guarded(&enemies[i].spr, spr_seen);
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
            // PC classmap.cpp:2201: a dead sub-boss only revives while it is
            // off screen (the player must not see it pop back).
            bool respawn_tick = TRUE;
            if (e->def->is_sub_boss) {
                const s16 sxb = e->x - camera.x;
                respawn_tick = (sxb < -40 || sxb >= 336);
            }
            if (e->respawn_t && respawn_tick && --e->respawn_t == 0) {
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
                e->no_gravity = 0;
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
                    // PC classMap::boss_show_intro_sprites: play the INTRO
                    // animation (file anim 16) after the landing drop.
                    if (d->intro_anim != 255 && d->intro_ms)
                        e->intro_t = (u16)(((u32)d->intro_ms * 6) / 100);
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
        if (e->intro_t) e->intro_t--;

        // ---- visibility (the AI only runs on-screen, like the PC move
        // early-exit; keeps off-screen chasers parked until seen) ---------
        s16 sx = e->x - camera.x;
        s16 sy = e->y - camera.y;
        bool vis = (sx >= -40 && sx < 336 && sy >= -40 && sy < 248);

        if (vis && stage_same_room(e->x + d->box_l + d->box_w / 2, player.x + 14)) {
            ai_tick(e);
            if (!e->alive) continue;    // AI_REPLACE_NPC removed it
            // boss hit blink (PC shows a white flash via palette; the boss
            // shares PAL0 with the HUD, so blink instead).
            if (d->is_boss) boss_screen = TRUE;
        } else if (d->is_boss) {
            boss_screen = FALSE;
        }

        // ---- gravity (walkers only; AI JUMP manages its own arc) ---------
        if (!d->fly && !e->jumping && !e->no_gravity) {
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
                    spr_release_guarded(&e->spr, spr_seen);
                    e->alive = FALSE;
                    continue;
                }
            }
        }
        // Patrol limits are a walker's leash. Flyers and swimmers chase the
        // player wherever he is (PC has no such limit): clamping them left a
        // chasing fish/bat pinned at the border with its state never finishing.
        if (!d->fly && !e->always_ahead) {
            if (e->x < e->x_min) { e->x = e->x_min; }
            if (e->x > e->x_max) { e->x = e->x_max; }
        }

        // ---- draw ----------------------------------------------------------
        if (e->spr) {
            bool vis2 = (sx >= -40 && sx < 336 && sy >= -40 && sy < 248);
            // Boss hit blink (PC shows a white flash via palette; the boss
            // shares PAL0 with the HUD, so blink instead).
            if (e->hurt_timer && ((e->hurt_timer >> 3) & 1)) vis2 = FALSE;
            SPR_setVisibility(e->spr, vis2 ? VISIBLE : HIDDEN);
            SPR_setHFlip(e->spr, e->dir < 0);
            // Sheet rows: 0 stand, 1 walk, 2 attack, 3 jump, 4 teleport
            // (PC ANIM_TYPE_*; BUG-011 - teleporting/jumping used to keep
            // the previous row, which is what made the Mummy look like he
            // faced the wrong way while repositioning).
            SPR_setAnimationLoop(e->spr, TRUE);
            if (e->turn_t && d->turn_anim != 255)
                spr_set_anim(e->spr, d->turn_anim);
            else if (e->dashing && d->slide_anim != 255)
                spr_set_anim(e->spr, d->slide_anim);
            else if (d->jatk_anim != 255 && (e->jatk || ((e->jumping || e->leaping) && e->shooting)))
                spr_set_anim(e->spr, d->jatk_anim);
            else if (d->attack_n > 0 && e->shooting) {
                // The attack plays once and holds its last frame until the
                // action ends; looping it re-showed the first (stand) frame
                // whenever the action ran a tick longer than the animation.
                spr_set_anim(e->spr, 2);
                SPR_setAnimationLoop(e->spr, e->shooting != 2);
            }
            else if (e->teleporting && d->teleport_anim != 255)
                spr_set_anim(e->spr, d->teleport_anim);
            else if (e->intro_t && d->intro_anim != 255)
                spr_set_anim(e->spr, d->intro_anim);
            else if (e->clinging && d->air_anim != 255)
                spr_set_anim(e->spr, d->air_anim);
            else if ((e->jumping || e->leaping) && d->jump_anim != 255)
                spr_set_anim(e->spr, d->jump_anim);
            else if (d->walk_n > 0)
                spr_set_anim(e->spr, e->moving ? 1 : 0);
            SPR_setPosition(e->spr, e->x - camera.x, e->y - camera.y);
        }

        // PC character.cpp:1921: a non-boss NPC never crosses the water line.
        if ((e->x != pre_x || e->y != pre_y) &&
            !enemy_water_ok(e, pre_x, pre_y, e->x, e->y)) {
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
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->spr) continue;
        // On "show" only live, on-screen enemies come back. A dead enemy keeps
        // its (hidden) sprite and enemies_update() skips it, so showing it here
        // left it frozen on screen after the in-game menu (same hole as the
        // objects, items_after_menu.png).
        const s16 sx = e->x - camera.x;
        const s16 sy = e->y - camera.y;
        const bool show = visible && e->alive &&
                          sx >= -40 && sx < 336 && sy >= -40 && sy < 248;
        SPR_setVisibility(e->spr, show ? VISIBLE : HIDDEN);
    }
}

// Room transition (see transition.c): the outgoing screen is a frozen copy of
// what is on screen, so its enemies slide with it. Visibility is set
// explicitly (clearing SGDK's auto modes) before the position write, which
// would otherwise recompute it.
// Blocking horizontal scroll (boss door): same as objects_sync_sprites().
void enemies_sync_sprites(void)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->spr || !e->alive) continue;
        s16 sx = e->x - camera.x;
        s16 sy = e->y - camera.y;
        SPR_setVisibility(e->spr, (sx >= -40 && sx < 336 &&
                                   sy >= -40 && sy < 248) ? VISIBLE : HIDDEN);
        SPR_setPosition(e->spr, sx, sy);
    }
}

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
// Damage one enemy (shield, hit window, weakness, death). Returns what
// enemies_try_hit() documents.
static u8 enemy_hit(u8 i, Enemy *e, u8 damage, s8 shot_dir, bool charged,
                    u8 weapon)
{
    const EnemyDef *d = e->def;
    const bool stage_boss = ((s16)i == boss_i);
    if (stage_boss && e->dropping) return 1;
    // PC classplayer.cpp:738: an NPC with its circle weapon up takes no damage;
    // the shot is just consumed.
    if (eshot_owner_has_circle(i)) return 1;

    // shielded enemy: a vanishing shot is reflected, no damage. Charged
    // shots (vanishes_on_hit = false) pass through the shield and damage
    // the enemy, as in PC classplayer.cpp:708.
    if (!charged && enemy_is_shielded(e, shot_dir)) return 2;

    // Intangible window after a hit (PC hit_duration): the shot is
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
    e->hurt_timer = stage_boss ? BOSS_HURT_FRAMES : NPC_HURT_FRAMES;
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
            if (d->is_sub_boss) explosion_spawn_big(e->x, e->y);
            else explosion_spawn(e->x, e->y);
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

u8 enemies_try_hit(s16 l, s16 t, s16 r, s16 b, u8 damage, s8 shot_dir,
                   bool charged, u8 weapon)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive || e->dying) continue;
        const EnemyDef *d = e->def;
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (l >= el + d->box_w || r <= el || t >= et + d->box_h || b <= et)
            continue;
        return enemy_hit(i, e, damage, shot_dir, charged, weapon);
    }
    return 0;
}

// PC classPlayer::damage_ground_npcs(): while the QUAKE projectile lives, every
// enemy on screen that stands on solid ground takes the weapon's weakness
// multiplier as damage (a 0 multiplier = immune; the hit window still applies).
void enemies_quake_hit(u8 weapon)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive || e->dying) continue;
        const EnemyDef *d = e->def;
        const s16 sx = e->x + d->box_l - camera.x;
        const s16 sy = e->y + d->box_t - camera.y;
        if (sx + d->box_w <= 0 || sx >= 320 || sy + d->box_h <= 0 || sy >= 224)
            continue;
        const s16 tx = (e->x + d->box_l + d->box_w / 2) >> 4;
        const s16 ty = (e->y + d->box_t + d->box_h) >> 4;
        const u16 lock = stage_terrain_at(stage_current_map, tx, ty);
        if (lock == TERRAIN_UNBLOCKED || lock == TERRAIN_STAIR ||
            lock == TERRAIN_WATER)
            continue;
        enemy_hit(i, e, 1, e->dir, TRUE, weapon);
    }
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

// Body box + facing of enemy `index` (CHAIN projectiles follow their owner)
u8 enemies_live(void)
{
    u8 n = 0;
    for (u8 i = 0; i < enemy_count; i++) if (enemies[i].alive) n++;
    return n;
}

bool enemies_get_box(u8 index, s16 *x, s16 *y, s16 *w, s16 *h, s8 *dir)
{
    if (index >= enemy_count) return FALSE;
    const Enemy *e = &enemies[index];
    if (!e->alive || e->dying) return FALSE;
    const EnemyDef *d = e->def;
    *x = (s16)(e->x + d->box_l);
    *y = (s16)(e->y + d->box_t);
    *w = (s16)d->box_w;
    *h = (s16)d->box_h;
    *dir = e->dir;
    return TRUE;
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
