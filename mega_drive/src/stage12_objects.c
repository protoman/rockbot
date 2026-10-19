#include "stage12_objects.h"
#include "resources.h"

const ObjectDef stage12_object_defs[STAGE12_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 2 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #1
    /* 3 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #2
    /* 4 */ { &object_disappearing_block_blue_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #3
    /* 5 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 6 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage12_object_spawns[STAGE12_OBJECT_SPAWN_COUNT] = {
    { 2, 0, 256, 176, 255, 255, 255 },
    { 5, 0, 2624, 96, 255, 255, 255 },
    { 3, 0, 1680, 176, 255, 255, 255 },
    { 2, 0, 1744, 176, 255, 255, 255 },
    { 4, 0, 1664, 128, 255, 255, 255 },
    { 2, 0, 1984, 208, 255, 255, 255 },
    { 3, 0, 2080, 176, 255, 255, 255 },
    { 3, 0, 1776, 128, 255, 255, 255 },
    { 2, 1, 352, 192, 255, 255, 255 },
    { 5, 1, 2240, 96, 255, 255, 255 },
    { 3, 1, 320, 144, 255, 255, 255 },
    { 0, 1, 1856, 176, 255, 255, 255 },
    { 2, 2, 192, 112, 255, 255, 255 },
    { 5, 2, 400, 80, 255, 255, 255 },
    { 1, 0, 2704, 144, 255, 255, 255 },
    { 0, 0, 2672, 144, 255, 255, 255 },
    { 6, 0, 2832, 96, 182, 3, 0 },
};
