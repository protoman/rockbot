#include <cmath>
#include <iostream>
#include "timerlib.h"
#include <SDL/SDL.h>
#include <unistd.h>

timerLib::timerLib()
{
    _paused_timer = 0;
    _ticks = 0;
    _paused_ticks = 0;
    _paused = false;
}

void timerLib::delay(int waitMS) const
{
    SDL_Delay(waitMS);
}

void timerLib::udelay(int useconds)
{
#ifndef PC
    SDL_Delay(useconds);
#else
    usleep(useconds);
#endif
}

unsigned long timerLib::getTimer() const
{
    return SDL_GetTicks() - _paused_ticks;
}

void timerLib::start_ticker()
{
    _ticks = getTimer();
}

long timerLib::get_ticks()
{
    return getTimer() - _ticks;
}

void timerLib::pause()
{
    _paused = true;
    _paused_timer = getTimer();
}

void timerLib::unpause()
{
    // unpausing without pause being called
    if (_paused == false) {
        return;
    }
    _paused = false;
    long paused_ticks = getTimer() - _paused_timer;
    _paused_ticks += paused_ticks;
}

bool timerLib::is_paused()
{
    return _paused;
}


