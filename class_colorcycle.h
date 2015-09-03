#ifndef CLASS_COLORCYCLE_H
#define CLASS_COLORCYCLE_H

#include <vector>
#include <map>
#include "defines.h"
#include "file/format/st_common.h"

#include "file/file_io.h"


/**
 * @brief
 *
 */
struct colorcycle_item {
    st_color color[COLOR_KEY_LENGTH]; /**< TODO */
    unsigned int duration[COLOR_KEY_LENGTH]; /**< TODO */

	colorcycle_item() {
		color[COLOR_KEY_GREEN] = st_color();
		color[COLOR_KEY_PURPLE] = st_color();
		color[COLOR_KEY_CYAN] = st_color();
		duration[COLOR_KEY_GREEN] = 0;
		duration[COLOR_KEY_PURPLE] = 0;
		duration[COLOR_KEY_CYAN] = 0;
	}

/**
 * @brief
 *
 * @param set_green
 * @param duration_green
 * @param set_purple
 * @param duration_purple
 * @param set_cyan
 * @param duration_cyan
 */
    colorcycle_item(st_color set_green, int duration_green, st_color set_purple, int duration_purple, st_color set_cyan, int duration_cyan)
	{
		color[COLOR_KEY_GREEN] = set_green;
		color[COLOR_KEY_PURPLE] = set_purple;
		color[COLOR_KEY_CYAN] = set_cyan;
		duration[COLOR_KEY_GREEN] = duration_green;
		duration[COLOR_KEY_PURPLE] = duration_purple;
		duration[COLOR_KEY_CYAN] = duration_cyan;
	}
};

/**
 * @brief
 *
 */
struct colorcycle_list {
    bool can_loop; // indicates if the list can go back to first item when finished /**< TODO */
    std::vector<struct colorcycle_item> colors; /**< TODO */
	colorcycle_list  () {
        colors.clear();
		can_loop = true;
	}
};

/**
 * @brief
 *
*/
class class_colorcycle
{
public:
	class_colorcycle();
    /**
     * @brief
     *
     */
    /**
     * @brief
     *
     * @param map_key
     * @return bool
     */
    void execute(void);
    /**
     * @brief
     *
     * @param key
     * @return unsigned int
     */
    unsigned int get_current_colormap_timer(unsigned int key);
    /**
     * @brief
     *
     * @param key
     * @return st_color
     */
    struct st_color get_current_color_item(unsigned int key);
    /**
     * @brief
     *
     * @param colormap_n
     */
    void set_colormap(int colormap_n);


    void reset_colorcycle();

    /**
     * @brief
     *
     * @param map_key
     * @return bool
     */
    bool have_colorcycle(unsigned int map_key);
    /**
     * @brief
     *
     * @param map_key
     * @param color_number
     */
    void set_color_key_number(unsigned int map_key, int color_number);

    void add_stage_colorcycle(short stage_n, CURRENT_FILE_FORMAT::file_colorcycle& colorcycle);

private:
    /**
     * @brief
     *
     * @param index
     * @param items
     */
    void add(int index, struct colorcycle_list items);

public:
private:
    int current_colormap; /**< TODO */
    unsigned int current_position[COLOR_KEY_LENGTH]; /**< TODO */
    int color_keys[3]; /**< TODO */
    int time_count[COLOR_KEY_LENGTH]; /**< TODO */
    std::map<int, struct colorcycle_list> colorcycle_map; /**< TODO */
    bool _first_run; /**< When this colormap is run for the first time, must ignore time and change colors ASAP */
};

#endif // CLASS_COLORCYCLE_H
