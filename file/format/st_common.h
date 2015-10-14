#ifndef ST_COMMON_H
#define ST_COMMON_H

// some small structures that do not need a separated file for each one
#include <SDL/SDL.h>				//Include da SDL
#include <vector>
#include <iostream>

#include "defines.h"

/**
 * @brief
 *
 */
struct st_position {
    short x;
    short y;
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
        x = 1;
        y = 1;
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
    short width;
    short height;
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
    short x;
    short y;
    short w;
    short h;
	st_rectangle () {
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

/**
 * @brief
 *
 * @param setX
 * @param setY
 * @param setW
 * @param setH
 */
    st_rectangle (int setX, int setY, int setW, int setH) {
		x = setX;
		y = setY;
		w = setW;
		h = setH;
	}
};

/**
 * @brief
 *
 */
struct st_color {
    int r;
    int g;
    int b;
	st_color () {
		r = -1;
		g = -1;
		b = -1;
	}
/**
 * @brief
 *
 * @param setR
 * @param setG
 * @param setB
 */
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


    SDL_Color get_pixel_color(Sint16 x, Sint16 y) const {
        if (!gSurface) {
            return SDL_Color();
        }
        if (x >= gSurface->w || y >= gSurface->h) {
            return SDL_Color();
        }

        SDL_Color theKey;
        Uint32 pixel = ((Uint32*)gSurface->pixels)[y * gSurface->pitch/4 + x];
        SDL_GetRGB(pixel, gSurface->format, &theKey.r, &theKey.g, &theKey.b);

        return theKey;
    }

    Uint32 get_pixel(Sint16 x, Sint16 y)
    {
        if (!gSurface) {
            return 0;
        }
        if (x >= gSurface->w || y >= gSurface->h) {
            return 0;
        }

        /*
        Uint32 pixel = ((Uint32*)gSurface->pixels)[y * gSurface->pitch/4 + x];
        return pixel;
        */


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
		//std::cout << "graphicsLib_gSurface::COPY" << std::endl;
		width = original.width;
		height = original.height;
		persistent = false;
        video_screen = original.video_screen;
        colorkey1_points = original.colorkey1_points;
        colorkey2_points = original.colorkey2_points;
        colorkey3_points = original.colorkey3_points;
        show_debug = false;
        if (original.width > 0) {
			gSurface = SDL_DisplayFormat(original.gSurface);
			//SDL_FreeSurface(original.gSurface);
		} else {
			gSurface = NULL;
		}
	}
    ~graphicsLib_gSurface()
	{
        freeGraphic();
	}


    void init_colorkeys() {

        Uint32 key_n1 = SDL_MapRGB(gSurface->format, COLORKEY1_R, COLORKEY1_G, COLORKEY1_B);
        Uint32 key_n2 = SDL_MapRGB(gSurface->format, COLORKEY2_R, COLORKEY2_G, COLORKEY2_B);
        Uint32 key_n3 = SDL_MapRGB(gSurface->format, COLORKEY3_R, COLORKEY3_G, COLORKEY3_B);

        SDL_LockSurface(gSurface);

        for (Sint16 y=0; y<gSurface->h; y++) {
            for (Sint16 x=0; x<gSurface->w; x++) {
                Uint32 pixel = get_pixel(x, y);
                if (pixel == key_n1) {
                    st_position pos = st_position(x, y);
                    colorkey1_points.push_back(pos);
                } else if (pixel == key_n2) {
                    //if (show_debug == true) std::cout << "key_n2.add" << std::endl;
                    st_position pos = st_position(x, y);
                    colorkey2_points.push_back(pos);
                } else if (pixel == key_n3) {
                    //if (show_debug == true) std::cout << "key_n3.add" << std::endl;
                    st_position pos = st_position(x, y);
                    colorkey3_points.push_back(pos);
                }
            }
        }

        SDL_UnlockSurface(gSurface);

    }

    void set_surface(SDL_Surface *surface) {
        if (gSurface != NULL && surface == NULL) {
            try {
                SDL_FreeSurface(gSurface);
            } catch (std::exception e) {}
        }
        gSurface = surface;
        // store colorkey values into lists so we can change colors later using the lists
        /*
        if (gSurface) {
            init_colorkeys();
        }
        */
    }

    void change_colorkey_color(Sint8 key_n, st_color new_color) {
        if (key_n < 0 || key_n > 3) { // we have only 3 color-keys, ignore everything else
            if (show_debug == true) std::cout << "change_colorkey_color LEAVE #1, key_n: " << (int)key_n << std::endl;
            return;
        }
        if (!gSurface) {
            if (show_debug == true) std::cout << "change_colorkey_color LEAVE #2" << std::endl;
            return;
        }
        Uint32 new_color_n = SDL_MapRGB(gSurface->format, new_color.r, new_color.g, new_color.b);

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
			width = -1;
			height = -1;
            if (video_screen == false && gSurface != NULL) {
                SDL_FreeSurface(gSurface);
            }
        }
		gSurface = NULL;
	}
};


/**
 * @brief
 *
 */
struct used_teleporter {
    short teleporter_n; // number of the map_link (this is hardcoded, must be rebuilt in 0.2)
    bool finished; // indicates if already used
    struct st_position old_player_pos;
    bool active; // if true, then after beating the boss, the player must be teleported back to the teleporter origin
    st_position old_map_scroll;
    short old_map_n;
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

struct st_map_colision {
    short block;
    short terrain_type;
    st_map_colision() {
        block = 0;
        terrain_type = 0;
    }
    st_map_colision(short set_block, short set_terrain_type) {
        block = set_block;
        terrain_type = set_terrain_type;
    }
};


#endif // ST_COMMON_H
