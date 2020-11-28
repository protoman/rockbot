#ifndef SCENESLIB_H
#define SCENESLIB_H

#include "defines.h"
#include "graphicslib.h"
#include "scenes/game_menu.h"
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
    void show_cheats_menu();
    void show_game_scene(e_game_scenes_types n);
    void show_player_walking_ending();
    void show_enemies_ending();
    void ending_show_single_enemy(int id, std::string name);
    void show_bosses_ending();
    short pick_stage(int last_stage);
    Uint8 select_player();
    void boss_intro(short stage_n);
    void game_scenes_show_unbeaten_intro();
    short select_save(bool is_new_game);
    void show_ending_scenes(e_PLAYERS selected_player);

private:
    // preload/unload
    void preload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_stage_select();

    Uint8 select_difficulty();
    void draw_save_details(int n, format_v4::st_save save);


private:
    graphicsLib_gSurface STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT];

    unsigned int _timer; // animation timer
    short _state; // animation state
    graphicsLib_gSurface* _blink_surface; // animation surface (light/dark)

    CURRENT_FILE_FORMAT::fio_scenes fio_scn;
    std::map<Uint8, Uint8> game_scenes_map;
    game_menu menu;


};

#endif // SCENESLIB_H
