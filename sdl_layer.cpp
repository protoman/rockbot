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