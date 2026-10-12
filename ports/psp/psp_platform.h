#pragma once

// All PSP-specific runtime behavior lives in psp_platform.cpp.

#include <string>
#include <vector>
#include <cstddef>

#include "../../file/format/st_common.h"

namespace psp_platform {

// --- Bootstrap ---
void setup();
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
