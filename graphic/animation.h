#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include "file/format/st_common.h"
#include "graphicslib.h"

/**
 * @brief enum describing animation (like hit or explosions) positioning type (dynamic follows the character)
 */
enum ANIMATION_TYPES {
    ANIMATION_STATIC,
    ANIMATION_DYNAMIC,
    ANIMATION_COUNT
};

/**
 * @brief animation class is used in small and temporary animations shown in map or characters such as explosions or hit-polygon
 */
class animation
{
public:
    animation(ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize, st_float_position* map_scroll);
    ~animation();

    st_float_position get_position() const;


    /**
     * @brief indicates if the animation is complete, so map can remove it
     * @return bool
     */
    bool finished() const;
    /**
     * @brief execute a animation. advance frame when delay-time is reached and set is_finished to true when the frames end and reaches the set number of repetitions.
     */
    void execute();

    void set_initial_delay(int delay);

private:
    const st_float_position* _ref_pos; /**< holds a pointer to the position it must follow in dynamic animation type */
    ANIMATION_TYPES _pos_type;
    st_float_position _static_pos;
    st_position _adjust_pos;
    graphicsLib_gSurface* _surface; /**< holds graphic surface */
    Uint8 _repeat_times; /**< number of times the whole frames will be shown */
    Uint8 _executed_times; /**< holds the number of times the whole frames were shown. if greater or equals to _repeat_times, set is_finished to true */
    int _frame_time;
    bool _finished;
    Uint8 _frames_number;
    Uint8 _direction;
    st_size _framesize;

    Uint8 _current_frame;
    int _current_frame_timer;
    Uint8 _max_repeat;
    Uint8 _repeated_times;
    st_float_position* _map_scroll;

    int initial_timer;
};




#endif // ANIMATION_H
