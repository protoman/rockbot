#include <iostream>
#include <cmath>
#include "jump.h"


// if not interrupted, jump takes 700 milisseconds
// jump max high is 74 px

#define JUMP_INITIAL_SPEED 5.375

#include "game.h"
extern game gameControl;


classjump::classjump() : started(false)
{
    JUMP_ACCELERATION = 0.25;
    JUMP_LIMIT = (TILESIZE*3)-6;
    JUMP_LIMIT = 240;
    state = NOJUMP;
    jumps_number = 0;
    start_terrain_type = TERRAIN_UNBLOCKED;
}

void classjump::start(bool bigjump_mode, int terrain_type)
{
    started = true;
    state = JUMPUP;
    is_bigjump = bigjump_mode;
    start_terrain_type = terrain_type;
    start_big_jump = bigjump_mode;
    if (start_terrain_type == TERRAIN_WATER) {
        is_bigjump = true;
        speed = -(JUMP_INITIAL_SPEED * 0.9);
    } else if (is_bigjump == true) {
        //acceleration = JUMP_ACCELERATION * 0.4;
        speed = -(JUMP_INITIAL_SPEED * 1.25);
    } else {
        speed = -JUMP_INITIAL_SPEED;
    }

    if (start_terrain_type == TERRAIN_WATER) {
        acceleration = JUMP_ACCELERATION * 0.3;
    } else {
        acceleration = JUMP_ACCELERATION;
    }
    acceleration = acceleration * gameControl.get_fps_speed_multiplier();
    jumps_number++;

    //std::cout << "CLASSJUMP::START::speed: " << speed << std::endl;

    moved = 0;
}

bool classjump::is_started()
{
    return started;
}

void classjump::execute(int terrain_type)
{
    if (!started) {
        return;
    }

    // if big jump was caused only because of water, acceleration changes if you are still in water or not
    if (start_terrain_type == TERRAIN_WATER && start_big_jump == false) {
        if (terrain_type == TERRAIN_WATER) {
            acceleration = JUMP_ACCELERATION * 0.3;
        } else {
            acceleration = JUMP_ACCELERATION * 0.7;
        }
    }


    //std::cout << "CLASSJUMP::EXECUTE[#1]::speed: " << speed << std::endl;
    speed += acceleration;
    moved += std::abs((double)speed);

    //std::cout << "CLASSJUMP::EXECUTE[#2]::speed: " << speed << std::endl;

    if (state == JUMPUP) {
        if (speed >= 0) {
            state = JUMPDOWN;
        } else if (is_bigjump == false && std::abs((double)moved) > JUMP_LIMIT) { // hardcoded limit of 3 tiles
            state = JUMPDOWN;
            std::cout << "OBJUMP RESET SPEED #3" << std::endl;
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
        std::cout << "%%%%%%%%%%%%%%%%% OBJUMP RESET SPEED #4 %%%%%%%%%%%%%%%%%%" << std::endl;
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

float classjump::get_speed()
{
    return speed * gameControl.get_fps_speed_multiplier();
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
