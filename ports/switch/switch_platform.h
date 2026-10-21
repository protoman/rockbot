#pragma once

// All Switch-specific runtime behavior lives in switch_platform.cpp.

#include <string>
#include <cstddef>

#include "../../file/format/st_common.h"

namespace switch_platform {

// --- Bootstrap ---
void setup();
void shutdown();
// Prefer argv0 (nro path from hbmenu); fall back to getcwd.
void get_filepath(std::string &filepath, int maxpathlen, const char *argv0 = nullptr);
bool is_applet_mode();

// --- Graphics / video ---
SDL_Surface *set_video_mode(int res_w, int res_h, int colors);

// --- SDL layer hooks (called from sdl_layer.cpp) ---
void adjust_window_size(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags);

// --- Config defaults ---
void get_default_keys(int keys_codes_copy[BTN_COUNT]);
void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT]);
void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode);
E_PLATFORM get_platform();

// --- Menu ---
void show_config_video();

} // namespace switch_platform
