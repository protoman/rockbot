#include "key_map.h"

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;

#include "timerlib.h"
extern timerLib timer;


#include "graphic/option_picker.h"

#include "strings_map.h"

#include "defines.h"


extern CURRENT_FILE_FORMAT::st_game_config game_config;

key_map::key_map()
{
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_jump) + std::string(": "));
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_fire) + std::string(": "));
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_dash) + std::string(": "));
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyl) + std::string(": "));
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyr) + std::string(": "));
    _keys_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_start) + std::string(": "));
}


void key_map::draw_screen()
{
    bool finished = false;
    st_position config_text_pos;
    st_position cursor_pos;
    short _pick_pos = 0;

    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    cursor_pos = config_text_pos;

    graphLib.clear_area(config_text_pos.x-1, config_text_pos.y-1, 180,  180, 0, 0, 0);
    input.clean();
    timer.delay(300);

    for (unsigned int i=0; i<_keys_list.size(); i++) {
        std::cout << "_keys_list[i]: " << _keys_list[i] << std::endl;
        graphLib.draw_text(config_text_pos.x, config_text_pos.y + i*CURSOR_SPACING, _keys_list[i].c_str());
        redraw_line(i);
    }
    graphLib.draw_text(config_text_pos.x, config_text_pos.y + _keys_list.size()*CURSOR_SPACING, "RETURN");
    draw_lib.update_screen();

    graphLib.drawCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));

    while (finished == false) {
        input.read_input();
        if (input.p1_input[BTN_START]) {
            if (_pick_pos == (short)_keys_list.size()) {
                finished = true;
            } else {
                graphLib.draw_text(config_text_pos.x, config_text_pos.y + _keys_list.size()*CURSOR_SPACING+CURSOR_SPACING*2, strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_pressnew)); //input code (number)
                draw_lib.update_screen();
                graphLib.clear_area(config_text_pos.x, config_text_pos.y + _keys_list.size()*CURSOR_SPACING+CURSOR_SPACING*2-1, 180,  CURSOR_SPACING+1, 0, 0, 0);
                ///@TODO - key_config[_pick_pos].key_type = new_key.key_type;
                ///@TODO - key_config[_pick_pos].key_number = new_key.key_number;
                redraw_line(_pick_pos);
                draw_lib.update_screen();
            }
        }
        if (input.p1_input[BTN_DOWN]) {
                soundManager.play_sfx(SFX_CURSOR);
                graphLib.eraseCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));
                _pick_pos++;
                if (_pick_pos >= (short)_keys_list.size()+1) {
                    _pick_pos = 0;
                }
                graphLib.drawCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_UP]) {
                soundManager.play_sfx(SFX_CURSOR);
                graphLib.eraseCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));
                _pick_pos--;
                if (_pick_pos < 0) {
                    _pick_pos = _keys_list.size();
                }
                graphLib.drawCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_QUIT]) {
            finished = true;
        }
        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
}

void key_map::redraw_line(short line) const
{
    std::cout << "******* key_map::redraw_line - line: " << line << std::endl;
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    graphLib.clear_area(config_text_pos.x+70-1, config_text_pos.y-1 + line*CURSOR_SPACING, 110,  CURSOR_SPACING+1, 0, 0, 0);

    std::stringstream key_n_ss;
    ///@TODO - key_n_ss << key_config[line].key_number;

    graphLib.draw_text(config_text_pos.x + 140, config_text_pos.y + line*CURSOR_SPACING, key_n_ss.str()); //input code (number)
}




Sint8 key_map::draw_config_input() const
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(100);
    input.clean();
    timer.delay(100);

    graphLib.clear_area(config_text_pos.x, config_text_pos.y, 195,  180, 0, 0, 0);
    std::vector<std::string> options;
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_input_buttons));
    if (game_config.input_mode == INPUT_MODE_DIGITAL) {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_directional) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_directional_digital));
    } else {
        options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_directional) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_directional_analog));
    }

    char temp_char[2]; // lets hope no crazy guy with 100 joysticks connect appear...
    sprintf(temp_char, "%d", (game_config.selected_input_device+1)); // +1 because count start in zero
    std::string selected_joystick_str(temp_char);


    sprintf(temp_char, "%d", input.get_joysticks_number());
    std::string max_joystick_str(temp_char);


    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_input_selected_joystick) + std::string(": ") + selected_joystick_str + std::string("/") + max_joystick_str);

    std::string turbo_mode_string = strings_map::get_instance()->get_ingame_string(strings_ingame_config_input_turbo_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_off);
    if (game_config.turbo_mode == true) {
        turbo_mode_string = strings_map::get_instance()->get_ingame_string(strings_ingame_config_input_turbo_mode) + std::string(": ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_on);
    }

    options.push_back(turbo_mode_string);


    Sint8 selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    return selected_option;
}

string key_map::build_button_config_line(string prefix, string sufix)
{
    int total_space = 24;
    prefix = strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + prefix;
    int spaces_to_add = total_space - prefix.length() - sufix.length();
    std::string spaces = "";
    for (int i=0; i<spaces_to_add; i++) {
        spaces += " ";
    }
    std::string res = prefix + spaces + sufix;
    return res;
}

Sint8 key_map::draw_config_buttons(CURRENT_FILE_FORMAT::st_game_config& game_config_copy)
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    timer.delay(100);
    input.clean();
    timer.delay(100);

    graphLib.clear_area(config_text_pos.x, config_text_pos.y, 180,  180, 0, 0, 0);
    std::vector<std::string> options;

    char btn_codes[BTN_COUNT][30];
// android Pelya's SDL use keyboard events for joystick buttons
#ifdef ANDROID
    sprintf(btn_codes[BTN_JUMP], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_JUMP]).c_str());
    sprintf(btn_codes[BTN_ATTACK], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_ATTACK]).c_str());
    sprintf(btn_codes[BTN_DASH], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_DASH]).c_str());
    sprintf(btn_codes[BTN_L], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_L]).c_str());
    sprintf(btn_codes[BTN_R], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_R]).c_str());
    sprintf(btn_codes[BTN_START], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_START]).c_str());
    sprintf(btn_codes[BTN_UP], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_UP]).c_str());
    sprintf(btn_codes[BTN_DOWN], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_DOWN]).c_str());
    sprintf(btn_codes[BTN_LEFT], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_LEFT]).c_str());
    sprintf(btn_codes[BTN_RIGHT], "[%s]", input.get_key_name(game_config_copy.keys_codes[BTN_RIGHT]).c_str());
#elif PC
    sprintf(btn_codes[BTN_JUMP], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_JUMP]).c_str(), game_config_copy.button_codes[BTN_JUMP]);
    sprintf(btn_codes[BTN_ATTACK], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_ATTACK]).c_str(), game_config_copy.button_codes[BTN_ATTACK]);
    sprintf(btn_codes[BTN_DASH], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_DASH]).c_str(), game_config_copy.button_codes[BTN_DASH]);
    sprintf(btn_codes[BTN_L], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_L]).c_str(), game_config_copy.button_codes[BTN_L]);
    sprintf(btn_codes[BTN_R], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_R]).c_str(), game_config_copy.button_codes[BTN_R]);
    sprintf(btn_codes[BTN_START], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_START]).c_str(), game_config_copy.button_codes[BTN_START]);
    sprintf(btn_codes[BTN_UP], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_UP]).c_str(), game_config_copy.button_codes[BTN_UP]);
    sprintf(btn_codes[BTN_DOWN], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_DOWN]).c_str(), game_config_copy.button_codes[BTN_DOWN]);
    sprintf(btn_codes[BTN_LEFT], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_LEFT]).c_str(), game_config_copy.button_codes[BTN_LEFT]);
    sprintf(btn_codes[BTN_RIGHT], "[%s/%d]", input.get_key_name(game_config_copy.keys_codes[BTN_RIGHT]).c_str(), game_config_copy.button_codes[BTN_RIGHT]);
#else
    sprintf(btn_codes[BTN_JUMP], "[%d]", game_config_copy.button_codes[BTN_JUMP]);
    sprintf(btn_codes[BTN_ATTACK], "[%d]", game_config_copy.button_codes[BTN_ATTACK]);
    sprintf(btn_codes[BTN_DASH], "[%d]", game_config_copy.button_codes[BTN_DASH]);
    sprintf(btn_codes[BTN_L], "[%d]", game_config_copy.button_codes[BTN_L]);
    sprintf(btn_codes[BTN_R], "[%d]", game_config_copy.button_codes[BTN_R]);
    sprintf(btn_codes[BTN_START], "[%d]", game_config_copy.button_codes[BTN_START]);
    sprintf(btn_codes[BTN_UP], "[%d]", game_config_copy.button_codes[BTN_UP]);
    sprintf(btn_codes[BTN_DOWN], "[%d]", game_config_copy.button_codes[BTN_DOWN]);
    sprintf(btn_codes[BTN_LEFT], "[%d]", game_config_copy.button_codes[BTN_LEFT]);
    sprintf(btn_codes[BTN_RIGHT], "[%d]", game_config_copy.button_codes[BTN_RIGHT]);
#endif
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_reset));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_jump), std::string(btn_codes[BTN_JUMP])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_fire), std::string(btn_codes[BTN_ATTACK])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_dash), std::string(btn_codes[BTN_DASH])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyl), std::string(btn_codes[BTN_L])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyr), std::string(btn_codes[BTN_R])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_start), std::string(btn_codes[BTN_START])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_up), std::string(btn_codes[BTN_UP])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_down), std::string(btn_codes[BTN_DOWN])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_left), std::string(btn_codes[BTN_LEFT])));
    options.push_back(build_button_config_line(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_right), std::string(btn_codes[BTN_RIGHT])));

    Sint8 selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    return selected_option;
}





void key_map::config_input()
{
    int selected_option = 0;

    while (selected_option != -1) {
        selected_option = draw_config_input();

        if (selected_option == 0) {
            config_buttons();
        } else if (selected_option == 1) {
            if (game_config.input_mode == INPUT_MODE_DIGITAL) {
                game_config.input_mode = INPUT_MODE_ANALOG;
            } else {
                game_config.input_mode = INPUT_MODE_DIGITAL;
            }
        } else if (selected_option == 2) {
            int sel_joy = int(game_config.selected_input_device);
            std::stringstream key_n_ss;
            key_n_ss << "SELECT JOYSTICK. Number: " << (input.get_joysticks_number()) << ", selected: " + sel_joy << std::endl;
            std::cout << key_n_ss.str() << std::endl;

            // increases joystick number until reaches max-1, then returns to zero
            if (input.get_joysticks_number() > sel_joy+1) {
                game_config.selected_input_device++;
                input.change_joystick();
            } else {
                game_config.selected_input_device = 0;
            }
        } else if (selected_option == 3) {
            game_config.turbo_mode = !game_config.turbo_mode;
        }
    }
}

void key_map::config_buttons()
{
    CURRENT_FILE_FORMAT::st_game_config game_config_copy = game_config;
    int selected_option = 0;
    st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);

    while (selected_option != -1) {
        selected_option = draw_config_buttons(game_config_copy);

        INPUT_COMMANDS selected_key = BTN_JUMP;
        if (selected_option == 0) {
            game_config.set_default_keys();
            return;
        } else if (selected_option != -1) {
            if (selected_option == 1) {
                selected_key = BTN_JUMP;
            } else if (selected_option == 2) {
                selected_key = BTN_ATTACK;
            } else if (selected_option == 3) {
                selected_key = BTN_DASH;
            } else if (selected_option == 4) {
                selected_key = BTN_L;
            } else if (selected_option == 5) {
                selected_key = BTN_R;
            } else if (selected_option == 6) {
                selected_key = BTN_START;
            } else if (selected_option == 7) {
                selected_key = BTN_UP;
            } else if (selected_option == 8) {
                selected_key = BTN_DOWN;
            } else if (selected_option == 9) {
                selected_key = BTN_LEFT;
            } else if (selected_option == 10) {
                selected_key = BTN_RIGHT;
            }

            graphLib.clear_area(menu_pos.x, menu_pos.y, 195,  180, 0, 0, 0);
            graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_pressanykey));
            draw_lib.update_screen();
            input.clean();
            timer.delay(200);
            bool is_joystick = input.pick_key_or_button(game_config_copy, selected_key);

            check_key_duplicates(game_config_copy, selected_key, is_joystick);
        }

        // check if jump/attack/start are set, can't leave if one of those is unset
        if (selected_option == -1) {
            if (is_key_set(BTN_JUMP, game_config_copy) == false) {
                std::string line = strings_map::get_instance()->get_ingame_string(strings_config_keys_unet) + std::string(" JUMP KEY");
                graphLib.draw_text(menu_pos.x, RES_H-40, line);
                timer.delay(20);
                input.clean_all();
                input.wait_keypress();
                selected_option = 0;
            } else if (is_key_set(BTN_ATTACK, game_config_copy) == false) {
                std::string line = strings_map::get_instance()->get_ingame_string(strings_config_keys_unet) + std::string(" ATTACK KEY");
                graphLib.draw_text(menu_pos.x, RES_H-40, line);
                timer.delay(20);
                input.clean_all();
                input.wait_keypress();
                selected_option = 0;
            } else if (is_key_set(BTN_START, game_config_copy) == false) {
                std::string line = strings_map::get_instance()->get_ingame_string(strings_config_keys_unet) + std::string(" START KEY");
                graphLib.draw_text(menu_pos.x, RES_H-40, line);
                timer.delay(20);
                input.clean_all();
                input.wait_keypress();
                selected_option = 0;
            }
        }



    }
    // apply changes to game-config
    apply_key_codes_changes(game_config_copy);

}

bool key_map::is_key_set(INPUT_COMMANDS key, CURRENT_FILE_FORMAT::st_game_config game_config_copy)
{
    bool have_joystick = false;
    if (input.get_joysticks_number() > 0 && game_config_copy.selected_input_device < input.get_joysticks_number()) {
        have_joystick = true;
    }
    if (game_config_copy.keys_codes[key] == -1 && (have_joystick == false || (have_joystick == true && game_config_copy.button_codes[key].value == -1))) {
        return false;
    }
    return true;
}

// if any key is duplicated in the config, reset it to default value
void key_map::check_key_duplicates(CURRENT_FILE_FORMAT::st_game_config& game_config_copy, Uint8 set_key, bool is_joystick)
{
    int default_keys_codes[BTN_COUNT];
    st_input_button_config default_button_codes[BTN_COUNT];

    std::cout << "KEYMAP::CHECK_DUPLICATES - is_joystick: " << is_joystick << ", set_key: " << (int)set_key << std::endl;

    game_config_copy.get_default_keys(default_keys_codes);
    game_config_copy.get_default_buttons(default_button_codes);

    for (int i=0; i<BTN_COUNT; i++) {
        if (is_joystick == false) {
            if (i != set_key && game_config_copy.keys_codes[i] == game_config_copy.keys_codes[set_key]) { // found duplicate
                game_config_copy.keys_codes[i] = -1; // disable key
            }
        } else {
            std::cout << "set_key[" << set_key << "] disabled!" << std::endl;
            if (i != set_key && game_config_copy.button_codes[i].type == game_config_copy.button_codes[set_key].type && game_config_copy.button_codes[i].value == game_config_copy.button_codes[set_key].value && game_config_copy.button_codes[i].axis_type == game_config_copy.button_codes[set_key].axis_type) { // found duplicate
                game_config_copy.button_codes[i].value = -1; // disable key
                game_config_copy.button_codes[i].type = JOYSTICK_INPUT_TYPE_NONE;
                game_config_copy.button_codes[i].axis_type = 0;
            }
        }
    }
}

void key_map::apply_key_codes_changes(CURRENT_FILE_FORMAT::st_game_config game_config_copy)
{
    for (int i=0; i<BTN_COUNT; i++) {
        game_config.keys_codes[i] = game_config_copy.keys_codes[i];
        std::cout << "old_config.btn[" << i << "][" << game_config.button_codes[i].value << "], new_config_btn[" << i << "][" << game_config_copy.button_codes[i].value << "]" << std::endl;
        game_config.button_codes[i].value = game_config_copy.button_codes[i].value;
        game_config.button_codes[i].type = game_config_copy.button_codes[i].type;
        game_config.button_codes[i].axis_type = game_config_copy.button_codes[i].axis_type;
    }
}





