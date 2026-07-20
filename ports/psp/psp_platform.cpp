#include "psp_platform.h"

#ifdef PSP

#include <pspkernel.h>
#include <psppower.h>
#include <pspiofilemgr.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
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

extern graphicsLib graphLib;
extern inputLib input;
extern timerLib timer;
extern draw draw_lib;
extern CURRENT_FILE_FORMAT::file_io fio;

PSP_MODULE_INFO("Rockbot", PSP_MODULE_USER, 1, 0);
// Explicit heap: negative "all-but-N" can leave almost nothing under PPSSPP after SDL_Init.
PSP_HEAP_SIZE_KB(20 * 1024);
PSP_MAIN_THREAD_STACK_SIZE_KB(64);

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

namespace psp_platform {

void setup()
{
    SetupCallbacks();
    scePowerSetClockFrequency(333, 333, 166);
}

void get_filepath(std::string &filepath, int maxpathlen)
{
    // Under PPSSPP/real PSP, cwd should be the folder that contains EBOOT.PBP.
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
    SceUID font_fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
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
    *font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 0, font_size);
    *outline_font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 0, font_size);
    *error_font = TTF_OpenFontRW(SDL_RWFromConstMem(psp_font_blob, (int)font_sz), 0, error_font_size);
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
    return SDLL_TTF_RenderUTF8_Blended(error_font, text, color);
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
    // Force opaque alpha on every non-black pixel (Convert can leave A=0).
    if (SDL_MUSTLOCK(formatted)) {
        SDL_LockSurface(formatted);
    }
    Uint32 *px = (Uint32 *)formatted->pixels;
    const int n = (formatted->pitch / 4) * formatted->h;
    const Uint32 amask = formatted->format->Amask;
    const Uint32 rgbmask = formatted->format->Rmask | formatted->format->Gmask | formatted->format->Bmask;
    for (int i = 0; i < n; i++) {
        const Uint32 rgb = px[i] & rgbmask;
        px[i] = rgb ? (rgb | amask) : 0;
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
    // Prefer Solid (palette) then convert; Blended/Shaded often leave A=0 on ARGB blit.
    SDL_Surface *textSF = TTF_RenderUTF8_Solid(font, text, color);
    if (!textSF) {
        SDL_Color bg = {0, 0, 0, 255};
        textSF = TTF_RenderUTF8_Shaded(font, text, color, bg);
    }
    if (!textSF) {
        textSF = TTF_RenderUTF8_Blended(font, text, color);
    }
    return textSF;
}

std::vector<std::string> read_games_directory(const std::string &games_path)
{
    // opendir/readdir via newlib can wedge PPSSPP (mutex storm). Probe known games via sceIo.
    std::vector<std::string> res;
    if (games_path.find("/games") == std::string::npos) {
        return res;
    }
    const char *candidates[] = { "RockDroid1", "RockDroid2", NULL };
    for (int i = 0; candidates[i] != NULL; ++i) {
        std::string probe = games_path + "/" + candidates[i] + "/game_enemy_list.dat";
        SceUID fd = sceIoOpen(probe.c_str(), PSP_O_RDONLY, 0777);
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
            SDL_SetError("psp_platform::rw_from_file: SDL_RWFromFP failed for %s", file ? file : "(null)");
        }
        return rw;
    }

    if (!file) {
        SDL_SetError("psp_platform::rw_from_file: NULL path");
        return NULL;
    }

    const SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
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
        return NULL;
    }
    return Mix_LoadWAV_RW(rw, 1);
}

SoundMusic *load_mus(const char *file)
{
    SDL_RWops *rw = rw_from_file(file, "rb");
    if (!rw) {
        return NULL;
    }
    return Mix_LoadMUS_RW(rw, 1);
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
