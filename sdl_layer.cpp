#include "sdl_layer.h"

int SDLL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {
    if (!surface) return -1;

#ifdef SDL2
    if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND) != 0) {
        return -1;
    }
    return SDL_SetSurfaceAlphaMod(surface, alpha);
#else
    return SDL_SetAlpha(surface, flag, alpha);
#endif
}

SDL_Surface * SDLL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
    #ifdef SDL2
        window = SDL_CreateWindow(
                        "RockBot", 
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        width,
                        height,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                    );
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        SDL_RendererInfo info;
        SDL_GetRendererInfo(renderer, &info);
        printf("Renderer backend: %s\n", info.name);

        SDL_RenderClear(renderer);

        return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, VIDEO_MODE_COLORS, 0, 255, 0, 255);
    #else
        return SDL_SetVideoMode(width, height, bpp, flags);
    #endif
}

int SDLL_SoftStretch(SDL_Surface * src,
                    const SDL_Rect * srcrect,
                    SDL_Surface * dst,
                    const SDL_Rect * dstrect) {

#ifdef SDL2
SDL_BlitScaled(src, srcrect, dst, dstrect);
#else
SDL_SoftStretch(src, srcrect, dst, dstrect);
#endif

}