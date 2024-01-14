#include "shareddata.h"
#include "defines.h"

#include <iostream>

SharedData* SharedData::_instance = NULL;
double SharedData::fps_movement_multiplier = 1;
int SharedData::max_fps = DEFAULT_FPS_MAX;
double scaleX = 1.0;
double scaleY = 1.0;
bool changed_window_size = false;

SharedData::SharedData()
{

}

SharedData *SharedData::get_instance()
{
    if (!_instance) {
        _instance = new SharedData();
        fps_movement_multiplier = 60.0/max_fps;
    }
    return _instance;
}



int SharedData::get_max_fps()
{
    return max_fps;
}

double SharedData::get_movement_multiplier()
{
    return fps_movement_multiplier;
}





