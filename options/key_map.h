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

    void check_key_duplicates(CURRENT_FILE_FORMAT::st_game_config game_config_copy, int set_key, bool is_joystick);

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

    int draw_config_keys() const;




private:
    std::vector<std::string> _keys_list; /**< TODO */
};

#endif // KEY_MAP_H
