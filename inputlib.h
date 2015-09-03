#ifndef INPUTLIB_H
#define INPUTLIB_H

#include "defines.h"
#include "SDL/SDL.h"

#include "file/format.h"

/**
 * @brief
 *
 */
class inputLib
{
public:
	inputLib();
    /**
     * @brief
     *
     */
    /**
     * @brief
     *
     */
    void init_joystick();
    /**
     * @brief
     *
     */
    void readInput();
    /**
     * @brief
     *
     * @param wait_period
     */
    void waitTime(int wait_period) const;
    /**
     * @brief
     *
     * @param int
     * @return int
     */
    int waitScapeTime(int);
    /**
     * @brief
     *
     */
    void clean();
    /**
     * @brief
     *
     */
    void wait_keypress(); /// waits intil a key is pressed

    bool pick_key_or_button(format_v_3_0_1::st_game_config &game_config_copy, INPUT_COMMANDS key);



protected:


private:

public:
    int p1_input[BTN_COUNT]; /**< TODO */
    int p2_input[BTN_COUNT]; /**< TODO */


private:
    SDL_Joystick *joystick1, *joystick2; /**< TODO */
    bool _used_keyboard; /**< TODO */
    bool _show_btn_debug;
};

#endif // INPUTLIB_H
