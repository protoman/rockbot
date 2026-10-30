/* Host test for the boss door's collision state (report 2026-10-30:
 * "boss_stuck_door.png - the DESTRIN SHIP should not move or teleport inside a
 * door").
 *
 * The door object (type 30) opens once when the player touches it, scrolls the
 * room and closes again. The port latched "sequence already ran" in `started`
 * and answered the collision with `!started`, so once the door was closed on
 * screen again it stopped blocking - the DR. DESTRIN SHIP could then teleport
 * into it (AI_TELEPORT -> the guard asks objects_block_box()).
 *
 * Runs the REAL src/object.c against the counting stubs.
 * Run: tools/run_host_tests.sh
 */
#include <genesis.h>
#include <stdio.h>
#include <string.h>

#include "stage_data.h"
#include "object.h"
#include "player.h"
#include "stage.h"
#include "resources.h"

extern void host_terrain_fill(u8 value);
extern void host_obj_door_set(u8 i, u8 started, u8 door_open);

/* Stage 13 map 0, the DR. DESTRIN room: "Door [Size4]" 16x64 at (2864,112). */
#define DOOR_X 2864
#define DOOR_Y 112
#define DOOR_W 16
#define DOOR_H 64

/* DESTRIN SHIP box (EnemyDef: box_l 0, box_t 0, box_w 44, box_h 48) sitting on
 * the row it inherits from the tank (y = 83). */
#define SHIP_X 2856
#define SHIP_Y 83
#define SHIP_W 44
#define SHIP_H 48

static ObjectDef defs[2];
static ObjectSpawn spawns[2];
static StageBgInfo bg_info[1];
static StageDef stage;

static u8 failures;

static void build_stage(void)
{
    memset(defs, 0, sizeof defs);
    defs[0].spr = &object_door_4_sprite;
    defs[0].type = 30;                 /* OBJ boss door */
    defs[0].frames = 1;
    defs[0].frame_t = 1;
    defs[0].w = DOOR_W;
    defs[0].h = DOOR_H;

    spawns[0].def = 0;
    spawns[0].map = 0;
    spawns[0].x = DOOR_X;
    spawns[0].y = DOOR_Y;

    memset(&stage, 0, sizeof stage);
    stage.map_count = 1;
    stage.map_w = 200;
    stage.map_h = 14;
    stage.tile = 16;
    stage.terrain = NULL;
    stage.object_defs = defs;
    stage.object_def_count = 1;
    stage.object_spawns = spawns;
    stage.object_spawn_count = 1;
    stage.bg_info = bg_info;
    stage_def = &stage;
    stage_current_map = 0;
    camera.x = 0;
    camera.y = 0;
    player.x = 8;
    player.y = 96;
    player.hp = 28;
}

static void expect(const char *what, bool got, bool want)
{
    if (got == want) {
        printf("  ok    %s -> %s\n", what, got ? "blocks" : "passes");
        return;
    }
    printf("  FAIL  %s -> %s (want %s)\n", what, got ? "blocks" : "passes",
           want ? "blocks" : "passes");
    failures++;
}

/* Ask the collision the way the enemy code does: an enemy body box. */
static bool blocks(s16 l, s16 t, s16 r, s16 b)
{
    objects_update();                  /* refreshes the any_solid_object cache */
    return objects_block_box(l, t, r, b);
}

int main(void)
{
    host_terrain_fill(TERRAIN_UNBLOCKED);
    build_stage();
    objects_reset();

    printf("door collision (stage 13 boss door at %d,%d %dx%d)\n",
           DOOR_X, DOOR_Y, DOOR_W, DOOR_H);

    /* A door that was never opened must block. */
    host_obj_door_set(0, 0, 0);
    expect("closed, never opened", blocks(DOOR_X, DOOR_Y, DOOR_X + DOOR_W,
                                          DOOR_Y + DOOR_H), TRUE);

    /* While the door is open (sequences run, slices hidden) it must not block. */
    host_obj_door_set(0, 1, 1);
    expect("open", blocks(DOOR_X, DOOR_Y, DOOR_X + DOOR_W, DOOR_Y + DOOR_H),
           FALSE);

    /* The state the port is left in once the door closed again: the sequence
     * latch is set, the slices are visible. It must block again - this is the
     * reported bug. */
    host_obj_door_set(0, 1, 0);
    expect("closed again after opening",
           blocks(DOOR_X, DOOR_Y, DOOR_X + DOOR_W, DOOR_Y + DOOR_H), TRUE);

    /* The precise report: the DESTRIN SHIP teleport destination overlapping the
     * door has to be seen as blocked, otherwise the ship reappears inside it. */
    expect("DESTRIN SHIP box over the door",
           blocks(SHIP_X, SHIP_Y, SHIP_X + SHIP_W, SHIP_Y + SHIP_H), TRUE);

    printf("\n%s: %u door collision failure(s)\n", failures ? "FAIL" : "OK",
           failures);
    return failures ? 1 : 0;
}
