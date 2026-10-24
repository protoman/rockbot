/* Host cost test: work per frame of objects_update() as a function of the
 * number of objects on screen.
 *
 * Report 2026-10-24 ("slowdown when several objects are on screen"): the
 * profiler in CHAOS CITY put objects_update at 85 subticks with 2-3 objects
 * visible and 263-350 with the whole list on screen (1280 subticks = a 60 Hz
 * frame). This reproduces the *work*, not the wall clock: it runs the REAL
 * src/object.c against counting stubs and prints how many predicate
 * evaluations, terrain lookups and sprite calls one frame costs.
 *
 * The regression it guards is mechanical: the type predicates (is_platform,
 * object_has_gravity, auto_anim, is_looping) are long chains over the PC ids
 * and must be answered once per object, not several times per object per
 * frame.
 *
 * Pre-fix the same harness printed 7/21/36/50 predicate answers per frame for
 * 2/6/10/14 objects; post-fix it prints 0 per frame (they are answered once per
 * object at spawn). The last column is a *model* of objects_update() in the
 * profiler's units, calibrated on the one real measurement (CHAOS CITY:
 * ~85 subticks with 3 objects on screen, ~350 with 14, i.e. about 3.6
 * subticks per counted work unit and a ~38 subtick base). 1280 subticks = one
 * 60 Hz frame.
 *
 * Build/run: tools/run_host_tests.sh
 */
#include <genesis.h>
#include <stdio.h>
#include <string.h>

#include "stage_data.h"
#include "object.h"
#include "player.h"
#include "stage.h"
#include "resources.h"

/* counters owned by stubs.c */
extern u32 host_terrain_calls;
extern u32 host_spr_calls;
extern void host_terrain_fill(u8 value);
/* counters owned by object.c under MD_HOST_TEST */
extern u32 host_pred_calls;

static ObjectDef defs[1];
static ObjectSpawn spawns[OBJECT_MAX];
static const u8 *terrain_arrays[1];
static StageBgInfo bg_info[1];

static void build_stage(u8 objects)
{
    /* one small-pill def: auto-animating, no gravity, not a platform */
    memset(defs, 0, sizeof defs);
    defs[0].spr = &object_energy_pill_small_sprite;
    defs[0].type = 7;              /* ENERGY_PILL_SMALL */
    defs[0].frames = 2;
    defs[0].frame_t = 6;
    defs[0].w = 10;
    defs[0].h = 10;
    defs[0].anim_auto = 1;

    for (u8 i = 0; i < objects; i++) {
        spawns[i].def = 0;
        spawns[i].map = 0;
        spawns[i].x = (s16)(40 + i * 16);   /* all on screen */
        spawns[i].y = 100;
    }

    static StageDef stage;
    memset(&stage, 0, sizeof stage);
    stage.map_count = 1;
    stage.map_w = 200;
    stage.map_h = 14;
    stage.tile = 16;
    terrain_arrays[0] = (const u8 *)0;
    bg_info[0].present = 0;
    stage.terrain = NULL;              /* stage_terrain_at is stubbed */
    stage.object_defs = defs;
    stage.object_def_count = 1;
    stage.object_spawns = spawns;
    stage.object_spawn_count = objects;
    stage.bg_info = bg_info;
    stage_def = &stage;
    stage_current_map = 0;
    camera.x = 0;
    camera.y = 0;
}

static void reset_counters(void)
{
    host_terrain_calls = 0;
    host_spr_calls = 0;
    host_pred_calls = 0;
}

int main(void)
{
    host_terrain_fill(TERRAIN_UNBLOCKED);
    /* place the player next to the pills but not touching them */
    player.x = 8;
    player.y = 96;
    player.hp = 28;

    printf("objects on screen | pred/frame | terrain/frame | sprite calls/frame"
           " | est. subticks\n");
    u8 worst = 0;
    for (u8 n = 2; n <= 14; n += 4) {
        build_stage(n);
        objects_reset();
        reset_counters();
        enum { FRAMES = 60 };
        for (u16 f = 0; f < FRAMES; f++) objects_update();
        const u32 pred = host_pred_calls / FRAMES;
        const u32 terr = host_terrain_calls / FRAMES;
        const u32 spr = host_spr_calls / FRAMES;
        const u32 work = pred + terr + spr;
        const u32 est = 38 + (work * 36) / 10;
        printf("%17u | %10u | %13u | %18u | %12u\n", n, pred, terr, spr, est);
        /* one answer per object: a few extra for the spawn/reset path */
        if (pred > n) worst = 1;
        if (terr > (u32)n + 2 || spr > (u32)n * 2 + 4) worst = 1;
    }

    if (worst) {
        printf("\nFAIL: type predicates are answered more than once per "
               "object per frame (the flag cache regressed)\n");
        return 1;
    }
    printf("\nOK: predicates are answered once per object per frame\n");
    return 0;
}
