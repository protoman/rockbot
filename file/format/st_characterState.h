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
        _animation_restarted = false;
        _on_last_frame = false;
        _on_attack_frame = false;
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

    bool animation_restarted() const { return _animation_restarted; }     // the animation "looped" once (went back to the first frame)
    void set_animation_restarted(bool value) { _animation_restarted = value; }
    bool on_last_frame() const { return _on_last_frame; }                 // reached the end of that animation loop
    void set_on_last_frame(bool value) { _on_last_frame = value; }
    bool on_attack_frame() const { return _on_attack_frame; }             // NPC reached the attack-frame number (it can attack now)
    void set_on_attack_frame(bool value) { _on_attack_frame = value; }

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
    bool _animation_restarted;
    bool _on_last_frame;
    bool _on_attack_frame;
};

#endif // ST_CHARACTERSTATE_H
