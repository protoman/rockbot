#include <cmath>
#include <iostream>
#include "timerlib.h"
#include "sdl_layer.h"
#include <unistd.h>
#ifdef PS3
#include <sys/systime.h>
#endif

timerLib::timerLib()
{
    _paused_timer = 0;
    _ticks = 0;
    _paused_ticks = 0;
    _paused = false;
}

void timerLib::delay(int waitMS) const
{
#ifdef PS3
	sysUsleep(waitMS * 1000);
#else
	SDLL_Delay(waitMS);
#endif
}

void timerLib::udelay(int useconds)
{
#ifdef PS3
	sysUsleep(useconds);
#elif !defined(PC)
	SDLL_Delay(useconds);
#else
    usleep(useconds);
#endif
}

unsigned long timerLib::getTimer() const
{
#ifdef PS3
	u64 sec = 0;
	u64 nsec = 0;
	sysGetCurrentTime(&sec, &nsec);
	return static_cast<unsigned long>(sec * 1000 + nsec / 1000000) - _paused_ticks;
#else
	return SDLL_GetTicks() - _paused_ticks;
#endif
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


