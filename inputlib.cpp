#include "inputlib.h"

#include <iostream>
#include <string>
#include <algorithm>

extern SDL_Event event;

#include "timerlib.h"
extern timerLib timer;


#include "game.h"
extern game gameControl;

#define DOUBLE_TAP_DELTA 500

extern bool leave_game;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
inputLib::inputLib() : _used_keyboard(false), held_button_count(0), held_button_timer(0), cheat_input_count(0), cheat_input_previous(-1)
{
	for (int i=0; i<BTN_COUNT; i++) {
		p1_input[i] = 0;
        p1_save_input[i] = 0;
	}
    _show_btn_debug = false;

    // Cheat is DASH, L, R, L, R
    cheat_input_sequence.push_back(input_sequence(BTN_UP));
    cheat_input_sequence.push_back(input_sequence(BTN_UP));
    cheat_input_sequence.push_back(input_sequence(BTN_DOWN));
    cheat_input_sequence.push_back(input_sequence(BTN_DOWN));
    cheat_input_sequence.push_back(input_sequence(BTN_LEFT));
    cheat_input_sequence.push_back(input_sequence(BTN_RIGHT));
    cheat_input_sequence.push_back(input_sequence(BTN_LEFT));
    cheat_input_sequence.push_back(input_sequence(BTN_RIGHT));
    cheat_input_sequence.push_back(input_sequence(BTN_DASH));
    cheat_input_sequence.push_back(input_sequence(BTN_SHIELD));
    cheat_input_is_active = false;

#ifdef ANDROID
    SharedData::get_instance()->game_config.get_default_keys(default_keys_codes);
#endif

}

void inputLib::init_joystick()
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::init_joystick - %i joysticks were found.", SDL_NumJoysticks() );
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::init_joystick - The names of the joysticks are:");
        for(int i=0; i < SDL_NumJoysticks(); i++ ) {
            __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::init_joystick - joy[%d][%s]", i, SDL_JoystickName(i));
        }
#endif
    SDL_JoystickEventState(SDL_ENABLE);
    joystick1 = SDL_JoystickOpen(SharedData::get_instance()->game_config.selected_input_device);
}

void inputLib::change_joystick()
{
    SDL_JoystickClose(joystick1);
    joystick1 = SDL_JoystickOpen(SharedData::get_instance()->game_config.selected_input_device);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void inputLib::clean()
{
	for (int i=0; i<BTN_COUNT; i++) {
        if (i != BTN_ATTACK) { // don't clean attack to save charging
            p1_input[i] = 0;
        }
	}
    while (SDL_PollEvent(&event)) {
        SDL_PumpEvents(); // check keyboard events
    }
}

void inputLib::clean_all()
{
    for (int i=0; i<BTN_COUNT; i++) {
        p1_input[i] = 0;
    }
    while (SDL_PollEvent(&event)) {
        SDL_PumpEvents(); // check keyboard events
    }
}

void inputLib::save()
{
    for (int i=0; i<BTN_COUNT; i++) {
        p1_save_input[i] = p1_input[i];

    }
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //

void inputLib::read_input(bool check_input_reset, bool must_check_input_cheat)
{
    _used_keyboard = false;
    if (check_input_reset == false) {
        held_button_count = 0;
        held_button_timer = timer.getTimer();
    }

    // copy last input
    for (int i=0; i<BTN_COUNT; i++) {
        p1_previous_input[i] = p1_input[i];
    }

    while (SDL_PollEvent(&event)) {

    #ifdef SDL2    
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int newWidth = event.window.data1;
            int newHeight = event.window.data2;

            SharedData::get_instance()->scaleX = newWidth / static_cast<float>(RES_W);
            SharedData::get_instance()->scaleY = newHeight / static_cast<float>(RES_H);
            SharedData::get_instance()->scale_window_size.width = newWidth;
            SharedData::get_instance()->scale_window_size.height = newHeight;
            SharedData::get_instance()->changed_window_size = true;
        }
    #else
        if (event.type == SDL_VIDEORESIZE) {
            SharedData::get_instance()->scaleX = event.resize.w / RES_W;
            SharedData::get_instance()->scaleY = event.resize.h / RES_H;
            SharedData::get_instance()->scale_window_size.width = event.resize.w;
            SharedData::get_instance()->scale_window_size.height = event.resize.h;
            SharedData::get_instance()->changed_window_size = true;
        }
    #endif        

        if (_show_btn_debug == false) {
            _show_btn_debug = true;
        }

        if (SharedData::get_instance()->game_config.input_type == INPUT_TYPE_DOUBLE || SharedData::get_instance()->game_config.input_type == INPUT_TYPE_KEYBOARD) {
            int *key_config_tmp = SharedData::get_instance()->game_config.keys_codes;

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
#ifdef ANDROID
                //__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### event.key.which[%d] ###", event.key.which);
                if (event.key.which == 0) {
                    //__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### USING DEFAULT ###");
                    key_config_tmp = default_keys_codes;
                } else {
                    //__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### USING USER-CONFIG ###");
                    key_config_tmp = SharedData::get_instance()->game_config.keys_codes;
                }
#endif
            }
            if (event.type == SDL_KEYDOWN) {
                for (int i=0; i<BTN_COUNT; i++) {
                    if (key_config_tmp[i] != -1 && key_config_tmp[i] == event.key.keysym.sym) {
                        p1_input[i] = 1;
                        _used_keyboard = true;
                        if (i == BTN_JUMP) {
                            p1_input[BTN_JUMP_TIMER] = timer.getTimer();
                        }
                    }
                }
            } else if (event.type == SDL_KEYUP) {
                for (int i=0; i<BTN_COUNT; i++) {
                    if (key_config_tmp[i] != -1 && key_config_tmp[i] == event.key.keysym.sym) {
                        p1_input[i] = 0;
                        _used_keyboard = true;
                        if (i == BTN_JUMP) {
                            p1_input[BTN_JUMP_TIMER] = 0;
                        }
                    }
                }
            }
            SDL_PumpEvents(); // check keyboard events
#if !defined(PLAYSTATION2) && !defined(PSP) && !defined(WII) && !defined(DREAMCAST)
            if (event.type == SDL_QUIT) {
                leave_game = true;
            }
#endif
        }


        if (_used_keyboard == true) { // next commands are all joystick only
            if (must_check_input_cheat == true) {
                check_cheat_input();
            }
            return;
        }


        if (SharedData::get_instance()->game_config.input_type == INPUT_TYPE_DOUBLE || SharedData::get_instance()->game_config.input_type == INPUT_TYPE_JOYSTICK) {
            if (event.type == SDL_JOYBUTTONDOWN) {
                if (check_input_reset) {
                    held_button_count++;
                    held_button_timer = timer.getTimer();
                }

                for (int i=0; i<BTN_COUNT; i++) {
                    if (SharedData::get_instance()->game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_BUTTON && SharedData::get_instance()->game_config.button_codes[i].value != -1 && SharedData::get_instance()->game_config.button_codes[i].value == event.jbutton.button) {
                        p1_input[i] = 1;
                        if (i == BTN_JUMP) {
                            p1_input[BTN_JUMP_TIMER] = timer.getTimer();
                        }
                    }
                }
            } else if (event.type == SDL_JOYBUTTONUP) {


                if (check_input_reset) {
                    held_button_count--;
                    if (held_button_count < 0) {
                        held_button_count = 0;
                    }
                    held_button_timer = timer.getTimer();
                }

                for (int i=0; i<BTN_COUNT; i++) {
                    if (SharedData::get_instance()->game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_BUTTON && SharedData::get_instance()->game_config.button_codes[i].value != -1 && SharedData::get_instance()->game_config.button_codes[i].value == event.jbutton.button) {
                        p1_input[i] = 0;
                        if (i == BTN_JUMP) {
                            p1_input[BTN_JUMP_TIMER] = 0;
                        }
                        break;
                    }
                }
            }
        }


        // check AXIS buttons //
        if (event.type == SDL_JOYAXISMOTION && (SharedData::get_instance()->game_config.input_mode == INPUT_MODE_ANALOG || SharedData::get_instance()->game_config.input_mode == INPUT_MODE_DOUBLE || SharedData::get_instance()->game_config.input_mode == INPUT_MODE_DIGITAL)) {

            // value is the axis (0 is X, 1 is Y)
            // axis_type is positive or negative (left/right or up/down)

            for (int i=0; i<BTN_COUNT; i++) {


                if (SharedData::get_instance()->game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_AXIS && SharedData::get_instance()->game_config.button_codes[i].value != -1 && SharedData::get_instance()->game_config.button_codes[i].value == event.jaxis.axis) {
                    if (SharedData::get_instance()->game_config.button_codes[i].axis_type > 0) {
                        if (event.jaxis.value > JOYVAL) {
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
                    } else if (SharedData::get_instance()->game_config.button_codes[i].axis_type < 0) {
                        if (event.jaxis.value < -JOYVAL) {
                            std::cout << "AXIS[" << i << "].NEGATIVE, value[" << event.jaxis.value << "], JOYVAL[" << JOYVAL << "]" << std::endl;
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
                    } else {
                        std::cout << "AXIS[" << i << "].RESET, value[" << event.jaxis.value << "], JOYVAL[" << JOYVAL << "]" << std::endl;
                        p1_input[i] = 0;
                    }
                }
            }
        }



        if (event.type == SDL_JOYHATMOTION) {
            if (SharedData::get_instance()->game_config.input_mode == INPUT_MODE_DIGITAL || SharedData::get_instance()->game_config.input_mode == INPUT_MODE_DOUBLE) {
                // check HAT input //

                for (int i=0; i<BTN_COUNT; i++) {
                    if (SharedData::get_instance()->game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_AXIS && SharedData::get_instance()->game_config.button_codes[i].value != -1 && SharedData::get_instance()->game_config.button_codes[i].value == event.jaxis.axis) {
                        if (SharedData::get_instance()->game_config.button_codes[i].axis_type > 0 && event.jaxis.value > JOYVAL) {
                            p1_input[i] = 1;
                        } else if (SharedData::get_instance()->game_config.button_codes[i].axis_type < 0 && event.jaxis.value < JOYVAL) {
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
                    }
                }



                // CODES: up - 1, right: 2, down: 4, left: 8
    #ifdef DREAMCAST
    /*
                if (event.jhat.value == 14) { // up

                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 1;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 13) { // right
                    p1_input[BTN_RIGHT] = 1;
                    p1_input[BTN_LEFT] = 0;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 11) { // down
                    p1_input[BTN_DOWN] = 1;
                    p1_input[BTN_UP] = 0;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 7) { // left
                    p1_input[BTN_LEFT] = 1;
                    p1_input[BTN_RIGHT] = 0;
                    _used_keyboard = false;
                }
    */
                if (event.jhat.value == 15 && _used_keyboard == false) {
                    p1_input[BTN_LEFT] = 0;
                    p1_input[BTN_RIGHT] = 0;
                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 0;
                }

    #else
    /*
     *          // hats are not used by default unless the platform qasks for it or user sets button to use //
                if (event.jhat.value == 1 || event.jhat.value == 3 || event.jhat.value == 9) { // up
                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 1;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 2 || event.jhat.value == 3 || event.jhat.value == 6) { // right
                    p1_input[BTN_RIGHT] = 1;
                    p1_input[BTN_LEFT] = 0;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 4 || event.jhat.value == 6 || event.jhat.value == 12) { // down
                    p1_input[BTN_DOWN] = 1;
                    p1_input[BTN_UP] = 0;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 8 || event.jhat.value == 9 || event.jhat.value == 12) { // left
                    p1_input[BTN_LEFT] = 1;
                    p1_input[BTN_RIGHT] = 0;
                    _used_keyboard = false;
                }
                if (event.jhat.value == 0 && _used_keyboard == false) {
                    p1_input[BTN_LEFT] = 0;
                    p1_input[BTN_RIGHT] = 0;
                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 0;
                }
    */

                // prevent double-direction input //
                if (p1_input[BTN_LEFT] == 1) {
                    p1_input[BTN_RIGHT] = 0;
                } else if (p1_input[BTN_UP] == 1) {
                    p1_input[BTN_DOWN] = 0;
                }
            }

#endif
        }
    }
    if (must_check_input_cheat) {
        check_cheat_input();
    }
}

void inputLib::check_cheat_input()
{
    for (int i=0; i<BTN_COUNT; i++) {
        if (p1_input[i] == 1) {
            if (cheat_input_sequence.at(cheat_input_count).key_n != i) {
                std::cout << "RESET cheat input - expected-button[" << (int)cheat_input_sequence.at(cheat_input_count).key_n << "], pressed-button[" << i << "]" << std::endl;
                reset_cheat_input_sequence();
                return;
            } else {
                std::cout << "CONTINUE cheat input" << std::endl;
                cheat_input_count++;
                if (cheat_input_count >= cheat_input_sequence.size()) {
                    std::cout << "FINISHED cheat input" << std::endl;
                    cheat_input_is_active = true;
                }
            }
        }
    }
}

void inputLib::reset_cheat_input_sequence()
{
    cheat_input_count = 0;
    for (unsigned int i=0; i<cheat_input_sequence.size(); i++) {
        cheat_input_sequence.at(i).activated = false;
    }
}

void inputLib::reset_cheat_input()
{
    cheat_input_is_active = false;
    reset_cheat_input_sequence();
}

bool inputLib::is_check_input_reset_command_activated()
{
    if (held_button_count > 1 && held_button_timer+5000 <= timer.getTimer()) {
        return true;
    }
    return false;
}

bool inputLib::is_check_input_cheat_command_activated()
{
    return cheat_input_is_active;
}

void inputLib::clean_event_queue()
{
    while (SDL_PollEvent(&event)) {
        SDL_PumpEvents();
    }
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
int inputLib::wait_scape_time(int wait_period) {
	int now_time = 0;

    timer.delay(50);
	now_time = timer.getTimer();
	wait_period = now_time + wait_period;

	while (now_time < wait_period) {
        if (p1_input[BTN_START] == 1) {
			return 1;
#if !defined(PLAYSTATION2) && !defined(PSP) && !defined(WII) && !defined(DREAMCAST)
        } else if (p1_input[BTN_QUIT] == 1) {
            /*
            dialogs dialogs_obj;
            if (dialogs_obj.show_leave_game_dialog() == true) {
                SDL_Quit();
                exit(0);
            }
            */
#endif
		}
        now_time = timer.getTimer();
        #ifdef PLAYSTATION
			RotateThreadReadyQueue(_MIXER_THREAD_PRIORITY);
		#endif
        timer.delay(1);
        read_input();
	}
    return 0;
}

void inputLib::clean_and_wait_scape_time(int wait_period)
{
    clean();
    wait_scape_time(wait_period);
}

void inputLib::wait_keypress()
{
    bool fim = false;
    while (!fim) {
        read_input();
        if (p1_input[BTN_START] == 1 || p1_input[BTN_JUMP] == 1) {
            fim = true;
        }
        timer.delay(1);
    }
    clean_confirm_button();
}

void inputLib::clean_confirm_button()
{
    for (int i=0; i<BTN_COUNT; i++) {
        if (i == BTN_JUMP) { // don't clean attack to save charging
            p1_input[i] = 0;
        }
    }
}

// returns false for keyboard, true for joysticks
bool inputLib::pick_key_or_button(CURRENT_FILE_FORMAT::st_game_config &game_config_copy, INPUT_COMMANDS key)
{
    clean();
    timer.delay(50);

    while (true) { // keep reading until a key is found
        while (SDL_PollEvent(&event)) {


            if (SharedData::get_instance()->game_config.input_type == INPUT_TYPE_DOUBLE || SharedData::get_instance()->game_config.input_type == INPUT_TYPE_KEYBOARD) {
                if (event.type == SDL_KEYDOWN) {

                    // do not allow user to reassign ESCAPE key
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        return false;
                    }
                    game_config_copy.keys_codes[key] = (int)event.key.keysym.sym;
                    return false;
                }
                SDL_PumpEvents();
            }
            if (SharedData::get_instance()->game_config.input_type == INPUT_TYPE_DOUBLE || SharedData::get_instance()->game_config.input_type == INPUT_TYPE_JOYSTICK) {
                if (event.type == SDL_JOYBUTTONDOWN) {
                    game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_BUTTON;
                    game_config_copy.button_codes[key].value = (int)event.jbutton.button;
                    return true;
                } else if (event.type == SDL_JOYHATMOTION) { //joy-hat events
                    game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_HAT;
                    game_config_copy.button_codes[key].value = event.jhat.value;
                    return true;
                } else if (event.type == SDL_JOYAXISMOTION) { // joy-axis event
                    // axis_type: indicate axis positive or negative
                    // value: indicate what is the axis (1 vertical, 0 horizontal)
                    if (event.jaxis.value < -JOYVAL) {
                        game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_AXIS;
                        game_config_copy.button_codes[key].value = (int)event.jaxis.axis;
                        game_config_copy.button_codes[key].axis_type = -1;
                        return true;
                    } else if (event.jaxis.value > JOYVAL) {
                        game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_AXIS;
                        game_config_copy.button_codes[key].value = (int)event.jaxis.axis;
                        game_config_copy.button_codes[key].axis_type = 1;
                        return true;
                    }
                }
            }


            timer.delay(1);
        }
    }
    return false;
}

int inputLib::get_joysticks_number()
{
    return SDL_NumJoysticks();
}

string inputLib::get_joystick_name(int n)
{
    if (n >= SDL_NumJoysticks()) {
        return std::string("NONE");
    }
    #ifdef SDL2
    SDL_Joystick *joy = SDL_JoystickOpen(n);
    if (joy) {
        std::string name = SDL_JoystickName(joy);
        SDL_JoystickClose(joy);
        return name;
    } else {
        return "NONE";
    }
    #else
    return std::string(SDL_JoystickName(n));
    #endif
}

std::string inputLib::get_key_name(int key)
{
    if (key == -1) {
        return std::string("UNSET");
    }

    std::string res = std::string(SDLL_GetKeyName(key));

    // convert common keys to 3 letter
    if (res.length() > 6) {
        res = res.substr(0, 6);
    }

    // uppercase
    for (std::string::iterator p = res.begin(); res.end() != p; ++p) {
        *p = toupper(*p);
    }

    return res;
}

void inputLib::read_input()
{
    this->read_input(false, false);
}
