#ifndef FPS_CONTROL_H
#define FPS_CONTROL_H

#include <SDL/SDL.h>

// === FPS limiter by Felipe Zacani === //

struct st_fps_data
{
    unsigned int frame_min;
    unsigned int frame_max;
    double frame_average;
    int fps;
};


class fps_control
{
public:
    fps_control();
    void initialize();
    bool limit();
    void set_max_fps(unsigned short max);
    float get_fps_speed_multiplier();


private:
    st_fps_data data;
    float max_frame_ticks;
    unsigned int last_second_ticks;
    int frame_count;

    unsigned int min_ticks;
    unsigned int max_ticks;
    double average_ticks;
    unsigned int last_frame_ticks;

    unsigned int current_ticks;
    unsigned int target_ticks;

    unsigned short fps_max;
    float fps_speed_multiplier;
};

#endif // FPS_CONTROL_H
