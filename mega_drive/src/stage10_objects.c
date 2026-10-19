#include "stage10_objects.h"
#include "resources.h"

const ObjectDef stage10_object_defs[STAGE10_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4 },  // Fall Block
    /* 1 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4 },  // Platform Fall (large)
    /* 2 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 3 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage10_object_spawns[STAGE10_OBJECT_SPAWN_COUNT] = {
    { 1, 0, 1456, 192, 255, 255, 255 },
    { 1, 0, 1792, 192, 255, 255, 255 },
    { 0, 0, 224, 160, 255, 255, 255 },
    { 0, 0, 272, 144, 255, 255, 255 },
    { 2, 0, 2256, 144, 255, 255, 255 },
    { 2, 1, 1696, 96, 255, 255, 255 },
    { 2, 2, 1040, 144, 255, 255, 255 },
    { 3, 1, 1824, 64, 120, 2, 1 },
};
