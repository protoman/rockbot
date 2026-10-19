#include "stage6_objects.h"
#include "resources.h"

const ObjectDef stage6_object_defs[STAGE6_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0 },  // Big Pill
    /* 1 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0 },  // Weapon Pill Big
    /* 2 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4 },  // Platform Fall (large)
    /* 3 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4 },  // Platform Flyier (large)
    /* 4 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #1
    /* 5 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0 },  // Disappearing Block #2
    /* 6 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
    /* 7 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage6_object_spawns[STAGE6_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 1712, 48, 255, 255, 255 },
    { 1, 0, 1840, 48, 255, 255, 255 },
    { 6, 0, 2032, 80, 255, 255, 255 },
    { 2, 1, 1472, 176, 255, 255, 255 },
    { 6, 1, 2640, 144, 255, 255, 255 },
    { 4, 2, 768, 144, 255, 255, 255 },
    { 4, 2, 1168, 176, 255, 255, 255 },
    { 5, 2, 1232, 160, 255, 255, 255 },
    { 2, 2, 2400, 112, 255, 255, 255 },
    { 3, 2, 2624, 128, 255, 255, 255 },
    { 2, 2, 2080, 80, 255, 255, 255 },
    { 6, 2, 672, 64, 255, 255, 255 },
    { 1, 1, 2736, 192, 255, 255, 255 },
    { 0, 1, 2704, 192, 255, 255, 255 },
    { 7, 1, 2816, 144, 182, 4, 1 },
};
