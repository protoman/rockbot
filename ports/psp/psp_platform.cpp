#include "psp_platform.h"

#ifdef PSP

#include <pspkernel.h>
#include <psppower.h>
#include <pspdebug.h>
#include <pspthreadman.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <unistd.h>
#include <malloc.h>

#include "../../shareddata.h"
#include "../../graphicslib.h"
#include "../../inputlib.h"
#include "../../timerlib.h"
#include "../../strings_map.h"
#include "../../graphic/option_picker.h"
#include "../../graphic/draw.h"
#include "../../file/file_io.h"
#include "../../aux_tools/stringutils.h"

extern graphicsLib graphLib;
extern inputLib input;
extern timerLib timer;
extern draw draw_lib;
extern CURRENT_FILE_FORMAT::file_io fio;

PSP_MODULE_INFO("Rockbot", PSP_MODULE_USER, 1, 0);
// SDL's PSP renderer (SDL_RenderCopy) uses VFPU instructions: without this attribute the real PSP raises
// "Coprocessor unusable" on the first frame (PPSSPP does not enforce it).
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
// Explicit heap: negative "all-but-N" can leave almost nothing under PPSSPP after SDL_Init.
PSP_HEAP_SIZE_KB(20 * 1024);
// 64 KB overflowed silently under the emulator but kills the thread on real hardware.
PSP_MAIN_THREAD_STACK_SIZE_KB(512);

namespace {

int exit_callback(int /*arg1*/, int /*arg2*/, void * /*common*/)
{
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize /*args*/, void * /*argp*/)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

int owned_mem_close(SDL_RWops *context)
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

} // namespace

#ifdef ROCKBOT_PSP_POISON
// Debug aid: real PSP memory is not zeroed (the emulator's is). Poison every C++ allocation.
void *operator new(std::size_t n)
{
    void *p = std::malloc(n ? n : 1);
    if (p) std::memset(p, 0xCD, n);
    return p;
}
void *operator new[](std::size_t n) { return operator new(n); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
#endif

namespace psp_platform {

static std::string g_base_dir; // folder that contains EBOOT.PBP, with trailing '/'

static volatile unsigned int g_progress = 0;   // bumped by log() and every frame
static SceUID g_main_thread = -1;
static u32 g_text_addr = 0, g_text_size = 0;

static bool g_no_music = false;    // no_music.txt next to EBOOT.PBP: do not load music (debugging)
static bool g_keep_aspect = false; // keep_aspect.txt next to EBOOT.PBP: letterbox instead of stretching

void log(const char *fmt, ...)
{
    g_progress++;
    // before the data folder is known, log next to the current directory
    std::string dir = g_base_dir;
    if (dir.empty()) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            return;
        }
        dir = std::string(cwd) + "/";
    }
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(line, sizeof(line) - 1, fmt, ap);
    va_end(ap);
    if (n >= 0 && n < (int)sizeof(line) - 40) {
        n += snprintf(line + n, sizeof(line) - 1 - n, " [free=%uK max=%uK]",
                      (unsigned)(sceKernelTotalFreeMemSize() / 1024), (unsigned)(sceKernelMaxFreeMemSize() / 1024));
    }
    if (n < 0) {
        return;
    }
    if (n > (int)sizeof(line) - 2) {
        n = (int)sizeof(line) - 2;
    }
    line[n++] = '\n';
    std::string path = dir + "rockbot_boot.log";
    SceUID fd = sceIoOpen(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, line, n);
        sceIoClose(fd);
    }
}

void heartbeat() { g_progress++; }

static int watchdog_thread(SceSize, void *)
{
    unsigned int last = g_progress;
    int idle = 0;
    for (;;) {
        sceKernelDelayThread(1000 * 1000);
        if (g_progress != last) {
            last = g_progress;
            idle = 0;
            continue;
        }
        idle++;
        if (idle == 10 || (idle > 10 && idle % 30 == 0)) {
            SceKernelThreadInfo info;
            std::memset(&info, 0, sizeof(info));
            info.size = sizeof(info);
            int r = sceKernelReferThreadStatus(g_main_thread, &info);
            log("WATCHDOG: no progress for %ds; main thread ref=%d status=%d waitType=%d waitId=0x%x stackSize=%d priority=%d",
                idle, r, info.status, info.waitType, (unsigned)info.waitId, (int)info.stackSize, info.currentPriority);
        }
    }
    return 0;
}

void setup(const char *argv0)
{
    // The PSP FPU traps on invalid operations / division by zero / underflow (PPSSPP and desktop CPUs
    // return NaN/inf quietly). Mask every trap and flush denormals to zero for this thread.
    {
        unsigned int fcr31;
        asm volatile("cfc1 %0, $31" : "=r"(fcr31));
        fcr31 &= ~((0x1fu << 7) | (0x3fu << 12) | (0x1fu << 2)); // enables, cause, flags
        fcr31 |= (1u << 24);                                      // FS: flush to zero
        asm volatile("ctc1 %0, $31" : : "r"(fcr31));
    }
    SetupCallbacks();
    scePowerSetClockFrequency(333, 333, 166);

    if (argv0 != NULL) {
        std::string path(argv0);
        std::string::size_type pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            // use it only if the game data really is there; otherwise keep the current directory
            std::string candidate = path.substr(0, pos + 1);
            SceIoStat st;
            if (sceIoGetstat((candidate + "games").c_str(), &st) >= 0) {
                g_base_dir = candidate;
                sceIoChdir(g_base_dir.c_str());
            }
        }
    }
    // everything the game prints (printf of graphics/font/file errors) goes to a file next to the EBOOT
    if (!g_base_dir.empty()) {
        std::string out = g_base_dir + "rockbot_stdout.log";
        freopen(out.c_str(), "w", stdout);
        freopen(out.c_str(), "a", stderr);
        setvbuf(stdout, NULL, _IOLBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
    atexit([]() { log("exit(): normal process exit"); });
    g_main_thread = sceKernelGetThreadId();
    SceKernelModuleInfo mi;
    std::memset(&mi, 0, sizeof(mi));
    mi.size = sizeof(mi);
    if (sceKernelQueryModuleInfo(sceKernelGetModuleIdByAddress((const void *)&setup), &mi) >= 0) {
        g_text_addr = mi.text_addr;
        g_text_size = mi.text_size;
    }
    SceUID wd = sceKernelCreateThread("rockbot_watchdog", watchdog_thread, 0x30, 0x4000, 0, NULL);
    if (wd >= 0) {
        sceKernelStartThread(wd, 0, NULL);
    }
    {
        SceIoStat ka;
        g_keep_aspect = !g_base_dir.empty() && sceIoGetstat((g_base_dir + "keep_aspect.txt").c_str(), &ka) >= 0;
        g_no_music = !g_base_dir.empty() && sceIoGetstat((g_base_dir + "no_music.txt").c_str(), &ka) >= 0;
    }
    char cwd[256];
    log("boot: module text=0x%08x size=0x%x stack=512K keep_aspect=%d no_music=%d", (unsigned)g_text_addr, (unsigned)g_text_size, (int)g_keep_aspect, (int)g_no_music);
    log("boot: argv0='%s' base='%s' cwd='%s'", argv0 ? argv0 : "(null)", g_base_dir.c_str(),
        getcwd(cwd, sizeof(cwd)) ? cwd : "(null)");
}

void get_filepath(std::string &filepath, int maxpathlen)
{
    // The folder of EBOOT.PBP, from argv[0]: the current directory depends on how the game
    // was started (XMB, PPSSPP game list or direct file load) and may not be that folder.
    if (!g_base_dir.empty()) {
        filepath = g_base_dir;
        return;
    }
    char *buffer = new char[maxpathlen];
    if (getcwd(buffer, maxpathlen) != NULL) {
        filepath = std::string(buffer);
        if (!filepath.empty() && filepath[filepath.size() - 1] != '/') {
            filepath += "/";
        }
    }
    delete[] buffer;
}

bool load_fonts(const char *path,
                TTF_Font **font,
                TTF_Font **outline_font,
                TTF_Font **error_font,
                int font_size,
                int error_font_size)
{
    // Load font via sceIo (newlib ftell/fread can fail under PPSSPP).
    SceUID font_fd = sceIoOpen(StringUtils::clean_filename(path ? path : "").c_str(), PSP_O_RDONLY, 0777);
    if (font_fd < 0) {
        printf("ERROR::initGraphics - sceIoOpen failed for '%s' (%d)\n", path, (int)font_fd);
        fflush(stdout);
        return false;
    }
    const SceOff font_sz = sceIoLseek(font_fd, 0, PSP_SEEK_END);
    sceIoLseek(font_fd, 0, PSP_SEEK_SET);
    if (font_sz <= 0 || font_sz > 2 * 1024 * 1024) {
        sceIoClose(font_fd);
        printf("ERROR::initGraphics - bad font size for '%s' (%ld)\n", path, (long)font_sz);
        fflush(stdout);
        return false;
    }
    // Keep buffer for process life: some SDL_ttf/FreeType builds reference the
    // memory face without copying; freeing caused "Text has zero width".
    static char *psp_font_blob = NULL;
    if (psp_font_blob) {
        free(psp_font_blob);
        psp_font_blob = NULL;
    }
    psp_font_blob = (char *)malloc((size_t)font_sz);
    if (!psp_font_blob) {
        sceIoClose(font_fd);
        printf("ERROR::initGraphics - malloc(%ld) failed for font\n", (long)font_sz);
        fflush(stdout);
        return false;
    }
    const int nread = sceIoRead(font_fd, psp_font_blob, (size_t)font_sz);
    sceIoClose(font_fd);
    if (nread != (int)font_sz) {
        free(psp_font_blob);
        psp_font_blob = NULL;
        printf("ERROR::initGraphics - sceIoRead got %d/%ld for '%s'\n", nread, (long)font_sz, path);
        fflush(stdout);
        return false;
    }
    *font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 1, font_size);
    *outline_font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 1, font_size);
    *error_font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 1, error_font_size);
    if (*font == NULL || *outline_font == NULL || *error_font == NULL) {
        printf("ERROR::initGraphics - TTF_OpenFontRW failed for '%s' (ttf='%s')\n",
               path, TTF_GetError());
        fflush(stdout);
        return false;
    }
    printf("PSP font ok height=%d ascent=%d\n", TTF_FontHeight(*font), TTF_FontAscent(*font));
    fflush(stdout);
    return true;
}

SDL_Surface *create_surface(int width, int height)
{
    // Direct ARGB8888: CreateRGB(16)+DisplayFormat doubles peak RAM and OOMs on stage start.
    SDL_Surface *temp_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!temp_surface) {
        printf("initSurface OOM %dx%d\n", width, height);
        fflush(stdout);
        return NULL;
    }
    SDL_SetSurfaceBlendMode(temp_surface, SDL_BLENDMODE_NONE);
    return temp_surface;
}

SDL_Surface *set_video_mode(int res_w, int res_h, int colors)
{
    return SDLL_SetVideoMode(res_w, res_h, colors, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_NOFRAME);
}

SDL_Surface *render_error_text(TTF_Font *error_font, const char *text, SDL_Color color)
{
    return TTF_RenderUTF8_Solid(error_font, text, color);
}

SDL_Surface *format_text_for_blit(SDL_Surface *text_sf)
{
    if (!text_sf) {
        return NULL;
    }
    SDL_Surface *formatted = SDL_ConvertSurfaceFormat(text_sf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDLL_FreeSurface(text_sf);
    if (!formatted) {
        return NULL;
    }

    if (SDL_MUSTLOCK(formatted)) {
        SDL_LockSurface(formatted);
    }

    Uint32 *px = (Uint32 *)formatted->pixels;
    const int n = (formatted->pitch / 4) * formatted->h;
    const SDL_PixelFormat *fmt = formatted->format;
    const Uint32 amask = fmt->Amask;
    const Uint32 rmask = fmt->Rmask;
    const Uint32 gmask = fmt->Gmask;
    const Uint32 bmask = fmt->Bmask;
    const int rshift = fmt->Rshift;
    const int gshift = fmt->Gshift;
    const int bshift = fmt->Bshift;
    const int ashift = fmt->Ashift;

    for (int i = 0; i < n; i++) {
        const Uint32 pixel = px[i];
        const Uint8 a = amask ? (Uint8)((pixel & amask) >> ashift) : 255;
        const Uint8 r = (Uint8)((pixel & rmask) >> rshift);
        const Uint8 g = (Uint8)((pixel & gmask) >> gshift);
        const Uint8 b = (Uint8)((pixel & bmask) >> bshift);

        // Transparent background: alpha 0 (Blended) or solid-font backdrop (rgb 0).
        if (a < 16 || (r == 0 && g == 0 && b == 0)) {
            px[i] = 0;
        } else {
            px[i] = SDL_MapRGBA(fmt, r, g, b, 255);
        }
    }

    if (SDL_MUSTLOCK(formatted)) {
        SDL_UnlockSurface(formatted);
    }

    SDL_SetSurfaceBlendMode(formatted, SDL_BLENDMODE_NONE);
    SDL_SetColorKey(formatted, SDL_TRUE, SDL_MapRGBA(formatted->format, 0, 0, 0, 0));
    return formatted;
}

SDL_Surface *render_utf8_text(TTF_Font *font, const char *text, SDL_Color color)
{
    // Solid palette text only: Blended/Shaded leave opaque RGB in the background bbox.
    return TTF_RenderUTF8_Solid(font, text, color);
}

std::vector<std::string> read_games_directory(const std::string &games_path)
{
    // opendir/readdir via newlib can wedge PPSSPP (mutex storm). Probe known games via sceIo.
    std::vector<std::string> res;
    if (games_path.find("/games") == std::string::npos) {
        return res;
    }
    // The real PSP firmware rejects paths with a doubled '/' (PPSSPP accepts them): no trailing '/'.
    std::string base = games_path;
    while (!base.empty() && base[base.size() - 1] == '/') {
        base.erase(base.size() - 1);
    }
    const char *candidates[] = { "RockDroid1", "RockDroid2", NULL };
    for (int i = 0; candidates[i] != NULL; ++i) {
        std::string probe = base + "/" + candidates[i] + "/game_enemy_list.dat";
        SceUID fd = sceIoOpen(StringUtils::clean_filename(probe).c_str(), PSP_O_RDONLY, 0777);
        log("games probe '%s' -> %d", probe.c_str(), (int)fd);
        if (fd >= 0) {
            sceIoClose(fd);
            res.push_back(std::string(candidates[i]));
        }
    }
    return res;
}

void map_list_reserves(size_t *objects, size_t *npcs, size_t *animations, size_t *level3_tiles)
{
    // Huge reserves of classnpc/object blew the 20MB heap (std::bad_alloc).
    if (objects) *objects = 48;
    if (npcs) *npcs = 48;
    if (animations) *animations = 32;
    if (level3_tiles) *level3_tiles = 128;
}

unsigned int ram_available_linear_max()
{
    const unsigned int RAM_BLOCK = 1024 * 1024;
    unsigned int size = 0;
    unsigned int sizeblock = RAM_BLOCK;

    while (sizeblock) {
        size += sizeblock;
        u8 *ram = (u8 *)malloc(size);
        if (!ram) {
            size -= sizeblock;
            sizeblock >>= 1;
        } else {
            free(ram);
        }
    }
    return size;
}

unsigned int ram_available()
{
    u8 **ram = NULL;
    unsigned int size = 0;
    unsigned int count = 0;

    for (;;) {
        if (!(count % 10)) {
            u8 **temp = (u8 **)realloc(ram, sizeof(u8 *) * (count + 10));
            if (!temp) {
                break;
            }
            ram = temp;
            size += (sizeof(u8 *) * 10);
        }

        unsigned int x = ram_available_linear_max();
        if (!x) {
            break;
        }

        ram[count] = (u8 *)malloc(x);
        if (!ram[count]) {
            break;
        }

        size += x;
        count++;
    }

    if (ram) {
        for (unsigned int x = 0; x < count; x++) {
            free(ram[x]);
        }
        free(ram);
    }
    return size;
}

void adjust_window_size(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags)
{
    if (win_w) *win_w = 480;
    if (win_h) *win_h = 272;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN;
    (void)requested_w;
    (void)requested_h;
}

void adjust_display_format_alpha(SDL_Surface *surface)
{
    if (surface != NULL) {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
}

bool get_present_rect(int game_w, int game_h, SDL_Rect *dst)
{
    // Stretching to the whole LCD is the path known to work on real hardware (and the fast one).
    // Letterboxing is opt-in: create an empty keep_aspect.txt next to EBOOT.PBP.
    if (!g_keep_aspect || game_w <= 0 || game_h <= 0) {
        return false;
    }
    const int lcd_w = 480;
    const int lcd_h = 272;
    int w = lcd_w;
    int h = (lcd_w * game_h) / game_w;
    if (h > lcd_h) {
        h = lcd_h;
        w = (lcd_h * game_w) / game_h;
    }
    dst->x = (lcd_w - w) / 2;
    dst->y = (lcd_h - h) / 2;
    dst->w = w;
    dst->h = h;
    static bool logged = false;
    if (!logged) {
        logged = true;
        log("present rect %d,%d %dx%d (keep aspect)", dst->x, dst->y, dst->w, dst->h);
    }
    return true;
}

RWopsPtr *rw_from_file(const char *file, const char *mode)
{
    const bool writing = mode && (mode[0] == 'w' || mode[0] == 'a' || std::strchr(mode, '+'));
    if (writing) {
        FILE *fp = std::fopen(file, mode);
        if (!fp) {
            SDL_SetError("psp_platform::rw_from_file: fopen(%s) failed errno=%d", file ? file : "(null)", errno);
            return NULL;
        }
        SDL_RWops *rw = SDL_RWFromFP(fp, SDL_TRUE);
        if (!rw) {
            std::fclose(fp);
            SDL_SetError("psp_platform::rw_from_file: SDL_RWFromFP failed for %s", file ? file : "(null)");
        }
        return rw;
    }

    if (!file) {
        SDL_SetError("psp_platform::rw_from_file: NULL path");
        return NULL;
    }

    // the real PSP firmware rejects a doubled '/' in sceIo paths (newlib's fopen and PPSSPP accept it)
    const std::string clean_file = StringUtils::clean_filename(file);
    const SceUID fd = sceIoOpen(clean_file.c_str(), PSP_O_RDONLY, 0777);
    if (fd < 0) {
        SDL_SetError("psp_platform::rw_from_file: sceIoOpen(%s) failed (%d)", file, (int)fd);
        return NULL;
    }

    const SceOff sz = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    if (sz <= 0 || sz > 32 * 1024 * 1024) {
        sceIoClose(fd);
        SDL_SetError("psp_platform::rw_from_file: bad size %ld for %s", (long)sz, file);
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
        SDL_SetError("psp_platform::rw_from_file: sceIoRead %d/%ld for %s", nread, (long)sz, file);
        return NULL;
    }

    SDL_RWops *rw = SDL_RWFromMem(buf, (int)sz);
    if (!rw) {
        std::free(buf);
        return NULL;
    }

    rw->close = owned_mem_close;
    return rw;
}

void prepare_color_key(SDL_Surface *surface, int sdl2_flag)
{
    if (surface != NULL && sdl2_flag) {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
}

SoundChunk *load_wav(const char *file)
{
    SDL_RWops *rw = rw_from_file(file, "rb");
    if (!rw) {
        log("load_wav '%s': cannot open (%s)", file ? file : "(null)", SDL_GetError());
        return NULL;
    }
    SoundChunk *chunk = Mix_LoadWAV_RW(rw, 1);
    log("load_wav '%s' -> %p%s%s", file ? file : "(null)", (void *)chunk, chunk ? "" : " ", chunk ? "" : Mix_GetError());
    return chunk;
}

SoundMusic *load_mus(const char *file)
{
    if (g_no_music) {
        return NULL;
    }
    SDL_RWops *rw = rw_from_file(file, "rb");
    if (!rw) {
        log("load_mus '%s': cannot open (%s)", file ? file : "(null)", SDL_GetError());
        return NULL;
    }
    log("load_mus '%s' decoding...", file ? file : "(null)");
    SoundMusic *music = Mix_LoadMUS_RW(rw, 1);
    log("load_mus '%s' -> %p%s%s", file ? file : "(null)", (void *)music, music ? "" : " ", music ? "" : Mix_GetError());
    return music;
}

void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}

void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_SHIELD].value = 3;
    button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DASH].value = 0;
    button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_JUMP].value = 1;
    button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_ATTACK].value = 2;
    button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_L].value = 4;
    button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_R].value = 5;
    button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_QUIT].value = -1;
    button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_START].value = 11;
    button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_LEFT].value = 7; // PSP maps axis as buttons
    button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_RIGHT].value = 9;
    button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_UP].value = 8;
    button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DOWN].value = 6;
}

void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode)
{
    if (input_type) *input_type = INPUT_TYPE_JOYSTICK;
    if (input_mode) *input_mode = INPUT_MODE_DIGITAL;
}

E_PLATFORM get_platform()
{
    return PLATFORM_PSP;
}

void show_config_video()
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 24;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(300);

    short selected_option = 0;

    while (selected_option != -1) {
        std::vector<std::string> options;
        if (SharedData::get_instance()->game_config.video_fullscreen == true) {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_windowed));
        } else {
            options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_video_fullscreen));
        }
        option_picker main_config_picker(false, config_text_pos, options, true);
        selected_option = main_config_picker.pick(selected_option + 1);
        if (selected_option == 0) {
            SharedData::get_instance()->game_config.video_fullscreen = !SharedData::get_instance()->game_config.video_fullscreen;
        }
        if (selected_option != -1) {
            fio.save_config(SharedData::get_instance()->game_config);
            // ask restart (same UX as game_menu::show_config_ask_restart)
            input.clean();
            timer.delay(300);
            st_position menu_pos(graphLib.get_config_menu_pos().x + 24, graphLib.get_config_menu_pos().y + 40);
            graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W, 180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
            graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart1));
            graphLib.draw_text(menu_pos.x, menu_pos.y + 10, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart2));
            graphLib.draw_text(menu_pos.x, menu_pos.y + 20, strings_map::get_instance()->get_ingame_string(strings_ingame_config_restart3));
            graphLib.draw_text(menu_pos.x, menu_pos.y + 40, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart));
            draw_lib.update_screen();
            input.wait_keypress();
            graphLib.clear_area(menu_pos.x, menu_pos.y, RES_W, 180, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
            draw_lib.update_screen();
            main_config_picker.draw();
        }
    }
}

} // namespace psp_platform

#else

namespace psp_platform {

void setup() {}
void get_filepath(std::string &, int) {}
bool load_fonts(const char *, TTF_Font **, TTF_Font **, TTF_Font **, int, int) { return false; }
SDL_Surface *create_surface(int, int) { return NULL; }
SDL_Surface *set_video_mode(int, int, int) { return NULL; }
SDL_Surface *render_error_text(TTF_Font *, const char *, SDL_Color) { return NULL; }
SDL_Surface *format_text_for_blit(SDL_Surface *text_sf)
{
    if (text_sf) SDLL_FreeSurface(text_sf);
    return NULL;
}
SDL_Surface *render_utf8_text(TTF_Font *, const char *, SDL_Color) { return NULL; }
std::vector<std::string> read_games_directory(const std::string &) { return std::vector<std::string>(); }
void map_list_reserves(size_t *, size_t *, size_t *, size_t *) {}
unsigned int ram_available_linear_max() { return 0; }
unsigned int ram_available() { return 0; }

void adjust_window_size(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags)
{
    if (win_w) *win_w = requested_w;
    if (win_h) *win_h = requested_h;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
}
void adjust_display_format_alpha(SDL_Surface *) {}
bool get_present_rect(int, int, SDL_Rect *) { return false; }
RWopsPtr *rw_from_file(const char *file, const char *mode) { return SDL_RWFromFile(file, mode); }
void prepare_color_key(SDL_Surface *, int) {}
SoundChunk *load_wav(const char *file) { return Mix_LoadWAV(file); }
SoundMusic *load_mus(const char *file) { return Mix_LoadMUS(file); }

void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}
void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        button_codes_copy[i].type = JOYSTICK_INPUT_TYPE_BUTTON;
        button_codes_copy[i].value = -1;
        button_codes_copy[i].axis_type = 0;
    }
}
void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode)
{
    if (input_type) *input_type = INPUT_TYPE_JOYSTICK;
    if (input_mode) *input_mode = INPUT_MODE_DIGITAL;
}
E_PLATFORM get_platform() { return PLATFORM_PSP; }
void show_config_video() {}

} // namespace psp_platform

#endif
