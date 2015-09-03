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
    /**
     * @brief
     *
     */
    void preloadScenes() const;
    /**
     * @brief
     *
     */
    void intro();
    /**
     * @brief
     *
     */
    void draw_main();
    /**
     * @brief
     *
     */
    void main_screen();
    /**
     * @brief
     *
     * @return short
     */
    short pick_stage();


    /**
     * @brief
     *
     */
    void unload_intro();
    /**
     * @brief
     *
     */
    void show_password(); // show the password fot the current save
    /**
     * @brief
     *
     * @param pos_n
     */
    void boss_intro(int pos_n) const;
    /**
     * @brief
     *
     */
    void show_destrin_ship_intro() const;
    /**
     * @brief
     *
     * @param stage_finished
     * @return short
     */
    short show_main_config(short stage_finished);

private:
	// preload/umload
    /**
     * @brief
     *
     */
    void preload_intro();
    /**
     * @brief
     *
     */
    void preload_stage_select() const;
    /**
     * @brief
     *
     */
    void preload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    /**
     * @brief
     *
     */
    void unload_faces();/// @TODO: move to graphLib and create a function show_face(st_position)
    /**
     * @brief
     *
     */
    void unload_stage_select();
    /**
     * @brief
     *
     */
    void intro_presents();

    /**
     * @brief
     *
     */
    void show_config_video();
    /**
     * @brief
     *
     */

    void show_config_video_PSP();
    /**
     * @brief
     *
     */

    void show_config_audio();
    /**
     * @brief
     *
     * @return bool
     */
    bool password_ball_selector();
    /**
     * @brief
     *
     * @return bool
     */
    bool password_set();
    /**
     * @brief
     *
     * @return bool
     */
    bool password_end_selector();
    /**
     * @brief
     *
     * @param ball_type
     */
    void password_number_selector(int ball_type);
    /**
     * @brief
     *
     * @return bool
     */
    bool show_password_input();
    /**
     * @brief
     *
     * @return Uint8
     */
    Uint8 select_player();
    /**
     * @brief
     *
     * @param lights
     * @param mode
     * @param adjustX
     * @param adjustY
     */
    void draw_lights_select_player(graphicsLib_gSurface& lights, int mode, int adjustX, int adjustY);
    /**
     * @brief
     *
     * @param vertical_first
     * @param initial_point
     * @param final_point
     * @param total_duration
     */
    void draw_castle_path(bool vertical_first, st_position initial_point, st_position final_point, short total_duration) const;




private:
    graphicsLib_gSurface INTRO_SURFACES[INTRO_SURFACES_COUNT]; /**< TODO */
    graphicsLib_gSurface STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT]; /**< TODO */
    st_position selected_stage; /**< TODO */
    graphicsLib_gSurface _password_selector; /**< TODO */

    unsigned int _timer; // animation timer /**< TODO */
    short _state; // animation state /**< TODO */
    graphicsLib_gSurface* _blink_surface; // animation surface (light/dark) /**< TODO */
    std::vector<std::string> options; /**< TODO */
    int _password_selected_balls[PASSWORD_GRID_SIZE][PASSWORD_GRID_SIZE]; /**< TODO */
};

#endif // SCENESLIB_H
