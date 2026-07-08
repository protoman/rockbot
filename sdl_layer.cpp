#include "sdl_layer.h"
#include <string>

#ifdef SDL2
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
	SDL_SetSurfaceAlphaMod(surface, alpha);
	return SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
}

SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	window = SDL_CreateWindow(
		"RockBot",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	printf("Renderer backend: %s\n", info.name);

	printf("SDL version: %s\n", SDLL_GetCompiledVersion());

	texture = SDL_CreateTexture(renderer,
								SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING,
								width, height);

	return SDL_GetWindowSurface(window);
}

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
					 SDL_Surface *dst, SDL_Rect *dstrect)
{

	return SDL_BlitScaled(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface)
{
	return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
}

void SDLL_WM_SetCaption(const char *title, const char *icon)
{
	SDL_SetWindowTitle(window, title);
}

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface)
{
	return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
}

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
	SDL_SetWindowIcon(window, icon);
}

int SDLL_Flip(SDL_Surface *screen)
{
	SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	return 0;
}

const char* SDLL_GetKeyName(int key)
{
	SDL_Keycode key_code = (SDL_Keycode)key;
	return SDL_GetKeyName(key_code);
}

const char *SDLL_JoystickName(int device_index)
{
	return SDL_JoystickNameForIndex(device_index);
}

int SDLL_Init(Uint32 flags)
{
	return SDL_Init(flags);
}

void SDLL_Quit()
{
	SDL_Quit();
}

int SDLL_NumJoysticks()
{
	return SDL_NumJoysticks();
}

int SDLL_ShowCursor(int toggle)
{
	return SDL_ShowCursor(toggle);
}

int SDLL_putenv(const char *variable)
{
	std::string var(variable);
	size_t pos = var.find('=');
	if (pos != std::string::npos) {
		std::string name = var.substr(0, pos);
		std::string val = var.substr(pos + 1);
		return SDL_setenv(name.c_str(), val.c_str(), 1);
	}
	return -1;
}

SDL_RWops *SDLL_RWFromFile(const char *file, const char *mode)
{
	return SDL_RWFromFile(file, mode);
}

void SDLL_FreeSurface(SDL_Surface *surface)
{
	SDL_FreeSurface(surface);
}

int SDLL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
	int sdl2_flag = (flag & SDL_SRCCOLORKEY) ? SDL_TRUE : SDL_FALSE;
	int res = SDL_SetColorKey(surface, sdl2_flag, key);
	if (flag & SDL_RLEACCEL) {
		SDL_SetSurfaceRLE(surface, 1);
	}
	return res;
}

int SDLL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	return SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
}

int SDLL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	return SDL_FillRect(dst, dstrect, color);
}

int SDLL_LockSurface(SDL_Surface *surface)
{
	return SDL_LockSurface(surface);
}

void SDLL_UnlockSurface(SDL_Surface *surface)
{
	SDL_UnlockSurface(surface);
}

void SDLL_Delay(Uint32 ms)
{
	SDL_Delay(ms);
}

Uint32 SDLL_GetTicks()
{
	return SDL_GetTicks();
}

const char *SDLL_GetError()
{
	return SDL_GetError();
}

int SDLL_PollEvent(SDL_Event *event)
{
	return SDL_PollEvent(event);
}

void SDLL_PumpEvents()
{
	SDL_PumpEvents();
}

SDL_Joystick *SDLL_JoystickOpen(int device_index)
{
	return SDL_JoystickOpen(device_index);
}

void SDLL_JoystickClose(SDL_Joystick *joystick)
{
	SDL_JoystickClose(joystick);
}

int SDLL_JoystickEventState(int state)
{
	return SDL_JoystickEventState(state);
}

SDL_Surface *SDLL_IMG_Load_RW(SDL_RWops *src, int freesrc)
{
	return IMG_Load_RW(src, freesrc);
}

const char *SDLL_IMG_GetError()
{
	return IMG_GetError();
}

int SDLL_TTF_Init()
{
	return TTF_Init();
}

TTF_Font *SDLL_TTF_OpenFont(const char *file, int size)
{
	return TTF_OpenFont(file, size);
}

TTF_Font *SDLL_TTF_OpenFontRW(SDL_RWops *src, int freesrc, int size)
{
	return TTF_OpenFontRW(src, freesrc, size);
}

void SDLL_TTF_SetFontOutline(TTF_Font *font, int outline)
{
	TTF_SetFontOutline(font, outline);
}

SDL_Surface *SDLL_TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
	return TTF_RenderUTF8_Solid(font, text, fg);
}

const char *SDLL_TTF_GetError()
{
	return TTF_GetError();
}

SDL_Surface *SDLL_zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
	return zoomSurface(src, zoomx, zoomy, smooth);
}

SDL_Surface *SDLL_rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth)
{
	return rotozoomSurface(src, angle, zoom, smooth);
}

int SDLL_Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	return Mix_OpenAudio(frequency, format, channels, chunksize);
}

int SDLL_Mix_Volume(int channel, int volume)
{
	return Mix_Volume(channel, volume);
}

int SDLL_Mix_VolumeMusic(int volume)
{
	return Mix_VolumeMusic(volume);
}

int SDLL_Mix_PlayMusic(Mix_Music *music, int loops)
{
	return Mix_PlayMusic(music, loops);
}

int SDLL_Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	return Mix_PlayChannel(channel, chunk, loops);
}

int SDLL_Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
	return Mix_PlayChannelTimed(channel, chunk, loops, ticks);
}

int SDLL_Mix_HaltChannel(int channel)
{
	return Mix_HaltChannel(channel);
}

int SDLL_Mix_HaltMusic()
{
	return Mix_HaltMusic();
}

Mix_Chunk *SDLL_Mix_LoadWAV(const char *file)
{
	return Mix_LoadWAV(file);
}

Mix_Music *SDLL_Mix_LoadMUS(const char *file)
{
	return Mix_LoadMUS(file);
}

void SDLL_Mix_FreeMusic(Mix_Music *music)
{
	Mix_FreeMusic(music);
}

void SDLL_Mix_FreeChunk(Mix_Chunk *chunk)
{
	Mix_FreeChunk(chunk);
}

void SDLL_Mix_CloseAudio()
{
	Mix_CloseAudio();
}

void SDLL_Mix_Quit()
{
	Mix_Quit();
}

const char *SDLL_Mix_GetError()
{
	return Mix_GetError();
}

#else

// SDL 1

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
	return SDL_SetAlpha(surface, flag, alpha);
}

SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	printf("SDL version: %s\n", SDLL_GetCompiledVersion());
	printf("SDLL_SetVideoMode - Requesting: width=%d, height=%d, bpp=%d, flags=0x%x\n", width, height, bpp, flags);

	SDL_Surface *screen = SDL_SetVideoMode(width, height, bpp, flags);

	if (screen == NULL) {
		const char *error = SDL_GetError();
		printf("SDLL_SetVideoMode - ERROR: Failed to set video mode. SDL_GetError: %s\n", error ? error : "(no error message)");
	} else {
		printf("SDLL_SetVideoMode - Success: screen=%p, w=%d, h=%d, bpp=%d\n",
			   (void *)screen, screen->w, screen->h, screen->format->BitsPerPixel);
	}

	return screen;
}

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
					 SDL_Surface *dst, SDL_Rect *dstrect)
{

	return SDL_SoftStretch(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface)
{
	return SDL_DisplayFormat(surface);
}

void SDLL_WM_SetCaption(const char *title, const char *icon)
{
	SDL_WM_SetCaption(title, icon);
}

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface)
{
	return SDL_DisplayFormatAlpha(surface);
}

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
	SDL_WM_SetIcon(icon, NULL);
}

int SDLL_Flip(SDL_Surface *screen)
{
	return SDL_Flip(screen);
}

const char* SDLL_GetKeyName(int key)
{
	SDLKey keysym = (SDLKey)key;
	return SDL_GetKeyName(keysym);
}

const char *SDLL_JoystickName(int device_index)
{
	return SDL_JoystickName(device_index);
}

int SDLL_Init(Uint32 flags)
{
	return SDL_Init(flags);
}

void SDLL_Quit()
{
	SDL_Quit();
}

int SDLL_NumJoysticks()
{
	return SDL_NumJoysticks();
}

int SDLL_ShowCursor(int toggle)
{
	return SDL_ShowCursor(toggle);
}

int SDLL_putenv(const char *variable)
{
	return SDL_putenv((char*)variable);
}

SDL_RWops *SDLL_RWFromFile(const char *file, const char *mode)
{
	return SDL_RWFromFile(file, mode);
}

void SDLL_FreeSurface(SDL_Surface *surface)
{
	SDL_FreeSurface(surface);
}

int SDLL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
	return SDL_SetColorKey(surface, flag, key);
}

int SDLL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	return SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
}

int SDLL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	return SDL_FillRect(dst, dstrect, color);
}

int SDLL_LockSurface(SDL_Surface *surface)
{
	return SDL_LockSurface(surface);
}

void SDLL_UnlockSurface(SDL_Surface *surface)
{
	SDL_UnlockSurface(surface);
}

void SDLL_Delay(Uint32 ms)
{
	SDL_Delay(ms);
}

Uint32 SDLL_GetTicks()
{
	return SDL_GetTicks();
}

const char *SDLL_GetError()
{
	return SDL_GetError();
}

int SDLL_PollEvent(SDL_Event *event)
{
	return SDL_PollEvent(event);
}

void SDLL_PumpEvents()
{
	SDL_PumpEvents();
}

SDL_Joystick *SDLL_JoystickOpen(int device_index)
{
	return SDL_JoystickOpen(device_index);
}

void SDLL_JoystickClose(SDL_Joystick *joystick)
{
	SDL_JoystickClose(joystick);
}

int SDLL_JoystickEventState(int state)
{
	return SDL_JoystickEventState(state);
}

SDL_Surface *SDLL_IMG_Load_RW(SDL_RWops *src, int freesrc)
{
	return IMG_Load_RW(src, freesrc);
}

const char *SDLL_IMG_GetError()
{
	return IMG_GetError();
}

int SDLL_TTF_Init()
{
	return TTF_Init();
}

TTF_Font *SDLL_TTF_OpenFont(const char *file, int size)
{
	return TTF_OpenFont(file, size);
}

TTF_Font *SDLL_TTF_OpenFontRW(SDL_RWops *src, int freesrc, int size)
{
	return TTF_OpenFontRW(src, freesrc, size);
}

void SDLL_TTF_SetFontOutline(TTF_Font *font, int outline)
{
	TTF_SetFontOutline(font, outline);
}

SDL_Surface *SDLL_TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
	return TTF_RenderUTF8_Solid(font, text, fg);
}

const char *SDLL_TTF_GetError()
{
	return TTF_GetError();
}

SDL_Surface *SDLL_zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
	return zoomSurface(src, zoomx, zoomy, smooth);
}

SDL_Surface *SDLL_rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth)
{
	return rotozoomSurface(src, angle, zoom, smooth);
}

int SDLL_Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	return Mix_OpenAudio(frequency, format, channels, chunksize);
}

int SDLL_Mix_Volume(int channel, int volume)
{
	return Mix_Volume(channel, volume);
}

int SDLL_Mix_VolumeMusic(int volume)
{
	return Mix_VolumeMusic(volume);
}

int SDLL_Mix_PlayMusic(Mix_Music *music, int loops)
{
	return Mix_PlayMusic(music, loops);
}

int SDLL_Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	return Mix_PlayChannel(channel, chunk, loops);
}

int SDLL_Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
	return Mix_PlayChannelTimed(channel, chunk, loops, ticks);
}

int SDLL_Mix_HaltChannel(int channel)
{
	return Mix_HaltChannel(channel);
}

int SDLL_Mix_HaltMusic()
{
	return Mix_HaltMusic();
}

Mix_Chunk *SDLL_Mix_LoadWAV(const char *file)
{
	return Mix_LoadWAV(file);
}

Mix_Music *SDLL_Mix_LoadMUS(const char *file)
{
	return Mix_LoadMUS(file);
}

void SDLL_Mix_FreeMusic(Mix_Music *music)
{
	Mix_FreeMusic(music);
}

void SDLL_Mix_FreeChunk(Mix_Chunk *chunk)
{
	Mix_FreeChunk(chunk);
}

void SDLL_Mix_CloseAudio()
{
	Mix_CloseAudio();
}

void SDLL_Mix_Quit()
{
	Mix_Quit();
}

const char *SDLL_Mix_GetError()
{
	return Mix_GetError();
}

#endif

// SDL 1 & 2
const char *SDLL_GetCompiledVersion()
{
	static char version[32];
	SDL_version compiled;
	SDL_VERSION(&compiled);

	snprintf(version, sizeof(version), "%d.%d.%d",
			 compiled.major, compiled.minor, compiled.patch);
	return version;
}
