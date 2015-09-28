#ifndef ST_COMMON_H
#define ST_COMMON_H

// some small structures that do not need a separated file for each one
#include <SDL/SDL.h>				//Include da SDL

/**
 * @brief
 *
 */
struct st_position {
    short x; /**< TODO */
    short y; /**< TODO */
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
    float x; /**< TODO */
    float y; /**< TODO */
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
    short width; /**< TODO */
    short height; /**< TODO */
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
    short x; /**< TODO */
    short y; /**< TODO */
    short w; /**< TODO */
    short h; /**< TODO */
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
    short r; /**< TODO */
    short g; /**< TODO */
    short b; /**< TODO */
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
    SDL_Surface *gSurface; /**< TODO */
    int width; /**< TODO */
    int height; /**< TODO */
    bool persistent; /**< TODO */
    bool video_screen; /**< video-screen is freed by SDL_Quit, so we must not free it mannually */
	graphicsLib_gSurface()
	{
		gSurface = NULL;
		width = 0;
		height = 0;
		persistent = false;
        video_screen = false;
	}
	// copy handler
/**
 * @brief
 *
 * @param original
 */
    graphicsLib_gSurface(const graphicsLib_gSurface &original)
	{
		//std::cout << "graphicsLib_gSurface::COPY" << std::endl;
		width = original.width;
		height = original.height;
		persistent = false;
        video_screen = original.video_screen;
		if (original.width > 0) {
			gSurface = SDL_DisplayFormat(original.gSurface);
			//SDL_FreeSurface(original.gSurface);
		} else {
			gSurface = NULL;
		}
	}
    /**
     * @brief
     *
     */
    ~graphicsLib_gSurface()
	{
        freeGraphic();
	}
    /**
     * @brief
     *
     */
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
    short teleporter_n; // number of the map_link (this is hardcoded, must be rebuilt in 0.2) /**< TODO */
    bool finished; // indicates if already used /**< TODO */
    struct st_position old_player_pos; /**< TODO */
    bool active; // if true, then after beating the boss, the player must be teleported back to the teleporter origin /**< TODO */
    st_position old_map_scroll; /**< TODO */
    short old_map_n; /**< TODO */
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
    Sint8 width; /**< TODO */
    Sint8 height; /**< TODO */
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
