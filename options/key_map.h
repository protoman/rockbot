#ifndef KEY_MAP_H
#define KEY_MAP_H

#include <string>
#include <vector>

#include "file/format.h"


/**
 * @brief
 *
 */
class key_map
{
public:
/**
 * @brief
 *
 */
    key_map();
    /**
     * @brief
     *
     */


    /**
     * @brief
     *
     */
    void config_input();

    void config_buttons();

    bool is_key_set(INPUT_COMMANDS key, format_v4::st_game_config game_config_copy);

    void check_key_duplicates(CURRENT_FILE_FORMAT::st_game_config& game_config_copy, Uint8 set_key, bool is_joystick);

    void apply_key_codes_changes(CURRENT_FILE_FORMAT::st_game_config game_config_copy);


private:
    /**
     * @brief
     *
     */
    void draw_screen();
    /**
     * @brief
     *
     * @param line
     */
    void redraw_line(short line) const;

    Sint8 draw_config_input() const;

    std::string build_button_config_line(std::string prefix, std::string sufix);

    Sint8 draw_config_buttons(format_v4::st_game_config &game_config_copy);




private:
    std::vector<std::string> _keys_list; /**< TODO */
};

#endif // KEY_MAP_H
