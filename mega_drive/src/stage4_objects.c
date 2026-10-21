#include "stage4_objects.h"
#include "resources.h"

const ObjectDef stage4_object_defs[STAGE4_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0 },  // Small Pill
    /* 1 */ { &object_1up_sprite, 9, 4, 12, 16, 16, 0 },  // 1UP
    /* 2 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 3 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage4_object_spawns[STAGE4_OBJECT_SPAWN_COUNT] = {
    { 2, 0, 1120, 80, 255, 255, 255 },
    { 0, 1, 1024, 116, 255, 255, 255 },
    { 0, 1, 992, 116, 255, 255, 255 },
    { 0, 1, 960, 116, 255, 255, 255 },
    { 0, 1, 928, 116, 255, 255, 255 },
    { 2, 1, 1856, 96, 255, 255, 255 },
    { 2, 2, 1856, 96, 255, 255, 255 },
    { 2, 1, 48, 112, 255, 255, 255 },
    { 3, 2, 2048, 96, 133, 7, 2 },
    { 1, 2, 2704, 64, 255, 255, 255 },
};
