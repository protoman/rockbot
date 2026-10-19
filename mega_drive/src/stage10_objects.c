#include "stage10_objects.h"
#include "resources.h"

const ObjectDef stage10_object_defs[STAGE10_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4, 400, 0 },  // Fall Block
    /* 1 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0 },  // Big Pill
    /* 2 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0 },  // E-Tank
    /* 3 */ { &object_weapons_tank_sprite, 4, 2, 18, 16, 16, 0, 0, 0 },  // W-Tank
    /* 4 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0 },  // Weapon Pill Big
    /* 5 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4, 0, 200 },  // Platform Fall (large)
    /* 6 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0 },  // Checkpoint
    /* 7 */ { &object_door_3_sprite, 30, 1, 1, 16, 48, 1, 0, 0 },  // Door [size 3]
    /* 8 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage10_object_spawns[STAGE10_OBJECT_SPAWN_COUNT] = {
    { 5, 0, 1456, 192, 255, 255, 255 },
    { 5, 0, 1792, 192, 255, 255, 255 },
    { 0, 0, 224, 160, 255, 255, 255 },
    { 2, 0, 336, 32, 255, 255, 255 },
    { 3, 0, 928, 80, 255, 255, 255 },
    { 0, 0, 272, 144, 255, 255, 255 },
    { 6, 0, 2256, 144, 255, 255, 255 },
    { 6, 1, 1696, 96, 255, 255, 255 },
    { 6, 2, 1040, 144, 255, 255, 255 },
    { 1, 1, 1632, 96, 255, 255, 255 },
    { 8, 1, 1824, 64, 120, 2, 1 },
    { 4, 1, 1776, 144, 255, 255, 255 },
    { 7, 2, 688, 80, 255, 255, 255 },
    { 7, 2, 992, 176, 255, 255, 255 },
    { 3, 1, 16, 80, 255, 255, 255 },
};
