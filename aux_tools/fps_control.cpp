#include "fps_control.h"

#define FPS_MAX 60


fps_control::fps_control()
{
}


void fps_control::initialize()
{
    max_frame_ticks = (1000.0/(float)FPS_MAX)+0.00001;
    frame_count = 0;
    last_second_ticks = SDL_GetTicks();
}

bool fps_control::limit()
{
    ++frame_count;
    target_ticks = last_second_ticks + static_cast<unsigned int>(frame_count * max_frame_ticks);
    current_ticks = SDL_GetTicks();

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
        current_ticks = SDL_GetTicks();
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
        last_second_ticks = SDL_GetTicks();
        return true;
    }

    return false;
}
