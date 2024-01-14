#ifndef OPTION_PICKER_H
#define OPTION_PICKER_H

#include <string>
#include <vector>

#include "../file/format/st_common.h"


/**
 * @brief
 *
 */
class option_picker
{
public:
/**
 * @brief
 *
 * @param draw_border
 * @param pos
 * @param options
 */
    option_picker(bool draw_border, st_position pos, std::vector<st_menu_option> options, bool show_return);
    option_picker(bool draw_border, st_position pos, std::vector<std::string> options, bool show_return);
    void change_option_label(unsigned int n, std::string label);
    Sint8 pick(int initial_pick_pos=0);
    void enable_check_input_reset_command();
    void enable_check_input_cheat_command();
    void show_reset_config_dialog();
    void wait_release_reset_config();
    void add_option_item(st_menu_option item);


    void draw();


private:
    bool initialized = false;
    st_position _position; /**< TODO */
    std::vector<st_menu_option> picker_item_list; /**< TODO */
    bool _draw_border; /**< TODO */
    Uint8 _pick_pos; /**< TODO */
    bool _show_return;
    int text_max_len;
    bool check_input_reset_command;                 // if this option is enabled, it will try to detect user holding two buttons for 5 seconds
    bool check_input_cheat_command;
};

#endif // OPTION_PICKER_H
