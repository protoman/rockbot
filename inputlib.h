#ifndef INPUTLIB_H
#define INPUTLIB_H

#include "defines.h"
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

#include "file/format.h"

/**
 * @brief
 *
 */
class inputLib
{
public:
	inputLib();
    void start_read();
    void stop_read();
    void init_joystick();
    void change_joystick();
    int waitScapeTime(int);
    void clean();
    void clean_all();     /// removes also the attack state
    void wait_keypress(); /// waits intil a key is pressed

    bool pick_key_or_button(CURRENT_FILE_FORMAT::st_game_config &game_config_copy, INPUT_COMMANDS key);

    int get_joysticks_number();

    std::string get_key_name(int key);

    void read_input();

    bool must_run_thread();


protected:


private:
    static int read_input_thread_function(void *ptr);

public:
    Uint8 p1_input[BTN_COUNT];


private:
    SDL_Joystick *joystick1;
    bool _used_keyboard;
    bool _show_btn_debug;
    SDL_Thread *read_thread;
    bool _run_thread;
};

#endif // INPUTLIB_H
