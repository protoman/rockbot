#include "inputlib.h"

#include<iostream>
#include <SDL/SDL_joystick.h>

extern SDL_Event event;

#include "timerlib.h"
extern timerLib timer;


#include "game.h"
extern game gameControl;

extern CURRENT_FILE_FORMAT::st_game_config game_config;

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
    game_config.get_default_keys(default_keys_codes);
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
#else
    printf("%i joysticks were found.\n\n", SDL_NumJoysticks());
    printf("The names of the joysticks are:\n");
    for(int i=0; i < SDL_NumJoysticks(); i++ ) {
        printf("[%d][%s]\n", i, SDL_JoystickName(i));
    }
#endif
    SDL_JoystickEventState(SDL_ENABLE);
    joystick1 = SDL_JoystickOpen(game_config.selected_input_device);
}

void inputLib::change_joystick()
{
    SDL_JoystickClose(joystick1);
    joystick1 = SDL_JoystickOpen(game_config.selected_input_device);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void inputLib::clean()
{
    //std::cout << "INPUT::CLEAN CALL" << std::endl;
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

void inputLib::read_input(bool check_input_reset, bool check_input_cheat)
{
    _used_keyboard = false;
    if (check_input_reset == false) {
        //std::cout << "held_button_count::RESET" << std::endl;
        held_button_count = 0;
        held_button_timer = timer.getTimer();
    }

    // copy last input
    for (int i=0; i<BTN_COUNT; i++) {
        p1_previous_input[i] = p1_input[i];
    }

    while (SDL_PollEvent(&event)) {


        //std::cout << ">>> INPUT::read_input[EVENT] <<<" << std::endl;

        if (_show_btn_debug == false) {
            _show_btn_debug = true;
        }

        if (game_config.input_type == INPUT_TYPE_DOUBLE || game_config.input_type == INPUT_TYPE_KEYBOARD) {
            int *key_config_tmp = game_config.keys_codes;
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
#ifdef ANDROID
                __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### event.key.which[%d] ###", event.key.which);
                if (event.key.which == 0) {
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### USING DEFAULT ###");
                    key_config_tmp = default_keys_codes;
                } else {
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### USING USER-CONFIG ###");
                    key_config_tmp = game_config.keys_codes;
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
                        //if (i == BTN_ATTACK) std::cout << "INPUT::readInput::KEYUP::ATTACK" << std::endl;
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
                std::cout << "LEAVE #1" << std::endl;
                leave_game = true;
            }
#endif
        }


        if (_used_keyboard == true) { // next commands are all joystick only
            if (check_input_cheat) {
                check_cheat_input();
            }
            return;
        }


        if (game_config.input_type == INPUT_TYPE_DOUBLE || game_config.input_type == INPUT_TYPE_JOYSTICK) {
            if (event.type == SDL_JOYBUTTONDOWN) {


                if (check_input_reset) {
                    held_button_count++;
                    std::cout << "held_button_count++ [" << held_button_count << "]" << std::endl;
                    held_button_timer = timer.getTimer();
                }

                //std::cout << "#1 INPUT::readInput - joystick button[" << (int)event.jbutton.button << "] pressed." << std::endl;
                for (int i=0; i<BTN_COUNT; i++) {
                    //std::cout << "#1 INPUT::readInput - button_codes[" << i << "]: " << game_config.button_codes[i] << std::endl;
                    if (game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_BUTTON && game_config.button_codes[i].value != -1 && game_config.button_codes[i].value == event.jbutton.button) {
                        //std::cout << "#1 INPUT::readInput - FOUND ACTION for i: " << i << std::endl;
                        p1_input[i] = 1;
                        if (i == BTN_JUMP) {
                            p1_input[BTN_JUMP_TIMER] = timer.getTimer();
                        }
                        //break;
                    }
                }
            } else if (event.type == SDL_JOYBUTTONUP) {


                if (check_input_reset) {
                    held_button_count--;
                    if (held_button_count < 0) {
                        held_button_count = 0;
                    }
                    std::cout << "held_button_count-- [" << held_button_count << "]" << std::endl;
                    held_button_timer = timer.getTimer();
                }

                //std::cout << "#2 INPUT::readInput - joystick button[" << event.jbutton.button << "] released" << std::endl;
                for (int i=0; i<BTN_COUNT; i++) {
                    if (game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_BUTTON && game_config.button_codes[i].value != -1 && game_config.button_codes[i].value == event.jbutton.button) {
                        //if (i == BTN_ATTACK) std::cout << "INPUT::readInput::BUTTONUP::ATTACK" << std::endl;
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
        if (event.type == SDL_JOYAXISMOTION && (game_config.input_mode == INPUT_MODE_ANALOG || game_config.input_mode == INPUT_MODE_DOUBLE || game_config.input_mode == INPUT_MODE_DIGITAL)) {

            //std::cout << "INPUT::read_input - event.SDL_JOYAXISMOTION, axis[" << (int)event.jaxis.axis << "], j.value[" << event.jaxis.value << "]" << std::endl;


            // value is the axis (0 is X, 1 is Y)
            // axis_type is positive or negative (left/right or up/down)

            for (int i=0; i<BTN_COUNT; i++) {

                if (game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_AXIS) {
                    //std::cout << "AXIS input config[" << i << "], value[" << game_config.button_codes[i].value << "]" << std::endl;
                    if (game_config.button_codes[i].value == event.jaxis.axis) {
                        //std::cout << "FOUND AXIS CONFIG, CONFIG.AXIS_TYPE[" << game_config.button_codes[i].axis_type << "]" << std::endl;



                        if (event.jaxis.value > JOYVAL) {
                            //std::cout << "GREATER THAN JOYVAL" << std::endl;
                        } else if (event.jaxis.value < -JOYVAL) {
                            //std::cout << "SMALLER THAN JOYVAL" << std::endl;
                        } else {
                            //std::cout << "NOT REACHED JOYVAL" << std::endl;
                        }
                    }
                }

                if (game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_AXIS && game_config.button_codes[i].value != -1 && game_config.button_codes[i].value == event.jaxis.axis) {
                    if (game_config.button_codes[i].axis_type > 0) {
                        if (event.jaxis.value > JOYVAL) {
                            //std::cout << "AXIS[" << i << "].POSITIVE" << std::endl;
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
                    } else if (game_config.button_codes[i].axis_type < 0) {
                        if (event.jaxis.value < -JOYVAL) {
                            //std::cout << "AXIS[" << i << "].NEGATIVE[" << event.jaxis.value << "][" << JOYVAL << "]" << std::endl;
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
                    } else {
                        p1_input[i] = 0;
                    }
                }
            }
        }

        /*
        if ((game_config.input_mode == INPUT_MODE_ANALOG || game_config.input_mode == INPUT_MODE_DOUBLE) && event.type == SDL_JOYAXISMOTION) {
            if (event.jaxis.axis == 0) {
                if (event.jaxis.value < -JOYVAL) {
                    p1_input[BTN_RIGHT] = 0;
                    p1_input[BTN_LEFT] = 1;
                    _used_keyboard = false;
                } else if (event.jaxis.value > JOYVAL) {
                    p1_input[BTN_RIGHT] = 1;
                    p1_input[BTN_LEFT] = 0;
                    _used_keyboard = false;
                } else if ((event.jaxis.value < JOYVAL || event.jaxis.value > -JOYVAL) && _used_keyboard == false) {
                    p1_input[BTN_RIGHT] = 0;
                    p1_input[BTN_LEFT] = 0;
                }
            }
            if (event.jaxis.axis == 1) {
                if (event.jaxis.value < -JOYVAL) {
                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 1;
                    _used_keyboard = false;
                } else if (event.jaxis.value > JOYVAL) {
                    p1_input[BTN_DOWN] = 1;
                    p1_input[BTN_UP] = 0;
                    _used_keyboard = false;
                } else if ((event.jaxis.value < JOYVAL || event.jaxis.value > -JOYVAL) && _used_keyboard == false) {
                    p1_input[BTN_DOWN] = 0;
                    p1_input[BTN_UP] = 0;
                }
            }
        }
        */


        if ((game_config.input_mode == INPUT_MODE_DIGITAL || game_config.input_mode == INPUT_MODE_DOUBLE) && event.type == SDL_JOYHATMOTION) {
            // check HAT input //
            if (event.type == SDL_JOYAXISMOTION && (game_config.input_mode == INPUT_MODE_ANALOG || game_config.input_mode == INPUT_MODE_DOUBLE)) {
                for (int i=0; i<BTN_COUNT; i++) {
                    if (game_config.button_codes[i].type == JOYSTICK_INPUT_TYPE_AXIS && game_config.button_codes[i].value != -1 && game_config.button_codes[i].value == event.jaxis.axis) {
                        if (game_config.button_codes[i].axis_type > 0 && event.jaxis.value > JOYVAL) {
                            p1_input[i] = 1;
                        } else if (game_config.button_codes[i].axis_type < 0 && event.jaxis.value < JOYVAL) {
                            p1_input[i] = 1;
                        } else {
                            p1_input[i] = 0;
                        }
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

#endif
        }
    }
    if (check_input_cheat) {
        check_cheat_input();
    }
}

void inputLib::check_cheat_input()
{
    //std::cout << "LEFT[" << (int)p1_input[BTN_LEFT] << "], DASH[" << (int)p1_input[BTN_DASH] << "], R[" << (int)p1_input[BTN_R] << "]" << std::endl;
    for (int i=0; i<BTN_COUNT; i++) {
        if (p1_input[i] == 1) {
            if (cheat_input_sequence.at(cheat_input_count).key_n != i) {
                //std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>> CHEAT RESET" << std::endl;
                reset_cheat_input_sequence();
                return;
            } else {
                cheat_input_count++;
                //std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>> CHEAT INC[" << cheat_input_count << "]" << std::endl;
                if (cheat_input_count >= cheat_input_sequence.size()) {
                    //std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>> CHEAT ACTIVATED!!!" << std::endl;
                    cheat_input_is_active = true;
                }
            }
        }
    }
}

void inputLib::reset_cheat_input_sequence()
{
    cheat_input_count = 0;
    for (int i=0; i<cheat_input_sequence.size(); i++) {
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

    //std::cout << "held_button_count-- [" << held_button_count << "]" << std::endl;

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

int inputLib::clean_and_wait_scape_time(int wait_period)
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
    //std::cout << "INPUT::CLEAN CALL" << std::endl;
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

    #ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button game_config.input_type[%d]", (int)game_config.input_type);
    #else
    std::cout << "### INPUT::pick_key_or_button game_config.input_type[" << (int)game_config.input_type << "]" << std::endl;
    #endif



    while (true) { // keep reading until a key is found
        while (SDL_PollEvent(&event)) {


#ifdef ANDROID
__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button - event.type[%d] ###", (int)event.type);
#else
std::cout << "### INPUT::pick_key_or_button[SDL_KEYDOWN][" << (int)event.key.keysym.sym << "]" << std::endl;
#endif

            if (game_config.input_type == INPUT_TYPE_DOUBLE || game_config.input_type == INPUT_TYPE_KEYBOARD) {
                if (event.type == SDL_KEYDOWN) {


#ifdef ANDROID
__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button - event.key.which[%d] ###", (int)event.key.which);
#else
std::cout << "### INPUT::pick_key_or_button[SDL_KEYDOWN][" << (int)event.key.which << "]" << std::endl;
#endif

                    // do not allow user to reassign ESCAPE key
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        #ifdef ANDROID
                        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### ERROR: Can't reassign ESCAPE key ###");
                        #else
                        std::cout << "ERROR: Can't reassign ESCAPE key." << std::endl;
                        #endif
                        return false;
                    }
                    game_config_copy.keys_codes[key] = (int)event.key.keysym.sym;
                    #ifdef ANDROID
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button[SDL_KEYDOWN][%d] ###", (int)event.key.keysym.sym);
                    #else
                    std::cout << "### INPUT::pick_key_or_button[SDL_KEYDOWN][" << (int)event.key.keysym.sym << "]" << std::endl;
                    #endif
                    return false;
                }
                SDL_PumpEvents();
            }
            if (game_config.input_type == INPUT_TYPE_DOUBLE || game_config.input_type == INPUT_TYPE_JOYSTICK) {
                if (event.type == SDL_JOYBUTTONDOWN) {
                    //std::cout << "INPUT::pick_key_or_button - key[" << (int)key << "], current.BTN[" << (int)game_config_copy.button_codes[key] << "], SET JOYBTN TO[" << (int)event.jbutton.button << "]" << std::endl;
                    game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_BUTTON;
                    game_config_copy.button_codes[key].value = (int)event.jbutton.button;
                    #ifdef ANDROID
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button[SDL_JOYBUTTONDOWN][%d] ###", (int)event.jbutton.button);
                    #else
                    std::cout << "### INPUT::pick_key_or_button[SDL_JOYBUTTONDOWN][" << (int)event.jbutton.button << "]" << std::endl;
                    #endif
                    return true;
                } else if (event.type == SDL_JOYHATMOTION) { //joy-hat events
                    game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_HAT;
                    game_config_copy.button_codes[key].value = event.jhat.value;
                    #ifdef ANDROID
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button[JOYSTICK_INPUT_TYPE_HAT][%d] ###", event.jhat.value);
                    #else
                    std::cout << "### ### INPUT::pick_key_or_button[JOYSTICK_INPUT_TYPE_HAT][" << event.jhat.value << "] ###" << std::endl;
                    #endif
                    return true;
                } else if (event.type == SDL_JOYAXISMOTION) { // joy-axis event
                    // axis_type: indicate axis positive or negative
                    // value: indicate what is the axis (1 vertical, 0 horizontal)
                    if (event.jaxis.value < -JOYVAL) {
                        std::cout << "KEY[" << (int)key << "]" << std::endl;
                        game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_AXIS;
                        game_config_copy.button_codes[key].value = (int)event.jaxis.axis;
                        game_config_copy.button_codes[key].axis_type = -1;
                        #ifdef ANDROID
                        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button[SDL_JOYAXISMOTION-][%d] ###", (int)event.jaxis.axis);
                        #else
                        std::cout << "### INPUT::pick_key_or_button[SDL_JOYAXISMOTION-][" << (int)event.jaxis.axis << "]" << std::endl;
                        #endif
                        return true;
                    } else if (event.jaxis.value > JOYVAL) {
                        std::cout << "KEY[" << (int)key << "]" << std::endl;
                        game_config_copy.button_codes[key].type = JOYSTICK_INPUT_TYPE_AXIS;
                        game_config_copy.button_codes[key].value = (int)event.jaxis.axis;
                        game_config_copy.button_codes[key].axis_type = 1;
                        #ifdef ANDROID
                        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button[JOYSTICK_INPUT_TYPE_AXIS+][%d] ###", event.jhat.value);
                        #else
                        std::cout << "### INPUT::pick_key_or_button[SDL_JOYAXISMOTION+][" << (int)event.jaxis.axis << "]" << std::endl;
                        #endif
                        return true;
                    }
                } else {
                    #ifdef ANDROID
                    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button unknown event_type[%d] ###", (int)event.type);
                    #else
                    //std::cout << "### INPUT::pick_key_or_button unknown event_type[[" << (int)event.type << "]" << std::endl;
                    #endif
                }
            } else {
                #ifdef ANDROID
                __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### INPUT::pick_key_or_button ignoring event_type[%d] ###", (int)event.type);
                #else
                std::cout << "### INPUT::pick_key_or_button ignoring event_type[[" << (int)event.type << "]" << std::endl;
                #endif
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
    return std::string(SDL_JoystickName(n));
}

std::string inputLib::get_key_name(int key)
{
    SDLKey keysym = (SDLKey)key;

    if (key == -1) {
        return std::string("UNSET");
    }

    std::string res = SDL_GetKeyName(keysym);

    // convert common keys to 3 letter
    if (res.length() > 6) {
        res = res.substr(0, 6);
    }

    // uppercase
    for (std::string::iterator p = res.begin(); res.end() != p; ++p) {
        *p = toupper(*p);
    }

    //std::cout << "get_key_name[" << (int)key << "][" << keysym << "][" << res << "]" << std::endl;

    return res;
}

void inputLib::read_input()
{
    this->read_input(false, false);
}
