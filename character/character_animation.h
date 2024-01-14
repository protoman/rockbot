#ifndef CHARACTER_ANIMATION_H
#define CHARACTER_ANIMATION_H

#include <iostream>
#include <string>

#include "../defines.h"
#include "../graphicslib.h"
#include "../file/v4/file_game_v4.h"
#include "../game_mediator.h"


class character_animation
{
public:
    // TODO: init frames data //
    character_animation();

    void init(std::string set_name, std::string filename, st_size size, CURRENT_FILE_FORMAT::st_sprite_data data[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT]);
    void show_sprite(st_position dest);
    void inc_sprite();


    // === getters and setters === //
    ANIM_TYPE get_type() const;
    void set_type(const ANIM_TYPE &value);
    ANIM_DIRECTION get_direction() const;
    void set_direction(ANIM_DIRECTION value);

private:
    std::string name;                                                                               // character name, used as key to fetch graphic
    int frame_n;                                                                                    // current frame of animation
    ANIM_DIRECTION direction;                                                                       // direction charater is facing
    ANIM_TYPE type;                                                                                 // type of animation
    CURRENT_FILE_FORMAT::st_sprite_data sprite_data[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT];            // data information for each sprite
    st_size sprite_size;                                                                       // size of each sprite
    long next_frame_timer;
    graphicsLib_gSurface* graphic_ref;
    bool initialized;

};

#endif // CHARACTER_ANIMATION_H
