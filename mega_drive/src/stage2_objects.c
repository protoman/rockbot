#include "stage2_objects.h"
#include "resources.h"

const ObjectDef stage2_object_defs[STAGE2_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4 },  // Platform Fall (large)
    /* 1 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4 },  // Platform Flyier (large)
    /* 2 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 3 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage2_object_spawns[STAGE2_OBJECT_SPAWN_COUNT] = {
    { 2, 0, 1632, 128, 255, 255, 255 },
    { 0, 2, 2176, 80, 255, 255, 255 },
    { 1, 2, 1648, 176, 255, 255, 255 },
    { 2, 2, 2592, 112, 255, 255, 255 },
    { 2, 2, 864, 96, 255, 255, 255 },
    { 0, 2, 1952, 144, 255, 255, 255 },
    { 1, 2, 2272, 160, 255, 255, 255 },
    { 3, 2, 2816, 112, 181, 3, 2 },
};
