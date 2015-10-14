#ifndef SCENESLIB_H
#define SCENESLIB_H

#include "defines.h"
#include "graphicslib.h"

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
    void preloadScenes() const;
    void draw_main();
    void main_screen();
    short pick_stage();
    void show_password(); // show the password fot the current save
    void boss_intro(Uint8 pos_n) const;
    short show_main_config(short stage_finished);

private:
    // preload/unload
    void preload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    void unload_stage_select();
    void show_config_video();
    void show_config_video_PSP();
    void show_config_audio();
    bool password_ball_selector();
    bool password_set();
    bool password_end_selector();
    void password_number_selector(int ball_type);
    bool show_password_input();
    Uint8 select_player();
    void draw_lights_select_player(graphicsLib_gSurface& lights, int mode, int adjustX, int adjustY);
    void draw_castle_path(bool vertical_first, st_position initial_point, st_position final_point, short total_duration) const;


private:
    graphicsLib_gSurface STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT];
    st_position selected_stage;
    graphicsLib_gSurface _password_selector;

    unsigned int _timer; // animation timer
    short _state; // animation state
    graphicsLib_gSurface* _blink_surface; // animation surface (light/dark)
    std::vector<std::string> options;
    Sint8 _password_selected_balls[PASSWORD_GRID_SIZE][PASSWORD_GRID_SIZE];


};

#endif // SCENESLIB_H
