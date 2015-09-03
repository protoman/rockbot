#ifndef DRAW_H
#define DRAW_H

#include "graphicslib.h"

#define FLASH_POINTS_N 10


/**
 * @brief
 *
 */
class draw
{
public:
/**
 * @brief
 *
 */
    draw();

public:
    void preload();
    void update_screen(); // replaces external calls to graphLib.updateScreen
    void update_colorcycle() const;
    void set_rain_enabled(bool enabled);
    void set_flash_enabled(bool enabled);
    void show_rain();
    void show_flash();
    void show_boss_intro_sprites(short boss_id, bool show_fall);
    void show_ready();
    void show_bubble(int x, int y);
    void reset_teleporter_colors();
    void set_teleport_small_colors(st_color color1, st_color color2);
    void show_teleport_small(int x, int y);
    void show_credits();
    void create_credits_text(graphicsLib_gSurface& surface);
    graphicsLib_gSurface* get_object_graphic(int obj_id);
    void remove_object_graphic(int obj_id);
    void show_ingame_warning(std::vector<std::string> message);

private:
    /**
     * @brief
     *
     * @param surface
     * @param initial_line
     */
    void draw_credit_line(graphicsLib_gSurface& surface, Uint8 initial_line);


private:
    graphicsLib_gSurface rain_obj;
    unsigned int _rain_timer;
    short int _rain_pos;
    bool _rain_enabled;

    graphicsLib_gSurface flash_obj;
    short int _flash_pos;
    unsigned int _flash_timer;
    bool _flash_enabled;
    st_position flash_points[FLASH_POINTS_N];

    struct graphicsLib_gSurface ready_message;
    graphicsLib_gSurface _bubble_gfx;
    // USED IN CREDITS
    std::vector<std::string> credits_list; /**< TODO */
    graphicsLib_gSurface _teleport_small_gfx;

    // GRAPHICS LISTS
    std::map<unsigned int, graphicsLib_gSurface> objects_sprite_list; // object_id, graphic




};

#endif // DRAW_H
