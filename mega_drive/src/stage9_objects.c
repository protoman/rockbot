#include "stage9_objects.h"
#include "resources.h"

const ObjectDef stage9_object_defs[STAGE9_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0 },  // Small Pill
    /* 1 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4 },  // Fall Block
    /* 2 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 3 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 4 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #1
    /* 5 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #2
    /* 6 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 7 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage9_object_spawns[STAGE9_OBJECT_SPAWN_COUNT] = {
    { 2, 0, 880, 112, 255, 255, 255 },
    { 4, 1, 2048, 128, 255, 255, 255 },
    { 5, 1, 2112, 144, 255, 255, 255 },
    { 6, 1, 2624, 144, 255, 255, 255 },
    { 6, 2, 2816, 64, 255, 255, 255 },
    { 3, 2, 2160, 112, 255, 255, 255 },
    { 1, 2, 2400, 192, 255, 255, 255 },
    { 1, 2, 2656, 176, 255, 255, 255 },
    { 1, 2, 2944, 192, 255, 255, 255 },
    { 0, 0, 2320, 84, 255, 255, 255 },
    { 0, 0, 2288, 84, 255, 255, 255 },
    { 0, 0, 2256, 84, 255, 255, 255 },
    { 0, 0, 2224, 84, 255, 255, 255 },
    { 0, 0, 2192, 84, 255, 255, 255 },
    { 0, 1, 1840, 132, 255, 255, 255 },
    { 0, 1, 1776, 132, 255, 255, 255 },
    { 2, 1, 2736, 192, 255, 255, 255 },
    { 3, 1, 2704, 192, 255, 255, 255 },
    { 7, 1, 2832, 144, 182, 1, 1 },
};
