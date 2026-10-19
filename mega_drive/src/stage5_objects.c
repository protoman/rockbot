#include "stage5_objects.h"
#include "resources.h"

const ObjectDef stage5_object_defs[STAGE5_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0 },  // E-Tank
    /* 2 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 3 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 4 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage5_object_spawns[STAGE5_OBJECT_SPAWN_COUNT] = {
    { 3, 0, 1776, 128, 255, 255, 255 },
    { 3, 1, 1328, 128, 255, 255, 255 },
    { 1, 2, 528, 176, 255, 255, 255 },
    { 0, 1, 1424, 176, 255, 255, 255 },
    { 2, 1, 1456, 176, 255, 255, 255 },
    { 4, 1, 1536, 128, 101, 3, 1 },
};
