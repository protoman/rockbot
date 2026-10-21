#include "stage7_objects.h"
#include "resources.h"

const ObjectDef stage7_object_defs[STAGE7_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 1 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage7_object_spawns[STAGE7_OBJECT_SPAWN_COUNT] = {
    { 0, 1, 2656, 144, 255, 255, 255 },
    { 0, 2, 48, 48, 255, 255, 255 },
    { 0, 2, 1856, 96, 255, 255, 255 },
    { 1, 1, 2832, 112, 182, 4, 1 },
};
