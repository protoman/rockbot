#include "enemy.h"

#include "stage.h"
#include "player.h"
#include "resources.h"

// Simplified AI (per plan): every enemy patrols horizontally, walkers get
// gravity, flyers keep their spawn height. Turn at wall / ledge / limits.
#define ENEMY_GRAVITY FIX16(0.25)
#define ENEMY_MAX_FALL FIX16(6)

typedef struct {
    const EnemyDef *def;
    Sprite *spr;
    s16 x, y;         // cell origin (matches PC position = art top-left)
    s16 x_min, x_max; // precomputed patrol limits (converter)
    s16 dir;          // -1 / +1 (data 0 = left, 1 = right)
    fix16 vy;
    u8 hp;
    bool alive;
} Enemy;

static Enemy enemies[ENEMY_MAX];
static u8 enemy_count;

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

void enemies_reset(void)
{
    for (u8 i = 0; i < enemy_count; i++) {
        if (enemies[i].spr) SPR_releaseSprite(enemies[i].spr);
        enemies[i].spr = NULL;
        enemies[i].alive = FALSE;
    }
    enemy_count = 0;

    for (u8 i = 0; i < ENEMY_SPAWN_COUNT && enemy_count < ENEMY_MAX; i++) {
        const EnemySpawn *sp = &stage0_enemy_spawns[i];
        if (sp->map != stage_current_map) continue;

        const EnemyDef *d = &enemy_defs[sp->def];
        Enemy *e = &enemies[enemy_count++];
        e->def = d;
        e->x = sp->x;
        e->y = sp->y;
        e->x_min = sp->x_min;
        e->x_max = sp->x_max;
        e->dir = (sp->dir == 0) ? -1 : 1;
        e->vy = 0;
        e->hp = d->hp;
        e->spr = SPR_addSprite(d->spr, e->x - camera.x, e->y - camera.y,
                               TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        e->alive = (e->spr != NULL);
        if (e->spr) {
            // World X can exceed the 10-bit VDP sprite field (1184 wraps to
            // 160 on-screen). AUTO_FAST hides by X against screen width and
            // skips the SAT write; recomputed on every SPR_setPosition.
            SPR_setVisibility(e->spr, AUTO_FAST);
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

        // ---- horizontal patrol ------------------------------------------
        s16 nx = e->x + e->dir * (s16)d->speed;

        s16 lead = (e->dir > 0) ? nx + d->box_l + d->box_w - 1 : nx + d->box_l;
        s16 col = lead >> 4;
        s16 t = e->y + d->box_t;
        s16 b = e->y + d->box_t + d->box_h;

        bool turn = box_hits_solid(lead, t, lead, b);   // wall in front
        if (!turn && !d->fly) {
            // ledge: no solid tile under the leading foot
            if (stage_terrain_at(stage_current_map, col, b >> 4) != TERRAIN_SOLID)
                turn = TRUE;
        }
        if (turn) {
            e->dir = -e->dir;
            nx = e->x;
        }

        e->x = nx;
        if (e->x < e->x_min) { e->x = e->x_min; e->dir = 1; }
        if (e->x > e->x_max) { e->x = e->x_max; e->dir = -1; }

        // ---- gravity (walkers only) --------------------------------------
        if (!d->fly) {
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

        // ---- draw ----------------------------------------------------------
        if (e->spr) {
            SPR_setHFlip(e->spr, e->dir < 0);
            SPR_setPosition(e->spr, e->x - camera.x, e->y - camera.y);
        }

        // ---- contact damage ------------------------------------------------
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (pl < el + d->box_w && pr > el && pt < et + d->box_h && pb > et)
            player_take_damage(TOUCH_DAMAGE_SMALL);
    }
}

bool enemies_try_hit(s16 l, s16 t, s16 r, s16 b)
{
    for (u8 i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        const EnemyDef *d = e->def;
        s16 el = e->x + d->box_l;
        s16 et = e->y + d->box_t;
        if (l >= el + d->box_w || r <= el || t >= et + d->box_h || b <= et)
            continue;

        e->hp--;
        if (e->hp == 0) {
            if (e->spr) SPR_releaseSprite(e->spr);
            e->spr = NULL;
            e->alive = FALSE;
        }
        return TRUE;
    }
    return FALSE;
}
