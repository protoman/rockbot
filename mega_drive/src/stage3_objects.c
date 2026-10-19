#include "stage3_objects.h"
#include "resources.h"

const ObjectDef stage3_object_defs[STAGE3_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_weapons_tank_sprite, 4, 2, 18, 16, 16, 0 },  // W-Tank
    /* 2 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 3 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 4 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage3_object_spawns[STAGE3_OBJECT_SPAWN_COUNT] = {
    { 1, 0, 1968, 64, 255, 255, 255 },
    { 0, 1, 704, 64, 255, 255, 255 },
    { 2, 1, 896, 64, 255, 255, 255 },
    { 3, 1, 2320, 128, 255, 255, 255 },
    { 3, 1, 1776, 144, 255, 255, 255 },
    { 3, 2, 384, 128, 255, 255, 255 },
    { 3, 0, 1600, 16, 255, 255, 255 },
    { 0, 1, 2128, 176, 255, 255, 255 },
    { 0, 2, 480, 176, 255, 255, 255 },
    { 2, 2, 448, 176, 255, 255, 255 },
    { 4, 2, 608, 128, 43, 3, 2 },
    { 0, 1, 16, 48, 255, 255, 255 },
};
