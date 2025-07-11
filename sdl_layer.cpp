#include "sdl_layer.h"

#ifdef SDL2
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *texture = nullptr;
#endif

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
#ifdef SDL2
    SDL_SetSurfaceAlphaMod(surface, alpha);
    return SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
#else
    return SDL_SetAlpha(surface, flag, alpha);
#endif
}

SDL_Surface *SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
#ifdef SDL2
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

    texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width, height);

    return SDL_GetWindowSurface(window);
#else
    return SDL_SetVideoMode(width, height, bpp, flags);
#endif
}

int SDLL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
                     SDL_Surface *dst, SDL_Rect *dstrect)
{

#ifdef SDL2
    return SDL_BlitScaled(src, srcrect, dst, dstrect);
#else
    return SDL_SoftStretch(src, srcrect, dst, dstrect);
#endif
}

SDL_Surface *SDLL_DisplayFormat(SDL_Surface *surface)
{
#ifdef SDL2
    return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
#else
    return SDL_DisplayFormat(surface);
#endif
}

void SDLL_WM_SetCaption(const char *title, const char *icon)
{
#ifdef SDL2
    SDL_SetWindowTitle(window, title);
#else
    SDL_WM_SetCaption(title, icon);
#endif
}

SDL_Surface *SDLL_DisplayFormatAlpha(SDL_Surface *surface)
{
#ifdef SDL2
    return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
#else
    return SDL_DisplayFormatAlpha(surface);
#endif
}

void SDLL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
#ifdef SDL2
    SDL_SetWindowIcon(window, icon);
#else
    SDL_WM_SetIcon(icon, NULL);
#endif
}

int SDLL_Flip(SDL_Surface *screen)
{
#ifdef SDL2
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    return 0;
#else
    return SDL_Flip(screen);
#endif
}

const char* SDLL_GetKeyName(int key)
{
#ifdef SDL2
    SDL_Keycode key_code = (SDL_Keycode)key;
    return SDL_GetKeyName(key_code);
#else
    SDLKey keysym = (SDLKey)key;
    return SDL_GetKeyName(keysym);
#endif
}

const char *SDLL_JoystickName(int device_index)
{
#ifdef SDL2
    return SDL_JoystickNameForIndex(device_index);
#else
    return SDL_JoystickName(device_index);
#endif
}