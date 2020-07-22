#ifndef STAGE_SELECT_H
#define STAGE_SELECT_H

#include <cstdlib>
#include <string>
#ifdef ANDROID
#include <android/log.h>
#endif


#include "graphicslib.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
//#include "game.h"

#define MAP_POINT_ANIMATION_DELAY 150

struct st_map_point_animation {
    long timer;
    int state;
    st_map_point_animation() {
        timer = 0;
        state = 0;
    }
};

/**
 * @brief
 *
 */
class stage_select
{
public:
	stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]);
    short finished_stages() const;
    void show_stage_face(int x, int y, int stage_n);
    st_position calc_face_pos(int stage_n);
    int pick_stage(int stage_n);

private:
    void place_face(std::string face_file, std::string botname, st_position pos) const;

private:
    graphicsLib_gSurface background;
    struct st_position highlight_pos;
    struct st_position select_pos;
    graphicsLib_gSurface *STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT];
    bool highlight_animation_white;
    unsigned int timer_intro;
    graphicsLib_gSurface *light_mode;
    std::vector<st_position> stage_select_point_list;
    graphicsLib_gSurface stage_select_map_point_surface;
    graphicsLib_gSurface beaten_surface;
    st_map_point_animation map_point_animation;
};

#endif // STAGE_SELECT_H
