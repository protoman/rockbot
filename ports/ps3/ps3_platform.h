#pragma once

// All PlayStation 3 specific runtime behavior lives in ps3_platform.cpp.

#include <string>
#include <cstddef>

#include "../../defines.h"
#include "../../file/format/st_common.h"

namespace ps3_platform {

// --- Bootstrap ---
void setup(const char *argv0);
void get_filepath(std::string &filepath, int maxpathlen);
void get_savepath(std::string &savepath);
void update();
// Keeps the RSX command buffer from filling up. Call before every video flip.
void rsx_command_buffer_guard();
// Polls pad 0 through libio (see ps3_platform.cpp). Analog values are 0..255, centre 128.
void pad_read(unsigned int *buttons, int *lx, int *ly);
// printf-style debug line to the PPU TTY (RPCS3 TTY.log); no stdio involved.
void log(const char *fmt, ...);

// --- Config defaults ---
void get_default_keys(int keys_codes_copy[BTN_COUNT]);
void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT]);
void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode);
E_PLATFORM get_platform();

} // namespace ps3_platform
