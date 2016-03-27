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
    void init_joystick();
    void change_joystick();
    void readInput();
    void waitTime(int wait_period) const;
    int waitScapeTime(int);
    void clean();
    void wait_keypress(); /// waits intil a key is pressed

    bool pick_key_or_button(CURRENT_FILE_FORMAT::st_game_config &game_config_copy, INPUT_COMMANDS key);

    int get_joysticks_number();



protected:


private:

public:
    Uint8 p1_input[BTN_COUNT];


private:
    SDL_Joystick *joystick1;
    bool _used_keyboard;
    bool _show_btn_debug;
    int turbo_timer;
    bool turbo_state;
};

#endif // INPUTLIB_H
