#ifndef ST_COMMON_H
#define ST_COMMON_H

// some small structures that do not need a separated file for each one
#include <SDL/SDL.h>				//Include da SDL
#include <vector>
#include <iostream>
#include <stdio.h>

#include "defines.h"

/**
 * @brief
 *
 */
struct st_position {
    Sint16 x;
    Sint16 y;
	st_position() {
		x = 0;
		y = 0;
	}
/**
 * @brief
 *
 * @param setX
 * @param setY
 */
    st_position (int setX, int setY) {
		x = setX;
		y = setY;
	}
    /**
     * @brief
     *
     * @param set_pt
     * @return st_position &operator
     */
    st_position& operator=(const st_position &set_pt)
	{
		x = set_pt.x;
		y = set_pt.y;
		return *this;
	}
    /**
     * @brief
     *
     * @param comp_pt
     * @return bool operator
     */
    bool operator==(const st_position &comp_pt) const
	{
		if (x == comp_pt.x && y == comp_pt.y) {
			return true;
		}
		return false;
	}
};



/**
 * @brief
 *
 */
struct st_float_position {
    float x;
    float y;
    st_float_position() {
        x = 0.0;
        y = 0.0;
    }
/**
 * @brief
 *
 * @param setX
 * @param setY
 */
    st_float_position (float setX, float setY) {
        x = setX;
        y = setY;
    }
    /**
     * @brief
     *
     * @param set_pt
     * @return st_position &operator
     */
    st_float_position& operator=(const st_position &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    /**
     * @brief
     *
     * @param comp_pt
     * @return bool operator
     */
    bool operator==(const st_position &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
    /**
     * @brief
     *
     * @param set_pt
     * @return st_position &operator
     */
    st_float_position& operator=(const st_float_position &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    /**
     * @brief
     *
     * @param comp_pt
     * @return bool operator
     */
    bool operator==(const st_float_position &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
};

/**
 * @brief
 *
 */
struct st_size {
    Sint16 width;
    Sint16 height;
	st_size()
	{
		width = 0;
        height = 0;
	}
/**
 * @brief
 *
 * @param w
 * @param h
 */
    st_size(int w, int h)
	{
		width = w;
        height = h;
	}
};


/**
 * @brief
 *
 */
struct st_rectangle {
    Sint16 x;
    Sint16 y;
    Sint16 w;
    Sint16 h;
	st_rectangle () {
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

    st_rectangle (int setX, int setY, int setW, int setH) {
		x = setX;
		y = setY;
		w = setW;
		h = setH;
	}
};

struct st_color {
    Sint16 r;
    Sint16 g;
    Sint16 b;
	st_color () {
		r = -1;
		g = -1;
		b = -1;
	}

    st_color (int setR, int setG, int setB) {
		r = setR;
		g = setG;
		b = setB;
	}
};

// structure that holds a graphic surface or memory section
/**
 * @brief
 *
 */
struct graphicsLib_gSurface {
    private:
        SDL_Surface *gSurface;


        SDL_Color get_pixel_color(Uint32 pixel) const {
            if (!gSurface) {
                return SDL_Color();
            }
            /*
            if (x >= gSurface->w || y >= gSurface->h) {
                return SDL_Color();
            }
            */

            SDL_Color theKey;
            //Uint32 pixel = ((Uint32*)gSurface->pixels)[y * gSurface->pitch/4 + x];
            SDL_GetRGB(pixel, gSurface->format, &theKey.r, &theKey.g, &theKey.b);

            return theKey;
        }

    public:
        st_color get_point_color(int x, int y) {
            Uint32 pixel = this->get_pixel(x, y);
            SDL_Color px_color = get_pixel_color(pixel);
            /*
            if (pixel != 0) {
                std::cout << "pixel[" << pixel << "]: [" << (int)px_color.r << "][" << (int)px_color.g << "][" << (int)px_color.b << "]" << std::endl;
            }
            */
            return st_color((int)px_color.r, (int)px_color.g, (int)px_color.b);
        }

        void set_point_color(int set_x, int set_y, int set_r, int set_g, int set_b) {
            /*
            if (set_r != 0 && set_g != 0 && set_b != 0) {
                std::cout << "set_point_color[" << set_x << "][" << set_y << "]: [" << set_r << "][" << set_g << "][" << set_b << "]" << std::endl;
            }
            */
            if (gSurface == NULL || gSurface->format == NULL) {
                return;
            }
            Uint32 new_color_n = SDL_MapRGB(gSurface->format, set_r, set_g, set_b);
            put_pixel(set_x, set_y, new_color_n);
        }


    public:
        int width;
        int height;
        bool persistent;
        bool video_screen; /**< video-screen is freed by SDL_Quit, so we must not free it mannually */
        std::vector<st_position> colorkey1_points;
        std::vector<st_position> colorkey2_points;
        std::vector<st_position> colorkey3_points;
        bool show_debug;

        graphicsLib_gSurface()
        {
            gSurface = NULL;
            width = 0;
            height = 0;
            persistent = false;
            video_screen = false;
            show_debug = false;
        }
        // copy CONSTRUCTOR
        graphicsLib_gSurface(const graphicsLib_gSurface &original)
        {

            if (original.gSurface == NULL) {
                gSurface = NULL;
                width = 0;
                height = 0;
                persistent = false;
                video_screen = original.video_screen;
                colorkey1_points = original.colorkey1_points;
                colorkey2_points = original.colorkey2_points;
                colorkey3_points = original.colorkey3_points;
                show_debug = false;
            } else {
                width = original.width;
                height = original.height;
                persistent = false;
                video_screen = original.video_screen;
                colorkey1_points = original.colorkey1_points;
                colorkey2_points = original.colorkey2_points;
                colorkey3_points = original.colorkey3_points;
                show_debug = false;
                if (original.width > 0) {
                    // copy surface
                    gSurface = SDL_DisplayFormatAlpha(original.gSurface);
                } else {
                    gSurface = NULL;
                }
            }
        }

        // assign constructor
        graphicsLib_gSurface& operator=(const graphicsLib_gSurface& original)
        {

            if (original.gSurface == NULL) {
                gSurface = NULL;
                width = 0;
                height = 0;
                persistent = false;
                video_screen = original.video_screen;
                colorkey1_points = original.colorkey1_points;
                colorkey2_points = original.colorkey2_points;
                colorkey3_points = original.colorkey3_points;
            } else {
                width = original.width;
                height = original.height;
                persistent = false;
                video_screen = original.video_screen;
                colorkey1_points = original.colorkey1_points;
                colorkey2_points = original.colorkey2_points;
                colorkey3_points = original.colorkey3_points;
                show_debug = false;
                if (original.width > 0) {
                    // copy surface
                    gSurface = SDL_DisplayFormatAlpha(original.gSurface);
                } else {
                    gSurface = NULL;
                }
            }
        }

        ~graphicsLib_gSurface()
        {
            setbuf(stdout, NULL);
            freeGraphic();
            colorkey1_points.clear();
            colorkey2_points.clear();
            colorkey3_points.clear();
        }


        bool is_on_tolerance(SDL_Color pixel_color, int r, int g, int b, int tolerance) {
            if (pixel_color.r < r-tolerance || pixel_color.r > r+tolerance) {
                return false;
            }
            if (pixel_color.g < g-tolerance || pixel_color.g > g+tolerance) {
                return false;
            }
            if (pixel_color.b < b-tolerance || pixel_color.b > b+tolerance) {
                return false;
            }
            return true;
        }


        std::vector<st_position> get_color_points(int r, int g, int b) {
            std::vector<st_position> res;

            /*
            if (show_debug) {
                std::cout << ">>>>>> get_color_points::START <<<<<<" << std::endl;
            }
            */

            for (int tolerance=0; tolerance<=6; tolerance++) {
                for (Sint16 y=0; y<gSurface->h; y++) {
                    for (Sint16 x=0; x<gSurface->w; x++) {
                        Uint32 pixel = get_pixel(x, y);
                        SDL_Color pixel_color = get_pixel_color(pixel);


                        //std::cout << "r[" << (int)r << "], point.r[" << (int)pixel_color.r << ", g[" << (int)g << "], point.g[" << (int)pixel_color.g << ", b[" << (int)b << "], point.b[" << (int)pixel_color.b << "]" << std::endl;

                        // ignore colorkey [74][125][123]/[75][125][125]
                        /*
                        if (!((int)pixel_color.r == 0 && (int)pixel_color.g == 0 && (int)pixel_color.b == 0)) {
                            if ((int)pixel_color.r != 74 || (int)pixel_color.g != 125 || (int)pixel_color.b != 123) {
                                std::cout << "px[" << x << "][" << y << "]: [" << (int)pixel_color.r << "][" << (int)pixel_color.g << "][" << (int)pixel_color.b << "]" << std::endl;
                            }
                        }
                        */

                        if (is_on_tolerance(pixel_color, r, g, b, tolerance) == true) {
                            st_position pos = st_position(x, y);
                            res.push_back(pos);
                        }
                    }
                }
                if (res.size() > 0) {
                    break;
                }
            }

            //std::cout << ">>>>>> get_color_points::END <<<<<<" << std::endl;

            return res;
        }

        void init_colorkeys() {

            SDL_LockSurface(gSurface);

            colorkey1_points = get_color_points(COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
            colorkey2_points = get_color_points(COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
            colorkey3_points = get_color_points(COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

            SDL_UnlockSurface(gSurface);

        }

        void set_surface(SDL_Surface *surface) {
            // free old surface memory
            if (gSurface != NULL) {
                try {
                    SDL_FreeSurface(gSurface);
                } catch (std::exception e) {}
            }
            if (surface != NULL) {
                gSurface = surface;
            } else {
                gSurface = NULL;
            }
        }

        void change_colorkey_color(Sint8 key_n, st_color new_color) {
            if (gSurface == NULL || gSurface->format == NULL) {
                return;
            }

            if (key_n < 0 || key_n > 3) { // we have only 3 color-keys, ignore everything else
                if (show_debug == true) {
                    std::cout << "change_colorkey_color LEAVE #1, key_n: " << (int)key_n << std::endl;
                }
                return;
            }

            Uint32 new_color_n = SDL_MapRGB(gSurface->format, new_color.r, new_color.g, new_color.b);

            //std::cout << "change_colorkey_color, key_n[" << (int)key_n << "], new_color_n[" << (int)new_color_n << "]" << std::endl;


            //std::cout << "change_colorkey_color - colorkey1_points.size[" << colorkey1_points.size() << "], colorkey2_points.size[" << colorkey2_points.size() << "], colorkey3_points.size[" << colorkey3_points.size() << "]" << std::endl;

            if (key_n == 0) {
                for (int i=0; i<colorkey1_points.size(); i++) {
                    put_pixel(colorkey1_points.at(i).x, colorkey1_points.at(i).y, new_color_n);
                }
            } else if (key_n == 1) {
                for (int i=0; i<colorkey2_points.size(); i++) {
                    put_pixel(colorkey2_points.at(i).x, colorkey2_points.at(i).y, new_color_n);
                }
            } else {
                for (int i=0; i<colorkey3_points.size(); i++) {
                    put_pixel(colorkey3_points.at(i).x, colorkey3_points.at(i).y, new_color_n);
                }
            }
        }

        void enable_show_debug() {
            show_debug = true;
        }


        SDL_Surface *get_surface() const {
            return gSurface;
        }

        void freeGraphic()
        {
            if (width > 0 && width <= 3200) { // 3200 check is to handle invalid projectiles (trash in memory)
                if (video_screen == false && gSurface != NULL) {
                    //std::cout << "GSURFACE::freeGraphic - w: " << width << std::endl;
                    width = -1;
                    height = -1;
                    SDL_FreeSurface(gSurface);
                }
            }
            gSurface = NULL;
        }


        bool is_null() {
            if (width <= 0 || height <= 0) {
                std::cout << "GSURFACE - invalid size[" << width << "][" << height << "]" << std::endl;
                return true;
            }
            if (gSurface == NULL) {
                std::cout << "GSURFACE - SDL-Surface is NULL" << std::endl;
                return true;
            }
        }


        Uint8 get_pixel_8bpp(Sint16 x, Sint16 y) {
            if (gSurface == NULL || gSurface->format == NULL) {
                return 0;
            }
            if (x >= gSurface->w || y >= gSurface->h) {
                return 0;
            }

            int bpp = gSurface->format->BytesPerPixel;

            /* Here p is the address to the pixel we want to retrieve */
            Uint8 *p = (Uint8 *) gSurface->pixels + y * gSurface->pitch + x * bpp;
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }

        }



        Uint32 get_pixel(Sint16 x, Sint16 y)
        {
            if (gSurface == NULL || gSurface->format == NULL) {
                return 0;
            }
            if (x >= gSurface->w || y >= gSurface->h) {
                return 0;
            }

            int bpp = gSurface->format->BytesPerPixel;

            /* Here p is the address to the pixel we want to retrieve */
            Uint8 *p = (Uint8 *) gSurface->pixels + y * gSurface->pitch + x * bpp;
            switch (bpp) {
            case 1:
                return *p;
            case 2:
                return *(Uint16 *) p;
            case 3:
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                    return p[0] << 16 | p[1] << 8 | p[2];
                else
                    return p[0] | p[1] << 8 | p[2] << 16;
            case 4:
                return *(Uint32 *) p;
            default:
                return 0;               /* shouldn't happen, but avoids warnings */
            }
        }

        void put_pixel(int x, int y, Uint32 pixel)
        {
            if (gSurface == NULL || gSurface->format == NULL) {
                return;
            }
            int bpp = gSurface->format->BytesPerPixel;
            /* Here p is the address to the pixel we want to set */
            Uint8 *p = (Uint8 *)gSurface->pixels + y * gSurface->pitch + x * bpp;

            switch(bpp) {
            case 1:
                *p = pixel;
                break;

            case 2:
                *(Uint16 *)p = pixel;
                break;

            case 3:
                if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    p[0] = (pixel >> 16) & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = pixel & 0xff;
                } else {
                    p[0] = pixel & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = (pixel >> 16) & 0xff;
                }
                break;

            case 4:
                *(Uint32 *)p = pixel;
                break;
            }
        }
};


/**
 * @brief
 *
 */
struct used_teleporter {
    Sint8 teleporter_n; // number of the map_link (this is hardcoded, must be rebuilt in 0.2)
    bool finished; // indicates if already used
    struct st_position old_player_pos;
    bool active; // if true, then after beating the boss, the player must be teleported back to the teleporter origin
    st_float_position old_map_scroll;
    Sint8 old_map_n;
    bool is_object;
	used_teleporter() {
        teleporter_n = -1;
		finished = false;
		old_player_pos.x = 0;
		old_player_pos.y = 0;
		active = false;
		old_map_scroll.x = 0;
		old_map_scroll.y = 0;
		old_map_n = 0;
        is_object = false;
	}
};




struct st_position_int8 {
    Sint8 x;
    Sint8 y;
    st_position_int8() {
        x = 0;
        y = 0;
    }
    st_position_int8 (int setX, int setY) {
        x = setX;
        y = setY;
    }
    st_position_int8& operator=(const st_position_int8 &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const st_position_int8 &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
    bool operator!=(const st_position_int8 &comp_pt) const
    {
        if (x != comp_pt.x || y != comp_pt.y) {
            return true;
        }
        return false;
    }
};


struct st_position_uint8 {
    Uint8 x;
    Uint8 y;
    st_position_uint8() {
        x = 0;
        y = 0;
    }
    st_position_uint8 (int setX, int setY) {
        x = setX;
        y = setY;
    }
    st_position_uint8& operator=(const st_position_uint8 &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const st_position_uint8 &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
};


struct st_size_int8 {
    Sint8 width;
    Sint8 height;
    st_size_int8()
    {
        width = 0;
        height = 0;
    }
    st_size_int8(int w, int h)
    {
        width = w;
        height = h;
    }
};

struct st_map_collision {
    Sint8 block;
    Sint8 terrain_type;
    st_map_collision() {
        block = 0;
        terrain_type = 0;
    }
    st_map_collision(Sint8 set_block, Sint8 set_terrain_type) {
        block = set_block;
        terrain_type = set_terrain_type;
    }
};


struct st_menu_option {
    std::string text;
    bool disabled;

    st_menu_option(std::string line) {
        text = line;
        disabled = false;
    }

    st_menu_option(std::string line, bool set_disabled) {
        text = line;
        disabled = set_disabled;
    }
};

struct st_input_button_config {
    int type;                                           // use joystick_input_types
    int value;                                          // stores button-n or hat-n or witch axis (0 horizontal1, 1 vertical 1, 2 hoz 2...)
    int axis_type;                                      // only for axis, tell is positive (+1) or negative (-1) movement
    st_input_button_config() {
        type = JOYSTICK_INPUT_TYPE_BUTTON;
        value = -1;
        axis_type = 0;
    }
};


#endif // ST_COMMON_H
