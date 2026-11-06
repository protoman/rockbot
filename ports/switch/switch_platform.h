#pragma once

// Nintendo Switch runtime specifics (devkitA64 + libnx + SDL2). Header-only and small: the game runs
// through the generic SDL2 path. This file holds paths, input defaults and the screen layout.

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <sys/stat.h>

#include "../common/present_rect.h"
#include "../../defines.h"
#include "../../file/format/st_common.h"

namespace switch_platform {

static const int LCD_W = 1280;
static const int LCD_H = 720;

// Homebrew is started from the Homebrew Menu as sdmc:/switch/<folder>/rockbot.nro. The game data
// (games/, shared/, fonts/) and the saves live in that same folder.
inline std::string &base_dir()
{
    static std::string dir = "sdmc:/switch/rockbot/";
    return dir;
}

inline bool has_games(const std::string &dir)
{
    struct stat st;
    return stat((dir + "games").c_str(), &st) == 0;
}

// argv0: the path the loader gives (e.g. sdmc:/switch/rockbot/rockbot.nro); emulators may pass a host path
// or only the file name. The folder that holds games/ wins: the NRO's folder, the Homebrew Menu folder,
// then the current directory.
inline void setup(const char *argv0)
{
    if (argv0 != NULL) {
        std::string path(argv0);
        size_t slash = path.find_last_of('/');
        if (slash != std::string::npos) {
            std::string dir = path.substr(0, slash + 1);
            if (has_games(dir) || !has_games("sdmc:/switch/rockbot/")) {
                base_dir() = dir;
            }
        }
    }
    if (!has_games(base_dir()) && has_games("./")) {
        base_dir() = "./";
    }
}

inline void get_filepath(std::string &filepath, int)
{
    filepath = base_dir();
}

inline void get_savepath(std::string &savepath)
{
    savepath = base_dir();
}

// Appends a line to <game folder>/rockbot_boot.log (diagnostics for people testing on a real console).
inline void log(const char *fmt, ...)
{
    std::FILE *f = std::fopen((base_dir() + "rockbot_boot.log").c_str(), "a");
    if (f == NULL) {
        f = std::fopen("rockbot_boot.log", "a"); // the data folder may not exist or not be writable
    }
    if (f == NULL) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    std::vfprintf(f, fmt, ap);
    va_end(ap);
    std::fputc('\n', f);
    std::fclose(f);
}

// 1280x720: the 320x240 frame is drawn at 3x (960x720) with borders left and right.
inline bool get_present_rect(int game_w, int game_h, SDL_Rect *dst)
{
    return present_rect::integer_scale(LCD_W, LCD_H, game_w, game_h, dst);
}

// SDL2 on the Switch numbers the joystick buttons:
// 0 A, 1 B, 2 X, 3 Y, 4 L-stick, 5 R-stick, 6 L, 7 R, 8 ZL, 9 ZR, 10 plus, 11 minus, 12 left, 13 up, 14 right, 15 down.
// Layout by position, like the PSP: bottom (B) jump, right (A) dash, left (Y) attack, top (X) shield.
inline void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}

inline void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    struct { int btn; int value; } map[] = {
        {BTN_SHIELD, 2}, {BTN_DASH, 0}, {BTN_JUMP, 1}, {BTN_ATTACK, 3}, {BTN_L, 6}, {BTN_R, 7},
        {BTN_QUIT, 11}, {BTN_START, 10}, {BTN_LEFT, 12}, {BTN_RIGHT, 14}, {BTN_UP, 13}, {BTN_DOWN, 15},
    };
    for (unsigned i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        button_codes_copy[map[i].btn].type = JOYSTICK_INPUT_TYPE_BUTTON;
        button_codes_copy[map[i].btn].value = map[i].value;
    }
}

inline void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode)
{
    if (input_type) *input_type = INPUT_TYPE_JOYSTICK;
    if (input_mode) *input_mode = INPUT_MODE_DIGITAL;
}

inline E_PLATFORM get_platform()
{
    return PLATFORM_SWITCH;
}

} // namespace switch_platform
