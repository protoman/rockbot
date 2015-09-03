#pragma GCC diagnostic ignored "-Wreorder"

#ifndef GRAPHICSLIB_H
#define GRAPHICSLIB_H

#include <string>
#include <vector>
#include <map>


#include <SDL/SDL.h>				//Include da SDL
#include <SDL/SDL_image.h>		//Include da biblioteca SDL_Image
#include <SDL/SDL_ttf.h>		// Include da biblioteca SDL_ttf
#include <SDL/SDL_endian.h>

#include "defines.h"
#include "file/format/st_common.h"
#include "class_colorcycle.h"
#include "file/format.h"

#ifdef PSP
#include "ports/psp/psp_ram.h"
#endif

#define INTRO_STARS_NUMBER 10
#define ANIM_STARS_BIG_STEP 6
#define ANIM_STARS_STEP 4
#define ANIM_STARS_SMALL_STEP 2

// ****************************************************************************** //
// graphicsLib is a layer to handle graphics. for future ports that do not use    //
// SDL, you can clone this class and use other platform-specific methods          //
// ****************************************************************************** //



/**
 * @brief
 *
 */
class graphicsLib
{
public:
/**
 * @brief
 *
 */
    graphicsLib();
    /**
     * @brief
     *
     */
    ~graphicsLib();

    /**
     * @brief
     *
     * @return bool
     */
    bool initGraphics();
    /**
     * @brief
     *
     */
    void preload();
    /**
     * @brief
     *
     */
    void updateScreen();


    int get_colorkey_n(COLOR_KEYS key);

    /**
     * @brief
     *
     */
    void loadTileset();
    /**
     * @brief
     *
     * @param st_position
     * @param st_position
     * @param
     */
    void placeTile(struct st_position, struct st_position, struct graphicsLib_gSurface*);

    void place_3rd_level_tile(int origin_x, int origin_y, int dest_x, int dest_y);


    /**
     * @brief
     *
     * @param rect
     * @param st_position
     * @param
     * @param
     */
    void copy_gamescreen_area(st_rectangle origin_rectangle, st_position pos, graphicsLib_gSurface *surfaceDestiny);

    /**
     * @brief
     *
     * @param rect
     * @param st_position
     * @param
     * @param
     */
    void copyArea(struct st_rectangle rect, struct st_position, struct graphicsLib_gSurface*, struct graphicsLib_gSurface*);


    // to be used in spceial cases when colormap issues happen as transitionscreen in game.cpp
    void copy_area_with_colormap_update(struct st_rectangle origin_rectangle, struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny);

    /**
     * @brief
     *
     * @param st_position
     * @param
     * @param
     */
    void copyArea(struct st_position, struct graphicsLib_gSurface*, struct graphicsLib_gSurface*); // overload, use the whole width and height of given image
    /**
     * @brief overload of copyArea, use the whole width and height of given image
     *
     * @param st_position
     * @param
     * @param
     */
    void copyAreaWithAdjust(struct st_position, struct graphicsLib_gSurface*, struct graphicsLib_gSurface*); // overload, use the whole width and height of given image

    /**
     * @brief overload for copyArea that uses screen_adjust
     *
     * @param std::string
     * @param
     */
    void surfaceFromFile(std::string, graphicsLib_gSurface *);
    /**
     * @brief
     *
     * @param
     */
    void showSurface(struct graphicsLib_gSurface*);
    /**
     * @brief
     *
     * @param surfaceOrigin
     * @param origin_rectangle
     */
    void showMapSurfaceRegion(struct graphicsLib_gSurface* surfaceOrigin, st_rectangle origin_rectangle);
    /**
     * @brief
     *
     * @param
     * @param origin_rectangle
     */
    void showSurfaceRegion(struct graphicsLib_gSurface*, const struct st_rectangle origin_rectangle);
    /**
     * @brief
     *
     * @param surfaceOrigin
     * @param origin_rectangle
     * @param pos_destiny
     */
    void showSurfaceRegionAt(struct graphicsLib_gSurface* surfaceOrigin, const struct st_rectangle origin_rectangle, struct st_position pos_destiny);

    /**
     * @brief
     *
     * @param surfaceOrigin
     * @param origin_rect
     * @param destiny_rect
     */
    void showSurfacePortion(struct graphicsLib_gSurface* surfaceOrigin, const struct st_rectangle origin_rect, struct st_rectangle destiny_rect);

    /**
     * @brief
     *
     * @param
     * @param st_position
     * @param fix_colors
     */
    void showSurfaceAt(struct graphicsLib_gSurface*, struct st_position, bool fix_colors);
    /**
     * @brief
     *
     * @param st_size
     * @param
     */

    void show_white_surface_at(struct graphicsLib_gSurface*, struct st_position);

    void initSurface(struct st_size, graphicsLib_gSurface *);
    /**
     * @brief
     *
     * @param st_rectangle
     * @param
     * @return graphicsLib_gSurface
     */
    struct graphicsLib_gSurface surfaceFromRegion(struct st_rectangle, struct graphicsLib_gSurface&);
    /**
     * @brief
     *
     */
    void blank_screen();
    /**
     * @brief
     *
     * @param surface
     */
    void blank_surface(struct graphicsLib_gSurface& surface);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param w
     * @param h
     */

    int draw_progressive_text(short int x, short int y, std::string text, bool interrupt);
    /**
     * @brief returns if was cut (by user  pressing a key) or not
     *
     * @param x
     * @param y
     * @param text
     */
    void draw_text(short int x, short int y, std::string text);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param text
     * @param surface
     */
    void draw_text(short int x, short int y, std::string text, struct graphicsLib_gSurface& surface);

    /**
     * @brief
     *
     * @param y
     * @param text
     */
    void draw_centered_text(short int y, std::string text, st_color font_color);
    /**
     * @brief
     *
     * @param y
     * @param text
     * @param surface
     */
    void draw_centered_text(short int y, std::string text, struct graphicsLib_gSurface& surface, st_color temp_font_color);

    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     * @return int
     */
    int getColorNumber(int r, int g, int b);
    /**
     * @brief
     *
     * @param st_position
     */
    void drawCursor(st_position);
    /**
     * @brief
     *
     * @param st_position
     */
    void eraseCursor(st_position);
    /**
     * @brief
     *
     * @param r
     * @param g
     * @param b
     */
    void blink_screen(int r, int g, int b);
    /**
     * @brief
     *
     * @param surface
     */
    void blink_surface_into_screen(struct graphicsLib_gSurface &surface);
    /**
     * @brief
     *
     */
    void load_icons();
    /**
     * @brief
     *
     * @param WEAPON_ICONS_ENUM
     * @param menu_pos
     * @param active
     */
    void draw_weapon_icon(short, st_position menu_pos, bool active);
    /**
     * @brief
     *
     * @param x_pos
     */
    void draw_menu_item(int x_pos);
    /**
     * @brief
     *
     * @param x_pos
     */
    void erase_menu_item(int x_pos);
    /**
     * @brief
     *
     * @param ITEM_ICONS_ENUM
     * @param pos
     */
    void draw_item_icon(enum ITEM_ICONS_ENUM, st_position pos);
    /**
     * @brief
     *
     * @param current_hp
     * @param player_frame
     */
    void draw_weapon_menu_bg(Uint8 current_hp, graphicsLib_gSurface *player_frame);

    /**
     * @brief
     *
     * @param hp
     * @param player_n
     * @param weapon_n
     */
    void draw_hp_bar(short int hp, short player_n, short weapon_n);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param w
     * @param h
     * @param r
     * @param g
     * @param b
     */
    void clear_area(short int x, short int y, short int w, short int h, short int r, short int g, short int b);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param w
     * @param h
     * @param r
     * @param g
     * @param b
     * @param surface
     */
    void clear_area_no_adjust(short int x, short int y, short int w, short int h, short int r, short int g, short int b);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param w
     * @param h
     * @param r
     * @param g
     * @param b
     * @param surface
     */

    void blank_area(short int x, short int y, short int w, short int h);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param w
     * @param h
     * @param surface
     */
    void blank_area(short int x, short int y, short int w, short int h, struct graphicsLib_gSurface& surface);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param text
     * @param interrupt
     * @return int
     */

    void clear_surface_area(short int x, short int y, short int w, short int h, short int r, short int g, short int b, struct graphicsLib_gSurface& surface) const;
    /**
     * @brief
     *
     * @param color_key
     * @param new_color
     */
    void change_colormap(unsigned int color_key, st_color new_color);
    /**
     * @brief
     *
     * @param map_n
     */
    void set_colormap(int map_n);

    void execute_colorcycle();

    /**
     * @brief
     *
     */
    void reset_colormap();

    void reset_image_colormap(graphicsLib_gSurface &surface);


    /**
     * @brief
     *
     * @param position
     */
    void show_config_bg(int position);
    /**
     * @brief
     *
     * @param old_pos
     * @param hp
     * @param player_n
     */
    void draw_weapon_cursor(st_position old_pos, short hp, short player_n);
    /**
     * @brief
     *
     * @param position
     * @param show_btn
     */
    void show_dialog(int position, bool show_btn=false);
    /**
     * @brief
     *
     * @param face_file
     * @param pos
     */
    void place_face(std::string face_file, st_position pos);
    /**
     * @brief
     *
     * @return st_position
     */
    st_position get_dialog_pos() const;
    /**
     * @brief
     *
     * @param key
     * @param new_color
     * @param surface
     */
    void change_surface_color(st_color key, st_color new_color, struct graphicsLib_gSurface* surface);
    /**
     * @brief
     *
     * @param key_n
     * @param new_color
     * @param surface
     */
    void change_surface_color(int key_n, st_color new_color, struct graphicsLib_gSurface* surface);
    /**
     * @brief
     *
     * @param btn_pos
     */
    void show_btn_a(st_position btn_pos);



    /**
     * @brief
     *
     * @param adjust
     */
    void set_screen_adjust(st_position adjust);

    /**
     * @brief
     *
     */
    void stop_stars_animation();
    /**
     * @brief
     *
     */
    void start_stars_animation();
    /**
     * @brief
     *
     * @param period
     */
    void wait_and_update_screen(int period); // used because can't use SDL_Flip inside a thread
    /**
     * @brief
     *
     * @return st_position
     */
    st_position get_config_menu_pos() const;
    /**
     * @brief
     *
     * @return st_size
     */
    st_size get_config_menu_size();

    /**
     * @brief
     *
     * @param surface
     */
    void set_colormap_white(struct graphicsLib_gSurface* surface);
    /**
     * @brief
     *
     * @param surface
     */
    void set_colormap_original(struct graphicsLib_gSurface* surface);


    /**
     * @brief
     *
     * @param surface
     */
    void set_colormap_current(struct graphicsLib_gSurface* surface);

    void set_colormap_current_tileset();

    void set_colormap_original_tileset();

    /**
     * @brief
     *
     * @param y_adjust
     * @param right
     * @param hp
     * @param player_n
     */
    void draw_horizontal_hp_bar(short int y_adjust, short int right, short int hp, short int player_n=3);
    /**
     * @brief
     *
     * @param pos
     */
    void draw_explosion(st_position pos);
    /**
     * @brief
     *
     * @param msg
     */
    void show_debug_msg(std::string msg);
    /**
     * @brief
     *
     */
    void reset_tileset_colormap();
    /**
     * @brief
     *
     * @param initial_point
     * @param final_point
     * @param duration
     */
    void draw_path(st_position initial_point, st_position final_point, short duration);


    void preload_stage_colorcycle();

    void add_stage_colorcycle(short stage_n, CURRENT_FILE_FORMAT::file_colorcycle &colorcycle);

    void update_colors();

    void update_surface_colormap(graphicsLib_gSurface *display_surface);



private:
    /**
     * @brief
     *
     * @param st_rectangle
     * @param st_position
     * @param
     * @param
     * @param fix_colors
     */
    void copySDLArea(struct st_rectangle, struct st_position, SDL_Surface*, SDL_Surface*, bool fix_colors);
    /**
     * @brief
     *
     * @param st_rectangle
     * @param st_rectangle
     * @param
     * @param
     */
    void copySDLPortion(struct st_rectangle, struct st_rectangle, SDL_Surface*, SDL_Surface*);
    /**
     * @brief
     *
     * @param filename
     * @return SDL_Surface
     */
    SDL_Surface *SDLSurfaceFromFile(std::string filename);
    /**
     * @brief
     *
     * @param src
     * @param dst
     * @param smooth_scale
     */
    void scale2x(SDL_Surface *src, SDL_Surface *dst, bool smooth_scale) const;
    /**
     * @brief
     *
     * @param pos
     * @param hp
     * @param player_n
     */
    void draw_horizontal_hp_bar(st_position pos, short int hp, short int player_n=3);
    /**
     * @brief
     *
     * @param player_n
     * @param weapon_n
     */
    void draw_vertical_hp_bar(short int player_n, short int weapon_n);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param size
     */
    void draw_star(short int x, short int y, int size);
    /**
     * @brief
     *
     * @param x
     * @param y
     * @param size
     */
    void erase_star(short int x, short int y, int size);
    /**
     * @brief
     *
     */
    void preload_faces() const;
    /**
     * @brief
     *
     */
    void unload_faces() const;
    /**
     * @brief
     *
     */
    void init_stars();
    /**
     * @brief
     *
     */
    void anim_stars();
    /**
     * @brief
     *
     * @param fullscreen
     */
    void set_video_mode();
    /**
     * @brief
     *
     */
    void preload_images();

    void update_surface_colormap(SDL_Surface *display_surface, SDL_Color set_colormap[COLOR_COUNT]);




public:
    int RES_DIFF_W; /**< TODO */
    int RES_DIFF_H; /**< TODO */
    struct graphicsLib_gSurface gameScreen; /**< TODO */
    st_position big_star_list[INTRO_STARS_NUMBER/2]; /**< TODO */
    st_position star_list[INTRO_STARS_NUMBER]; /**< TODO */
    st_position small_star_list[INTRO_STARS_NUMBER]; /**< TODO */

	// graphics used in several places
    graphicsLib_gSurface small_explosion; /**< TODO */
    graphicsLib_gSurface projectile_surface[FS_MAX_PROJECTILES]; /**< TODO */
    graphicsLib_gSurface bomb_explosion_surface; /**< TODO */
    graphicsLib_gSurface e_tank[2]; /**< TODO */
    graphicsLib_gSurface w_tank[2]; /**< TODO */
    graphicsLib_gSurface s_tank[2]; /**< TODO */
    graphicsLib_gSurface energy_balancer; /**< TODO */

    graphicsLib_gSurface explosion32; /**< TODO */
    graphicsLib_gSurface explosion16; /**< TODO */
    graphicsLib_gSurface dash_dust; /**< TODO */
    graphicsLib_gSurface hit; /**< TODO */
    graphicsLib_gSurface water_splash; /**< TODO */

    graphicsLib_gSurface armor_icon_arms;
    graphicsLib_gSurface armor_icon_body;
    graphicsLib_gSurface armor_icon_legs;


private:

    SDL_Color colormap[COLOR_COUNT]; /**< TODO */
    SDL_Color colormap_original[COLOR_COUNT]; /**< TODO */
    SDL_Color colormap_white[COLOR_COUNT]; /**< TODO */
    class_colorcycle colorcycle_manager; /**< TODO */
    unsigned int color_keys[3]; /**< TODO */
    std::map<std::string, graphicsLib_gSurface> FACES_SURFACES; /**< TODO */
	// TODO: free those pointers
    TTF_Font *font; /**< TODO */
    TTF_Font *lowercase_font; /**< TODO */
    SDL_Surface *game_screen;									// we do not put this into a graphicsLib_gSurface because this is meant to be used only internally /**< TODO */
    SDL_Surface *game_screen_scaled; /**< TODO */
    SDL_Surface *tileset;										// we do not put this into a graphicsLib_gSurface because this is meant to be used only internally /**< TODO */
    std::vector<struct graphicsLib_gSurface> faces;				// faces for players and npcs /**< TODO */
    std::vector<struct graphicsLib_gSurface> weapon_icons;		// weapon icons, used in menu and energy bars /**< TODO */
    std::vector<struct graphicsLib_gSurface> small_weapon_icons;		// weapon icons, used in menu and energy bars /**< TODO */
    std::vector<struct graphicsLib_gSurface> item_icons;		// item icons, used in menu and energy bars /**< TODO */

    struct graphicsLib_gSurface ingame_menu; /**< TODO */
    struct graphicsLib_gSurface config_menu; /**< TODO */
    struct graphicsLib_gSurface dialog_surface; /**< TODO */
    struct graphicsLib_gSurface _btn_a_surface; /**< TODO */

    st_position _dialog_pos; /**< TODO */
    st_position _config_menu_pos; /**< TODO */

    unsigned int stars_timer; /**< TODO */

    st_position _screen_adjust; /**< TODO */

    SDL_PixelFormat screen_pixel_format; /**< TODO */

    bool _show_stars; /**< TODO */

    unsigned int _explosion_animation_timer; /**< TODO */
    int _explosion_animation_pos; /**< TODO */
    int _debug_msg_pos; /**< TODO */
    st_position _screen_resolution_adjust; /**< TODO */
    bool _must_set_colors;
    short int _video_filter;                                     // copy from game_options, so graphlib isn't affected when the option is changed while game running
    long _timer;


#ifdef PSP
    psp_ram _ram_counter;
    //std::cout << "unload_stage::RAM::BF='" << ram_counter.ramAvailable() << "'" << std::endl;
#endif
};

#endif // GRAPHICSLIB_H
