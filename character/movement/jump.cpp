#include <iostream>
#include <cmath>
#include "jump.h"


// if not interrupted, jump takes 700 milisseconds
// jump max high is 74 px

#define JUMP_INITIAL_SPEED 4.875


classjump::classjump() : started(false)
{
    JUMP_ACCELERATION = 0.25;
    JUMP_LIMIT = (TILESIZE*3)-6;
    JUMP_LIMIT = 240;
    state = NOJUMP;
    jumps_number = 0;
}

void classjump::start(bool bigjump_mode)
{
    started = true;
    state = JUMPUP;
    is_bigjump = bigjump_mode;
    if (is_bigjump == true) {
        acceleration = JUMP_ACCELERATION * 0.4;
    } else {
        acceleration = JUMP_ACCELERATION;
    }
    jumps_number++;
    speed = -JUMP_INITIAL_SPEED;
    moved = 0;
}

bool classjump::is_started()
{
    return started;
}

void classjump::execute()
{
    if (!started) {
        return;
    }

    speed += acceleration;
    moved += std::abs((double)speed);
    //std::cout << ", speed.pos[" << speed << "], acceleration[" << acceleration << "]" << std::endl;

    if (state == JUMPUP) {
        if (speed >= 0) {
            state = JUMPDOWN;
        } else if (is_bigjump == false && std::abs((double)moved) > JUMP_LIMIT) { // hardcoded limit of 3 tiles
            state = JUMPDOWN;
            speed = 0;
        }
    } else {
        if (speed > GRAVITY_MAX_SPEED) { // do not surpass the speed limit
            speed = GRAVITY_MAX_SPEED;
        }
    }
}

void classjump::interrupt()
{
    if (!started) {
        return;
    }
    if (state != JUMPUP) {
        state = JUMPDOWN;
        speed = 0;
        return;
    }
    // do not interrupt jump if it already passed 80% of total height
    if (is_bigjump == false && moved >= JUMP_LIMIT*0.8) {
        return;
    }

    state = JUMPDOWN;
    speed = 0;
}

void classjump::finish()
{
    jumps_number = 0;
    state = NOJUMP;
    speed = 0;
    started = false;
}

int classjump::get_speed()
{
    return speed;
}

short classjump::get_jumps_number()
{
    return jumps_number;
}

void classjump::set_jump_acceleration(double value)
{
    JUMP_ACCELERATION = value;

}

void classjump::set_jump_limit(int value)
{
    JUMP_LIMIT = value;
}
