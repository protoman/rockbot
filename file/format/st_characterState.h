#ifndef ST_CHARACTERSTATE_H
#define ST_CHARACTERSTATE_H

#include <iostream>
#include "../../defines.h"

/**
 * What a character is doing and showing: the animation (type, frame, direction, timer) and a few flags.
 * The animation fields are private on purpose: they are read through the accessors and written through the set_*
 * methods (character::set_animation_type / set_direction add the rules, like resetting the frame).
 */
class st_characterState {
public:
    st_characterState()
    {
        _direction = ANIM_DIRECTION_RIGHT;
        _animation_type = ANIM_TYPE_STAND;
        _animation_state = 0;
        _animation_timer = 0;
        _animation_inverse = false;
        attack_timer = 0;
        slide_distance = 0;
        invisible = false;
        frozen = false;
        frozen_timer = 0;
    }

    // --- animation ---
    Uint8 direction() const { return _direction; }                       // facing (ANIM_DIRECTION_xxx)
    void set_direction(Uint8 value) { _direction = value; }
    Uint8 *direction_ref() { return &_direction; }                        // for projectiles that follow the owner's facing
    int animation_type() const { return _animation_type; }                // animation type as walk, jump, attack
    void set_animation_type(int value) { _animation_type = value; }
    int animation_state() const { return _animation_state; }              // current animation frame position
    void set_animation_state(int value) { _animation_state = value; }
    unsigned int animation_timer() const { return _animation_timer; }     // used to control animation speed
    void set_animation_timer(unsigned int value) { _animation_timer = value; }
    bool animation_inverse() const { return _animation_inverse; }         // the frames go from the last to the first
    void set_animation_inverse(bool value) { _animation_inverse = value; }

    // --- other state ---
    unsigned int attack_timer; // used to control time of attack
    int slide_distance; // holds the amount of pixels character have slided
    bool invisible;
    bool frozen;
    unsigned int frozen_timer;

private:
    Uint8 _direction;
    int _animation_type;
    int _animation_state;
    unsigned int _animation_timer;
    bool _animation_inverse;
};

#endif // ST_CHARACTERSTATE_H
