#include "object.h"
#include "sprite_guard.h"
#include "sfx.h"

#include "stage.h"
#include "player.h"
#include "resources.h"
#include "dialogs.h"
#include "enemy.h"

// Map objects (pills, 1UP, fall/fly platforms, checkpoint, boss teleporter).
// Behavior mirrors PC objects/object.cpp for the types shipped in
// stage0_objects.c:
//   7  pill      pickup, hp+3 (cap PLAYER_INITIAL_HP)
//   9  1UP       pickup, +1 life
//   10 fall      land -> fall 4px/frame -> blocked/off-screen -> return
//   11 fly       land -> rise 4px/frame -> blocked/off-screen -> return
//                (return carries the player, PC has no presence gate)
//   29 checkpoint touch -> respawn point + looping flag animation
//   32 boss teleporter, looping anim; non-solid trigger (PC consumes the
//       object block for this type): capsule-centre -> stage_teleport_boss

typedef struct {
    const ObjectDef *def;
    Sprite *spr;
    s16 x, y;
    s16 home_y;      // start position; RETURN climbs back to it
    s16 home_x;      // LR platform origin
    s16 distance;    // px travelled since MOVE started
    u8 state;
    u8 delay;        // activation-delay countdown (OBJ_START_DELAY)
    u8 anim_t;       // ticks inside the current frame
    u8 frame;
    u8 dest_x, dest_y, dest_map;  // type 32: teleport destination (tiles)
    bool started;
    bool active;
    bool visible;    // disappearing blocks toggle this (collision follows it)
    u8 anim_play;    // one-shot animation requested (block appearing)
    u8 vis_t;        // frames in the current visible/hidden phase
    u8 hp;           // destructible wall
    u16 duration;    // frames left before release (0 = permanent)
    s8 dir;          // COIL/JET facing
    u8 drain;        // JET ammo drain counter
    fix16 fall_speed;// crusher (33) accelerating fall
} MapObj;

static MapObj objs[OBJECT_MAX];
static u8 obj_count;

// Types that animate without player interaction (auto_start = 1 in data).
static bool auto_anim(u8 type)
{
    // pickups (small/big pills), 1UP, boss teleporter, COIL/JET items
    return type == 5 || type == 6 || type == 7 || type == 8 || type == 9 ||
           type == 12 || type == 13 ||
           type == 18 || type == 19 || type == 21 || type == 24 ||
           type == 25 || type == 26 || type == 31 || type == 32;
}

static bool is_platform(u8 type)
{
    // disappearing/active blocks, fall/fly/LR platforms, destructible wall,
    // boss door, JET EAGLE, damaging platform (RockDroid2 28). COIL (13) is a
    // trigger, not a platform. The slim platform (27) is one-way and is
    // handled by slim_solid_now() instead, so it does not block from the side.
    return type == 1 || type == 2 || type == 10 || type == 11 || type == 12 ||
           type == 14 || type == 22 || type == 28 || type == 30;
}

// PC OBJ_ACTIVE_OPENING_SLIM_PLATFORM (27) only collides with its TOP edge:
// classmap.cpp:1560 skips it while the character moves up and the slim branch
// (classmap.cpp:1447) clears every other block. Modelled as a one-way
// platform: landing and standing only, no side or ceiling collision.
static bool slim_solid_now(const MapObj *o)
{
    return o->active && o->def->type == 27 && o->visible;
}

// PC object::gravity() (objects/object.cpp:228): every object type not in
// this exclusion list falls at GRAVITY_SPEED while on screen. The list is the
// PC enum order: fixed/moving platforms, fly items, rays, track/death/damaging
// platforms, crushers, timed bombs, expanding spikes and the walker.
#define OBJ_GRAVITY_SPEED 4
static bool object_has_gravity(u8 type)
{
    switch (type) {
    case 0:  case 1:  case 2:                 // up/down, left/right, block
    case 10: case 11: case 12:                // fall/fly platforms, fly item
    case 18: case 19: case 20:                // rays, track platform
    case 22:                                  // destructible wall/box stays put
    case 24: case 25:                         // death rays
    case 14: case 27: case 28:                // active block/opening/damaging
    case 38: case 39: case 40: case 41:       // active diagonal/ahead/up
    case 33: case 34: case 35:                // crusher, loop/down platforms
    case 42: case 43: case 44: case 45:       // jump-shoot, timed bomb, spike, walker
        return FALSE;
    default:
        return TRUE;
    }
}

// A disappearing block only collides while in its visible phase.
static bool solid_now(const MapObj *o)
{
    if (!o->active || !is_platform(o->def->type)) return FALSE;
    if (o->def->type == 30) return !o->started;   // boss door until opened
    return o->visible;
}

// PC object.cpp: DISAPPEARNING_VISIBLE_TIME / HIDDEN_TIME (1500/2000ms) at
// 60fps. Type 2 cycles on its own; the first phase starts visible so the
// player never meets an invisible block at stage entry.
#define BLOCK_VISIBLE_FRAMES 90
#define BLOCK_HIDDEN_FRAMES 120

// PC object obj_timer is milliseconds. Disappearing-block #1/#2/#3 use
// 1000/2000/3000ms as the initial hidden delay; the port used to start every
// block visible at once, so they could not be crossed in sequence.
static u8 obj_ms_frames(u16 ms)
{
    u16 f = (u16)(((u32)ms * 6) / 100);
    return (f > 255) ? 255 : (u8)f;
}

static bool is_looping(const ObjectDef *d)
{
    // PC file_object.animation_loop: the checkpoint's flag keeps waving after
    // activation, the boss teleporter animates forever, platforms play once.
    return d->anim_loop != 0;
}

// PC object::test_change_position for a pure vertical step, terrain only
// (the player-blocking branch is commented out on PC as well). Destination
// bottom corners (x+2, x+w-2) at row y+dy+h-2 must both be non-SOLID.
static bool platform_can_move(const MapObj *o, s16 dy)
{
    s16 row = (o->y + dy + o->def->h - 2) >> 4;
    s16 c1 = (o->x + 2) >> 4;
    s16 c2 = (o->x + o->def->w - 2) >> 4;
    if (c1 < 0 || c1 >= stage_def->map_w || c2 < 0 || c2 >= stage_def->map_w)
        return FALSE;
    if (dy > 0 && row > stage_def->map_h)
        return TRUE;                 // falling out of the screen is allowed
    if (row < 0)
        return TRUE;                 // rising out of the top; off-screen then returns
    // PC object::test_change_position treats only UNBLOCKED, WATER and SPIKE
    // as passable. STAIR (ladders), doors and hard blocks stop a falling box;
    // the old `!= SOLID` let boxes fall through the ladder column.
    const u16 t1 = stage_terrain_at(stage_current_map, c1, row);
    const u16 t2 = stage_terrain_at(stage_current_map, c2, row);
    return (t1 == TERRAIN_UNBLOCKED || t1 == TERRAIN_WATER ||
            t1 == TERRAIN_SPIKE) &&
           (t2 == TERRAIN_UNBLOCKED || t2 == TERRAIN_WATER ||
            t2 == TERRAIN_SPIKE);
}

// TRUE when the player rests exactly on this platform's top edge.
static bool player_on_top(const MapObj *o)
{
    s16 l, t, r, b;
    player_get_box(&l, &t, &r, &b);
    return player.on_ground && b == o->y &&
           l < o->x + o->def->w && r > o->x;
}

static void obj_stop(MapObj *o)
{
    o->state = OBJ_STATE_STAND;
    o->started = FALSE;
    o->frame = 0;
    o->anim_t = 0;
    if (o->spr) spr_set_frame(o->spr, 0);
}

static u16 spr_seen;   // sprite-engine generation of the handles below

static void obj_release(MapObj *o)
{
    spr_release_guarded(&o->spr, spr_seen);
    o->active = FALSE;
}

// Boss door open/close. The PC art has a single frame, so the frame-based
// animation in object::show_boss_door() does nothing visible there either; the
// intent is a door that slides open. Animate the sprite down into the floor
// (down) or back up (up), one VDP frame per step so the world keeps drawing.
static void door_slide(MapObj *o, bool down)
{
    if (!o->spr) return;
    const u8 steps = 10;
    for (u8 i = 1; i <= steps; i++) {
        s16 dy = (s16)((o->def->h * i) / steps);
        if (!down) dy = (s16)(o->def->h - dy);
        SPR_setVisibility(o->spr, VISIBLE);
        SPR_setPosition(o->spr, o->x - camera.x, (s16)(o->y - camera.y + dy));
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

void objects_reset(void)
{
    for (u8 i = 0; i < obj_count; i++) {
        spr_release_guarded(&objs[i].spr, spr_seen);
        objs[i].active = FALSE;
    }
    obj_count = 0;

    for (u8 i = 0; i < stage_def->object_spawn_count && obj_count < OBJECT_MAX; i++) {
        const ObjectSpawn *sp = &stage_def->object_spawns[i];
        if (sp->map != stage_current_map) continue;

        const ObjectDef *d = &stage_def->object_defs[sp->def];
        MapObj *o = &objs[obj_count++];
        o->def = d;
        o->x = sp->x;
        o->y = sp->y;
        o->home_y = sp->y;
        o->home_x = sp->x;
        o->hp = (d->type == 22) ? 3 : 0;
        o->distance = 0;
        o->state = OBJ_STATE_STAND;
        o->delay = 0;
        o->anim_t = 0;
        o->frame = 0;
        o->dest_x = sp->dest_x;
        o->dest_y = sp->dest_y;
        o->dest_map = sp->dest_map;
        o->started = FALSE;
        o->active = TRUE;
        o->visible = TRUE;
        o->vis_t = 0;
        if (d->type == 2 && d->timer) {
            o->visible = FALSE;
            o->vis_t = obj_ms_frames(d->timer);
        }
        o->duration = 0;
        o->fall_speed = 0;
        o->spr = SPR_addSprite(d->spr, o->x - camera.x, o->y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        spr_seen = spr_gen;
        if (o->spr) {
            // manual visibility + manual frame advance (see objects_update)
            SPR_setAutoAnimation(o->spr, FALSE);
            // HIDDEN at spawn: stage_load() spawns objects while the camera is
            // still at 0, so their "screen" x is their world x — VISIBLE here
            // wrapped the whole room's objects onto the screen for a frame.
            SPR_setVisibility(o->spr, HIDDEN);
        }
    }
}

// Enemy drop tables (PC classMap::drop_item, classmap.cpp:996). The PC picks
// the object definition from the global object list; the port carries its own
// definitions because a stage's list only has the types that stage spawns.
// PC normal difficulty: >=97 big energy, >=95 big weapon, >=80 small energy,
// >=65 small weapon, >=50 coin (no coin object in this port), else nothing.
static const ObjectDef *drop_def(u8 type)
{
    static const ObjectDef defs[] = {
        { &object_energy_pill_big_sprite,   5, 2, 6, 14, 14, 0, 0, 0 },
        { &object_weapons_pill_big_sprite,  6, 2, 6, 14, 14, 0, 0, 0 },
        { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0, 0, 0 },
        { &object_weapons_pill_small_sprite,8, 2, 6, 10, 10, 0, 0, 0 },
    };
    for (u8 i = 0; i < 4; i++) {
        if (defs[i].type == type) return &defs[i];
    }
    return NULL;
}

// Spawn a dropped item at a world position (enemy centre). Duration matches
// PC object::set_duration(4500): 4.5s at 60fps.
void objects_drop_item(s16 cx, s16 cy, bool subboss)
{
    u8 r = (u8)(random() % 100);
    u8 type;
    if (subboss)      type = 5;     // PC: sub-boss always big energy
    else if (r >= 97) type = 5;
    else if (r >= 95) type = 6;
    else if (r >= 80) type = 7;
    else if (r >= 65) type = 8;
    else return;

    const ObjectDef *d = drop_def(type);
    if (!d) return;

    MapObj *o = NULL;
    for (u8 i = 0; i < obj_count; i++) {
        if (!objs[i].active) { o = &objs[i]; break; }
    }
    if (!o) {
        if (obj_count >= OBJECT_MAX) return;
        o = &objs[obj_count++];
    }

    o->def = d;
    o->x = (s16)(cx - d->w / 2);
    o->y = (s16)(cy - d->h / 2);
    o->home_x = o->x;
    o->home_y = o->y;
    o->distance = 0;
    o->state = OBJ_STATE_STAND;
    o->delay = 0;
    o->anim_t = 0;
    o->frame = 0;
    o->dest_x = o->dest_y = o->dest_map = 0;
    o->started = FALSE;
    o->active = TRUE;
    o->visible = TRUE;
    o->vis_t = 0;
    o->hp = 0;
    o->fall_speed = 0;
    o->duration = 270;

    s16 sx = o->x - camera.x;
    s16 sy = o->y - camera.y;
    o->spr = SPR_addSprite(d->spr, sx, sy,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    spr_seen = spr_gen;
    if (o->spr) {
        SPR_setAutoAnimation(o->spr, FALSE);
        bool on_screen = (sx + d->w > 0 && sx < 320 &&
                          sy + d->h > 0 && sy < 224);
        SPR_setVisibility(o->spr, on_screen ? VISIBLE : HIDDEN);
    }
}

// Player item weapons (PC player_items 16/17: COIL FROG type 13, JET EAGLE
// type 12). Art is emitted once by convert_objects.py stage 0.
static const ObjectDef coil_def =
    { &object_item_jump_sprite, 13, 5, 2, 28, 18, 0, 0, 0 };
static const ObjectDef jet_def =
    { &object_item_jet_sprite, 12, 4, 18, 37, 9, 2, 0, 0 };

bool objects_spawn_weapon_item(bool jet)
{
    // PC classMap::have_player_object(): only one item platform at a time.
    for (u8 i = 0; i < obj_count; i++) {
        if (objs[i].active &&
            (objs[i].def->type == 12 || objs[i].def->type == 13))
            return FALSE;
    }

    MapObj *o = NULL;
    for (u8 i = 0; i < obj_count; i++) {
        if (!objs[i].active) { o = &objs[i]; break; }
    }
    if (!o) {
        if (obj_count >= OBJECT_MAX) return FALSE;
        o = &objs[obj_count++];
    }

    const ObjectDef *d = jet ? &jet_def : &coil_def;
    s16 l, t, r, b;
    player_get_box(&l, &t, &r, &b);
    o->def = d;
    o->dir = (player.facing < 0) ? -1 : 1;
    o->x = (o->dir > 0) ? (s16)(r + 2) : (s16)(l - 2 - d->w);
    o->y = jet ? (s16)(player.y + 16) : (s16)(b - d->h);
    o->home_x = o->x;
    o->home_y = o->y;
    o->distance = 0;
    o->state = OBJ_STATE_STAND;
    o->delay = 0;
    o->anim_t = 0;
    o->frame = 0;
    o->dest_x = o->dest_y = o->dest_map = 0;
    o->started = FALSE;
    o->active = TRUE;
    o->visible = TRUE;
    o->vis_t = 0;
    o->hp = 0;
    o->duration = jet ? 270 : 150;   // PC 4500ms / 2500ms
    o->drain = 0;
    o->fall_speed = 0;
    o->spr = SPR_addSprite(d->spr, o->x - camera.x, o->y - camera.y,
                           TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    if (o->spr) {
        SPR_setAutoAnimation(o->spr, FALSE);
        SPR_setHFlip(o->spr, o->dir < 0);
        SPR_setVisibility(o->spr, VISIBLE);
    }
    return TRUE;
}

static void obj_carry_player(s16 dy)
{
    player.y += dy;
    if (dy >= 0) return;
    // Lifting the player into a ceiling is instant death (PC damage(999),
    // classmap.cpp:1644). The platform's own move is already wall-checked, but
    // the player is taller than it, so his head can enter a solid tile.
    s16 l, t, r, b;
    player_get_box(&l, &t, &r, &b);
    s16 row = t >> 4;
    if (stage_terrain_at(stage_current_map, (l + 1) >> 4, row) == TERRAIN_SOLID ||
        stage_terrain_at(stage_current_map, (r - 1) >> 4, row) == TERRAIN_SOLID) {
        player_crush();
    }
}

// TRUE when an axis-aligned box touches a terrain cell a moving object may
// not enter. JET EAGLE must disappear on the first contact (PC object collision).
static bool terrain_box_blocked(s16 x, s16 y, s16 w, s16 h)
{
    for (s16 ty = y >> 4; ty <= (y + h - 1) >> 4; ty++) {
        for (s16 tx = x >> 4; tx <= (x + w - 1) >> 4; tx++) {
            const u16 t = stage_terrain_at(stage_current_map, tx, ty);
            if (t != TERRAIN_UNBLOCKED && t != TERRAIN_WATER &&
                t != TERRAIN_SPIKE)
                return TRUE;
        }
    }
    return FALSE;
}

static void obj_move(MapObj *o)
{
    const ObjectDef *d = o->def;

    if (o->delay > 0) {
        o->delay--;
        return;
    }

    if (d->type == 10) {                            // OBJ_FALL_PLATFORM
        if (o->state == OBJ_STATE_MOVE) {
            s16 dy = d->speed;
            if (platform_can_move(o, dy)) {
                bool carry = player_on_top(o);   // evaluate before o->y changes
                o->y += dy;
                o->distance += dy;
                if (carry) obj_carry_player(dy);
            } else {
                o->state = OBJ_STATE_RETURN;
            }
            if (o->state == OBJ_STATE_MOVE && o->y > 224 + 48)
                o->state = OBJ_STATE_RETURN;        // PC: RES_H + TILESIZE*3
        } else if (o->state == OBJ_STATE_RETURN) {
            if (!player_on_top(o)) {                // PC: no return under the player
                if (o->distance == 0) {
                    obj_stop(o);
                } else {
                    s16 dy = -(d->speed / 2);
                    if (dy == 0) dy = -1;
                    o->y += dy;
                    o->distance += dy;
                    if (o->distance <= 0) {
                        o->y = o->home_y;
                        obj_stop(o);
                    }
                }
            }
        }
    } else if (d->type == 11) {                     // OBJ_FLY_PLATFORM
        if (o->state == OBJ_STATE_MOVE) {
            s16 dy = -d->speed;
            if (platform_can_move(o, dy)) {
                bool carry = player_on_top(o);   // evaluate before o->y changes
                o->y += dy;
                o->distance -= dy;
                if (carry) obj_carry_player(dy);
            } else {
                o->state = OBJ_STATE_RETURN;
            }
            if (o->state == OBJ_STATE_MOVE && o->y < -(d->h + 16))
                o->state = OBJ_STATE_RETURN;        // PC: -(h + TILESIZE)
        } else if (o->state == OBJ_STATE_RETURN) {
            if (o->distance == 0) {
                obj_stop(o);
            } else {
                s16 dy = d->speed / 2;              // carries the player (no gate)
                if (dy == 0) dy = 1;
                bool carry = player_on_top(o);       // evaluate before o->y changes
                o->y += dy;
                o->distance -= dy;
                if (carry) obj_carry_player(dy);
                if (o->distance <= 0) {
                    o->y = o->home_y;
                    obj_stop(o);
                }
            }
        }
    }
}

void objects_player_land(s16 l, s16 r, s16 top)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active || o->y != top) continue;
        if (!is_platform(o->def->type) && !slim_solid_now(o)) continue;
        // match the X range too: two platforms can share a top edge, and the
        // old y-only test always started the first one in the array
        if (l >= o->x + o->def->w || r <= o->x)
            continue;
        // RockDroid2 platforms that react to being stood on without moving:
        // the slim platform opens, the damaging one starts its damage cycle
        // (PC object::start() from classmap.cpp:1715/1740).
        if (o->def->type == 27 || o->def->type == 28) {
            if (!o->started) {
                o->started = TRUE;
                o->state = 0;
                o->vis_t = 0;
            }
            return;
        }
        if (o->state == OBJ_STATE_STAND) {
            o->started = TRUE;
            o->state = OBJ_STATE_MOVE;
            o->delay = OBJ_START_DELAY;
            o->distance = 0;
        }
        return;
    }
}

s16 objects_land_check(s16 l, s16 r, s16 head_new, s16 feet_new)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (o->active && o->def->type == 13) {   // COIL FROG: spring trigger
            if (head_new <= o->y && feet_new > o->y &&
                l <= o->x + o->def->w && r >= o->x) {
                player_super_jump();
                obj_release(o);
                return -1;
            }
            continue;
        }
        if (!solid_now(o)) continue;
        // PC classmap.cpp:1654 — descending with the head at/above the top
        // and any horizontal touch. X is inclusive: the side clamp can leave
        // the box exactly at the face (r == o->x) and the landing must still
        // catch (corner entry).
        if (head_new <= o->y && feet_new > o->y &&
            l <= o->x + o->def->w && r >= o->x)
            return o->y;
    }
    // slim platforms last: they are one-way, but still land the player
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!slim_solid_now(o)) continue;
        if (head_new <= o->y && feet_new > o->y &&
            l <= o->x + o->def->w && r >= o->x)
            return o->y;
    }
    return -1;
}

bool objects_stand_check(s16 l, s16 r, s16 feet)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!solid_now(o) && !slim_solid_now(o)) continue;
        if (o->y == feet && l < o->x + o->def->w && r > o->x)
            return TRUE;
    }
    return FALSE;
}

bool objects_block_x(s16 nx, s16 t, s16 b, s16 dx, s16 *out_x)
{
    s16 l = nx + PLAYER_BOX_L;
    s16 r = nx + PLAYER_BOX_R;
    bool hit = FALSE;
    s16 best = 0;
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!solid_now(o)) continue;
        if (l < o->x + o->def->w && r > o->x &&
            t < o->y + o->def->h && b > o->y) {
            s16 cand = (dx > 0) ? (o->x - PLAYER_BOX_R)
                                : (o->x + o->def->w - PLAYER_BOX_L);
            if (!hit || (dx > 0 ? cand < best : cand > best)) {
                best = cand;
                hit = TRUE;
            }
        }
    }
    if (hit) *out_x = best;
    return hit;
}

s16 objects_head_block(s16 l, s16 r, s16 head_new, s16 feet)
{
    s16 best = -1;
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!solid_now(o)) continue;
        // rising into the bottom: the feet are still below the top (a jump
        // straight off the top has feet <= o->y and passes freely)
        if (feet > o->y && head_new < o->y + o->def->h &&
            l < o->x + o->def->w && r > o->x) {
            s16 stop = o->y + o->def->h - PLAYER_BOX_T;
            if (stop > best) best = stop;
        }
    }
    return best;
}

// Fixed-width zero-padded number (no libc in this toolchain). Negative
// values get one extra leading '-' char.
static char *fmt_num(char *p, s16 v, u8 width)
{
    char tmp[7];
    u8 i = 0;
    bool neg = v < 0;
    u16 u = neg ? (u16)(-v) : (u16)v;
    do {
        tmp[i++] = '0' + (u % 10);
        u /= 10;
    } while (u != 0 && i < 5);
    while (i < width) tmp[i++] = '0';
    if (neg) tmp[i++] = '-';
    while (i > 0) *p++ = tmp[--i];
    return p;
}

void objects_debug_text(u8 tile_y, s16 px, s16 py, s16 camx)
{
    char line[24];
    char *p = line;
    *p++ = 'P'; *p++ = 'X'; p = fmt_num(p, px, 4);
    *p++ = ' '; *p++ = 'P'; *p++ = 'Y'; p = fmt_num(p, py, 3);
    *p++ = ' '; *p++ = 'C'; *p++ = 'X'; p = fmt_num(p, camx, 4);
    *p = 0;
    VDP_drawText(line, 0, tile_y);

    char buf[40];
    p = buf;
    u8 n = 0;
    for (u8 i = 0; i < obj_count && n < 3; i++) {
        MapObj *o = &objs[i];
        if (!o->active || !is_platform(o->def->type)) continue;
        *p++ = 'A' + n;
        *p++ = '0' + o->state;
        *p++ = o->started ? '1' : '0';
        *p++ = ':';
        p = fmt_num(p, o->y, 4);
        *p++ = ' ';
        n++;
    }
    if (n == 0) *p++ = '-';
    *p = 0;
    VDP_drawText(buf, 0, tile_y + 1);
}

// Re-close every boss door. Called when the player respawns: a door opened in
// a previous life used to stay open for the rest of the stage, so its scroll
// lock never came back (the port does not reload the map on death).
void objects_reset_doors(void)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (o->def->type != 30 || !o->started) continue;
        o->started = FALSE;
        o->frame = 0;
        if (o->spr) spr_set_frame(o->spr, 0);
    }
}

// TRUE when a boss door occupies the column, open or closed. The scroll lock
// must take the live door state over the converted table: a door column that
// is also listed in the map's locks would otherwise stay locked after the
// door opens (no scroll into the boss room).
bool objects_boss_door_present(s16 tile_x)
{
    for (u8 i = 0; i < obj_count; i++) {
        const MapObj *o = &objs[i];
        if (o->def->type != 30) continue;
        const s16 x0 = o->x >> 4;
        const s16 x1 = (o->x + o->def->w - 1) >> 4;
        if (tile_x >= x0 && tile_x <= x1) return TRUE;
    }
    return FALSE;
}

bool objects_boss_door_locked(s16 tile_x)
{
    for (u8 i = 0; i < obj_count; i++) {
        const MapObj *o = &objs[i];
        if (o->def->type != 30 || o->started) continue;
        const s16 x0 = o->x >> 4;
        const s16 x1 = (o->x + o->def->w - 1) >> 4;
        if (tile_x >= x0 && tile_x <= x1) return TRUE;
    }
    return FALSE;
}

void objects_set_visible(bool visible)
{
    for (u8 i = 0; i < obj_count; i++)
        if (objs[i].spr)
            SPR_setVisibility(objs[i].spr, visible ? VISIBLE : HIDDEN);
}

// Room transition (see transition.c): the outgoing screen is a frozen copy of
// what is on screen, so its objects slide with it.
void objects_shift_y(s16 dy)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active || !o->spr) continue;
        const ObjectDef *d = o->def;
        s16 sx = o->x - camera.x;
        s16 sy = o->y - camera.y + dy;
        bool on_screen = (sx + d->w > 0 && sx < 320 &&
                          sy + d->h > 0 && sy < 224);
        SPR_setVisibility(o->spr, on_screen ? VISIBLE : HIDDEN);
        if (on_screen) SPR_setPosition(o->spr, sx, sy);
    }
}

// Blocking horizontal scroll (boss door): the camera moves while the objects
// stay put in world space, so their sprites must be re-placed from the world
// position - otherwise the door stays glued to the screen while the stage
// slides under it (SEAHORSE sub-boss door).
void objects_sync_sprites(void)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active || !o->spr) continue;
        const ObjectDef *d = o->def;
        s16 sx = o->x - camera.x;
        s16 sy = o->y - camera.y;
        bool on_screen = (sx + d->w > 0 && sx < 320 &&
                          sy + d->h > 0 && sy < 224);
        SPR_setVisibility(o->spr, on_screen ? VISIBLE : HIDDEN);
        if (on_screen) SPR_setPosition(o->spr, sx, sy);
    }
}

// Player shots vs destructible walls (OBJ_DESTRUCTIBLE_WALL): the PC block
// vanishes after enough hits (block003 art has a broken frame). Returns TRUE
// when the shot hit something (the caller kills the shot).
bool objects_try_hit(s16 l, s16 t, s16 r, s16 b, u8 dmg)
{
    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active || o->def->type != 22 || !o->visible) continue;
        if (l >= o->x + o->def->w || r <= o->x ||
            t >= o->y + o->def->h || b <= o->y) continue;
        if (o->hp > dmg) {
            o->hp -= dmg;
        } else {
            obj_release(o);
            sfx_play(SFX_OBJECT_BREAK);   // PC object.cpp:1785
        }
        return TRUE;
    }
    return FALSE;
}

// PC object::move_crusher (object.cpp:1343) + check_player_crushed (:1376).
// The crusher starts embedded in the ceiling, falls while accelerating 0.1
// px/frame^2, waits CRUSHER_DELAY on the ground, then climbs back at the
// object's speed. Whoever it catches between it and the floor dies
// (classmap.cpp:1644 -> damage(999), the port's player_crush()).
// The PC also paints the shaft it leaves behind (show_crusher: frame 1
// repeated up to the start point). The port draws the body only: the shaft
// would need a plane redraw on every step.
#define CRUSHER_WAIT_FRAMES 48      // PC CRUSHER_DELAY 800 ms
#define CRUSHER_X_TOL 3             // PC CRUSHER_X_TOLERANCE
#define CRUSHER_Y_TOL 4             // PC CRUSHER_Y_TOLERANCE

static void crusher_update(MapObj *o)
{
    const ObjectDef *d = o->def;
    const s16 cx = o->x + d->w / 2;
    const u16 under = stage_terrain_at(stage_current_map, cx >> 4,
                                       (o->y + d->h) >> 4);
    switch (o->state) {
    case 0:                         // INIT: drop out of the ceiling
        o->y += F16_toInt(o->fall_speed);
        o->fall_speed += FIX16(0.1);
        if (under == TERRAIN_UNBLOCKED) o->state = 1;
        break;
    case 1:                         // FOUND_FREE_WALL: fall to the ground
        o->y += F16_toInt(o->fall_speed);
        o->fall_speed += FIX16(0.1);
        if (under != TERRAIN_UNBLOCKED) {
            o->state = 2;
            o->vis_t = CRUSHER_WAIT_FRAMES;
        }
        break;
    case 2:                         // FOUND_HIT_GROUND: rest on it
        o->fall_speed = 0;
        if (o->vis_t && --o->vis_t == 0) o->state = 3;
        break;
    default:                        // RETURNING: climb back to the ceiling
        o->y -= (d->speed ? d->speed : 1);
        if (o->y <= o->home_y) {
            o->y = o->home_y;
            o->state = 0;
            o->fall_speed = 0;
        }
        break;
    }

    if (o->state > 1) return;       // only a moving crusher kills
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);
    if (pt > o->y + d->h - CRUSHER_Y_TOL) return;
    // PC tests both bottom corners of the player box against the crusher's
    // width, minus a small tolerance on each side.
    const s16 lo = o->x + CRUSHER_X_TOL;
    const s16 hi = o->x + d->w - CRUSHER_X_TOL;
    if ((pl >= lo && pl <= hi) || (pr >= lo && pr <= hi))
        player_crush();
}

void objects_update(void)
{
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);

    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active) continue;
        const ObjectDef *d = o->def;

        // Dropped items live for a limited time (PC object duration); an
        // active JET EAGLE stays until its ammo runs out (PC object.cpp:875).
        bool jet_riding = (d->type == 12 && o->started);
        if (o->duration && !jet_riding && --o->duration == 0) {
            obj_release(o);
            continue;
        }

        // ---- JET EAGLE: horizontal flight carrying the player ---------------
        if (d->type == 12 && o->started) {
            s16 dx = (s16)d->speed * o->dir;
            bool carry = player_on_top(o);
            s16 nx = o->x + dx;
            bool blocked = terrain_box_blocked(nx, o->y, d->w, d->h);
            if (!blocked && carry) {
                s16 l, t, r, b;
                player_get_box(&l, &t, &r, &b);
                blocked = terrain_box_blocked(l + dx, t, r - l, b - t);
            }
            if (blocked) {
                // PC JET EAGLE is destroyed on any wall/terrain contact; do
                // not drag the rider into the block.
                obj_release(o);
                continue;
            }
            o->x = nx;
            if (carry) player.x += dx;
            if (++o->drain >= 15) {          // PC consumes 1 unit/240ms
                o->drain = 0;
                if (player_weapon() == 10) {
                    player_use_weapon_ammo();
                    if (player_weapon_ammo() == 0) {
                        obj_release(o);
                        continue;
                    }
                }
            }
        }

        // ---- platform state machine (before pickup: player box is fresh) --
        if (is_platform(d->type) && o->started)
            obj_move(o);
        if (d->type == 33) crusher_update(o);

        // OBJ_MOVING_PLATFORM_LEFTRIGHT: starts when the player stands on it,
        // ping-pongs over +/- limit, carrying the player.
        if (d->type == 1) {
            if (!o->started && player_on_top(o)) o->started = TRUE;
            if (o->started) {
                s16 lim = d->limit ? d->limit : 64;
                s16 dirn = (o->state == OBJ_STATE_RETURN) ? -1 : 1;
                s16 nx = o->x + dirn * (d->speed ? d->speed : 1);
                if (nx > o->home_x + lim) { nx = o->home_x + lim; o->state = OBJ_STATE_RETURN; }
                else if (nx < o->home_x - lim) { nx = o->home_x - lim; o->state = OBJ_STATE_MOVE; }
                s16 mdx = nx - o->x;
                bool carry = player_on_top(o);
                o->x = nx;
                if (carry) player.x += mdx;
            }
        }

        // Fire/lava beams (types 18/19/24/25): visible for 90 frames, hidden
        // for 90 (PC toggles the ray state). They damage on contact.
        if (d->type == 18 || d->type == 19 || d->type == 24 ||
            d->type == 25) {
            if (++o->vis_t >= 90) {
                o->vis_t = 0;
                o->visible = !o->visible;
                if (o->visible) sfx_play(SFX_BEAM);   // PC object.cpp:1102
            }
        }

        // ---- screen visibility -------------------------------------------
        s16 sx = o->x - camera.x;
        s16 sy = o->y - camera.y;
        bool on_screen = (sx + d->w > 0 && sx < 320 &&
                          sy + d->h > 0 && sy < 224);
        // disappearing blocks (PC timings). Type 2 cycles on its own; type
        // 14 stays up until the player stands on it once, then cycles
        // (OBJ_ACTIVE_DISAPPEARING_BLOCK).
        if (d->type == 2) {
            if (!o->visible && o->vis_t) {
                if (--o->vis_t == 0) {
                    o->visible = TRUE;
                    o->anim_play = 1;      // PC reset_animation() + SFX
                    o->frame = 0;
                    o->anim_t = 0;
                    sfx_play(SFX_BLOCK_APPEAR);
                }
            } else {
                u8 lim = o->visible ? BLOCK_VISIBLE_FRAMES : BLOCK_HIDDEN_FRAMES;
                if (++o->vis_t >= lim) {
                    o->vis_t = 0;
                    o->visible = !o->visible;
                }
            }
        } else if (d->type == 14) {
            u8 visible_lim = d->timer ? obj_ms_frames(d->timer)
                                      : BLOCK_VISIBLE_FRAMES;
            if (!o->started) {
                if (o->visible && player_on_top(o))
                    { o->started = TRUE; o->vis_t = 0; }
            } else if (o->visible) {
                if (++o->vis_t >= visible_lim)
                    { o->vis_t = 0; o->visible = FALSE; }
            } else if (++o->vis_t >= BLOCK_HIDDEN_FRAMES) {
                o->vis_t = 0;
                o->visible = TRUE;
                o->started = FALSE;
                o->anim_play = 1;          // PC reset_animation() + SFX
                o->frame = 0;
                o->anim_t = 0;
                sfx_play(SFX_BLOCK_APPEAR);
            }
        } else if (d->type == 27) {
            // PC object.cpp:1039 (OBJ_ACTIVE_OPENING_SLIM_PLATFORM): once the
            // player stands on it, it opens for the object's timer and comes
            // back DISAPPEARNING_HIDDEN_TIME (2000 ms) later. A hidden slim
            // platform keeps updating (object.cpp:426 keeps its exception).
            if (o->started) {
                u8 lim = d->timer ? obj_ms_frames(d->timer)
                                  : BLOCK_VISIBLE_FRAMES;
                if (++o->vis_t >= lim) {
                    o->vis_t = 0;
                    o->visible = FALSE;
                    o->started = FALSE;
                }
            } else if (!o->visible) {
                if (++o->vis_t >= BLOCK_HIDDEN_FRAMES) {
                    o->vis_t = 0;
                    o->visible = TRUE;
                    o->frame = 0;
                    o->anim_t = 0;
                    o->anim_play = 1;
                }
            }
        } else if (d->type == 28) {
            // PC object.cpp:1051 (OBJ_DAMAGING_PLATFORM): the timer ticks
            // from the moment the player stood on it; when it expires whoever
            // is on top takes TOUCH_DAMAGE_SMALL, the second tick resets it.
            if (o->started) {
                u8 lim = d->timer ? obj_ms_frames(d->timer) : 6;
                if (++o->vis_t >= lim) {
                    o->vis_t = 0;
                    if (o->state == 0) {
                        o->state = 1;
                        o->frame = (d->frames > 1) ? 1 : 0;
                        if (o->spr) spr_set_frame(o->spr, o->frame);
                        if (player_on_top(o))
                            player_take_damage(TOUCH_DAMAGE_SMALL);
                    } else {
                        o->state = 0;
                        o->started = FALSE;
                        o->frame = 0;
                        if (o->spr) spr_set_frame(o->spr, 0);
                        o->anim_t = 0;
                    }
                }
            }
        }
        if (o->spr) {
            SPR_setVisibility(o->spr,
                              (on_screen && o->visible) ? VISIBLE : HIDDEN);
            if (on_screen) SPR_setPosition(o->spr, sx, sy);
        }
        if (!on_screen) continue;   // no pickup/anim while fully off-screen

        // ---- gravity (PC object::gravity) -------------------------------
        if (object_has_gravity(d->type)) {
            for (u8 step = OBJ_GRAVITY_SPEED; step > 0; step--) {
                if (platform_can_move(o, (s16)step)) {
                    bool carry = player_on_top(o);
                    o->y += step;
                    if (carry) player.y += step;
                    break;
                }
            }
            if (o->y > stage_def->map_h * stage_def->tile + 32) {
                obj_release(o);
                continue;
            }
        }

        // ---- player overlap: pickup / checkpoint -------------------------
        // The boss door is solid, so collision clamps the player flush against
        // its side (pr == o->x). A strict overlap test never fired there and
        // the door stayed shut while the player pushed it. Treat contact as
        // overlap for the door; all other objects keep the strict test.
        bool touch = (pl < o->x + d->w && pr > o->x &&
                      pt < o->y + d->h && pb > o->y);
        if (!touch && d->type == 30)
            touch = (pr >= o->x && pl <= o->x + d->w &&
                     pt < o->y + d->h && pb > o->y);
        if (touch) {
            if (d->type == 5) {                     // big energy pill (+8)
                sfx_play(SFX_PILL);
                u8 hp = player.hp + 8;
                player.hp = (hp > PLAYER_INITIAL_HP) ? PLAYER_INITIAL_HP : hp;
                obj_release(o);
                continue;
            }
            if (d->type == 6 || d->type == 8) {      // weapon pill big/small
                sfx_play(SFX_PILL);
                u8 add = (d->type == 6) ? 8 : 3;
                u8 n = player.weapon;
                u16 v = player.ammo[n] + add;
                player.ammo[n] = (v > PLAYER_INITIAL_HP) ? PLAYER_INITIAL_HP
                                                         : (u8)v;
                obj_release(o);
                continue;
            }
            if (d->type == 7) {                     // small energy pill (+3)
                sfx_play(SFX_PILL);
                u8 hp = player.hp + 3;
                player.hp = (hp > PLAYER_INITIAL_HP) ? PLAYER_INITIAL_HP : hp;
                obj_release(o);
                continue;
            }
            if (d->type == 15 || d->type == 16 || d->type == 17) {
                sfx_play(SFX_ITEM);
                u8 kind = (u8)(d->type - 15);
                player_add_armor(kind);
                obj_release(o);
                dialog_show_armor(kind);
                continue;
            }
            if (d->type == 23) {                    // special tank
                sfx_play(SFX_ITEM);
                player_add_special_tank();
                obj_release(o);
                continue;
            }
            if (d->type == 30) {                    // boss door: open on touch
                // PC classmap.cpp:1611: a door stays shut while a boss or
                // sub-boss is alive on the left side of it.
                if (enemies_subboss_alive_left(o->x >> 4)) continue;
                // PC object.cpp:1189: OPENING -> OPENED -> horizontal_screen
                // move -> CLOSING. The PC data ships ONE door frame
                // (max_frames = width/width-1 = 0), so its frame animation is a
                // no-op and the door simply vanished; slide the sprite into the
                // floor and back (see door_slide) so the opening reads.
                sfx_play(SFX_DOOR_OPEN);
                door_slide(o, TRUE);             // open: sink out of sight
                // The scroll moves the player ~2.5 tiles forward itself (PC
                // horizontal_screen_move), leaving him near the left edge.
                stage_horizontal_screen_move(1);
                door_slide(o, FALSE);            // closes behind the player
                continue;
            }
            if (d->type == 21 || d->type == 26 || d->type == 31) {
                // Castle teleporters: same capsule trigger as the stage boss
                // teleporter (PC object_teleport_boss).
                if (d->type == 26 && !o->started) o->started = TRUE;
                stage_teleport_boss(o->dest_map, o->dest_x, o->dest_y);
                return;
            }
            if (d->type == 18 || d->type == 19 || d->type == 24 ||
                d->type == 25) {
                if (o->visible) player_take_damage(4);
                continue;
            }
            if (d->type == 9) {                     // 1UP
                sfx_play(SFX_ITEM);
                player_add_life();
                obj_release(o);
                continue;
            }
            if (d->type == 3 || d->type == 4) {      // E-Tank / W-Tank
                sfx_play(SFX_ITEM);
                player_add_tank(d->type == 3);
                obj_release(o);
                continue;
            }
            if (d->type == 29 && !o->started) {     // checkpoint (PC once)
                o->started = TRUE;
                player_set_checkpoint(o->x + d->w / 2 - 15, o->y + d->h);
            }
            if (d->type == 32) {                    // OBJ_STAGE_BOSS_TELEPORTER
                // PC is_on_teleporter_capsulse (character.cpp:2303): the
                // player fully inside the object's Y span (frame top/height,
                // PC position.y + frameSize) and his frame-centre X strictly
                // inside the centre band (|16-w|/2 inset from each side).
                s16 fh = PLAYER_BOX_B;              // art frame height 29
                if (o->y < player.y && o->y + d->h > player.y + fh) {
                    s16 inset = 16 - d->w;
                    if (inset < 0) inset = -inset;
                    inset >>= 1;
                    s16 px = player.x + 14;         // PC + frameSize.width/2
                    if (px > o->x + inset && px < o->x + d->w - inset) {
                        player.facing = 1;          // PC set_direction RIGHT
                        // stage_load -> objects_reset rebuilds objs[]; abort
                        stage_teleport_boss(o->dest_map, o->dest_x, o->dest_y);
                        return;
                    }
                }
            }
        }

        // ---- animation ----------------------------------------------------
        // The damaging platform's two frames are its two states (idle/damage),
        // driven by the state machine above, so it must not auto-advance.
        if (d->type != 28 && (auto_anim(d->type) || o->started ||
                              o->anim_play)) {
            if (++o->anim_t >= d->frame_t) {
                o->anim_t = 0;
                if (o->frame + 1 < d->frames) {
                    o->frame++;
                } else if (is_looping(d)) {
                    o->frame = 0;
                } else {
                    o->anim_play = 0;      // one-shot finished
                }
                if (o->spr) spr_set_frame(o->spr, o->frame);
            }
        }
    }
}
