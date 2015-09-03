#include <cmath>
#include <iostream>
#include "trajectory_parabola.h"

// http://coral.ufsm.br/gef/Dinamica/dinami10.pdf

trajectory_parabola::trajectory_parabola() : _max_point(1), _a(-1), _p0(0), _v(0),_reducefactor(1), _initialized(false), _dist(0)
{
    // do nothing. this should be used only as requirement for being used as non-pointer member in other classes
}

trajectory_parabola::trajectory_parabola(int distance) : _max_point(1), _a(-1), _p0(0), _v(0),_reducefactor(1), _initialized(false)
{
    set_distance(distance);
}

void trajectory_parabola::set_initial_x(int p0)
{
    _p0 = p0;
}

void trajectory_parabola::set_acceleration(int a)
{
    _a = a;
}

void trajectory_parabola::set_distance(int dist)
{
    _dist = std::abs((float)dist);
    _v = get_velocity();
    _initialized = true;
    _max_point = get_y_point(_dist/2);
    _reducefactor = _max_point/64;
    if (_reducefactor <= 0) {
        _reducefactor = 0.4;
    }
    //std::cout << ">>>> DEBUG #2 _reducefactor: " << _reducefactor << ", _max_point: " << _max_point << " speed: " << _v << " <<<<<" << std::endl;
}

int trajectory_parabola::get_y_point(int n) const
{
    if (!_initialized) {
        return 0;
    }
    n = std::abs((float)n);
    float y = _p0 + _v*n + (_a*(pow(n, 2)+n))/2; // y(t) = y(0) + v(0)*t + ½ a*t² (t = n, in this case)



    int reduced_y = 0;
    if (_reducefactor != 0) {
        reduced_y = (int)(y/_reducefactor);
    }
    if (reduced_y <= -21474834) {
        std::cout << ">>>parabola::get_y_point - n: " << n << ", _max_point: " << _reducefactor << ", y: " << y << ", reduced_y: " << reduced_y << std::endl;
    }
    return reduced_y;
}

float trajectory_parabola::get_velocity()
{
    // y = p0 + n*v + ((n² + n)*a)/2 -> nv = p0 - y + ((n² + n)*a)/2 -> v = (p0 - y + ((n² + n)*a)/2)/n
    int n = std::abs((float)_dist);
    int y = 0;
    float v = std::abs((float)(_p0 - y + ((n*n + n)*_a)/2)/n);
    return v;
}


