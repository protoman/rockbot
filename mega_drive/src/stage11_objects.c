#include "stage11_objects.h"
#include "resources.h"

const ObjectDef stage11_object_defs[STAGE11_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 2 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4 },  // Platform Flyier (large)
    /* 3 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #1
    /* 4 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #2
    /* 5 */ { &object_disappearing_block_blue_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #3
    /* 6 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 7 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage11_object_spawns[STAGE11_OBJECT_SPAWN_COUNT] = {
    { 3, 0, 2192, 176, 255, 255, 255 },
    { 4, 0, 2128, 176, 255, 255, 255 },
    { 5, 0, 2128, 128, 255, 255, 255 },
    { 6, 0, 2304, 112, 255, 255, 255 },
    { 2, 1, 464, 192, 255, 255, 255 },
    { 2, 1, 672, 160, 255, 255, 255 },
    { 2, 1, 864, 160, 255, 255, 255 },
    { 2, 1, 1024, 192, 255, 255, 255 },
    { 2, 1, 1072, 208, 255, 255, 255 },
    { 6, 2, 336, 144, 255, 255, 255 },
    { 0, 2, 400, 128, 255, 255, 255 },
    { 1, 2, 432, 96, 255, 255, 255 },
    { 7, 2, 560, 48, 40, 2, 2 },
};
