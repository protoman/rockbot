#include "stage11_objects.h"
#include "resources.h"

const ObjectDef stage11_object_defs[STAGE11_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4 },  // Platform Flyier (large)
    /* 1 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 2 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage11_object_spawns[STAGE11_OBJECT_SPAWN_COUNT] = {
    { 1, 0, 2304, 112, 255, 255, 255 },
    { 0, 1, 464, 192, 255, 255, 255 },
    { 0, 1, 672, 160, 255, 255, 255 },
    { 0, 1, 864, 160, 255, 255, 255 },
    { 0, 1, 1024, 192, 255, 255, 255 },
    { 0, 1, 1072, 208, 255, 255, 255 },
    { 1, 2, 336, 144, 255, 255, 255 },
    { 2, 2, 560, 48, 40, 2, 2 },
};
