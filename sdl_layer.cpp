#include "sdl_layer.h"

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
