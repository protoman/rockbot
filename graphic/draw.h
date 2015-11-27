#ifndef DRAW_H
#define DRAW_H

#include "graphicslib.h"

#define FLASH_POINTS_N 10
#define SNOW_PARTICLES_NUMBER 20

struct st_snow_particle {
    st_float_position position;
    float speed;                    // vertical speed. horizontal speed is the same for all
    float x_dist;                   // used to check how many pixels were moved in balance to change direction
    short direction;                // if balancing left or right
    st_snow_particle(st_float_position pos, float spd)
    {
        position = pos;
        speed = spd;
        x_dist = 0;
        if ((int)position.x % 2 == 0) {
            direction = ANIM_DIRECTION_LEFT;
        } else {
            direction = ANIM_DIRECTION_RIGHT;
        }
    }
};

class draw
{
public:
    draw();

public:
    void preload();
    void show_gfx();
    void update_screen(); // replaces external calls to graphLib.updateScreen
    void set_gfx(Uint8 gfx);
    Uint8 get_gfx();
    void set_flash_enabled(bool enabled);
    void show_boss_intro_sprites(short boss_id, bool show_fall);
    void show_ready();
    void show_bubble(int x, int y);
    void set_teleport_small_colors(st_color color1, st_color color2);
    void show_teleport_small(int x, int y);
    void show_credits();
    void create_credits_text(graphicsLib_gSurface& surface);
    graphicsLib_gSurface* get_object_graphic(int obj_id);
    void remove_object_graphic(int obj_id);
    void show_ingame_warning(std::vector<std::string> message);

    void fade_in_screen(int r, int g, int b);
    void fade_out_screen(int r, int g, int b);


private:
    void draw_credit_line(graphicsLib_gSurface& surface, Uint8 initial_line);
    void show_rain();
    void show_flash();
    void generate_snow_particles();
    void show_snow_effect();


private:
    graphicsLib_gSurface rain_obj;
    unsigned int _effect_timer;
    short int _rain_pos;

    graphicsLib_gSurface flash_obj;
    short int _flash_pos;
    unsigned int _flash_timer;
    st_position flash_points[FLASH_POINTS_N];

    struct graphicsLib_gSurface ready_message;
    graphicsLib_gSurface _bubble_gfx;
    // USED IN CREDITS
    std::vector<std::string> credits_list; /**< TODO */
    graphicsLib_gSurface _teleport_small_gfx;

    // GRAPHICS LISTS
    std::map<unsigned int, graphicsLib_gSurface> objects_sprite_list; // object_id, graphic

    // USED IN SNOW EFFECT
    std::vector<st_snow_particle> _snow_particles;

    Uint8 screen_gfx;
    bool flash_effect_enabled;


};

#endif // DRAW_H
