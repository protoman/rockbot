#ifndef OPTION_PICKER_H
#define OPTION_PICKER_H

#include "file/format/st_common.h"
#include <string>
#include <vector>


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
    void change_option_label(int n, std::string label);
    Sint8 pick();


    /**
     * @brief
     *
     * @param pick_pos
     */
    void set_picker_initial_pos(Uint8 pick_pos);
    /**
     * @brief
     *
     */
    void draw();


private:
    st_position _position; /**< TODO */
    std::vector<st_menu_option> _items; /**< TODO */
    bool _draw_border; /**< TODO */
    Uint8 _pick_pos; /**< TODO */
    bool _show_return;
    int text_max_len;
};

#endif // OPTION_PICKER_H
