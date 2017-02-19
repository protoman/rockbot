#include "fps_control.h"

#include <iostream>

#define DEFULT_PLAYER_SPEED 1.2
#define DEFAULT_FPS_MAX 60

#include "timerlib.h"
extern timerLib timer;


fps_control::fps_control()
{
    fps_max = DEFAULT_FPS_MAX;
    fps_speed_multiplier = 1.0;
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
    //std::cout << "timer.ticks[" << timer.getTimer() << "], sdl.ticks[" << current_ticks << "]" << std::endl;

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

void fps_control::set_max_fps(unsigned short max)
{
    fps_max = max;
    float percent = (100 * fps_max) / DEFAULT_FPS_MAX;

    std::cout << "FPS_CONTROL.set_max_fps[" << max << "], percent[" << percent << "]" << std::endl;

    if (fps_max < DEFAULT_FPS_MAX) {
        fps_speed_multiplier = 1.0 + (percent / 100);
    } else if (fps_max == DEFAULT_FPS_MAX) {
        fps_speed_multiplier = 1.0;
    } else {
        fps_speed_multiplier = 1.0 - (percent / 100);
    }
    max_frame_ticks = (1000.0/(float)fps_max)+0.00001;
}

float fps_control::get_fps_speed_multiplier()
{
    //std::cout << "FPS_CONTROL::get_fps_speed_multiplier[" << fps_speed_multiplier << "]" << std::endl;
    return fps_speed_multiplier;
}
