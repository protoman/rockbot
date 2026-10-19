#include "stage8_objects.h"
#include "resources.h"

const ObjectDef stage8_object_defs[STAGE8_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 2 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #1
    /* 3 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #2
    /* 4 */ { &object_disappearing_block_blue_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #3
    /* 5 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 6 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage8_object_spawns[STAGE8_OBJECT_SPAWN_COUNT] = {
    { 3, 0, 2288, 128, 255, 255, 255 },
    { 2, 0, 2128, 144, 255, 255, 255 },
    { 4, 0, 2608, 176, 255, 255, 255 },
    { 5, 0, 1776, 96, 255, 255, 255 },
    { 0, 1, 1120, 80, 255, 255, 255 },
    { 5, 1, 112, 64, 255, 255, 255 },
    { 5, 1, 2624, 128, 255, 255, 255 },
    { 0, 2, 128, 160, 255, 255, 255 },
    { 1, 2, 192, 160, 255, 255, 255 },
    { 2, 2, 1008, 128, 255, 255, 255 },
    { 3, 2, 1072, 144, 255, 255, 255 },
    { 5, 2, 384, 128, 255, 255, 255 },
    { 1, 1, 2720, 176, 255, 255, 255 },
    { 0, 1, 2688, 176, 255, 255, 255 },
    { 6, 1, 2800, 128, 182, 5, 1 },
};
