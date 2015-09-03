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
    option_picker(bool draw_border, st_position pos, std::vector<std::string> options, bool show_return);
    /**
     * @brief
     *
     * @return int
     */
    int pick();


    /**
     * @brief
     *
     * @param pick_pos
     */
    void set_picker_initial_pos(int pick_pos);
    /**
     * @brief
     *
     */
    void draw();


private:
    st_position _position; /**< TODO */
    std::vector<std::string> _items; /**< TODO */
    bool _draw_border; /**< TODO */
    short _pick_pos; /**< TODO */
    bool _show_return;
};

#endif // OPTION_PICKER_H
