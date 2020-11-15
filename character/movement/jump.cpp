#include <iostream>
#include <cmath>
#include "jump.h"


// if not interrupted, jump takes 700 milisseconds
// jump max high is 74 px

#include "../../game.h"
extern game gameControl;


classjump::classjump() : started(false)
{
    JUMP_INITIAL_SPEED = 5.675 * SharedData::get_instance()->get_movement_multiplier();
    JUMP_ACCELERATION = 0.25 * SharedData::get_instance()->get_movement_multiplier();
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
        speed = -(JUMP_INITIAL_SPEED * 1.40);
    } else {
        speed = -JUMP_INITIAL_SPEED;
    }

    if (start_terrain_type == TERRAIN_WATER) {
        acceleration = JUMP_ACCELERATION * 0.3;
    } else {
        acceleration = JUMP_ACCELERATION;
    }
    jumps_number++;

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

    speed += acceleration;
    moved += std::abs((double)speed);

    if (state == JUMPUP) {
        if (speed >= 0) {
            state = JUMPDOWN;
        } else if (is_bigjump == false && std::abs((double)moved) > JUMP_LIMIT) { // hardcoded limit of 3 tiles
            state = JUMPDOWN;
            speed = 0;
        }
    } else {
        if (speed > GRAVITY_MAX_SPEED * SharedData::get_instance()->get_movement_multiplier()) { // do not surpass the speed limit
            speed = GRAVITY_MAX_SPEED * SharedData::get_instance()->get_movement_multiplier();
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

float classjump::get_speed()
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
