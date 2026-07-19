#include "sdl_layer_psp.h"

#ifdef PSP
#include <pspiofilemgr.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

static int psp_owned_mem_close(SDL_RWops *context)
{
    if (context) {
        std::free(context->hidden.mem.base);
        context->hidden.mem.base = NULL;
        context->hidden.mem.here = NULL;
        context->hidden.mem.stop = NULL;
        SDL_FreeRW(context);
    }
    return 0;
}

void SDLL_PSP_AdjustWindowSize(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags)
{
    if (win_w) *win_w = 480;
    if (win_h) *win_h = 272;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN;
    (void)requested_w;
    (void)requested_h;
}

void SDLL_PSP_AdjustDisplayFormatAlpha(SDL_Surface *surface)
{
    if (surface != NULL) {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
}

RWopsPtr *SDLL_PSP_RWFromFile(const char *file, const char *mode)
{
    const bool writing = mode && (mode[0] == 'w' || mode[0] == 'a' || std::strchr(mode, '+'));
    if (writing) {
        FILE *fp = std::fopen(file, mode);
        if (!fp) {
            SDL_SetError("SDLL_PSP_RWFromFile: fopen(%s) failed errno=%d", file ? file : "(null)", errno);
            return NULL;
        }
        SDL_RWops *rw = SDL_RWFromFP(fp, SDL_TRUE);
        if (!rw) {
            SDL_SetError("SDLL_PSP_RWFromFile: SDL_RWFromFP failed for %s", file ? file : "(null)");
        }
        return rw;
    }

    if (!file) {
        SDL_SetError("SDLL_PSP_RWFromFile: NULL path");
        return NULL;
    }

    const SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0) {
        SDL_SetError("SDLL_PSP_RWFromFile: sceIoOpen(%s) failed (%d)", file, (int)fd);
        return NULL;
    }

    const SceOff sz = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    if (sz <= 0 || sz > 32 * 1024 * 1024) {
        sceIoClose(fd);
        SDL_SetError("SDLL_PSP_RWFromFile: bad size %ld for %s", (long)sz, file);
        return NULL;
    }

    void *buf = std::malloc((size_t)sz);
    if (!buf) {
        sceIoClose(fd);
        SDL_OutOfMemory();
        return NULL;
    }

    const int nread = sceIoRead(fd, buf, (size_t)sz);
    sceIoClose(fd);
    if (nread != (int)sz) {
        std::free(buf);
        SDL_SetError("SDLL_PSP_RWFromFile: sceIoRead %d/%ld for %s", nread, (long)sz, file);
        return NULL;
    }

    SDL_RWops *rw = SDL_RWFromMem(buf, (int)sz);
    if (!rw) {
        std::free(buf);
        return NULL;
    }

    rw->close = psp_owned_mem_close;
    return rw;
}

void SDLL_PSP_PrepareColorKey(SDL_Surface *surface, int sdl2_flag)
{
    if (surface != NULL && sdl2_flag) {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
}

SoundChunk *SDLL_PSP_LoadWAV(const char *file)
{
    SDL_RWops *rw = SDLL_PSP_RWFromFile(file, "rb");
    if (!rw) {
        return NULL;
    }
    return Mix_LoadWAV_RW(rw, 1);
}

SoundMusic *SDLL_PSP_LoadMUS(const char *file)
{
    SDL_RWops *rw = SDLL_PSP_RWFromFile(file, "rb");
    if (!rw) {
        return NULL;
    }
    return Mix_LoadMUS_RW(rw, 1);
}
#else

void SDLL_PSP_AdjustWindowSize(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags)
{
    if (win_w) *win_w = requested_w;
    if (win_h) *win_h = requested_h;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
}

void SDLL_PSP_AdjustDisplayFormatAlpha(SDL_Surface *surface)
{
    (void)surface;
}

RWopsPtr *SDLL_PSP_RWFromFile(const char *file, const char *mode)
{
    return SDL_RWFromFile(file, mode);
}

void SDLL_PSP_PrepareColorKey(SDL_Surface *surface, int sdl2_flag)
{
    (void)surface;
    (void)sdl2_flag;
}

SoundChunk *SDLL_PSP_LoadWAV(const char *file)
{
    return Mix_LoadWAV(file);
}

SoundMusic *SDLL_PSP_LoadMUS(const char *file)
{
    return Mix_LoadMUS(file);
}
#endif
