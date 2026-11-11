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
// true once the PS button / XMB asked the game to quit (the game quits by itself; update() forces it after 3 s)
bool exit_requested();
// logs the reason and closes the game at once (a file the game cannot run without could not be read)
void fatal_exit(const char *reason);
// Keeps the RSX command buffer from filling up. Call before every video flip.
void rsx_command_buffer_guard();
void flip_heartbeat();
unsigned int flip_number();
// Console debugging of the all-white stage: R3 cycles through test variants (0 = normal), see classmap.cpp show_map().
int debug_variant();
// Called on every flip with the presented surface: writes screenshots (BMP) to /dev_hdd0/tmp/rockbot/
void flip_debug_dump(SDL_Surface *screen);
// Polls pad 0 through libio (see ps3_platform.cpp). Analog values are 0..255, centre 128.
void pad_read(unsigned int *buttons, int *lx, int *ly);
// printf-style debug line to the PPU TTY (RPCS3 TTY.log); no stdio involved.
void log(const char *fmt, ...);
// writes the buffered log lines to the log file now (progress markers do it at once)
void log_flush();
// writes the pending lines from the calling thread (only right before the process exits)
void log_flush_now();
// The "STUCK" report of the helper thread says which file operation the game thread was in: set around every file
// operation (file/fio_diag.h). Both are no-ops on other platforms.
void mark_file(const char *op, const char *path);
void mark_file_done();
// one step into the flight recorder (the last 32 are printed in the STUCK report); what/detail are copied
void trail(const char *what, const char *detail);
// logs the game thread's stack size and usage and the heap numbers
void vitals(const char *where);
// which part of the screen flip the game thread is in (NULL = not in a flip); printed in the STUCK report
void flip_phase(const char *phase);

// --- Config defaults ---
void get_default_keys(int keys_codes_copy[BTN_COUNT]);
void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT]);
void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode);
E_PLATFORM get_platform();

} // namespace ps3_platform

// Progress marker in rockbot_ps3.log: the last one written shows where the game stopped (empty on other platforms).
#ifdef PS3
#define PS3_TRACE(...) do { ps3_platform::log(__VA_ARGS__); ps3_platform::log_flush(); } while (0)
#define PS3_STEP(what, detail) ps3_platform::trail((what), (detail))
#define PS3_VITALS(where) ps3_platform::vitals(where)
#else
#define PS3_TRACE(...) ((void)0)
#define PS3_STEP(what, detail) ((void)0)
#define PS3_VITALS(where) ((void)0)
#endif
