#include "stage1_objects.h"
#include "resources.h"

const ObjectDef stage1_object_defs[STAGE1_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0 },  // Small Pill
    /* 1 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4 },  // Fall Block
    /* 2 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 3 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage1_object_spawns[STAGE1_OBJECT_SPAWN_COUNT] = {
    { 1, 0, 768, 128, 255, 255, 255 },
    { 2, 0, 1904, 64, 255, 255, 255 },
    { 0, 1, 1248, 20, 255, 255, 255 },
    { 0, 1, 1264, 20, 255, 255, 255 },
    { 0, 1, 1280, 20, 255, 255, 255 },
    { 0, 1, 1296, 20, 255, 255, 255 },
    { 1, 1, 2000, 176, 255, 255, 255 },
    { 2, 1, 2624, 80, 255, 255, 255 },
    { 1, 2, 1056, 144, 255, 255, 255 },
    { 1, 2, 1104, 112, 255, 255, 255 },
    { 1, 2, 1168, 112, 255, 255, 255 },
    { 1, 2, 1232, 144, 255, 255, 255 },
    { 1, 2, 1280, 128, 255, 255, 255 },
    { 3, 1, 2832, 80, 182, 1, 1 },
};
