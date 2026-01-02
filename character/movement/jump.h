#ifndef JUMP_H
#define JUMP_H

#include "../../defines.h"
#include "shareddata.h"

enum E_JUMP_STATE {
    NOJUMP,
    JUMPUP,
    JUMPDOWN
};

class classjump
{
public:
    classjump();

    void start(bool bigjump_mode, int terrain_type);
    bool is_started();
    void execute(int terrain_type);
    void interrupt(); // used to interrupt a jump-up by releasing the button
    void finish(); // hit ground, must stop all jumping
    float get_speed();
    short get_max_jump_number();
    void set_jump_acceleration(double value);
    void set_jump_limit(int value);
    void set_speed_multiplier(float multi);


private:
    bool started;
    float acceleration;
    float speed;
    E_JUMP_STATE state;
    short max_jump_number;               // how many consecutive jumps used (for double/triple jumping)
    float moved;                        // this is used to hardcode the number of pixels a jump should move in maximum
    bool is_bigjump;
    double JUMP_ACCELERATION;
    float JUMP_LIMIT;
    int start_terrain_type;
    bool start_big_jump;
    double JUMP_INITIAL_SPEED;
    float speed_multiplier = 1.0;
};

#endif // JUMP_H
