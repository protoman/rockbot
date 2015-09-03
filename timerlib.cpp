#include <cmath>
#include <iostream>
#include "timerlib.h"
#include <SDL/SDL.h>

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

unsigned long timerLib::getTimer() const
{
    //std::cout << "timerLib::getTimer - SDL_GetTicks(): " << SDL_GetTicks() << ", _paused_ticks: " << _paused_ticks << std::endl;
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
    _paused_timer = SDL_GetTicks();
    std::cout << "PAUSE - _paused_timer: " << _paused_timer << std::endl;
}

void timerLib::unpause()
{
    _paused = false;
    long paused_ticks = SDL_GetTicks() - _paused_timer;
    std::cout << "UNPAUSE - _paused_ticks: " << _paused_ticks << ", paused_ticks: " << paused_ticks << std::endl;
    _paused_ticks += paused_ticks;
}

bool timerLib::is_paused()
{
    return _paused;
}


