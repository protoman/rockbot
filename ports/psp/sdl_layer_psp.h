#pragma once

#include "../../sdl_layer.h"

void SDLL_PSP_AdjustWindowSize(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags);
void SDLL_PSP_AdjustDisplayFormatAlpha(SDL_Surface *surface);
RWopsPtr *SDLL_PSP_RWFromFile(const char *file, const char *mode);
void SDLL_PSP_PrepareColorKey(SDL_Surface *surface, int sdl2_flag);
SoundChunk *SDLL_PSP_LoadWAV(const char *file);
SoundMusic *SDLL_PSP_LoadMUS(const char *file);
