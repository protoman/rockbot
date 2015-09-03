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



option_picker::option_picker(bool draw_border, st_position pos, std::vector<std::string> options, bool show_return) : _pick_pos(0)
{
    _position = pos;
    _draw_border = draw_border;

    if (_draw_border) {
        _position.x += 12+CURSOR_SPACING;
        _position.y += 12;
    }
    _items = options;
    _show_return = show_return;
	draw();
}

int option_picker::pick()
{
    bool finished = false;
    input.clean();
    input.waitTime(100);

    //std::cout << "option_picker::option_picker::START" << std::endl;
	graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));

    while (finished == false) {
        input.readInput();
        if (input.p1_input[BTN_START] || input.p1_input[BTN_JUMP]) {
            //std::cout << "option_picker::option_picker::END #1" << std::endl;
			graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
            draw_lib.update_screen();
            if (_show_return == true) {
                _pick_pos--;
            }
            return _pick_pos;
        }
        if (input.p1_input[BTN_DOWN]) {
                soundManager.play_sfx(SFX_CURSOR);
				graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
				_pick_pos++;
                if (_pick_pos >= (short)_items.size()) {
					_pick_pos = 0;
                }
				graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_UP]) {
                soundManager.play_sfx(SFX_CURSOR);
				graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
				_pick_pos--;
				if (_pick_pos < 0) {
					_pick_pos = _items.size()-1;
                }
				graphLib.drawCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
        }
        if (input.p1_input[BTN_QUIT] || input.p1_input[BTN_ATTACK]) {
            //std::cout << "option_picker::option_picker::END #2" << std::endl;
            return -1;
        }
        input.clean();
        input.waitTime(10);
        draw_lib.update_screen();
    }
	graphLib.eraseCursor(st_position(_position.x-CURSOR_SPACING, _position.y+(_pick_pos*CURSOR_SPACING)));
    input.waitTime(10);
    draw_lib.update_screen();
    if (_show_return == true) {
        _pick_pos--;
    }
    return _pick_pos;
}


void option_picker::set_picker_initial_pos(int pick_pos)
{
    //std::cout << "set_picker_initial_pos - pos: " << pick_pos << std::endl;
	_pick_pos = pick_pos;
}

void option_picker::draw()
{
    if (_show_return == true) {
        _items.insert(_items.begin(), "RETURN");
    }
	for (unsigned int i=0; i<_items.size(); i++) {
        graphLib.draw_text(_position.x, _position.y + (12 * i), _items.at(i));
	}
}

