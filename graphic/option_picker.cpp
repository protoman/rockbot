#include "option_picker.h"

extern std::string FILEPATH;
#include "../soundlib.h"
extern soundLib soundManager;

#include "../graphicslib.h"
extern graphicsLib graphLib;

#include "../inputlib.h"
extern inputLib input;

#include "draw.h"
extern draw draw_lib;

#include "../timerlib.h"
extern timerLib timer;


#include "../strings_map.h"

#include "../file/file_io.h"
extern CURRENT_FILE_FORMAT::file_io fio;


option_picker::option_picker(bool draw_border, st_position pos, std::vector<st_menu_option> options, bool show_return) : _pick_pos(0)
{
    _position = pos;
    _draw_border = draw_border;

    if (_draw_border) {
        _position.x += 12 + CURSOR_SPACING;
        _position.y += 12;
    }
    picker_item_list = options;
    _show_return = show_return;
    if (_show_return == true) {

        picker_item_list.insert(picker_item_list.begin(), st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_return)));
    }

    _pick_pos = 0;

    check_input_reset_command = false;
    check_input_cheat_command = false;

    draw();
}

option_picker::option_picker(bool draw_border, st_position pos, std::vector<string> options, bool show_return)
{
    std::vector<st_menu_option> option_list;
    for (int i=0; i<options.size(); i++) {
        option_list.push_back(st_menu_option(options.at(i)));
    }

    _position = pos;
    _draw_border = draw_border;

    if (_draw_border) {
        _position.x += 12 + CURSOR_SPACING;
        _position.y += 12;
    }
    picker_item_list = option_list;
    _show_return = show_return;
    if (_show_return == true) {
        picker_item_list.insert(picker_item_list.begin(), st_menu_option(strings_map::get_instance()->get_ingame_string(strings_config_return)));
    }

    check_input_reset_command = false;
    check_input_cheat_command = false;


    draw();

}

void option_picker::change_option_label(unsigned int n, string label)
{
    if (n >= 0 && n < picker_item_list.size()) {
        picker_item_list.at(n).text = label;
    }
}

Sint8 option_picker::pick(int initial_pick_pos)
{
    bool finished = false;
    input.clean_all();
    timer.delay(100);
    _pick_pos = initial_pick_pos;

    if (picker_item_list.size() == 0) {
        return 0;
    }
    if (_pick_pos < 0 || _pick_pos >= picker_item_list.size()) {
        _pick_pos = 0;
    }

    if (!initialized) {
        graphLib.copy_picker_bg(_position.x-CURSOR_SPACING, _position.y,  CURSOR_SPACING, picker_item_list.size()*CURSOR_SPACING);
        initialized = true;
    }

	graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));

    while (finished == false) {
        input.read_input(check_input_reset_command, check_input_cheat_command);
        if (check_input_reset_command == true && input.is_check_input_reset_command_activated()) {
            show_reset_config_dialog();
        } else if (check_input_cheat_command == true && input.is_check_input_cheat_command_activated()) {
            input.reset_cheat_input();
            return MAIN_MENU_CHEAT_RETURN;
        }

        if (input.p1_input[BTN_START] || input.p1_input[BTN_JUMP]) {
            if (picker_item_list.at(_pick_pos).disabled == true) {
                soundManager.play_sfx(SFX_NPC_HIT);
            } else {
                graphLib.eraseCursorWithBG(_pick_pos, st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
                draw_lib.update_screen();
                if (_show_return == true) {
                    _pick_pos--;
                }
                return _pick_pos;
            }
        }
        if (input.p1_input[BTN_DOWN]) {
            soundManager.play_sfx(SFX_CURSOR);
            graphLib.eraseCursorWithBG(_pick_pos, st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            _pick_pos++;
            if (_pick_pos >= picker_item_list.size()) {
                _pick_pos = 0;
            }
            graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_UP]) {
            soundManager.play_sfx(SFX_CURSOR);
            graphLib.eraseCursorWithBG(_pick_pos, st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            if (_pick_pos == 0) {
                _pick_pos = picker_item_list.size()-1;
            } else {
                _pick_pos--;
            }
            graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_QUIT] || input.p1_input[BTN_ATTACK]) {
            graphLib.eraseCursorWithBG(_pick_pos, st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            return -1;
        }

        if (_pick_pos < 0 || _pick_pos >= picker_item_list.size()) {
            _pick_pos = 0;
        }

        input.clean();
        timer.delay(10);
        draw_lib.update_screen();
    }
	graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
    timer.delay(10);
    draw_lib.update_screen();

    if (_show_return == true) {
        _pick_pos--;
    }
    // erase cursor
    return _pick_pos;
}

void option_picker::enable_check_input_reset_command()
{
    check_input_reset_command = true;
}

void option_picker::enable_check_input_cheat_command()
{
    check_input_cheat_command = true;
}

void option_picker::show_reset_config_dialog()
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &screen_copy);
    wait_release_reset_config();
    graphLib.draw_text(20, 20, "NOW PRESS TWO BUTTONS TOGETHER"); // those strings must remain in english, as the language could be the reason for the reset
    graphLib.draw_text(20, 32, "AND HOLD IT FOR 5 SECONDS");
    graphLib.draw_text(20, 44, "TO RESET CONFIGURATION.");
    graphLib.draw_text(20, 60, "OR WAIT 10 SECONDS TO");
    graphLib.draw_text(20, 72, "RETURN.");
    graphLib.updateScreen();
    long init_timer = timer.getTimer();
    while (input.is_check_input_reset_command_activated() == false) {
        input.read_input(true, false);
        timer.delay(1);
        if (init_timer+10000 < timer.getTimer()) {
            break;
        } else if (input.is_check_input_reset_command_activated() == true) {
            // reset the configuration file //
            SharedData::get_instance()->game_config.reset();
            fio.save_config(SharedData::get_instance()->game_config);
            break;
        }
    }

    wait_release_reset_config();
    graphLib.copyArea(st_position(0, 0), &screen_copy, &graphLib.gameScreen);
    graphLib.updateScreen();
}

void option_picker::wait_release_reset_config()
{
    graphLib.draw_text(20, 20, "PLEASE RELEASE BUTTONS"); // those strings must remain in english, as the language could be the reason for the reset
    graphLib.updateScreen();
    while (input.is_check_input_reset_command_activated() == true) {
        input.read_input(true, false);
        timer.delay(1);
    }
}

void option_picker::add_option_item(st_menu_option item)
{
    picker_item_list.push_back(item);
}



void option_picker::draw()
{

    text_max_len = 0;
    for (unsigned int i=0; i<picker_item_list.size(); i++) {
        std::string line = picker_item_list.at(i).text;
        int line_len = line.length();
        if (line_len > text_max_len) {
            text_max_len = line_len;
        }
    }

    for (unsigned int i=0; i<picker_item_list.size(); i++) {
        st_menu_option menu_item = picker_item_list.at(i);

        if (menu_item.disabled == true) {
            graphLib.draw_text(_position.x, _position.y + (12 * i), menu_item.text, st_color(100, 100, 100));
        } else {
            graphLib.draw_text(_position.x, _position.y + (12 * i), menu_item.text);
        }
	}
}

