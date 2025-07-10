#pragma once

#ifdef SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL2_rotozoom.h>

# global
SDL_Window* window;
SDL_Renderer* renderer;

# fallback
#define SDL_HWSURFACE 0
#define SDL_DOUBLEBUF 1
#define SDL_RESIZABLE 2
#define SDL_FULLSCREEN 3

#else 

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_endian.h>
#include <SDL/SDL_rotozoom.h>

#endif

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);
SDL_Surface * SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags);