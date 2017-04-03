#ifndef SCENESLIB_H
#define SCENESLIB_H

#include "defines.h"
#include "graphicslib.h"
#include "file/fio_scenes.h"


/**
 * @brief
 *
 */
class scenesLib
{
public:
/**
 * @brief
 *
 */
    scenesLib();
    void preloadScenes();
    void draw_main();
    void main_screen();
    short pick_stage(int last_stage);
    void boss_intro(Uint8 pos_n) const;
    short show_main_config(short stage_finished, bool called_from_game);
    void game_scenes_show_unbeaten_intro();
    void show_game_scene(e_game_scenes_types n);
    void show_player_ending();
    void show_enemies_ending();

private:
    // preload/unload
    void preload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_stage_select();

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


    void config_int_value(Uint8 &value_ref, int min, int max);
    Uint8 select_player();
    Uint8 select_difficulty();
    void draw_lights_select_player(graphicsLib_gSurface& lights, int mode, int adjustX, int adjustY);
    void draw_castle_path(bool vertical_first, st_position initial_point, st_position final_point, short total_duration) const;


private:
    graphicsLib_gSurface STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT];

    unsigned int _timer; // animation timer
    short _state; // animation state
    graphicsLib_gSurface* _blink_surface; // animation surface (light/dark)

    CURRENT_FILE_FORMAT::fio_scenes fio_scn;
    std::map<Uint8, Uint8> game_scenes_map;


};

#endif // SCENESLIB_H
