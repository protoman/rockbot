#include "sdl_layer.h"
#ifdef PSP
#include "ports/psp/psp_platform.h"
#endif
#include <string>
#include <map>
#include <cstdio>
#include <cerrno>
#include <cstring>

// =============================================
// SDL3
// =============================================
#ifdef SDL3

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

static MIX_Mixer *sdl3_mixer = NULL;
static std::map<int, MIX_Track*> sdl3_tracks;
static int sdl3_next_track_id = 0;

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
	SDL_SetSurfaceAlphaMod(surface, alpha);
	return SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
}

static SDL_Surface *sdl3_screen_surface = NULL;

SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	if (window == NULL) {
		window = SDL_CreateWindow(
			"RockBot",
			width,
			height,
			SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, NULL);
		printf("Renderer backend: %s\n", SDL_GetRendererName(renderer));
		printf("SDL version: %s\n", SDLL_GetCompiledVersion());
	} else {
		SDL_SetWindowSize(window, width, height);
	}

	if (texture) {
		SDL_DestroyTexture(texture);
	}
	texture = SDL_CreateTexture(renderer,
								SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING,
								width, height);

	if (sdl3_screen_surface) {
		SDL_DestroySurface(sdl3_screen_surface);
	}
	sdl3_screen_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
	return sdl3_screen_surface;
}

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
					 SDL_Surface *dst, SDL_Rect *dstrect)
{
	return SDL_BlitSurfaceScaled(src, srcrect, dst, dstrect, SDL_SCALEMODE_NEAREST);
}

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface)
{
	return SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
}

void SDLL_WM_SetCaption(const char *title, const char *icon)
{
	SDL_SetWindowTitle(window, title);
}

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface)
{
	return SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
}

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
	SDL_SetWindowIcon(window, icon);
}

int SDLL_Flip(SDL_Surface *screen)
{
	SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, NULL, NULL);
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
	int count;
	SDL_JoystickID *ids = SDL_GetJoysticks(&count);
	if (device_index < count) {
		return SDL_GetJoystickNameForID(ids[device_index]);
	}
	return "Unknown";
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
	int count;
	SDL_GetJoysticks(&count);
	return count;
}

int SDLL_ShowCursor(int toggle)
{
	if (toggle > 0) {
		SDL_ShowCursor();
		return 1;
	} else if (toggle == 0) {
		SDL_HideCursor();
		return 0;
	}
	return SDL_CursorVisible() ? 1 : 0;
}

int SDLL_putenv(const char *variable)
{
	std::string var(variable);
	size_t pos = var.find('=');
	if (pos != std::string::npos) {
		std::string name = var.substr(0, pos);
		std::string val = var.substr(pos + 1);
		return SDL_setenv_unsafe(name.c_str(), val.c_str(), true);
	}
	return -1;
}

SDL_IOStream *SDLL_RWFromFile(const char *file, const char *mode)
{
	return SDL_IOFromFile(file, mode);
}

void SDLL_FreeSurface(SDL_Surface *surface)
{
	SDL_DestroySurface(surface);
}

int SDLL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
	bool enabled = (flag & SDL_SRCCOLORKEY) ? true : false;
	int res = SDL_SetSurfaceColorKey(surface, enabled, key);
	if (flag & SDL_RLEACCEL) {
		SDL_SetSurfaceRLE(surface, true);
	}
	return res;
}

int SDLL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	(void)flags; (void)depth; (void)Rmask; (void)Gmask; (void)Bmask; (void)Amask;
	return SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
}

int SDLL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	return SDL_FillSurfaceRect(dst, dstrect, color);
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
	return (Uint32)SDL_GetTicks();
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
	return SDL_OpenJoystick(device_index);
}

void SDLL_JoystickClose(SDL_Joystick *joystick)
{
	SDL_CloseJoystick(joystick);
}

int SDLL_JoystickEventState(int state)
{
	(void)state;
	SDL_SetJoystickEventsEnabled(true);
	return 1;
}

SDL_Surface *SDLL_IMG_Load_RW(SDL_IOStream *src, bool closeio)
{
	return IMG_Load_IO(src, closeio);
}

const char *SDLL_IMG_GetError()
{
	return SDL_GetError();
}

int SDLL_TTF_Init()
{
	return TTF_Init();
}

TTF_Font *SDLL_TTF_OpenFont(const char *file, int size)
{
	return TTF_OpenFont(file, (float)size);
}

TTF_Font *SDLL_TTF_OpenFontRW(SDL_IOStream *src, bool freesrc, int size)
{
	return TTF_OpenFontIO(src, freesrc, (float)size);
}

void SDLL_TTF_SetFontOutline(TTF_Font *font, int outline)
{
	TTF_SetFontOutline(font, outline);
}

SDL_Surface *SDLL_TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
	return TTF_RenderText_Solid(font, text, SDL_strlen(text), fg);
}

SDL_Surface *SDLL_TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg)
{
	return TTF_RenderText_Blended(font, text, SDL_strlen(text), fg);
}

const char *SDLL_TTF_GetError()
{
	return SDL_GetError();
}

SDL_Surface *SDLL_zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
#ifdef HAS_SDL3_GFX
	return zoomSurface(src, zoomx, zoomy, smooth);
#else
	(void)src; (void)zoomx; (void)zoomy; (void)smooth;
	return NULL;
#endif
}

SDL_Surface *SDLL_rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth)
{
#ifdef HAS_SDL3_GFX
	return rotozoomSurface(src, angle, zoom, smooth);
#else
	(void)src; (void)angle; (void)zoom; (void)smooth;
	return NULL;
#endif
}

int SDLL_Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	(void)chunksize;
	if (!MIX_Init()) {
		printf("WARNING: MIX_Init failed.\n");
	}

	SDL_AudioSpec spec;
	SDL_zerop(&spec);
	spec.freq = frequency;
	spec.format = (SDL_AudioFormat)format;
	spec.channels = (Uint8)channels;

	sdl3_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (!sdl3_mixer) return -1;
	return 0;
}

int SDLL_Mix_Volume(int channel, int volume)
{
	(void)channel;
	if (sdl3_mixer) {
		MIX_SetMixerGain(sdl3_mixer, volume / 128.0f);
	}
	return volume;
}

int SDLL_Mix_VolumeMusic(int volume)
{
	if (sdl3_mixer) {
		MIX_SetMixerGain(sdl3_mixer, volume / 128.0f);
	}
	return volume;
}

int SDLL_Mix_PlayMusic(SoundMusic *audio, int loops)
{
	if (!sdl3_mixer) return -1;

	MIX_Track *track = MIX_CreateTrack(sdl3_mixer);
	if (!track) return -1;

	MIX_SetTrackAudio(track, (MIX_Audio*)audio);

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (loops == -1) ? -1 : loops);

	bool ok = MIX_PlayTrack(track, props);
	SDL_DestroyProperties(props);

	int track_id = sdl3_next_track_id++;
	sdl3_tracks[track_id] = track;
	return ok ? 0 : -1;
}

int SDLL_Mix_PlayChannel(int channel, SoundChunk *chunk, int loops)
{
	(void)channel;
	if (!sdl3_mixer) return -1;

	MIX_Track *track = MIX_CreateTrack(sdl3_mixer);
	if (!track) return -1;

	MIX_SetTrackAudio(track, (MIX_Audio*)chunk);

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (loops == -1) ? -1 : loops);

	MIX_PlayTrack(track, props);
	SDL_DestroyProperties(props);

	int track_id = sdl3_next_track_id++;
	sdl3_tracks[track_id] = track;
	return track_id;
}

int SDLL_Mix_PlayChannelTimed(int channel, SoundChunk *chunk, int loops, int ticks)
{
	(void)channel;
	if (!sdl3_mixer) return -1;

	MIX_Track *track = MIX_CreateTrack(sdl3_mixer);
	if (!track) return -1;

	MIX_SetTrackAudio(track, (MIX_Audio*)chunk);

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, 0);
	if (ticks > 0) {
		SDL_SetNumberProperty(props, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, ticks);
	}

	MIX_PlayTrack(track, props);
	SDL_DestroyProperties(props);

	int track_id = sdl3_next_track_id++;
	sdl3_tracks[track_id] = track;
	return track_id;
}

int SDLL_Mix_HaltChannel(int channel)
{
	auto it = sdl3_tracks.find(channel);
	if (it != sdl3_tracks.end()) {
		MIX_StopTrack(it->second, 0);
		sdl3_tracks.erase(it);
	}
	return 0;
}

int SDLL_Mix_HaltMusic()
{
	for (auto &pair : sdl3_tracks) {
		MIX_StopTrack(pair.second, 0);
	}
	sdl3_tracks.clear();
	return 0;
}

SoundChunk *SDLL_Mix_LoadWAV(const char *file)
{
	return (SoundChunk*)MIX_LoadAudio(sdl3_mixer, file, false);
}

SoundMusic *SDLL_Mix_LoadMUS(const char *file)
{
	return (SoundMusic*)MIX_LoadAudio(sdl3_mixer, file, false);
}

void SDLL_Mix_FreeMusic(SoundMusic *audio)
{
	if (audio) MIX_DestroyAudio((MIX_Audio*)audio);
}

void SDLL_Mix_FreeChunk(SoundChunk *chunk)
{
	if (chunk) MIX_DestroyAudio((MIX_Audio*)chunk);
}

void SDLL_Mix_CloseAudio()
{
	if (sdl3_mixer) {
		MIX_DestroyMixer(sdl3_mixer);
		sdl3_mixer = NULL;
	}
	sdl3_tracks.clear();
}

void SDLL_Mix_Quit()
{
	MIX_Quit();
}

const char *SDLL_Mix_GetError()
{
	return SDL_GetError();
}

// =============================================
// SDL2
// =============================================
#elif defined(SDL2)

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
	(void)bpp;
	(void)flags;

	int win_w = width;
	int win_h = height;
	Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#ifdef PSP
	psp_platform::adjust_window_size(width, height, &win_w, &win_h, &window_flags);
#endif

	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

	window = SDL_CreateWindow(
		"RockBot",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		win_w,
		win_h,
		window_flags);
	if (window == NULL) {
		printf("SDLL_SetVideoMode: SDL_CreateWindow failed: %s\n", SDL_GetError());
		return NULL;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}
	if (renderer == NULL) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	}
	if (renderer == NULL) {
		printf("SDLL_SetVideoMode: SDL_CreateRenderer failed: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info) == 0) {
		printf("Renderer backend: %s\n", info.name);
	}
	printf("SDL version: %s window=%dx%d game=%dx%d\n",
		SDLL_GetCompiledVersion(), win_w, win_h, width, height);

	texture = SDL_CreateTexture(renderer,
								SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING,
								width, height);
	if (texture == NULL) {
		printf("SDLL_SetVideoMode: SDL_CreateTexture failed: %s\n", SDL_GetError());
		return NULL;
	}

	// SDL2 forbids combining SDL_GetWindowSurface() with a Renderer.
	// Software backbuffer (game res) + present via texture (scaled to window).
	SDL_Surface *screen = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
	if (screen == NULL) {
		printf("SDLL_SetVideoMode: SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
		return NULL;
	}
	SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);
	SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));
	return screen;
}

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
					 SDL_Surface *dst, SDL_Rect *dstrect)
{
	return SDL_BlitScaled(src, srcrect, dst, dstrect);
}

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface)
{
	SDL_Surface *out = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
	return out;
}

void SDLL_WM_SetCaption(const char *title, const char *icon)
{
	SDL_SetWindowTitle(window, title);
}

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface)
{
	SDL_Surface *out = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
#ifdef PSP
	psp_platform::adjust_display_format_alpha(out);
#endif
	return out;
}

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
	SDL_SetWindowIcon(window, icon);
}

int SDLL_Flip(SDL_Surface *screen)
{
	if (screen == NULL || texture == NULL || renderer == NULL) {
		return -1;
	}
	if (SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch) != 0) {
		return -1;
	}
	// Copy RGB as-is; ignore per-pixel alpha (avoids empty frame if A was left 0).
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
#ifdef PSP
	return psp_platform::rw_from_file(file, mode);
#else
	return SDL_RWFromFile(file, mode);
#endif
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
#ifdef PSP
	psp_platform::prepare_color_key(surface, sdl2_flag);
#endif
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

SDL_Surface *SDLL_TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg)
{
	return TTF_RenderUTF8_Blended(font, text, fg);
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

int SDLL_Mix_PlayMusic(SoundMusic *music, int loops)
{
	return Mix_PlayMusic(music, loops);
}

int SDLL_Mix_PlayChannel(int channel, SoundChunk *chunk, int loops)
{
	return Mix_PlayChannel(channel, chunk, loops);
}

int SDLL_Mix_PlayChannelTimed(int channel, SoundChunk *chunk, int loops, int ticks)
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

SoundChunk *SDLL_Mix_LoadWAV(const char *file)
{
#ifdef PSP
	return psp_platform::load_wav(file);
#else
	return Mix_LoadWAV(file);
#endif
}

SoundMusic *SDLL_Mix_LoadMUS(const char *file)
{
#ifdef PSP
	return psp_platform::load_mus(file);
#else
	return Mix_LoadMUS(file);
#endif
}

void SDLL_Mix_FreeMusic(SoundMusic *music)
{
	Mix_FreeMusic(music);
}

void SDLL_Mix_FreeChunk(SoundChunk *chunk)
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

// =============================================
// SDL1
// =============================================
#else

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
#ifdef PSP
	return psp_platform::rw_from_file(file, mode);
#else
	return SDL_RWFromFile(file, mode);
#endif
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

int SDLL_Mix_PlayMusic(SoundMusic *music, int loops)
{
	return Mix_PlayMusic(music, loops);
}

int SDLL_Mix_PlayChannel(int channel, SoundChunk *chunk, int loops)
{
	return Mix_PlayChannel(channel, chunk, loops);
}

int SDLL_Mix_PlayChannelTimed(int channel, SoundChunk *chunk, int loops, int ticks)
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

SoundChunk *SDLL_Mix_LoadWAV(const char *file)
{
#ifdef PSP
	return psp_platform::load_wav(file);
#else
	return Mix_LoadWAV(file);
#endif
}

SoundMusic *SDLL_Mix_LoadMUS(const char *file)
{
#ifdef PSP
	return psp_platform::load_mus(file);
#else
	return Mix_LoadMUS(file);
#endif
}

void SDLL_Mix_FreeMusic(SoundMusic *music)
{
	Mix_FreeMusic(music);
}

void SDLL_Mix_FreeChunk(SoundChunk *chunk)
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

// =============================================
// SDL 1, 2 & 3
// =============================================
const char *SDLL_GetCompiledVersion()
{
	static char version[32];
#ifdef SDL3
	int compiled = SDL_GetVersion();
	snprintf(version, sizeof(version), "%d.%d.%d",
			 SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
#else
	SDL_version compiled;
	SDL_VERSION(&compiled);
	snprintf(version, sizeof(version), "%d.%d.%d",
			 compiled.major, compiled.minor, compiled.patch);
#endif
	return version;
}

void SDLL_GetRGB(Uint32 pixel, SDL_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b)
{
#ifdef SDL3
	const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);
	SDL_Palette *palette = SDL_GetSurfacePalette(surface);
	SDL_GetRGB(pixel, details, palette, r, g, b);
#else
	SDL_GetRGB(pixel, surface->format, r, g, b);
#endif
}

Uint32 SDLL_MapRGB(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b)
{
#ifdef SDL3
	const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);
	return SDL_MapRGBA(details, NULL, r, g, b, 255);
#else
	// SDL2 MapRGB sets A=0 on ARGB8888 → transparent fills → black Flip / invisible text.
	if (surface != NULL && surface->format != NULL && surface->format->Amask != 0) {
		return SDL_MapRGBA(surface->format, r, g, b, 255);
	}
	return SDL_MapRGB(surface->format, r, g, b);
#endif
}

Uint32 SDLL_MapRGBA(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
#ifdef SDL3
	const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);
	return SDL_MapRGBA(details, NULL, r, g, b, a);
#else
	return SDL_MapRGBA(surface->format, r, g, b, a);
#endif
}
