#include "stage9_objects.h"
#include "resources.h"

const ObjectDef stage9_object_defs[STAGE9_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0, 0, 0, 1, 1 },  // Small Pill
    /* 1 */ { &object_fall_block_sprite, 10, 2, 6, 16, 16, 4, 400, 0, 0, 0 },  // Fall Block
    /* 2 */ { &object_energy_pill_big_sprite, 5, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Big Pill
    /* 3 */ { &object_energy_tank_sprite, 3, 2, 18, 16, 16, 0, 0, 0, 1, 1 },  // E-Tank
    /* 4 */ { &object_weapons_tank_sprite, 4, 2, 18, 16, 16, 0, 0, 0, 1, 1 },  // W-Tank
    /* 5 */ { &object_weapons_pill_big_sprite, 6, 2, 6, 14, 14, 0, 0, 0, 1, 1 },  // Weapon Pill Big
    /* 6 */ { &object_disappearing_block_sprite, 2, 5, 4, 16, 16, 0, 1000, 0, 0, 0 },  // Disappearing Block #1
    /* 7 */ { &object_disappearing_block_green_sprite, 2, 5, 4, 16, 16, 0, 2000, 0, 0, 0 },  // Disappearing Block #2
    /* 8 */ { &object_disappearing_block_orange_sprite, 14, 5, 4, 16, 16, 0, 500, 0, 0, 0 },  // Active Disappearing Block
    /* 9 */ { &object_fire_ray_vertical_sprite, 19, 10, 1, 16, 80, 0, 800, 0, 1, 1 },  // Fire Ray (Vertical)
    /* 10 */ { &object_fire_ray_horizontal_sprite, 18, 20, 1, 80, 16, 0, 0, 0, 1, 1 },  // Fire Ray (Horizontal)
    /* 11 */ { &object_lava_beam_sprite, 24, 3, 6, 48, 64, 0, 0, 0, 1, 1 },  // Lava Beam (Vertical)
    /* 12 */ { &object_deathray_small_horz_sprite, 25, 60, 1, 64, 16, 0, 20, 0, 1, 1 },  // Lava Bean Small 1(horizontal)
    /* 13 */ { &object_deathray_small_horz_sprite, 25, 60, 1, 64, 16, 0, 300, 0, 1, 1 },  // Lava Bean Small 2(horizontal)
    /* 14 */ { &object_deathray_small_horz_sprite, 25, 60, 1, 64, 16, 0, 600, 0, 1, 1 },  // Lava Bean Small 3(horizontal)
    /* 15 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0, 0, 0, 0, 1 },  // Checkpoint
    /* 16 */ { &object_teleporter_stage_boss_sprite, 32, 3, 7, 32, 64, 0, 0, 0, 1, 1 },  // Stage Boss Teleporter
};

const ObjectSpawn stage9_object_spawns[STAGE9_OBJECT_SPAWN_COUNT] = {
    { 3, 0, 272, 80, 255, 255, 255 },
    { 2, 0, 880, 112, 255, 255, 255 },
    { 9, 0, 400, 64, 255, 255, 255 },
    { 9, 0, 496, 64, 255, 255, 255 },
    { 9, 0, 1184, 64, 255, 255, 255 },
    { 10, 0, 2256, 48, 1, 255, 255 },
    { 10, 0, 1264, 112, 1, 255, 255 },
    { 10, 0, 240, 48, 1, 255, 255 },
    { 11, 0, 2480, 16, 255, 255, 255 },
    { 12, 0, 1792, 48, 0, 255, 255 },
    { 13, 0, 1792, 96, 0, 255, 255 },
    { 6, 1, 2048, 128, 255, 255, 255 },
    { 7, 1, 2112, 144, 255, 255, 255 },
    { 8, 1, 2160, 128, 255, 255, 255 },
    { 10, 1, 448, 128, 1, 255, 255 },
    { 9, 1, 80, 160, 255, 255, 255 },
    { 9, 1, 176, 160, 255, 255, 255 },
    { 9, 1, 1808, 144, 255, 255, 255 },
    { 15, 1, 2624, 144, 255, 255, 255 },
    { 13, 1, 1280, 112, 1, 255, 255 },
    { 12, 1, 1584, 48, 0, 255, 255 },
    { 13, 1, 1584, 80, 0, 255, 255 },
    { 14, 1, 1584, 176, 0, 255, 255 },
    { 9, 2, 144, 80, 255, 255, 255 },
    { 12, 2, 1888, 48, 0, 255, 255 },
    { 11, 2, 2640, 16, 255, 255, 255 },
    { 14, 2, 1888, 176, 0, 255, 255 },
    { 15, 2, 2816, 64, 255, 255, 255 },
    { 13, 2, 1584, 96, 1, 255, 255 },
    { 5, 2, 2160, 112, 255, 255, 255 },
    { 3, 2, 2064, 112, 255, 255, 255 },
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
    { 5, 1, 2704, 192, 255, 255, 255 },
    { 16, 1, 2832, 144, 182, 1, 1 },
    { 4, 0, 2976, 32, 255, 255, 255 },
};
