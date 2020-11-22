#ifndef FPS_CONTROL_H
#define FPS_CONTROL_H

#define DEFULT_PLAYER_SPEED 1.2
#define FPS_MINIMAL_LIMIT 50            // if fps is lower or equal this value, we increment fail-count
#define FPS_MINIMAL_MAX_FAIL 10         // if we have 10 consecutive times fps is under limit, we take action

#include <iostream>
#include <SDL/SDL.h>
#include <vector>
#include "shareddata.h"

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
    void fps_count();
    int get_current_frame_n();
    int get_frame_drop_n();
    bool get_failed_min_fps();
    void reset_failed_min_fps();
    void set_frameskip(int skip_n);
    bool must_skip_frame();

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

    unsigned long fps_timer;
    int fps_counter;
    // if we are getting less than 60 fps, this will tell main loop how many times it will run full until drop one
    unsigned short frame_drop_period;
    char _fps_buffer[128];

    unsigned int fps_min_fail_count; // counts the number of sequential times the FPS is under minumal limit
    bool failed_min_fps;

    std::vector<int> frameskip_list;
};

#endif // FPS_CONTROL_H
