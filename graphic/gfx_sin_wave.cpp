#include "gfx_sin_wave.h"
#include <math.h>

extern graphicsLib graphLib;

#include "timerlib.h"
extern timerLib timer;

#define SIN_STEPS 6


gfx_sin_wave::gfx_sin_wave(graphicsLib_gSurface *set_surface)
{
    angle = 0;
    speed = 1;
    max_amplitude = TILESIZE*2;
    amplitude = max_amplitude;
    surface = set_surface;
}

void gfx_sin_wave::set_speed(int set_speed)
{
    speed = set_speed;
}

void gfx_sin_wave::set_max_amplitude(int set_max_amplitude)
{
    max_amplitude = set_max_amplitude;
}

void gfx_sin_wave::show(int x, int y)
{
    float angle_max = 3.14 * SIN_STEPS;
    float angle_step = angle_max / surface->width;
    float angle = 0;
    for (int j=0; j<max_amplitude; j++) {
        graphLib.clear_area(x, y, surface->width, surface->height, 0, 0, 0);
        for (int i=0; i<surface->height; i++) {
            float pos_x = (sin(angle) + x)*amplitude;
            int pos_y = i + y;
            angle += angle_step;
            //std::cout << "i[" << i << "], pos_x[" << pos_x << "], pos_y[" << pos_y << "]" << std::endl;
            graphLib.showSurfacePortion(surface, st_rectangle(0, i, surface->width, 1), st_rectangle(pos_x, pos_y, surface->width, 1));
        }
        amplitude--;
        graphLib.updateScreen();
        timer.delay(40);
    }
}

