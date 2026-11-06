#include "stage4_objects.h"
#include "resources.h"

const ObjectDef stage4_object_defs[STAGE4_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0, 0, 0, 1, 1 },  // Small Pill
    /* 1 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Big Pill
    /* 2 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0, 1, 1 },  // E-Tank
    /* 3 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Weapon Pill Big
    /* 4 */ { &object_1up_sprite, 9, 4, 12, 16, 16, 0, 0, 0, 1, 1 },  // 1UP
    /* 5 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0, 0, 1 },  // Checkpoint
    /* 6 */ { &object_door_3_sprite, 30, 1, 1, 16, 48, 1, 0, 0, 0, 0 },  // Door [size 3]
    /* 7 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0, 1, 1 },  // Stage Boss Teleporter
};

const ObjectSpawn stage4_object_spawns[STAGE4_OBJECT_SPAWN_COUNT] = {
    { 5, 0, 1120, 80, 255, 255, 255 },
    { 1, 1, 3152, 128, 255, 255, 255 },
    { 2, 1, 1152, 80, 255, 255, 255 },
    { 0, 1, 1024, 116, 255, 255, 255 },
    { 0, 1, 992, 116, 255, 255, 255 },
    { 0, 1, 960, 116, 255, 255, 255 },
    { 0, 1, 928, 116, 255, 255, 255 },
    { 2, 1, 2640, 64, 255, 255, 255 },
    { 5, 1, 1856, 96, 255, 255, 255 },
    { 5, 2, 1856, 96, 255, 255, 255 },
    { 5, 1, 48, 112, 255, 255, 255 },
    { 1, 2, 1936, 144, 255, 255, 255 },
    { 3, 2, 1904, 144, 255, 255, 255 },
    { 6, 0, 1744, 128, 255, 255, 255 },
    { 6, 0, 2048, 128, 255, 255, 255 },
    { 7, 2, 2048, 96, 133, 7, 2 },
    { 1, 1, 928, 48, 255, 255, 255 },
    { 4, 2, 2704, 64, 255, 255, 255 },
};
