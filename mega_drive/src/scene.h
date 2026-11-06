#ifndef SCENE_H
#define SCENE_H

#include <genesis.h>

// Scene indices in src/scenes_data.c (PC scenes_list.dat order).
#define SCENE_OPENING_UNBEATEN 0

// Play one PC scene script. Returns early when START is pressed (the PC
// sceneShow::_interrupt_scene semantics: START skips the whole scene).
void scene_play(u8 index);

#endif
