#ifndef GAME_MENU_H
#define GAME_MENU_H

#include <cstdlib>
#include <string>

#include "defines.h"
#include "graphicslib.h"
#include "strings_map.h"
#include "game_mediator.h"
#include "file/file_io.h"
#include "file/fio_strings.h"
#include "graphic/option_picker.h"
#include "options/key_map.h"

class game_menu
{
public:
    game_menu();

    void show_extra_menu();
    short show_main_config(short stage_finished, bool called_from_game);

    // platform dependant configs
    void show_config_android();
    void show_config_video();
    void show_config_video_PSP();
    void show_config_wii();
    void show_config_PS2();
    void show_config_ask_restart();
    void show_config_audio();
    void show_config_language();
    void show_config_performance();
    void show_config_warning_android_play_services();
    void show_config_warning_android_cloud_save();
    void show_config_warning_android_hide_controls();
    void show_config_extras();
    void config_int_value(Uint8 &value_ref, int min, int max);

    void music_player();
    void sfx_player();
};

#endif // GAME_MENU_H
