#include "psp_music.h"

#ifdef PSP

#include <xmp.h>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <string>

#include "psp_platform.h"
#include "../../aux_tools/stringutils.h"

namespace psp_music {

namespace {

const unsigned int MAGIC = 0x504d5801; // "XMP"+1: tells our handles from Mix_Music pointers

struct XmpMusic {
    unsigned int magic;
    xmp_context ctx;
    void *data;        // module file, kept until the handle is freed
    long size;
    bool started;      // xmp_start_player() called
    int xmp_loop;      // xmp_play_buffer() loop argument: 0 = forever, N = play N times
    volatile bool playing;
};

XmpMusic *g_current = NULL;
volatile int g_volume = 128;

void hook(void *udata, Uint8 *stream, int len)
{
    XmpMusic *m = (XmpMusic *)udata;
    if (m == NULL || !m->playing) {
        std::memset(stream, 0, len);
        return;
    }
    int r = xmp_play_buffer(m->ctx, stream, len, m->xmp_loop);
    if (r < 0) { // end of the module (or an error): silence from now on
        m->playing = false;
        std::memset(stream, 0, len);
        return;
    }
    const int volume = g_volume;
    if (volume < 128) {
        Sint16 *samples = (Sint16 *)stream;
        const int count = len / 2;
        for (int i = 0; i < count; i++) {
            samples[i] = (Sint16)((samples[i] * volume) >> 7);
        }
    }
}

void stop_current()
{
    if (g_current != NULL) {
        Mix_HookMusic(NULL, NULL); // takes the audio lock: the hook is not running after this
        g_current->playing = false;
        if (g_current->started) {
            xmp_end_player(g_current->ctx);
            g_current->started = false;
        }
        g_current = NULL;
    }
}

} // namespace

bool is_tracker_file(const char *file)
{
    if (file == NULL) {
        return false;
    }
    const char *dot = std::strrchr(file, '.');
    if (dot == NULL) {
        return false;
    }
    return strcasecmp(dot, ".mod") == 0 || strcasecmp(dot, ".s3m") == 0 ||
           strcasecmp(dot, ".xm") == 0 || strcasecmp(dot, ".it") == 0;
}

SoundMusic *load(const char *file)
{
    SDL_RWops *rw = psp_platform::rw_from_file(file, "rb");
    if (rw == NULL) {
        psp_platform::log("psp_music: cannot open '%s' (%s)", file ? file : "(null)", SDL_GetError());
        return NULL;
    }
    const Sint64 size = SDL_RWsize(rw);
    if (size <= 0 || size > 8 * 1024 * 1024) {
        SDL_RWclose(rw);
        psp_platform::log("psp_music: bad size %ld for '%s'", (long)size, file);
        return NULL;
    }
    void *data = std::malloc((size_t)size);
    if (data == NULL || SDL_RWread(rw, data, 1, (size_t)size) != (size_t)size) {
        std::free(data);
        SDL_RWclose(rw);
        psp_platform::log("psp_music: cannot read '%s'", file);
        return NULL;
    }
    SDL_RWclose(rw);

    XmpMusic *m = (XmpMusic *)std::calloc(1, sizeof(XmpMusic));
    if (m == NULL) {
        std::free(data);
        return NULL;
    }
    m->magic = MAGIC;
    m->data = data;
    m->size = (long)size;
    m->ctx = xmp_create_context();
    if (m->ctx == NULL || xmp_load_module_from_memory(m->ctx, m->data, m->size) < 0) {
        psp_platform::log("psp_music: libxmp cannot load '%s'", file);
        if (m->ctx != NULL) {
            xmp_free_context(m->ctx);
        }
        std::free(data);
        std::free(m);
        return NULL;
    }
    psp_platform::log("psp_music: loaded '%s' (%ld bytes)", file, (long)size);
    return (SoundMusic *)m;
}

bool owns(const SoundMusic *music)
{
    return music != NULL && ((const XmpMusic *)music)->magic == MAGIC;
}

int play(SoundMusic *music, int loops)
{
    XmpMusic *m = (XmpMusic *)music;
    if (!owns(music)) {
        return -1;
    }
    Mix_HaltMusic();
    stop_current();
    if (xmp_start_player(m->ctx, 44100, 0) < 0) {
        psp_platform::log("psp_music: xmp_start_player failed");
        return -1;
    }
    m->started = true;
    m->xmp_loop = (loops < 0) ? 0 : loops + 1;
    m->playing = true;
    g_current = m;
    Mix_HookMusic(hook, m);
    return 0;
}

void halt()
{
    stop_current();
}

void free_music(SoundMusic *music)
{
    XmpMusic *m = (XmpMusic *)music;
    if (!owns(music)) {
        return;
    }
    if (g_current == m) {
        stop_current();
    }
    xmp_release_module(m->ctx);
    xmp_free_context(m->ctx);
    std::free(m->data);
    m->magic = 0;
    std::free(m);
}

void set_volume(int volume)
{
    if (volume >= 0) {
        g_volume = volume > 128 ? 128 : volume;
    }
}

} // namespace psp_music

#else
namespace psp_music {
bool is_tracker_file(const char *) { return false; }
SoundMusic *load(const char *) { return NULL; }
bool owns(const SoundMusic *) { return false; }
int play(SoundMusic *, int) { return -1; }
void halt() {}
void free_music(SoundMusic *) {}
void set_volume(int) {}
}
#endif
