#include "stage12_objects.h"
#include "resources.h"

const ObjectDef stage12_object_defs[STAGE12_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 1 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage12_object_spawns[STAGE12_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 2624, 96, 255, 255, 255 },
    { 0, 1, 2240, 96, 255, 255, 255 },
    { 0, 2, 400, 80, 255, 255, 255 },
    { 1, 0, 2832, 96, 182, 3, 0 },
};
