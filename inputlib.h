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
    int waitScapeTime(int);
    void clean();
    void clean_all();     /// removes also the attack state
    void save();
    void wait_keypress(); /// waits intil a key is pressed

    bool pick_key_or_button(CURRENT_FILE_FORMAT::st_game_config &game_config_copy, INPUT_COMMANDS key);

    int get_joysticks_number();

    std::string get_key_name(int key);

    void read_input();
    void read_input(bool check_input_reset);
    bool is_check_input_reset_command_activated();

protected:


public:
    Uint8 p1_input[BTN_COUNT];
    Uint8 p1_save_input[BTN_COUNT];


private:
    SDL_Joystick *joystick1;
    bool _used_keyboard;
    bool _show_btn_debug;
    int held_button_count;
    long held_button_timer;
};

#endif // INPUTLIB_H
