#include "option_picker.h"

extern std::string FILEPATH;
#include "soundlib.h"
extern soundLib soundManager;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "graphic/draw.h"
extern draw draw_lib;

#include "timerlib.h"
extern timerLib timer;


#include "strings_map.h"

extern CURRENT_FILE_FORMAT::st_game_config game_config;
#include "file/file_io.h"
extern CURRENT_FILE_FORMAT::file_io fio;


option_picker::option_picker(bool draw_border, st_position pos, std::vector<st_menu_option> options, bool show_return) : _pick_pos(0)
{

    //std::cout << "#2 option_picker -  pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;

    _position = pos;
    _draw_border = draw_border;

    if (_draw_border) {
        _position.x += 12 + CURSOR_SPACING;
        _position.y += 12;
    }
    _items = options;
    _show_return = show_return;
    if (_show_return == true) {
        _items.insert(_items.begin(), st_menu_option("RETURN"));
    }

    _pick_pos = 0;

    //std::cout << "#3 option_picker -  pos.x: " << _position.x << ", pos.y: " << _position.y << std::endl;

    check_input_reset_command = false;

    draw();
}

option_picker::option_picker(bool draw_border, st_position pos, std::vector<string> options, bool show_return)
{
    //std::cout << "#1 option_picker -  pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;
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
    _items = option_list;
    _show_return = show_return;
    if (_show_return == true) {
        _items.insert(_items.begin(), st_menu_option("RETURN"));
    }

    check_input_reset_command = false;


    draw();

}

void option_picker::change_option_label(int n, string label)
{
    if (n >= 0 && n < _items.size()) {
        _items.at(n).text = label;
    }
}

Sint8 option_picker::pick(int initial_pick_pos)
{
    bool finished = false;
    input.clean();
    timer.delay(100);
    _pick_pos = initial_pick_pos;

	graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));

    while (finished == false) {
        input.read_input(check_input_reset_command);
        if (check_input_reset_command == true && input.is_check_input_reset_command_activated()) {
            std::cout << "RESET ACTIVE!!" << std::endl;
            show_reset_config_dialog();
        }

        if (input.p1_input[BTN_START] || input.p1_input[BTN_JUMP]) {
            if (_items.at(_pick_pos).disabled == true) {
                soundManager.play_sfx(SFX_NPC_HIT);
            } else {
                //std::cout << "option_picker::option_picker::END #1" << std::endl;
                graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
                draw_lib.update_screen();
                if (_show_return == true) {
                    _pick_pos--;
                }
                return _pick_pos;
            }
        }
        if (input.p1_input[BTN_DOWN]) {
            //std::cout << "option_picker::option_picker::DOWN" << std::endl;
            soundManager.play_sfx(SFX_CURSOR);
            graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            _pick_pos++;
            if (_pick_pos >= (short)_items.size()) {
                _pick_pos = 0;
            }
            graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_UP]) {
            //std::cout << "option_picker::option_picker::UP" << std::endl;
            soundManager.play_sfx(SFX_CURSOR);
            graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            if (_pick_pos == 0) {
                _pick_pos = _items.size()-1;
            } else {
                _pick_pos--;
            }
            graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_QUIT] || input.p1_input[BTN_ATTACK]) {
            graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            //std::cout << "option_picker::option_picker::END #2" << std::endl;
            return -1;
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

void option_picker::show_reset_config_dialog()
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &screen_copy);
    wait_release_reset_config();
    graphLib.clear_area(0, 0, RES_W, RES_H, 40, 0, 0);
    graphLib.draw_text(20, 20, "NOW PRESS TWO BUTTONS TOGETHER");
    graphLib.draw_text(20, 32, "AND HOLD IT FOR 5 SECONDS");
    graphLib.draw_text(20, 44, "TO RESET CONFIGURATION.");
    graphLib.draw_text(20, 60, "OR WAIT 10 SECONDS TO");
    graphLib.draw_text(20, 72, "RETURN.");
    graphLib.updateScreen();
    long init_timer = timer.getTimer();
    while (input.is_check_input_reset_command_activated() == false) {
        input.read_input(true);
        timer.delay(1);
        if (init_timer+10000 < timer.getTimer()) {
            break;
        } else if (input.is_check_input_reset_command_activated() == true) {
            std::cout << "RESET CONFIG!!!" << std::endl;
            // reset the configuration file //
            game_config.reset();
            fio.save_config(game_config);
            break;
        }
    }

    wait_release_reset_config();
    graphLib.copyArea(st_position(0, 0), &screen_copy, &graphLib.gameScreen);
    graphLib.updateScreen();
}

void option_picker::wait_release_reset_config()
{
    graphLib.clear_area(0, 0, RES_W, RES_H, 40, 0, 0);
    graphLib.draw_text(20, 20, "PLEASE RELEASE BUTTONS");
    graphLib.updateScreen();
    while (input.is_check_input_reset_command_activated() == true) {
        input.read_input(true);
        timer.delay(1);
    }
}



void option_picker::draw()
{

    text_max_len = 0;
    for (int i=0; i<_items.size(); i++) {
        std::string line = _items.at(i).text;
        int line_len = line.length();
        //std::cout << "line_len[" << i << "]: " << line_len << std::endl;
        if (line_len > text_max_len) {
            text_max_len = line_len;
        }
    }

    //std::cout << "OPTION_PICKER::text_max_len: " << text_max_len << std::endl;

    graphLib.clear_area(_position.x, _position.y, text_max_len*8, _items.size()*12, 0, 0, 0);
	for (unsigned int i=0; i<_items.size(); i++) {
        st_menu_option menu_item = _items.at(i);

        //std::cout << "menu_item: " << menu_item.text << std::endl;
        if (menu_item.disabled == true) {
            graphLib.draw_text(_position.x, _position.y + (12 * i), menu_item.text, st_color(100, 100, 100));
        } else {
            graphLib.draw_text(_position.x, _position.y + (12 * i), menu_item.text);
        }
	}
}

