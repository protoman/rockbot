#include "sdl_layer.h"

#ifdef SDL2
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
#endif

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
    if (!surface)
        return -1;

#ifdef SDL2
    if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND) != 0)
    {
        return -1;
    }
    return SDL_SetSurfaceAlphaMod(surface, alpha);
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

    SDL_RenderClear(renderer);

    return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bpp, 0, 255, 0, 255);
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
    return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
#else
    return SDL_DisplayFormatAlpha(surface);
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
    return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
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