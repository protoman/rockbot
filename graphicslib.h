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
#include "file/format.h"
#include "game_mediator.h"

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


enum e_flip_type {
    flip_type_horizontal,
    flip_type_vertical,
    flip_type_both
};

struct anim_tile_timer {
    unsigned long timer;                        // timer animation was last executed
    Uint8 frame_pos;                            // current frame animation is
    Uint8 max_frames;                           // maximum number of frames this anim-tile have
    int frames_delay[ANIM_TILE_MAX_FRAMES];     // the delay for each animation frame TBD
    anim_tile_timer(int frames_n, unsigned long time_set) {
        timer = time_set;
        frame_pos = 0;
        max_frames = frames_n;

        /// @TODO, set those times
        for (int i=0; i<ANIM_TILE_MAX_FRAMES; i++) {
            frames_delay[i] = 150;
        }
    }
};


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

    void loadTileset(std::string file);

    void placeTile(struct st_position, struct st_position, struct graphicsLib_gSurface*);

    void place_easymode_block_tile(st_position destiny, graphicsLib_gSurface &surface);

    void place_hardmode_block_tile(st_position destiny, graphicsLib_gSurface &surface);


    void place_anim_tile(int anim_tile_id, struct st_position pos_destiny, struct graphicsLib_gSurface* dest_surface);

    void update_anim_tiles_timers();

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

    void set_surface_alpha(int alpha, graphicsLib_gSurface &surface);

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

    void draw_text(short int x, short int y, std::string text, st_color color);


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
    Uint8 getColorNumber(Uint8 r, Uint8 g, Uint8 b);
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
    void blink_screen(Uint8 r, Uint8 g, Uint8 b);
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

    void draw_weapon_tooltip_icon(short weapon_n, st_position position);

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
    void draw_weapon_menu_bg(Uint8 current_hp, graphicsLib_gSurface *player_frame, short max_hp);

    /**
     * @brief
     *
     * @param hp
     * @param player_n
     * @param weapon_n
     */
    void draw_hp_bar(short int hp, short player_n, short weapon_n, short int max_hp);
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


    void draw_rectangle(st_rectangle area, int r, int g, int b, int alpha);

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
     * @param position
     */
    void show_config_bg(Uint8 position);
    /**
     * @brief
     *
     * @param old_pos
     * @param hp
     * @param player_n
     */
    void draw_weapon_cursor(st_position old_pos, short hp, short player_n, short max_hp);
    /**
     * @brief
     *
     * @param position
     * @param show_btn
     */
    void show_dialog(Uint8 position);

    void show_dialog_button(Uint8 position);

    void place_face(std::string face_file, st_position pos);
    /**
     * @brief
     *
     * @return st_position
     */
    st_position get_dialog_pos() const;


    void change_surface_color(Sint8 colorkey_n, st_color new_color, struct graphicsLib_gSurface* surface);

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
    /**
     * @brief
     *
     * @param y_adjust
     * @param right
     * @param hp
     * @param player_n
     */
    void draw_horizontal_hp_bar(short int y_adjust, short int right, short int hp, short int player_n, short max_hp);
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

    void draw_path(st_position initial_point, st_position final_point, short duration);
    graphicsLib_gSurface flip_image(graphicsLib_gSurface original, e_flip_type flip_mode);


private:
    void copySDLArea(struct st_rectangle, struct st_position, SDL_Surface*, SDL_Surface*, bool fix_colors);
    void copySDLPortion(struct st_rectangle, struct st_rectangle, SDL_Surface*, SDL_Surface*);
    SDL_Surface *SDLSurfaceFromFile(std::string filename);
    void scale2x(SDL_Surface *src, SDL_Surface *dst, bool smooth_scale) const;
    void draw_horizontal_hp_bar(st_position pos, short int hp, short int player_n, short max_hp);
    void draw_vertical_hp_bar(short int player_n, short int weapon_n);
    void draw_star(short int x, short int y, int size);
    void erase_star(short int x, short int y, int size);
    void preload_faces() const;
    void unload_faces() const;
    void init_stars();
    void anim_stars();
    void set_video_mode();
    void preload_images();
    void preload_anim_tiles();




public:
    int RES_DIFF_W;
    int RES_DIFF_H;
    struct graphicsLib_gSurface gameScreen;
    st_position big_star_list[INTRO_STARS_NUMBER/2];
    st_position star_list[INTRO_STARS_NUMBER];
    st_position small_star_list[INTRO_STARS_NUMBER];

	// graphics used in several places
    graphicsLib_gSurface small_explosion;
    std::vector<graphicsLib_gSurface> projectile_surface;
    graphicsLib_gSurface bomb_explosion_surface;
    graphicsLib_gSurface e_tank[2];
    graphicsLib_gSurface w_tank[2];
    graphicsLib_gSurface s_tank[2];
    graphicsLib_gSurface energy_balancer;

    graphicsLib_gSurface explosion32;
    graphicsLib_gSurface explosion16;
    graphicsLib_gSurface dash_dust;
    graphicsLib_gSurface hit;
    graphicsLib_gSurface water_splash;

    graphicsLib_gSurface armor_icon_arms;
    graphicsLib_gSurface armor_icon_body;
    graphicsLib_gSurface armor_icon_legs;


private:

    std::map<std::string, graphicsLib_gSurface> FACES_SURFACES;

    std::vector<struct graphicsLib_gSurface> ANIM_TILES_SURFACES;   // hold animated-tiles surface
    std::vector<struct anim_tile_timer> ANIM_TILES_TIMERS;

	// TODO: free those pointers
    TTF_Font *font;
    TTF_Font *lowercase_font;

    SDL_Surface *game_screen;									// we do not put this into a graphicsLib_gSurface because this is meant to be used only internally
    SDL_Surface *game_screen_scaled;
    SDL_Surface *tileset;										// we do not put this into a graphicsLib_gSurface because this is meant to be used only internally
    std::vector<struct graphicsLib_gSurface> faces;				// faces for players and npcs
    std::vector<struct graphicsLib_gSurface> weapon_icons;		// weapon icons, used in menu and energy bars
    std::vector<struct graphicsLib_gSurface> small_weapon_icons;		// weapon icons, used in menu and energy bars
    std::vector<struct graphicsLib_gSurface> item_icons;		// item icons, used in menu and energy bars

    struct graphicsLib_gSurface ingame_menu;
    struct graphicsLib_gSurface config_menu;
    struct graphicsLib_gSurface dialog_surface;
    struct graphicsLib_gSurface _btn_a_surface;

    struct graphicsLib_gSurface _easymode_block;
    struct graphicsLib_gSurface _hardmode_block;

    st_position _dialog_pos;
    st_position _config_menu_pos;

    unsigned int stars_timer;

    st_position _screen_adjust;

    SDL_PixelFormat screen_pixel_format;

    bool _show_stars;

    unsigned int _explosion_animation_timer;
    int _explosion_animation_pos;
    int _debug_msg_pos;
    st_position _screen_resolution_adjust;
    Uint8 _video_filter;                                     // copy from game_options, so graphlib isn't affected when the option is changed while game running
    long _timer;
    st_color color_keys[3];                                 // holds the 3 color-keys we use for changing colors


#ifdef PSP
    psp_ram _ram_counter;
    //std::cout << "unload_stage::RAM::BF='" << ram_counter.ramAvailable() << "'" << std::endl;
#endif
};

#endif // GRAPHICSLIB_H
