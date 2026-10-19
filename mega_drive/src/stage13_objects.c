#include "stage13_objects.h"
#include "resources.h"

const ObjectDef stage13_object_defs[STAGE13_OBJECT_DEF_COUNT] = {
    /* 0 */ { &object_energy_pill_small_sprite, 7, 2, 6, 10, 10, 0 },  // Small Pill
    /* 1 */ { &object_checkpoint_sprite, 29, 2, 18, 32, 64, 0 },  // Checkpoint
};

const ObjectSpawn stage13_object_spawns[STAGE13_OBJECT_SPAWN_COUNT] = {
    { 0, 0, 192, 120, 255, 255, 255 },
    { 0, 0, 208, 120, 255, 255, 255 },
    { 0, 0, 224, 120, 255, 255, 255 },
    { 0, 0, 464, 104, 255, 255, 255 },
    { 0, 0, 480, 104, 255, 255, 255 },
    { 1, 0, 1664, 112, 255, 255, 255 },
    { 1, 2, 80, 144, 16, 9, 255 },
    { 0, 0, 240, 120, 255, 255, 255 },
};
