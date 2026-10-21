#include "stage6_objects.h"
#include "resources.h"

const ObjectDef stage6_object_defs[STAGE6_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4 },  // Platform Fall (large)
    /* 1 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4 },  // Platform Flyier (large)
    /* 2 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 3 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage6_object_spawns[STAGE6_OBJECT_SPAWN_COUNT] = {
    { 2, 0, 2032, 80, 255, 255, 255 },
    { 0, 1, 1472, 176, 255, 255, 255 },
    { 2, 1, 2640, 144, 255, 255, 255 },
    { 0, 2, 2400, 112, 255, 255, 255 },
    { 1, 2, 2624, 128, 255, 255, 255 },
    { 0, 2, 2080, 80, 255, 255, 255 },
    { 2, 2, 672, 64, 255, 255, 255 },
    { 3, 1, 2816, 144, 182, 4, 1 },
};
