#include "switch_platform.h"

#ifdef SWITCH

#include <switch.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>

#include "../../sdl_layer.h"
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

namespace {

bool g_sockets_ok = false;

} // namespace

namespace switch_platform {

bool is_applet_mode()
{
    return appletGetAppletType() != AppletType_Application;
}

void setup()
{
    // nxlink stdout over network when debugging via USB/net.
    if (R_SUCCEEDED(socketInitializeDefault())) {
        g_sockets_ok = true;
        nxlinkStdio();
    }
    // Prefer docked / handheld native resolution via SDL fullscreen window.
    appletSetFocusHandlingMode(AppletFocusHandlingMode_NoSuspend);

    if (is_applet_mode()) {
        // Album applet ≈ 40MB heap — RockDroid2 preload can OOM mid-image (EXIT #05).
        printf("WARNING: Switch applet mode (low memory). Launch via title redirection.\n");
        fflush(stdout);
    }
}

void shutdown()
{
    if (g_sockets_ok) {
        socketExit();
        g_sockets_ok = false;
    }
}

void get_filepath(std::string &filepath, int maxpathlen, const char *argv0)
{
    filepath.clear();

    // hbmenu passes the full nro path as argv[0] (more reliable than getcwd).
    if (argv0 != nullptr && argv0[0] != '\0') {
        std::string arg(argv0);
        const std::string::size_type slash = arg.find_last_of('/');
        if (slash != std::string::npos) {
            filepath = arg.substr(0, slash + 1);
            if (chdir(filepath.c_str()) != 0) {
                printf("switch_platform::get_filepath: chdir('%s') failed\n", filepath.c_str());
                fflush(stdout);
            }
            return;
        }
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

SDL_Surface *set_video_mode(int res_w, int res_h, int colors)
{
    return SDLL_SetVideoMode(res_w, res_h, colors, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_FULLSCREEN);
}

void adjust_window_size(int /*requested_w*/, int /*requested_h*/, int *win_w, int *win_h, Uint32 *window_flags)
{
    // Switch 1 docked / handheld: 1280x720; game framebuffer stays 320x240 and is scaled by SDL.
    if (win_w) *win_w = 1280;
    if (win_h) *win_h = 720;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
}

void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}

void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    // SDL2 Switch pad button indices (libnx / SDL_sysjoystick).
    // A=0 B=1 X=2 Y=3 L=6 R=7 Plus=10 Minus=11 DPad L/U/R/D = 12/13/14/15
    button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_JUMP].value = 0; // A
    button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_ATTACK].value = 1; // B
    button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_SHIELD].value = 2; // X
    button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DASH].value = 3; // Y
    button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_L].value = 6;
    button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_R].value = 7;
    button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_START].value = 10; // Plus
    button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_QUIT].value = 11; // Minus
    button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_LEFT].value = 12;
    button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_UP].value = 13;
    button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_RIGHT].value = 14;
    button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DOWN].value = 15;
}

void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode)
{
    if (input_type) *input_type = INPUT_TYPE_JOYSTICK;
    if (input_mode) *input_mode = INPUT_MODE_DIGITAL;
}

E_PLATFORM get_platform()
{
    return PLATFORM_SWITCH;
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

} // namespace switch_platform

#else

namespace switch_platform {

void setup() {}
void shutdown() {}
void get_filepath(std::string &, int, const char *) {}
bool is_applet_mode() { return false; }
SDL_Surface *set_video_mode(int, int, int) { return NULL; }

void adjust_window_size(int requested_w, int requested_h, int *win_w, int *win_h, Uint32 *window_flags)
{
    if (win_w) *win_w = requested_w;
    if (win_h) *win_h = requested_h;
    if (window_flags) *window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
}

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

E_PLATFORM get_platform() { return PLATFORM_SWITCH; }
void show_config_video() {}

} // namespace switch_platform

#endif
