#ifndef STAGE_SELECT_H
#define STAGE_SELECT_H

#include "graphicslib.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
//#include "game.h"


/**
 * @brief
 *
 */
class stage_select
{
public:
	stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]);
    /**
     * @brief
     *
     * @return short
     */
    short finished_stages() const;
    /**
     * @brief
     *
     * @return st_position
     */
    struct st_position select();

private:
    /**
     * @brief
     *
     */
    void load_graphics();
    /**
     * @brief
     *
     * @param x_inc
     * @param y_inc
     */
    void move_highlight(short int x_inc, short int y_inc);
    /**
     * @brief
     *
     */
    void show_highlight();
    /**
     * @brief
     *
     * @param face_file
     * @param botname
     * @param pos
     */
    void place_face(std::string face_file, std::string botname, st_position pos) const;
    /**
     * @brief
     *
     */
    void animate_highlight();
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param erase_eyes
     */
    void draw_eyes(int x, int y, bool erase_eyes);

private:
    graphicsLib_gSurface s_light; /**< TODO */
    graphicsLib_gSurface s_dark; /**< TODO */
    graphicsLib_gSurface eyes_surface; /**< TODO */
    graphicsLib_gSurface background; /**< TODO */
    struct st_position highlight_pos; /**< TODO */
    struct st_position select_pos; /**< TODO */
    graphicsLib_gSurface *STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT]; /**< TODO */
    bool highlight_animation_white; /**< TODO */
    unsigned int timer_intro; /**< TODO */
    graphicsLib_gSurface *light_mode; /**< TODO */
};

#endif // STAGE_SELECT_H
