#ifndef DRAW_H
#define DRAW_H

#include "../graphicslib.h"

#include "../soundlib.h"

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
    graphicsLib_gSurface* get_input_surface(e_INPUT_IMAGES input);
    void update_screen(); // replaces external calls to graphLib.updateScreen
    void set_gfx(Uint8 gfx, short mode);
    Uint8 get_gfx();
    void set_flash_enabled(bool enabled);
    void show_boss_intro_sprites(int boss_id, bool show_fall);
    void show_ready();
    void show_bubble(int x, int y);
    void set_teleport_small_colors(st_color color1, st_color color2);
    void show_teleport_small(int x, int y);
    int show_credits_text(bool can_leave, std::vector<string> credit_text);
    void draw_credits_line(int i, std::vector<string> credit_text, int posY);
    int show_credits(bool can_leave);
    void show_about();
    std::vector<string> create_engine_credits_text();
    graphicsLib_gSurface* get_object_graphic(int obj_id);
    void remove_object_graphic(int obj_id);
    void show_ingame_warning(std::vector<std::string> message);
    void fade_in_screen(int r, int g, int b, int total_delay);
    void fade_out_screen(int r, int g, int b, int total_delay);
    void add_fade_out_effect(int r, int g, int b);
    void remove_fade_out_effect();
    void pixelate_screen();
    void add_weapon_tooltip(short weapon_n, const st_position &player_pos, const Uint8 &direction);
    void clear_maps_dynamic_background_list();
    void add_dynamic_background(std::string filename, int auto_scroll_mode, st_color bg_color);
    void set_dynamic_bg_alpha(std::string filename, int alpha);

    graphicsLib_gSurface* get_dynamic_background(std::string filename);
    graphicsLib_gSurface* get_dynamic_foreground(std::string filename);

    void show_hud(int hp, int player_n, int selected_weapon, int selected_weapon_value);
    void show_hud_vintage(int hp, int player_n, int selected_weapon, int selected_weapon_value);
    void draw_enery_ball(int value, int x_pos, graphicsLib_gSurface &ball_surface);
    void draw_game_menu_weapon_bar(int weapon_n, int percent, int value);
    void draw_energy_bar(short int hp, short int player_n, short int weapon_n, short int max_hp);

    void set_boss_hp(int hp);
    void show_boss_intro_bg();
    void draw_explosion(st_position center_point, int radius, int angle_inc);
    void draw_player_death(st_position center_point, int frame_n);
    int get_death_animation_frames_n();
    void show_weapon_tooltip();

    void draw_in_game_menu(short selected_weapon_n);
    void draw_weapon_menu_weapon(short selected_weapon_n);


private:
    void draw_credit_line(graphicsLib_gSurface& surface, Uint8 initial_line, std::vector<string> credit_text);
    void show_rain();
    void show_flash();
    void generate_snow_particles();
    void show_snow_effect();
    void show_train_effect();
    void show_lightingbolt_effect();
    void show_shadow_top_effect();
    void show_inferno_effect();
    void free_inferno_surface();
    st_float_position get_radius_point(st_position center_point, int radius, float angle);
    //void create_dynamic_background_surface(graphicsLib_gSurface& dest_surface, graphicsLib_gSurface& image_surface, int auto_scroll_mode);


private:
    graphicsLib_gSurface boss_intro_bg;
    graphicsLib_gSurface rain_obj;
    unsigned int _effect_timer;
    short int _rain_pos;

    graphicsLib_gSurface flash_obj;
    short int _flash_pos;
    unsigned int _flash_timer;
    st_position flash_points[FLASH_POINTS_N];

    graphicsLib_gSurface _bubble_gfx;
    // USED IN CREDITS
    graphicsLib_gSurface _teleport_small_gfx;
    int teleport_small_frame_count;
    int teleport_small_frame;
    unsigned long teleport_small_frame_timer;

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

    // used in HUD
    graphicsLib_gSurface hud_player_hp_ball;
    graphicsLib_gSurface hud_player_wpn_ball;
    graphicsLib_gSurface hud_boss_hp_ball;
    int _boss_current_hp;

    // used in weapoins menu
    struct graphicsLib_gSurface ingame_menu_bg_img;

    // WEAPON ICON TOOLTIP
    const st_position* _weapon_tooltip_pos_ref;         // holds a pointer to the position it must follow in dynamic animation type
    int _weapon_tooltip_timer;                          // timer, if less than current time, will show the tooltip
    short _weapon_tooltip_n;                            // weapon number to show
    const Uint8* _weapon_tooltip_direction_ref;         // reference to player direction, so we can draw the icon in the correct position
    st_float_position *_map_scroll_ref;                 // reference to map scroll, so we can adjust position
    int current_alpha;                                  // used for fade effect that runs each time update-screen is called
    st_color current_alpha_color;
    graphicsLib_gSurface current_alpha_surface;

    struct graphicsLib_gSurface _death_animation;
    int _death_animation_frames_n = 0;

    // used to avoid having multiple copies of same background for all 3 maps in same stage
    std::map<std::string, graphicsLib_gSurface> maps_dynamic_background_list;

    graphicsLib_gSurface input_images_map[INPUT_IMAGES_COUNT];


};

#endif // DRAW_H
