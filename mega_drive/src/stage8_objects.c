#include "stage8_objects.h"
#include "resources.h"

const ObjectDef stage8_object_defs[STAGE8_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 1 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage8_object_spawns[STAGE8_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 1776, 96, 255, 255, 255 },
    { 0, 1, 112, 64, 255, 255, 255 },
    { 0, 1, 2624, 128, 255, 255, 255 },
    { 0, 2, 384, 128, 255, 255, 255 },
    { 1, 1, 2800, 128, 182, 5, 1 },
};
