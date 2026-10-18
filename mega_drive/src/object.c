#include "object.h"
#include "sfx.h"

#include "stage.h"
#include "player.h"
#include "resources.h"

// Map objects (pills, 1UP, fall/fly platforms, checkpoint, boss teleporter).
// Behavior mirrors PC objects/object.cpp for the types shipped in
// stage0_objects.c:
//   7  pill      pickup, hp+3 (cap PLAYER_INITIAL_HP)
//   9  1UP       pickup, no-op (lives are not modelled yet — M3 HUD)
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
    s16 distance;    // px travelled since MOVE started
    u8 state;
    u8 delay;        // activation-delay countdown (OBJ_START_DELAY)
    u8 anim_t;       // ticks inside the current frame
    u8 frame;
    u8 dest_x, dest_y, dest_map;  // type 32: teleport destination (tiles)
    bool started;
    bool active;
} MapObj;

static MapObj objs[OBJECT_MAX];
static u8 obj_count;

// Types that animate without player interaction (auto_start = 1 in data).
static bool auto_anim(u8 type)
{
    return type == 7 || type == 9 || type == 32;  // pill, 1UP, boss teleporter
}

static bool is_platform(u8 type)
{
    return type == 10 || type == 11; // OBJ_FALL_PLATFORM, OBJ_FLY_PLATFORM
}

static bool is_looping(const ObjectDef *d)
{
    // checkpoint frames are states (inactive/activated), so it must not
    // loop; the teleporter animates forever; platforms play once
    return d->type == 32;
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
    return stage_terrain_at(stage_current_map, c1, row) != TERRAIN_SOLID &&
           stage_terrain_at(stage_current_map, c2, row) != TERRAIN_SOLID;
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
    if (o->spr) SPR_setFrame(o->spr, 0);
}

static void obj_release(MapObj *o)
{
    if (o->spr) SPR_releaseSprite(o->spr);
    o->spr = NULL;
    o->active = FALSE;
}

void objects_reset(void)
{
    for (u8 i = 0; i < obj_count; i++) {
        if (objs[i].spr) SPR_releaseSprite(objs[i].spr);
        objs[i].spr = NULL;
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
        o->spr = SPR_addSprite(d->spr, o->x - camera.x, o->y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (o->spr) {
            // manual visibility + manual frame advance (see objects_update)
            SPR_setAutoAnimation(o->spr, FALSE);
            SPR_setVisibility(o->spr, VISIBLE);
        }
    }
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
        if (!o->active || !is_platform(o->def->type) || o->y != top)
            continue;
        // match the X range too: two platforms can share a top edge, and the
        // old y-only test always started the first one in the array
        if (l >= o->x + o->def->w || r <= o->x)
            continue;
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
        if (!o->active || !is_platform(o->def->type)) continue;
        // PC classmap.cpp:1654 — descending with the head at/above the top
        // and any horizontal touch. X is inclusive: the side clamp can leave
        // the box exactly at the face (r == o->x) and the landing must still
        // catch (corner entry).
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
        if (!o->active || !is_platform(o->def->type)) continue;
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
        if (!o->active || !is_platform(o->def->type)) continue;
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
        if (!o->active || !is_platform(o->def->type)) continue;
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

void objects_set_visible(bool visible)
{
    for (u8 i = 0; i < obj_count; i++)
        if (objs[i].spr)
            SPR_setVisibility(objs[i].spr, visible ? VISIBLE : HIDDEN);
}

void objects_update(void)
{
    s16 pl, pt, pr, pb;
    player_get_box(&pl, &pt, &pr, &pb);

    for (u8 i = 0; i < obj_count; i++) {
        MapObj *o = &objs[i];
        if (!o->active) continue;
        const ObjectDef *d = o->def;

        // ---- platform state machine (before pickup: player box is fresh) --
        if (is_platform(d->type) && o->started)
            obj_move(o);

        // ---- screen visibility -------------------------------------------
        s16 sx = o->x - camera.x;
        s16 sy = o->y - camera.y;
        bool on_screen = (sx + d->w > 0 && sx < 320 &&
                          sy + d->h > 0 && sy < 224);
        if (o->spr) {
            SPR_setVisibility(o->spr, on_screen ? VISIBLE : HIDDEN);
            if (on_screen) SPR_setPosition(o->spr, sx, sy);
        }
        if (!on_screen) continue;   // no pickup/anim while fully off-screen

        // ---- player overlap: pickup / checkpoint -------------------------
        if (pl < o->x + d->w && pr > o->x && pt < o->y + d->h && pb > o->y) {
            if (d->type == 7) {                     // small energy pill
                sfx_play(SFX_PILL);
                u8 hp = player.hp + 3;
                player.hp = (hp > PLAYER_INITIAL_HP) ? PLAYER_INITIAL_HP : hp;
                obj_release(o);
                continue;
            }
            if (d->type == 9) {                     // 1UP: consume, no-op
                sfx_play(SFX_ITEM);
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
        if (auto_anim(d->type) || o->started) {
            if (++o->anim_t >= d->frame_t) {
                o->anim_t = 0;
                if (o->frame + 1 < d->frames) {
                    o->frame++;
                } else if (is_looping(d)) {
                    o->frame = 0;
                }
                if (o->spr) SPR_setFrame(o->spr, o->frame);
            }
        }
    }
}
