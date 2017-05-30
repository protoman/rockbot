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


/**
 * @brief
 *
 */
class stage_select
{
public:
	stage_select(graphicsLib_gSurface stage_ref[STAGE_SELECT_COUNT]);
    short finished_stages() const;
    int pick_stage_map(int start_stage);

    int pick_stage(int start_stage);
    void draw_stage_select_text_info(int stage_n);

    bool walk_path(int incx, int incy, st_position &pos, CURRENT_FILE_FORMAT::file_stage_select map_data);

private:
    void move_highlight(Sint8 x_inc, Sint8 y_inc);
    void show_highlight();
    void place_face(std::string face_file, std::string botname, st_position pos) const;
    void animate_highlight();
    void draw_eyes(Uint8 x, Uint8 y, bool erase_eyes);

private:
    graphicsLib_gSurface s_light;
    graphicsLib_gSurface s_dark;
    graphicsLib_gSurface eyes_surface;
    graphicsLib_gSurface background;
    struct st_position highlight_pos;
    struct st_position select_pos;
    graphicsLib_gSurface *STAGE_SELECT_SURFACES[STAGE_SELECT_COUNT];
    bool highlight_animation_white;
    unsigned int timer_intro;
    graphicsLib_gSurface *light_mode;
};

#endif // STAGE_SELECT_H
