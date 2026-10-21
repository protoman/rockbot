#include "stage3_objects.h"
#include "resources.h"

const ObjectDef stage3_object_defs[STAGE3_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 1 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage3_object_spawns[STAGE3_OBJECT_SPAWN_COUNT] = {
    { 0, 1, 2320, 128, 255, 255, 255 },
    { 0, 1, 1776, 144, 255, 255, 255 },
    { 0, 2, 384, 128, 255, 255, 255 },
    { 0, 0, 1600, 16, 255, 255, 255 },
    { 1, 2, 608, 128, 43, 3, 2 },
};
