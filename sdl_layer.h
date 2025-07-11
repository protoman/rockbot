#pragma once

#ifdef SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL2_rotozoom.h>

// global
extern SDL_Window *window;
extern SDL_Renderer *renderer;

// fallback sdl1
#define SDL_HWSURFACE 0x00000001
#define SDL_ASYNCBLIT 0x00000004
#define SDL_ANYFORMAT 0x10000000
#define SDL_HWPALETTE 0x20000000
#define SDL_DOUBLEBUF 0x40000000
#define SDL_FULLSCREEN 0x80000000
#define SDL_RESIZABLE 0x00000010
#define SDL_NOFRAME 0x00000020

#define SDL_SRCALPHA 0x00010000
#define SDL_SRCCOLORKEY 0x00001000

#else

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_endian.h>
#include <SDL/SDL_rotozoom.h>

#endif

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);

SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags);

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface);

void SDLL_WM_SetCaption(const char *title, const char *icon);

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface);

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask);

int SDLL_Flip(SDL_Surface *screen);

const char* SDLL_GetKeyName(int key);

const char *SDLL_JoystickName(int device_index);