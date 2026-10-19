#include "stage7_objects.h"
#include "resources.h"

const ObjectDef stage7_object_defs[STAGE7_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0 },  // Big Pill
    /* 1 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0 },  // E-Tank
    /* 2 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0 },  // Weapon Pill Big
    /* 3 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0, 1000, 0 },  // Disappearing Block #1
    /* 4 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0, 2000, 0 },  // Disappearing Block #2
    /* 5 */ { &object_disappearing_block_blue_sprite, 2, 5, 4, 16, 16, 0, 3000, 0 },  // Disappearing Block #3
    /* 6 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0 },  // Checkpoint
    /* 7 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage7_object_spawns[STAGE7_OBJECT_SPAWN_COUNT] = {
    { 3, 0, 1616, 80, 255, 255, 255 },
    { 4, 0, 1664, 80, 255, 255, 255 },
    { 3, 0, 1104, 112, 255, 255, 255 },
    { 4, 0, 1152, 96, 255, 255, 255 },
    { 5, 0, 1200, 96, 255, 255, 255 },
    { 5, 0, 1712, 80, 255, 255, 255 },
    { 1, 0, 1808, 32, 255, 255, 255 },
    { 0, 1, 1312, 80, 255, 255, 255 },
    { 6, 1, 2656, 144, 255, 255, 255 },
    { 6, 2, 48, 48, 255, 255, 255 },
    { 6, 2, 1856, 96, 255, 255, 255 },
    { 0, 1, 2608, 192, 255, 255, 255 },
    { 2, 1, 2720, 160, 255, 255, 255 },
    { 7, 1, 2832, 112, 182, 4, 1 },
    { 1, 2, 2896, 64, 255, 255, 255 },
};
