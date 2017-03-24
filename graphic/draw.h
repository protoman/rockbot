#ifndef DRAW_H
#define DRAW_H

#include "graphicslib.h"

#include "soundlib.h"

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
    void set_gfx(Uint8 gfx, short mode);
    Uint8 get_gfx();
    void set_flash_enabled(bool enabled);
    void show_boss_intro_sprites(short boss_id, bool show_fall);
    void show_ready();
    void show_bubble(int x, int y);
    void set_teleport_small_colors(st_color color1, st_color color2);
    void show_teleport_small(int x, int y);
    int show_credits(bool can_leave);
    void create_credits_text(graphicsLib_gSurface& surface);
    graphicsLib_gSurface* get_object_graphic(int obj_id);
    void remove_object_graphic(int obj_id);
    void show_ingame_warning(std::vector<std::string> message);
    void fade_in_screen(int r, int g, int b);
    void fade_out_screen(int r, int g, int b, int total_delay);
    void pixelate_screen();
    void add_weapon_tooltip(short weapon_n, const st_position &player_pos, const Uint8 &direction);
    void clear_maps_dynamic_background_list();
    void add_dynamic_background(std::string filename, int auto_scroll_mode, st_color bg_color);
    graphicsLib_gSurface* get_dynamic_background(std::string filename);
    graphicsLib_gSurface* get_dynamic_foreground(std::string filename);
    void show_player_hp(int hp, int player_n, int selected_weapon, int selected_weapon_value);

    void draw_castle_path(bool instant, st_position initial_point, st_position final_point);

private:
    void draw_credit_line(graphicsLib_gSurface& surface, Uint8 initial_line);
    void show_rain();
    void show_flash();
    void generate_snow_particles();
    void show_snow_effect();
    void show_train_effect();
    void show_lightingbolt_effect();
    void show_shadow_top_effect();
    void show_inferno_effect();
    void free_inferno_surface();
    void show_weapon_tooltip();
    //void create_dynamic_background_surface(graphicsLib_gSurface& dest_surface, graphicsLib_gSurface& image_surface, int auto_scroll_mode);



private:
    graphicsLib_gSurface rain_obj;
    unsigned int _effect_timer;
    short int _rain_pos;

    graphicsLib_gSurface flash_obj;
    short int _flash_pos;
    unsigned int _flash_timer;
    st_position flash_points[FLASH_POINTS_N];

    graphicsLib_gSurface _bubble_gfx;
    // USED IN CREDITS
    std::vector<std::string> credits_list; /**< TODO */
    graphicsLib_gSurface _teleport_small_gfx;

    // GRAPHICS LISTS
    std::map<unsigned int, graphicsLib_gSurface> objects_sprite_list; // object_id, graphic

    // USED IN SNOW EFFECT
    std::vector<st_snow_particle> _snow_particles;
    graphicsLib_gSurface snow_flacke;
    graphicsLib_gSurface shadow_line;
    graphicsLib_gSurface _inferno_surface;
    int _inferno_alpha;
    short _inferno_alpha_mode; // 0 increasing, 1 decreasing


    // USED IN TRAIN EFFECT
    int _train_effect_timer;
    int _train_effect_state;
    Mix_Chunk* _train_sfx;

    // USED IN LIGHTINGBOLT
    int _lightingbolt_effect_timer;
    int _lightingbolt_effect_state;

    Uint8 screen_gfx;
    Uint8 screen_gfx_mode;
    bool flash_effect_enabled;

    // WEAPON ICON TOOLTIP
    const st_position* _weapon_tooltip_pos_ref;         // holds a pointer to the position it must follow in dynamic animation type
    int _weapon_tooltip_timer;                          // timer, if less than current time, will show the tooltip
    short _weapon_tooltip_n;                            // weapon number to show
    const Uint8* _weapon_tooltip_direction_ref;         // reference to player direction, so we can draw the icon in the correct position
    st_float_position *_map_scroll_ref;                 // reference to map scroll, so we can adjust position


    // used to avoid having multiple copies of same background for all 3 maps in same stage
    std::map<std::string, graphicsLib_gSurface> maps_dynamic_background_list;


};

#endif // DRAW_H
