#pragma once

// Tracker music (.mod/.s3m/.xm/.it) on the PSP.
//
// The SDK's SDL_mixer plays tracker files with libmodplug, which reads unaligned memory and
// crashes a real PSP (PPSSPP tolerates it). They are decoded with libxmp-lite instead and fed to
// the mixer through Mix_HookMusic. Other formats (mp3/ogg/wav) still go through SDL_mixer.

#include "../../sdl_layer.h"

namespace psp_music {

bool is_tracker_file(const char *file);
SoundMusic *load(const char *file);              // an opaque handle; NULL on error
bool owns(const SoundMusic *music);              // true for handles created by load()
int play(SoundMusic *music, int loops);          // same loops meaning as Mix_PlayMusic; 0 = ok
void halt();                                     // stops the tracker music (if any)
void free_music(SoundMusic *music);
void set_volume(int volume);                     // 0..128, like Mix_VolumeMusic
unsigned int take_decode_us();                   // microseconds spent decoding since the last call (profiling)

}
