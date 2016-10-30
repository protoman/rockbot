#ifndef GFX_SIN_WAVE_H
#define GFX_SIN_WAVE_H


#include "defines.h"
#include "graphicslib.h"

class gfx_sin_wave
{
public:
    gfx_sin_wave(graphicsLib_gSurface *set_surface);
    void set_speed(int set_speed);
    void set_max_amplitude(int set_max_amplitude);
    void show(int x, int y);

private:
    int angle;
    int speed;
    int amplitude;
    int max_amplitude;
    graphicsLib_gSurface *surface;
};

#endif // GFX_SIN_WAVE_H
