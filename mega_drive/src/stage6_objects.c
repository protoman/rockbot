#include "stage6_objects.h"
#include "resources.h"

const ObjectDef stage6_object_defs[STAGE6_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0 },  // Big Pill
    /* 1 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0 },  // E-Tank
    /* 2 */ { &object_weapons_tank_sprite, 4, 2, 18, 16, 16, 0, 0, 0 },  // W-Tank
    /* 3 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0 },  // Weapon Pill Big
    /* 4 */ { &object_platform_down_sprite, 10, 2, 6, 32, 16, 4, 0, 200 },  // Platform Fall (large)
    /* 5 */ { &object_platform_up_sprite, 11, 2, 6, 32, 16, 4, 0, 200 },  // Platform Flyier (large)
    /* 6 */ { &object_special_tank_sprite, 23, 2, 566, 16, 16, 0, 0, 0 },  // Special-Tank
    /* 7 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0, 1000, 0 },  // Disappearing Block #1
    /* 8 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0, 2000, 0 },  // Disappearing Block #2
    /* 9 */ { &object_disappearing_block_orange_sprite, 14, 5, 4, 16, 16, 0, 500, 0 },  // Active Disappearing Block
    /* 10 */ { &object_armor_arms_sprite, 15, 1, 566, 18, 14, 0, 0, 0 },  // Armor Piece [Arms]
    /* 11 */ { &object_block003_sprite, 22, 1, 1, 32, 32, 0, 0, 0 },  // Destructible Block
    /* 12 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0 },  // Checkpoint
    /* 13 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0 },  // Stage Boss Teleporter
};

const ObjectSpawn stage6_object_spawns[STAGE6_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 1712, 48, 255, 255, 255 },
    { 3, 0, 1840, 48, 255, 255, 255 },
    { 1, 0, 2784, 176, 255, 255, 255 },
    { 12, 0, 2032, 80, 255, 255, 255 },
    { 4, 1, 1472, 176, 255, 255, 255 },
    { 12, 1, 2640, 144, 255, 255, 255 },
    { 7, 2, 768, 144, 255, 255, 255 },
    { 9, 2, 960, 160, 255, 255, 255 },
    { 7, 2, 1168, 176, 255, 255, 255 },
    { 8, 2, 1232, 160, 255, 255, 255 },
    { 9, 2, 1536, 128, 255, 255, 255 },
    { 2, 2, 1952, 48, 255, 255, 255 },
    { 1, 2, 1920, 48, 255, 255, 255 },
    { 10, 2, 3040, 80, 255, 255, 255 },
    { 4, 2, 2400, 112, 255, 255, 255 },
    { 5, 2, 2624, 128, 255, 255, 255 },
    { 6, 2, 1888, 48, 255, 255, 255 },
    { 9, 2, 832, 160, 255, 255, 255 },
    { 4, 2, 2080, 80, 255, 255, 255 },
    { 9, 2, 896, 144, 255, 255, 255 },
    { 9, 2, 1664, 112, 255, 255, 255 },
    { 9, 2, 1712, 80, 255, 255, 255 },
    { 9, 2, 1776, 80, 255, 255, 255 },
    { 9, 2, 1600, 128, 255, 255, 255 },
    { 9, 2, 2480, 128, 255, 255, 255 },
    { 9, 2, 2704, 144, 255, 255, 255 },
    { 12, 2, 672, 64, 255, 255, 255 },
    { 3, 1, 2736, 192, 255, 255, 255 },
    { 0, 1, 2704, 192, 255, 255, 255 },
    { 13, 1, 2816, 144, 182, 4, 1 },
    { 11, 1, 336, 192, 255, 255, 255 },
};
