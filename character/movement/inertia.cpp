#include <iostream>
#include "inertia.h"

#define INERTIA_STOP_LIMIT 42 // @TODO - implementar desaceleração
#define INERTIA_HALF_STOP_LIMIT 36 // @TODO - implementar desaceleração
#define INERTIA_SPEED_NORMAL 2
#define INERTIA_SPEED_SLOW 5        // float part of movement

inertia::inertia()
{
    _counter = 0;
    _sub_counter = 0;
    _started = false;
}

int inertia::execute()
{
    if (_started == false) {
        return 0;
    }
    if (_counter < INERTIA_STOP_LIMIT) {
        if (_counter < INERTIA_HALF_STOP_LIMIT) {
            _counter += INERTIA_SPEED_NORMAL;
            return INERTIA_SPEED_NORMAL;
        } else {
            _sub_counter++;
            if (_sub_counter > 2) {
                _counter += INERTIA_SPEED_NORMAL;
                _sub_counter = 0;
                return INERTIA_SPEED_NORMAL;
            } else {
                return 0;
            }
        }
    } else {
        stop();
        return 0;
    }
}

void inertia::start()
{
    _started = true;
    _counter = 0;
    _sub_counter = 0;
}

void inertia::stop()
{
    _started = false;
    _counter = 0;
}

bool inertia::is_started()
{
    return _started;
}
