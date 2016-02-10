#include "key_map.h"

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;

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
    input.waitTime(300);

    for (unsigned int i=0; i<_keys_list.size(); i++) {
        graphLib.draw_text(config_text_pos.x, config_text_pos.y + i*CURSOR_SPACING, _keys_list[i].c_str());
        redraw_line(i);
    }
    graphLib.draw_text(config_text_pos.x, config_text_pos.y + _keys_list.size()*CURSOR_SPACING, "RETURN");
    draw_lib.update_screen();

    graphLib.drawCursor(st_position(cursor_pos.x-CURSOR_SPACING, cursor_pos.y+(_pick_pos*CURSOR_SPACING)));

    while (finished == false) {
        input.readInput();
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
        input.waitTime(10);
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
    input.waitTime(100);
    input.clean();
    input.waitTime(100);

    graphLib.clear_area(config_text_pos.x, config_text_pos.y, 180,  180, 0, 0, 0);
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


    Sint8 selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    return selected_option;
}

Sint8 key_map::draw_config_buttons() const
{
    st_position config_text_pos;
    config_text_pos.x = graphLib.get_config_menu_pos().x + 74;
    config_text_pos.y = graphLib.get_config_menu_pos().y + 40;
    input.clean();
    input.waitTime(100);
    input.clean();
    input.waitTime(100);

    graphLib.clear_area(config_text_pos.x, config_text_pos.y, 180,  180, 0, 0, 0);
    std::vector<std::string> options;
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_reset));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_jump));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_fire));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_dash));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyl));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_keyr));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_start));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_up));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_down));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_left));
    options.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_config_set) + std::string(" ") + strings_map::get_instance()->get_ingame_string(strings_ingame_config_key_right));

    Sint8 selected_option = 0;
    option_picker main_config_picker(false, config_text_pos, options, true);
    selected_option = main_config_picker.pick();
    return selected_option;
}





void key_map::config_input()
{
    CURRENT_FILE_FORMAT::st_game_config game_config_copy = game_config;
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
        }
    }
    // apply changes to game-config
    apply_key_codes_changes(game_config_copy);
}

void key_map::config_buttons()
{
    CURRENT_FILE_FORMAT::st_game_config game_config_copy = game_config;
    int selected_option = 0;
    while (selected_option != -1) {
        selected_option = draw_config_buttons();

        INPUT_COMMANDS selected_key = BTN_JUMP;
        if (selected_option == 0) {
            game_config.set_default_keys();
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
            } else if (selected_option == 8) {
                selected_key = BTN_UP;
            } else if (selected_option == 9) {
                selected_key = BTN_DOWN;
            } else if (selected_option == 10) {
                selected_key = BTN_LEFT;
            } else if (selected_option == 11) {
                selected_key = BTN_RIGHT;
            }

            st_position menu_pos(graphLib.get_config_menu_pos().x + 74, graphLib.get_config_menu_pos().y + 40);
            graphLib.clear_area(menu_pos.x, menu_pos.y, 180,  180, 0, 0, 0);
            graphLib.draw_text(menu_pos.x, menu_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_pressanykey));
            input.clean();
            input.waitTime(20);
            bool is_joystick = input.pick_key_or_button(game_config_copy, selected_key);
            check_key_duplicates(game_config_copy, selected_key, is_joystick);
        }
    }
    // apply changes to game-config
    apply_key_codes_changes(game_config_copy);

}

// if any key is duplicated in the config, reset it to default value
void key_map::check_key_duplicates(CURRENT_FILE_FORMAT::st_game_config game_config_copy, Uint8 set_key, bool is_joystick)
{
    int default_keys_codes[BTN_COUNT];
    int default_button_codes[BTN_COUNT];

    game_config_copy.get_default_keys(default_keys_codes);
    game_config_copy.get_default_buttons(default_button_codes);

    for (int i=0; i<BTN_COUNT; i++) {
        if (is_joystick == false) {
            if (i != set_key && game_config_copy.keys_codes[i] == game_config_copy.keys_codes[set_key]) { // found duplicate
                game_config_copy.keys_codes[i] = default_keys_codes[i]; // reset duplicate to default value, avoiding setting two buttons with same key
            }
        } else {
            if (i != set_key && game_config_copy.button_codes[i] == game_config_copy.button_codes[set_key]) { // found duplicate
                game_config_copy.button_codes[i] = default_button_codes[i]; // reset duplicate to default value, avoiding setting two buttons with same key
            }
        }
    }
}

void key_map::apply_key_codes_changes(CURRENT_FILE_FORMAT::st_game_config game_config_copy)
{
    for (int i=0; i<BTN_COUNT; i++) {
        game_config.keys_codes[i] = game_config_copy.keys_codes[i];
        game_config.button_codes[i] = game_config_copy.button_codes[i];
    }
}





