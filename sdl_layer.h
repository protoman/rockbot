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

// Macro wrapper
#define SDLL_MUSTLOCK(surface) SDL_MUSTLOCK(surface)

/**
 * @brief Sets the alpha value of a surface.
 */
int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);

/**
 * @brief Sets the video mode.
 */
SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags);

/**
 * @brief Performs a fast stretch blit.
 */
int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

/**
 * @brief Converts a surface to the display format.
 */
SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface);

/**
 * @brief Sets the window caption.
 */
void SDLL_WM_SetCaption(const char *title, const char *icon);

/**
 * @brief Converts a surface to the display format with alpha.
 */
SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface);

/**
 * @brief Sets the window icon.
 */
void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask);

/**
 * @brief Swaps screen buffers (flips the screen).
 */
int SDLL_Flip(SDL_Surface *screen);

/**
 * @brief Gets the name of a key.
 */
const char* SDLL_GetKeyName(int key);

/**
 * @brief Gets the name of a joystick device.
 */
const char *SDLL_JoystickName(int device_index);

/**
 * @brief Gets the compiled SDL version.
 */
const char *SDLL_GetCompiledVersion();

// NEW WRAPPERS

/**
 * @brief Initializes SDL subsystem.
 */
int SDLL_Init(Uint32 flags);

/**
 * @brief Shuts down SDL subsystem.
 */
void SDLL_Quit();

/**
 * @brief Gets the number of joysticks.
 */
int SDLL_NumJoysticks();

/**
 * @brief Shows or hides cursor.
 */
int SDLL_ShowCursor(int toggle);

/**
 * @brief Sets an SDL environment variable.
 */
int SDLL_putenv(const char *variable);

/**
 * @brief Opens a data stream from a file.
 */
SDL_RWops *SDLL_RWFromFile(const char *file, const char *mode);

/**
 * @brief Frees a surface.
 */
void SDLL_FreeSurface(SDL_Surface *surface);

/**
 * @brief Sets the color key.
 */
int SDLL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);

/**
 * @brief Blits a surface area to a destination surface.
 */
int SDLL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

/**
 * @brief Creates an RGB surface.
 */
SDL_Surface *SDLL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

/**
 * @brief Fills a rectangle with a color.
 */
int SDLL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);

/**
 * @brief Locks a surface.
 */
int SDLL_LockSurface(SDL_Surface *surface);

/**
 * @brief Unlocks a surface.
 */
void SDLL_UnlockSurface(SDL_Surface *surface);

/**
 * @brief Delays execution.
 */
void SDLL_Delay(Uint32 ms);

/**
 * @brief Gets elapsed ticks.
 */
Uint32 SDLL_GetTicks();

/**
 * @brief Gets last SDL error.
 */
const char *SDLL_GetError();

/**
 * @brief Polls for pending events.
 */
int SDLL_PollEvent(SDL_Event *event);

/**
 * @brief Pumps event loop.
 */
void SDLL_PumpEvents();

/**
 * @brief Opens a joystick device.
 */
SDL_Joystick *SDLL_JoystickOpen(int device_index);

/**
 * @brief Closes a joystick device.
 */
void SDLL_JoystickClose(SDL_Joystick *joystick);

/**
 * @brief Sets joystick event state.
 */
int SDLL_JoystickEventState(int state);

/**
 * @brief Loads a image from a stream.
 */
SDL_Surface *SDLL_IMG_Load_RW(SDL_RWops *src, int freesrc);

/**
 * @brief Gets last SDL_image error.
 */
const char *SDLL_IMG_GetError();

/**
 * @brief Initializes SDL_ttf.
 */
int SDLL_TTF_Init();

/**
 * @brief Opens a font file.
 */
TTF_Font *SDLL_TTF_OpenFont(const char *file, int size);

/**
 * @brief Opens a font from stream.
 */
TTF_Font *SDLL_TTF_OpenFontRW(SDL_RWops *src, int freesrc, int size);

/**
 * @brief Sets outline width of a font.
 */
void SDLL_TTF_SetFontOutline(TTF_Font *font, int outline);

/**
 * @brief Renders UTF-8 text to surface.
 */
SDL_Surface *SDLL_TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg);

/**
 * @brief Gets last SDL_ttf error.
 */
const char *SDLL_TTF_GetError();

/**
 * @brief Zooms a surface.
 */
SDL_Surface *SDLL_zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth);

/**
 * @brief Rotates and zooms a surface.
 */
SDL_Surface *SDLL_rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth);

/**
 * @brief Opens the mixer audio device.
 */
int SDLL_Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);

/**
 * @brief Sets volume of a channel.
 */
int SDLL_Mix_Volume(int channel, int volume);

/**
 * @brief Sets volume of music.
 */
int SDLL_Mix_VolumeMusic(int volume);

/**
 * @brief Plays music.
 */
int SDLL_Mix_PlayMusic(Mix_Music *music, int loops);

/**
 * @brief Plays a sound chunk on a channel.
 */
int SDLL_Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);

/**
 * @brief Plays a sound chunk on a channel with time limit.
 */
int SDLL_Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

/**
 * @brief Halts a channel.
 */
int SDLL_Mix_HaltChannel(int channel);

/**
 * @brief Halts music.
 */
int SDLL_Mix_HaltMusic();

/**
 * @brief Loads a WAV/SFX chunk from file.
 */
Mix_Chunk *SDLL_Mix_LoadWAV(const char *file);

/**
 * @brief Loads music from file.
 */
Mix_Music *SDLL_Mix_LoadMUS(const char *file);

/**
 * @brief Frees music.
 */
void SDLL_Mix_FreeMusic(Mix_Music *music);

/**
 * @brief Frees a sound chunk.
 */
void SDLL_Mix_FreeChunk(Mix_Chunk *chunk);

/**
 * @brief Closes the mixer audio.
 */
void SDLL_Mix_CloseAudio();

/**
 * @brief Quits the mixer subsystem.
 */
void SDLL_Mix_Quit();

/**
 * @brief Gets last mixer error.
 */
const char *SDLL_Mix_GetError();
