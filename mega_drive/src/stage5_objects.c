#include "stage5_objects.h"
#include "resources.h"

const ObjectDef stage5_object_defs[STAGE5_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 1 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage5_object_spawns[STAGE5_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 1776, 128, 255, 255, 255 },
    { 0, 1, 1328, 128, 255, 255, 255 },
    { 1, 1, 1536, 128, 101, 3, 1 },
};
