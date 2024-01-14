#include "fps_control.h"

#include <algorithm>    // std::find

#include "../timerlib.h"
extern timerLib timer;

#include "../graphicslib.h"
extern graphicsLib graphLib;


fps_control::fps_control() : fps_timer(0)
{
    fps_max = SharedData::get_instance()->get_max_fps();
    fps_counter = 0;
    fps_min_fail_count = 0;
    failed_min_fps = false;
}


void fps_control::initialize()
{
    max_frame_ticks = (1000.0/(float)fps_max)+0.00001;
    frame_count = 0;
    last_second_ticks = timer.getTimer();
}

bool fps_control::limit()
{
    if (timer.is_paused()) {
        return true;
    }
    ++frame_count;
    target_ticks = last_second_ticks + static_cast<unsigned int>(frame_count * max_frame_ticks);
    current_ticks = timer.getTimer();

    average_ticks += current_ticks - last_frame_ticks;
    if (current_ticks - last_frame_ticks <= min_ticks)
    {
        min_ticks = current_ticks - last_frame_ticks;
    }

    if (current_ticks - last_frame_ticks >= max_frame_ticks)
    {
        max_ticks = current_ticks - last_frame_ticks;
    }

    if (current_ticks < target_ticks)
    {
        SDL_Delay(target_ticks - current_ticks);
        current_ticks = timer.getTimer();
    }

    last_frame_ticks = current_ticks;

    if (current_ticks - last_second_ticks >= 1000)
    {
        data.fps = frame_count;
        data.frame_average = average_ticks / frame_count;
        data.frame_min = min_ticks;
        data.frame_max = max_ticks;

        frame_count = 0;
        min_ticks = 1000;
        max_ticks = 0;
        average_ticks = 0;
        last_second_ticks = timer.getTimer();
        return true;
    }

    return false;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void fps_control::fps_count()
{
    if (timer.is_paused()) {
        return;
    }
    fps_counter++;
    if (fps_timer <= timer.getTimer()) {
        sprintf(_fps_buffer, "FPS: %d", fps_counter);
        if (fps_counter <= fps_timer-4) {
            frame_drop_period = fps_timer/(fps_timer-fps_counter);
        } else {
            frame_drop_period = 0;
        }
        fps_counter = 0;
        fps_timer = timer.getTimer()+1000;
    }
    if (fps_counter > 1) {
        /*
        if (fps_counter <= FPS_MINIMAL_LIMIT) {
            fps_min_fail_count++;
        } else {
            fps_min_fail_count = 0;
        }
        if (fps_min_fail_count >= FPS_MINIMAL_MAX_FAIL) {
            failed_min_fps = true;
        }
        */
        std::string temp_str(_fps_buffer);
        graphLib.draw_text(12, 2, temp_str);
    }
}

int fps_control::get_current_frame_n()
{
    return fps_counter;
}

int fps_control::get_frame_drop_n()
{
    if (frame_drop_period < 2) {
        return 2;
    }
    return frame_drop_period;
}

bool fps_control::get_failed_min_fps()
{
    return failed_min_fps;
}

void fps_control::reset_failed_min_fps()
{
    failed_min_fps = false;
}

void fps_control::set_frameskip(int skip_n)
{
    frameskip_list.clear();
    if (skip_n == 1) {
        frameskip_list.push_back(1);
    } else if (skip_n == 2) {
        frameskip_list.push_back(1);
        frameskip_list.push_back(6);
    } else if (skip_n == 3) {
        frameskip_list.push_back(1);
        frameskip_list.push_back(4);
        frameskip_list.push_back(7);
    } else if (skip_n == 4) {
        frameskip_list.push_back(1);
        frameskip_list.push_back(3);
        frameskip_list.push_back(6);
        frameskip_list.push_back(9);
    } else if (skip_n == 5) {
        frameskip_list.push_back(1);
        frameskip_list.push_back(3);
        frameskip_list.push_back(5);
        frameskip_list.push_back(7);
        frameskip_list.push_back(9);
    }
}

bool fps_control::must_skip_frame()
{
    int frame_unit_part = frame_count % 10;
    if (std::find(frameskip_list.begin(), frameskip_list.end(), frame_unit_part) != frameskip_list.end()) {
        return true;
    }
    return false;
}

