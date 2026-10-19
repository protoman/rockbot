#include "stage11_objects.h"
#include "resources.h"

const ObjectDef stage11_object_defs[STAGE11_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0 },  // Big Pill
    /* 1 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0 },  // E-Tank
    /* 2 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0 },  // Weapon Pill Big
    /* 3 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4, 0, 200 },  // Platform Flyier (large)
    /* 4 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0, 1000, 0 },  // Disappearing Block #1
    /* 5 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0, 2000, 0 },  // Disappearing Block #2
    /* 6 */ { &object_disappearing_block_blue_sprite, 2, 5, 4, 16, 16, 0, 3000, 0 },  // Disappearing Block #3
    /* 7 */ { &object_disappearing_block_orange_sprite, 14, 5, 4, 16, 16, 0, 500, 0 },  // Active Disappearing Block
    /* 8 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0 },  // Checkpoint
    /* 9 */ { &object_door_3_sprite, 30, 1, 1, 16, 48, 1, 0, 0 },  // Door [size 3]
    /* 10 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage11_object_spawns[STAGE11_OBJECT_SPAWN_COUNT] = {
    { 4, 0, 2192, 176, 255, 255, 255 },
    { 5, 0, 2128, 176, 255, 255, 255 },
    { 6, 0, 2128, 128, 255, 255, 255 },
    { 7, 0, 2080, 96, 255, 255, 255 },
    { 1, 0, 16, 32, 255, 255, 255 },
    { 8, 0, 2304, 112, 255, 255, 255 },
    { 3, 1, 464, 192, 255, 255, 255 },
    { 3, 1, 672, 160, 255, 255, 255 },
    { 3, 1, 864, 160, 255, 255, 255 },
    { 3, 1, 1024, 192, 255, 255, 255 },
    { 3, 1, 1072, 208, 255, 255, 255 },
    { 1, 2, 16, 48, 255, 255, 255 },
    { 7, 2, 688, 192, 255, 255, 255 },
    { 7, 2, 720, 144, 255, 255, 255 },
    { 7, 2, 752, 112, 255, 255, 255 },
    { 8, 2, 336, 144, 255, 255, 255 },
    { 0, 2, 400, 128, 255, 255, 255 },
    { 2, 2, 432, 96, 255, 255, 255 },
    { 9, 1, 1952, 16, 255, 255, 255 },
    { 10, 2, 560, 48, 40, 2, 2 },
};
