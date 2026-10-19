#include "stage1_objects.h"
#include "resources.h"

const ObjectDef stage1_object_defs[STAGE1_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0, 0, 0 },  // Small Pill
    /* 1 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4, 400, 0 },  // Fall Block
    /* 2 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0 },  // Big Pill
    /* 3 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0 },  // E-Tank
    /* 4 */ { &object_weapons_tank_sprite, 4, 2, 18, 16, 16, 0, 0, 0 },  // W-Tank
    /* 5 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0 },  // Weapon Pill Big
    /* 6 */ { &object_armor_body_sprite, 16, 1, 566, 18, 14, 0, 0, 0 },  // Armor Piece [Body]
    /* 7 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0 },  // Checkpoint
    /* 8 */ { &object_door_4_sprite, 30, 1, 1, 16, 64, 0, 0, 0 },  // Door [Size4]
    /* 9 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage1_object_spawns[STAGE1_OBJECT_SPAWN_COUNT] = {
    { 1, 0, 768, 128, 255, 255, 255 },
    { 7, 0, 1904, 64, 255, 255, 255 },
    { 0, 1, 1248, 20, 255, 255, 255 },
    { 0, 1, 1264, 20, 255, 255, 255 },
    { 0, 1, 1280, 20, 255, 255, 255 },
    { 0, 1, 1296, 20, 255, 255, 255 },
    { 3, 1, 1344, 16, 255, 255, 255 },
    { 2, 1, 1312, 16, 255, 255, 255 },
    { 1, 1, 2000, 176, 255, 255, 255 },
    { 7, 1, 2624, 80, 255, 255, 255 },
    { 1, 2, 1056, 144, 255, 255, 255 },
    { 1, 2, 1104, 112, 255, 255, 255 },
    { 1, 2, 1168, 112, 255, 255, 255 },
    { 1, 2, 1232, 144, 255, 255, 255 },
    { 1, 2, 1280, 128, 255, 255, 255 },
    { 6, 2, 1760, 176, 255, 255, 255 },
    { 4, 2, 1488, 128, 255, 255, 255 },
    { 3, 2, 1440, 128, 255, 255, 255 },
    { 2, 0, 2576, 144, 255, 255, 255 },
    { 2, 1, 2720, 128, 255, 255, 255 },
    { 2, 2, 864, 112, 255, 255, 255 },
    { 5, 1, 2672, 128, 255, 255, 255 },
    { 8, 0, 2064, 112, 255, 255, 255 },
    { 8, 0, 2368, 112, 255, 255, 255 },
    { 9, 1, 2832, 80, 182, 1, 1 },
    { 2, 1, 688, 48, 255, 255, 255 },
};
