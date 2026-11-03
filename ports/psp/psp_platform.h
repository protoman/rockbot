#pragma once

// All PSP-specific runtime behavior lives in psp_platform.cpp.

#include <string>
#include <vector>
#include <cstddef>

#include "../../file/format/st_common.h"

namespace psp_platform {

// --- Bootstrap ---
// argv0: the program path given by the loader (e.g. ms0:/PSP/GAME/Rockbot1/EBOOT.PBP). The data
// folder is taken from it; the current directory is not reliable when launched from the XMB.
void setup(const char *argv0 = NULL);
// Appends a line to <game folder>/rockbot_boot.log (diagnostics on real hardware).
void log(const char *fmt, ...);
// Tells the hang watchdog the game is alive (called once per frame).
void heartbeat();
// Called once per game logic tick (fps_control::limit): the perf line reports ticks per second.
void tick();
// Profiling of the screen update (microseconds): texture upload, clear+copy, present.
void flip_timing(unsigned int swap_us, unsigned int upload_us, unsigned int copy_us, unsigned int present_us);
// Options read at boot from empty marker files next to EBOOT.PBP.
bool use_rgb565();   // rgb565.txt: 16-bit screen texture (half the upload, slightly fewer colors)
void get_filepath(std::string &filepath, int maxpathlen);

// --- Graphics / fonts / video ---
bool load_fonts(const char *path,
                TTF_Font **font,
                TTF_Font **outline_font,
                TTF_Font **error_font,
                int font_size,
                int error_font_size);

SDL_Surface *create_surface(int width, int height);
SDL_Surface *set_video_mode(int res_w, int res_h, int colors);

SDL_Surface *render_error_text(TTF_Font *error_font, const char *text, SDL_Color color);
SDL_Surface *format_text_for_blit(SDL_Surface *text_sf); // takes ownership of text_sf
SDL_Surface *render_utf8_text(TTF_Font *font, const char *text, SDL_Color color);

// --- Filesystem ---
std::vector<std::string> read_games_directory(const std::string &games_path);

// --- Memory / map heuristics ---
void map_list_reserves(size_t *objects, size_t *npcs, size_t *animations, size_t *level3_tiles);
unsigned int ram_available_linear_max();
unsigned int ram_available();

// --- SDL layer hooks (called from sdl_layer.cpp) ---
void adjust_window_size(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags);
void adjust_display_format_alpha(SDL_Surface *surface);
// Destination rect for presenting the game frame on the 480x272 LCD.
// Returns false when the frame should fill the whole screen (stretched).
// Otherwise keeps the game's aspect ratio, centered with black bars; the GU
// renderer does this scaling in hardware, so it costs nothing extra.
bool get_present_rect(int game_w, int game_h, SDL_Rect *dst);
RWopsPtr *rw_from_file(const char *file, const char *mode);
void prepare_color_key(SDL_Surface *surface, int sdl2_flag);
SoundChunk *load_wav(const char *file);
SoundMusic *load_mus(const char *file);

// --- Config defaults ---
void get_default_keys(int keys_codes_copy[BTN_COUNT]);
void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT]);
void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode);
E_PLATFORM get_platform();

// --- Menu ---
void show_config_video();

} // namespace psp_platform
