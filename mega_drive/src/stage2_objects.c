#include "stage2_objects.h"
#include "resources.h"

const ObjectDef stage2_object_defs[STAGE2_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Big Pill
    /* 1 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0, 1, 1 },  // E-Tank
    /* 2 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Weapon Pill Big
    /* 3 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4, 0, 200, 0, 0 },  // Platform Fall (large)
    /* 4 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4, 0, 200, 0, 0 },  // Platform Flyier (large)
    /* 5 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0, 0, 1 },  // Checkpoint
    /* 6 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0, 1, 1 },  // Stage Boss Teleporter
};

const ObjectSpawn stage2_object_spawns[STAGE2_OBJECT_SPAWN_COUNT] = {
    { 5, 0, 1632, 128, 255, 255, 255 },
    { 3, 2, 2176, 80, 255, 255, 255 },
    { 4, 2, 1648, 176, 255, 255, 255 },
    { 5, 2, 2592, 112, 255, 255, 255 },
    { 5, 2, 864, 96, 255, 255, 255 },
    { 3, 2, 1952, 144, 255, 255, 255 },
    { 4, 2, 2272, 160, 255, 255, 255 },
    { 1, 2, 112, 80, 255, 255, 255 },
    { 0, 1, 672, 144, 255, 255, 255 },
    { 0, 2, 704, 64, 255, 255, 255 },
    { 0, 2, 2704, 160, 255, 255, 255 },
    { 2, 2, 2656, 160, 255, 255, 255 },
    { 6, 2, 2816, 112, 181, 3, 2 },
    { 0, 2, 1600, 32, 255, 255, 255 },
};
