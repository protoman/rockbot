#pragma once

// PlayStation Vita runtime specifics (VitaSDK + SDL2). Header-only: everything here is small.
// The Vita is far stronger than the PSP, so the game runs through the generic SDL2 path; this file
// only holds paths, input defaults and the screen layout.

#include <cstdio>
#include <cstdarg>
#include <string>
#include <sys/stat.h>

#include "../../defines.h"
#include "../../file/format/st_common.h"

namespace vita_platform {

// The game data (games/, shared/, fonts/) is inside the VPK (app0:), which is read-only.
// Saves and logs go to ux0:data/rockbot/.
static const char *const DATA_DIR = "ux0:data/rockbot/";
static const int LCD_W = 960;
static const int LCD_H = 544;

inline void get_filepath(std::string &filepath, int)
{
    filepath = "app0:/";
}

inline void get_savepath(std::string &savepath)
{
    mkdir("ux0:data", 0777);
    mkdir(DATA_DIR, 0777);
    savepath = DATA_DIR;
}

// Appends a line to ux0:data/rockbot/rockbot_boot.log (diagnostics for people testing on a real Vita).
inline void log(const char *fmt, ...)
{
    std::FILE *f = std::fopen("ux0:data/rockbot/rockbot_boot.log", "a");
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

// Destination rect on the 960x544 LCD: integer scale only (a fractional factor draws some pixel rows taller
// than others), centered with black borders.
inline bool get_present_rect(int game_w, int game_h, SDL_Rect *dst)
{
    if (game_w <= 0 || game_h <= 0) {
        return false;
    }
    int scale = LCD_W / game_w;
    if (LCD_H / game_h < scale) {
        scale = LCD_H / game_h;
    }
    if (scale < 1) {
        return false;
    }
    dst->w = game_w * scale;
    dst->h = game_h * scale;
    dst->x = (LCD_W - dst->w) / 2;
    dst->y = (LCD_H - dst->h) / 2;
    return true;
}

// SDL2 on the Vita numbers the joystick buttons like on the PSP:
// 0 triangle, 1 circle, 2 cross, 3 square, 4 L, 5 R, 6 down, 7 left, 8 up, 9 right, 10 select, 11 start.
inline void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}

inline void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    struct { int btn; int value; } map[] = {
        {BTN_SHIELD, 3}, {BTN_DASH, 0}, {BTN_JUMP, 1}, {BTN_ATTACK, 2}, {BTN_L, 4}, {BTN_R, 5},
        {BTN_QUIT, -1}, {BTN_START, 11}, {BTN_LEFT, 7}, {BTN_RIGHT, 9}, {BTN_UP, 8}, {BTN_DOWN, 6},
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
    return PLATFORM_VITA;
}

} // namespace vita_platform
