#define B sp[i - b]
#define D sp[i - (i>0?1:0)]
#define F sp[i + (i<wd?1:0)]
#define H sp[i + h]
#define E  sp[i]
#define E0 tp[i*2]
#define E1 tp[i*2 + 1]
#define E2 tp[i*2 + tpitch]
#define E3 tp[i*2 + 1 + tpitch]

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;


#include "defines.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

#include "inputlib.h"
extern inputLib input;

#include "timerlib.h"
extern timerLib timer;

#include "soundlib.h"
extern soundLib soundManager;


#include "graphicslib.h"

#include "file/format.h"
extern CURRENT_FILE_FORMAT::st_save game_save;

extern CURRENT_FILE_FORMAT::file_game game_data;

extern CURRENT_FILE_FORMAT::st_game_config game_config;

extern graphicsLib_gSurface _explosion_surface;



#include "file/file_io.h"

#include "class_config.h"

#include "strings_map.h"


graphicsLib::graphicsLib() : _show_stars(false), game_screen(NULL), _explosion_animation_timer(0), _explosion_animation_pos(0), _timer(0)
{

	tileset = NULL;
	stars_timer = 0;


	RES_DIFF_W = 0;
	RES_DIFF_H = 0;
    _debug_msg_pos = 0;

    color_keys[0].r = 55;
    color_keys[0].g = 255;
    color_keys[0].b = 0;

    color_keys[1].r = 255;
    color_keys[1].g = 0;
    color_keys[1].b = 255;

    color_keys[2].r = 0;
    color_keys[2].g = 255;
    color_keys[2].b = 255;

}

graphicsLib::~graphicsLib()
{
}


bool graphicsLib::initGraphics()
{
	string filename;
    _video_filter = game_config.video_filter;

#ifdef DREAMCAST
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 ) {
        std::cout << "SDL Error" << std::endl;
        std::cout << "Unable to init SDL. Error: " << SDL_GetError() << std::endl;
        std::fflush(stdout);
		exit(-1);
    }
#else
	// GRAPHIC LIB
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER|SDL_INIT_AUDIO) < 0 ) {
        std::cout << "SDL Error" << std::endl;
        std::cout << "Unable to init SDL. Error: " << SDL_GetError() << std::endl;
        std::fflush(stdout);
		exit(-1);
    }
#endif
	atexit(SDL_Quit);


    printf("INFO: There are %d joysticks attached\n", SDL_NumJoysticks());
#if defined(PLAYSTATION2) || defined(WII)
    if (SDL_NumJoysticks() <= 0) {
        std::cout << "No joysticks found" << std::endl;
        exit(-1);
    }
#endif
	input.init_joystick();
	// FONT
	TTF_Init();
    filename = GAMEPATH + "/fonts/pressstart2p.ttf";

	char *buffer = new char[filename.size()+1];
	std::strcpy(buffer, filename.c_str());
	SDL_RWops *fileRW;
	fileRW = SDL_RWFromFile(buffer, "rb");
	if (!fileRW) {
		printf("ERROR::initGraphics - could not open '%s' font\n", buffer);
        delete[] buffer;
		return false;
	} else {
        font = TTF_OpenFontRW(fileRW, 1, FONT_SIZE);
    }
    delete[] buffer;

	// GAME SCREEN
	SDL_ShowCursor( SDL_DISABLE );
#ifdef PC
    SDL_WM_SetCaption("Rockbot Game", "Rockbot Game");
#endif
    set_video_mode();
#ifdef PC
    std::string icon_filename = FILEPATH + "images/faces/rockbot.png";
    SDL_RWops *rwop = SDL_RWFromFile(icon_filename.c_str(), "rb");
    if (rwop) {
        SDL_Surface* icon_img = IMG_Load_RW(rwop, 1);
        if (icon_img != NULL) {
            SDL_WM_SetIcon(icon_img, NULL);
        }
    } else {
        std::cout << "graphicsLib::initGraphics(set-window-icon): rwop is NULL " << std::endl;
    }
#endif
	// other loading methods


    return true;
}

void graphicsLib::preload()
{
    load_icons();
    loadTileset("default.png");
    preload_faces();
    preload_images();
    preload_anim_tiles();
}


void graphicsLib::updateScreen()
{
	if (!game_screen) {
        std::cout << "FATAL-ERROR::updateScreen game_screen is NULL\n";
		return;
    }


    if (_show_stars == true) {
        anim_stars(); /// @TODO - move to draw class
    }


    // clear non-used areas
    if (game_screen->w > RES_W) {
        clear_area_no_adjust(0, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0);
        clear_area_no_adjust(_screen_resolution_adjust.x+RES_W, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0);
    }
    if (game_screen->h > RES_H) {
        clear_area_no_adjust(0, 0, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0);
        clear_area_no_adjust(0, _screen_resolution_adjust.y + RES_H, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0);
    }


    if (_video_filter == VIDEO_FILTER_NOSCALE) {
        //std::cout << "GRAPH::updateScreen NO SCALE" << std::endl;
        SDL_Flip(game_screen);
    } else {
        //std::cout << "GRAPH::updateScreen SCALE, _video_filter: " << (int)_video_filter << std::endl;
        if (_video_filter == VIDEO_FILTER_BITSCALE) {
            scale2x(game_screen, game_screen_scaled, false);
        } else if (_video_filter == VIDEO_FILTER_SCALE2x) {
            scale2x(game_screen, game_screen_scaled, true);
        } else {
            copySDLArea(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), game_screen, game_screen_scaled, true);
        }
        SDL_Flip(game_screen_scaled);
    }
}





SDL_Surface *graphicsLib::SDLSurfaceFromFile(string filename)
{
	SDL_RWops *rwop;
	SDL_Surface *spriteCopy;
	SDL_Surface *display_surface;

    rwop=SDL_RWFromFile(filename.c_str(), "rb");

    if (!rwop) {
        std::cout << "DEBUG.loadSpriteFile - Error in graphicsLib::SDLSurfaceFromFile - file '" << filename << "' not found\n";
		return NULL;
	}
	spriteCopy = IMG_Load_RW(rwop, 1);
    if (spriteCopy == NULL) {
        std::cout << "[graphicsLib::SDLSurfaceFromFile] Error on IMG_Load_RW, could not load image '" << filename << "'" << std::endl;
        printf("IMG_LoadPNG_RW: %s\n", IMG_GetError());
    }

    return spriteCopy;
}


void graphicsLib::surfaceFromFile(string filename, struct graphicsLib_gSurface* res)
{
    if (filename == "/home/iuri/Desenvolvimento/rockbot/build//games/Rockbot2/images/projectiles/projectile_normal.png") {
        std::cout << "DEBUG #1" << std::endl;
    }
    if (res == NULL) {
        return;
    }
    res->freeGraphic();
    res->set_surface(SDLSurfaceFromFile(filename));

    if (res->get_surface() == NULL) {
        std::cout << "surfaceFromFile - error loading file: '" << filename << "'" << std::endl;
        _debug_msg_pos = 1;
        show_debug_msg(filename);
        _debug_msg_pos = 0;
        printf("graphicsLib::surfaceFromFile - error loading SDL surface from file '%s'.\n", filename.c_str());
        fflush(stdout);
        timer.delay(1000);
        show_debug_msg("EXIT #05");
        exit(-1);
	} else {
        //std::cout << "surfaceFromFile - file: '" << filename << "'" << std::endl;
        res->width = res->get_surface()->w;
        res->height = res->get_surface()->h;
	}
}

void graphicsLib::loadTileset(std::string file)
{
    string filename = FILEPATH + "images/tilesets/" + file;

    if (tileset != NULL) {
        SDL_FreeSurface(tileset);
    }

	tileset = SDLSurfaceFromFile(filename);
	if (tileset == NULL) {
        cout << "ERROR::GRAPHLIB::loadTileset: Could not find file '" << filename << "'\n";
        show_debug_msg("EXIT #06");
		exit(-1);
	}
}


void graphicsLib::copySDLArea(struct st_rectangle origin_rectangle, struct st_position destiny_pos, SDL_Surface* surfaceOrigin, SDL_Surface* surfaceDestiny, bool fix_colors=true)
{
    UNUSED(fix_colors);
    if (!surfaceDestiny) {
        std::cout << "copySDLArea - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.1");
        exit(-1);
    }
    copySDLPortion(origin_rectangle, st_rectangle(destiny_pos.x, destiny_pos.y, origin_rectangle.w, origin_rectangle.h), surfaceOrigin, surfaceDestiny);
}

void graphicsLib::copySDLPortion(st_rectangle original_rect, st_rectangle destiny_rect, SDL_Surface *surfaceOrigin, SDL_Surface *surfaceDestiny)
{
    SDL_Rect src, dest;
    src.x = original_rect.x;
    src.y = original_rect.y;
    src.w = original_rect.w;
    src.h = original_rect.h;
    dest.x = destiny_rect.x;
    dest.y = destiny_rect.y;
    dest.w = destiny_rect.w;
    dest.h = destiny_rect.h;

    if (!surfaceOrigin) {
        cout << "copySDLArea - ERROR surfaceOrigin is NULL\n";
        show_debug_msg("EXIT #20");
        exit(-1);
    }
    if (!surfaceDestiny) {
        std::cout << "copySDLPortion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21");
        exit(-1);
    }

    if (surfaceDestiny == game_screen) { // if painting on game_screen, use position adjusts
        dest.x += _screen_resolution_adjust.x;
        dest.y += _screen_resolution_adjust.y;
        //std::cout << "graphicsLib::copySDLPortion - dest.x: " << dest.x << ", dest.w: " << dest.w << std::endl;
    }

    //std::cout << ">> graphicsLib::copySDLPortion- origin.w: " << surfaceOrigin->w << ", origin.h: " << surfaceOrigin->h << ", src.x: " << src.x << ", src.y: " << src.y << std::endl;

    SDL_BlitSurface(surfaceOrigin, &src, surfaceDestiny, &dest);
}

void graphicsLib::copy_gamescreen_area(st_rectangle origin_rectangle, st_position pos, graphicsLib_gSurface *surfaceDestiny)
{
    origin_rectangle.x += _screen_resolution_adjust.x;
    origin_rectangle.y += _screen_resolution_adjust.y;
    copyArea(origin_rectangle, pos, &gameScreen, surfaceDestiny);
}

void graphicsLib::copyArea(struct st_rectangle origin_rectangle, struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny)
{
    if (!surfaceDestiny->get_surface()) {
        std::cout << "copyArea - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.3");
        exit(-1);
    }
    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}


void graphicsLib::copyArea(struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny)
{
    if (!surfaceDestiny->get_surface()) {
        std::cout << "copyArea - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.4");
        exit(-1);
    }
    st_rectangle origin_rectangle(0, 0, surfaceOrigin->width, surfaceOrigin->height);
    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}

void graphicsLib::copyAreaWithAdjust(struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny)
{
    if (!surfaceDestiny->get_surface()) {
        std::cout << "copyAreaWithAdjust - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.4");
        exit(-1);
    }
    int w = surfaceOrigin->width;
    int h = surfaceOrigin->height;
    st_rectangle origin_rectangle(0, 0, w, h);
    pos.x += _screen_adjust.x;

    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}

void graphicsLib::placeTile(struct st_position pos_origin, struct st_position pos_destiny, struct graphicsLib_gSurface* gSurface)
{
    if (!gSurface->get_surface()) {
        std::cout << "placeTile - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.5");
        exit(-1);
    }
    struct st_rectangle origin_rectangle;

	origin_rectangle.x = pos_origin.x * TILESIZE;
	origin_rectangle.y = pos_origin.y * TILESIZE;

	origin_rectangle.w = TILESIZE;
    origin_rectangle.h = TILESIZE;


    pos_destiny.x += _screen_adjust.x;
    copySDLArea(origin_rectangle, pos_destiny, tileset, gSurface->get_surface());
}

void graphicsLib::place_easymode_block_tile(st_position destiny, graphicsLib_gSurface& surface)
{
    destiny.x += _screen_adjust.x;

    copySDLArea(st_rectangle(0, 0, TILESIZE, TILESIZE), destiny, _easymode_block.get_surface(), surface.get_surface());
}

void graphicsLib::place_hardmode_block_tile(st_position destiny, graphicsLib_gSurface &surface)
{
    destiny.x += _screen_adjust.x;
    copySDLArea(st_rectangle(0, 0, TILESIZE, TILESIZE), destiny, _hardmode_block.get_surface(), surface.get_surface());
}

void graphicsLib::place_anim_tile(int anim_tile_id, st_position pos_destiny, struct graphicsLib_gSurface* dest_surface)
{
    if (anim_tile_id >= ANIM_TILES_SURFACES.size()) {
        std::cout << "place_anim_tile - ERROR Invalid anim-tile-id: " << anim_tile_id << " - ignoring..." << std::endl;
        return;
    }
    if (!ANIM_TILES_SURFACES.at(anim_tile_id).get_surface()) {
        std::cout << "place_anim_tile - ERROR surfaceDestiny is NULL for id " << anim_tile_id << " - ignoring..." << std::endl;
        show_debug_msg("EXIT place_anim_tile");
        exit(-1);
    }

    struct st_rectangle origin_rectangle;

    origin_rectangle.x = ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos * TILESIZE;
    origin_rectangle.y = 0;
    origin_rectangle.w = TILESIZE;
    origin_rectangle.h = TILESIZE;

    pos_destiny.x += _screen_adjust.x;

    copySDLArea(origin_rectangle, pos_destiny, ANIM_TILES_SURFACES.at(anim_tile_id).get_surface(), dest_surface->get_surface());

}

void graphicsLib::update_anim_tiles_timers()
{
    for (int anim_tile_id=0; anim_tile_id<ANIM_TILES_TIMERS.size(); anim_tile_id++) {
        if (ANIM_TILES_TIMERS.at(anim_tile_id).timer < timer.getTimer()) {
            ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos++;
            if (ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos >= ANIM_TILES_TIMERS.at(anim_tile_id).max_frames) {
                ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos = 0;
            }
            ANIM_TILES_TIMERS.at(anim_tile_id).timer = timer.getTimer() + ANIM_TILES_TIMERS.at(anim_tile_id).frames_delay[ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos];
        }
    }
}

void graphicsLib::place_3rd_level_tile(int origin_x, int origin_y, int dest_x, int dest_y)
{
    st_position pos_destiny(dest_x+_screen_adjust.x, dest_y);
    if (origin_x < -1) {
        int anim_tile_id = (origin_x * -1) - 2;
        place_anim_tile(anim_tile_id, pos_destiny, &graphLib.gameScreen);
        return;
    }

    struct st_rectangle origin_rectangle(origin_x*TILESIZE, origin_y*TILESIZE, TILESIZE, TILESIZE);
    copySDLArea(origin_rectangle, pos_destiny, tileset, game_screen);
}



void graphicsLib::showSurface(struct graphicsLib_gSurface* surfaceOrigin)
{
    if (!game_screen) {
        std::cout << "showSurface - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.6");
        exit(-1);
    }
	struct st_rectangle origin_rectangle;
	struct st_position pos_destiny;

	origin_rectangle.x = 0;
	origin_rectangle.y = 0;
	origin_rectangle.w = RES_W;
	origin_rectangle.h = RES_H;
	pos_destiny.x = 0;
	pos_destiny.y = 0;
    //copySDLArea(origin_rectangle, pos_destiny, surfaceOrigin->gSurface, game_screen);
    showSurfacePortion(surfaceOrigin, origin_rectangle, st_rectangle(_screen_adjust.x, _screen_adjust.y, surfaceOrigin->width, surfaceOrigin->height));
}


void graphicsLib::showSurfaceRegion(struct graphicsLib_gSurface* surfaceOrigin, const struct st_rectangle origin_rectangle)
{
	struct st_position pos_destiny;

	pos_destiny.x = 0;
	pos_destiny.y = 0;
	showSurfaceRegionAt(surfaceOrigin, origin_rectangle, pos_destiny);
}

void graphicsLib::showSurfaceRegionAt(struct graphicsLib_gSurface* surfaceOrigin, const struct st_rectangle origin_rectangle, struct st_position pos_destiny)
{
    if (!game_screen) {
        std::cout << "showSurfaceRegionAt - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.4");
        exit(-1);
    }
    copySDLArea(origin_rectangle, pos_destiny, surfaceOrigin->get_surface(), game_screen);
}

void graphicsLib::showSurfacePortion(graphicsLib_gSurface *surfaceOrigin, const st_rectangle origin_rect, st_rectangle destiny_rect)
{
    if (!game_screen) {
        std::cout << "showSurfacePortion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.2");
        exit(-1);
    }
    copySDLPortion(origin_rect, destiny_rect, surfaceOrigin->get_surface(), game_screen);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void graphicsLib::showSurfaceAt(struct graphicsLib_gSurface* surfaceOrigin, struct st_position pos, bool fix_colors=true)
{
    if (!game_screen) {
        std::cout << "showSurfaceAt - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.7");
        exit(-1);
    }

	struct st_rectangle origin_rectangle;
	struct st_position pos_destiny;

    if (!surfaceOrigin->get_surface()) {
		std::cout << "Error: no data in surfaceOrigin at graphicsLib::showSurfaceAt." << std::endl;
		//exit(-1);
		return;
	}

	origin_rectangle.x = 0;
	origin_rectangle.y = 0;
    origin_rectangle.w = surfaceOrigin->get_surface()->w;
    origin_rectangle.h = surfaceOrigin->get_surface()->h;
	pos_destiny.x = pos.x;
	pos_destiny.y = pos.y;

    copySDLArea(origin_rectangle, pos_destiny, surfaceOrigin->get_surface(), game_screen, fix_colors);
}

void graphicsLib::show_white_surface_at(graphicsLib_gSurface *surfaceOrigin, st_position pos)
{
    // create a new surface
    struct graphicsLib_gSurface tmp;
    initSurface(st_size(surfaceOrigin->width, surfaceOrigin->height), &tmp);
    copyArea(st_position(0, 0), surfaceOrigin, &tmp);
    //struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny
    showSurfaceAt(&tmp, pos, false);
}

void graphicsLib::initSurface(struct st_size size, struct graphicsLib_gSurface* gSurface)
{
    if (game_screen == NULL) {
        return;
    }
    gSurface->freeGraphic();
    SDL_Surface* temp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE , size.width, size.height, VIDEO_MODE_COLORS, 0, 0, 0, 0);

    if (!temp_surface) {
        show_debug_msg("EXIT #21.INIT #1");
        exit(-1);
    }


    SDL_FillRect(temp_surface, NULL, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
    SDL_SetColorKey(temp_surface, SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));

    gSurface->set_surface(temp_surface);


    if (!gSurface->get_surface()) {
        show_debug_msg("EXIT #21.INIT #2");
        exit(-1);
    }
	gSurface->width = size.width;
    gSurface->height = size.height;
}

void graphicsLib::set_surface_alpha(int alpha, graphicsLib_gSurface& surface)
{
    SDL_SetAlpha(surface.get_surface(), SDL_SRCALPHA, alpha);
}


struct graphicsLib_gSurface graphicsLib::surfaceFromRegion(struct st_rectangle rect_origin, struct graphicsLib_gSurface& originalSurface)
{

	struct st_position destiny_pos;

	destiny_pos.x = 0;
	destiny_pos.y = 0;
	struct graphicsLib_gSurface res;
	initSurface(st_size(rect_origin.w, rect_origin.h), &res);

    if (!res.get_surface()) {
        std::cout << "surfaceFromRegion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.8");
        exit(-1);
    }

    /// @NOTE: removed for optimization test
    copySDLArea(rect_origin, destiny_pos, originalSurface.get_surface(), res.get_surface());
    /// @NOTE: removed for optimization test
	return res;
}



void graphicsLib::blank_screen() {
	SDL_FillRect(game_screen, NULL, SDL_MapRGB(game_screen->format, 0, 0, 0));
}

void graphicsLib::blank_surface(graphicsLib_gSurface &surface)
{
    SDL_FillRect(surface.get_surface(), NULL, SDL_MapRGB(game_screen->format, 0, 0, 0));
}

void graphicsLib::blank_area(short int x, short int y, short int w, short int h) {
    clear_area(x, y, w, h, 0, 0, 0);
}

void graphicsLib::blank_area(short x, short y, short w, short h, graphicsLib_gSurface &surface)
{
    clear_surface_area(x, y, w, h, 0, 0, 0, surface);
}

void graphicsLib::draw_rectangle(st_rectangle area, int r, int g, int b, int alpha)
{

    graphicsLib_gSurface transparent_area;
    graphLib.initSurface(st_size(area.w, area.h), &transparent_area);
    graphLib.clear_surface_area(0, 0, area.w, area.h, r, g, b, transparent_area);

    set_surface_alpha(alpha, transparent_area);
    showSurfaceAt(&transparent_area, st_position(area.x, area.y), false);

}


int graphicsLib::draw_progressive_text(short int x, short int y, string text, bool interrupt) {
    UNUSED(interrupt);
	//SDL_Color font_color = {255,255,255};
	string temp_text;
	char temp_char;
	int text_x = 0;
	int text_y = 0;
	unsigned int i;

	if (!font) {
		printf("ERROR - no fount found - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #09");
		exit(-1);
	}


	for (i=0; i<text.size(); i++) {
		temp_char = text.at(i);

		temp_text = "";
		temp_text += temp_char;

		draw_text(text_x*9+x, text_y*11+y, temp_text);
		text_x++;
		if (temp_char == '\n') {
				text_x = 0;
				text_y++;
		}
		updateScreen();
		/*
        if (interrupt) {
			cut = input.waitScapeTime(5);
			if (cut) { return 1; }
		} else {
			input.waitTime(5);
		}
		*/
		timer.delay(15);
	}
	return 0;
}




void graphicsLib::draw_text(short int x, short int y, string text) {
    draw_text(x, y, text, st_color(250, 250, 250));
}

void graphicsLib::draw_text(short x, short y, string text, st_color color)
{
    if (text.length() <= 0) {
        return;
    }
    SDL_Color font_color;
    font_color.r = color.r;
    font_color.g = color.g;
    font_color.b = color.b;
    x += _screen_resolution_adjust.x;
    y += _screen_resolution_adjust.y;
    SDL_Rect text_pos={x, y, 0, 0};
    if (!font) {
        printf("graphicsLib::draw_text - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #10");
        exit(-1);
        // handle error
    }
    SDL_Surface* textSF = TTF_RenderText_Solid(font, text.c_str(), font_color);
    if (!textSF) {
        return;
    }
    SDL_Surface* textSF_format = SDL_DisplayFormat(textSF);
    SDL_FreeSurface(textSF);

    if (!textSF_format) {
        return;
    }
    SDL_BlitSurface(textSF_format, 0, game_screen, &text_pos);
    SDL_FreeSurface(textSF_format);
}


void graphicsLib::draw_text(short x, short y, string text, graphicsLib_gSurface &surface)
{
	SDL_Color font_color;
	font_color.r = 255;
	font_color.g = 255;
	font_color.b = 255;
	SDL_Rect text_pos={x, y, 0, 0};
	if (!font) {
		printf("graphicsLib::draw_text - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #11");
		exit(-1);
		// handle error
	}
    text_pos.x += _screen_resolution_adjust.x;
    text_pos.y += _screen_resolution_adjust.y;
    SDL_Surface* textSF = TTF_RenderText_Solid(font, text.c_str(), font_color);
    SDL_Surface* textSF_format = SDL_DisplayFormat(textSF);
    SDL_FreeSurface(textSF);
    SDL_BlitSurface(textSF_format, 0, surface.get_surface(), &text_pos);
    SDL_FreeSurface(textSF_format);
}

void graphicsLib::draw_centered_text(short y, string text, st_color font_color)
{
    draw_centered_text(y, text, gameScreen, font_color);
}

void graphicsLib::draw_centered_text(short y, string text, graphicsLib_gSurface &surface, st_color temp_font_color)
{
	SDL_Color font_color;
    font_color.r = temp_font_color.r;
    font_color.g = temp_font_color.g;
    font_color.b = temp_font_color.b;
	SDL_Rect text_pos={0, y, 0, 0};
	if (!font) {
		printf("graphicsLib::draw_text - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #12");
		exit(-1);
		// handle error
	}
	SDL_Surface* textSF = TTF_RenderText_Solid(font, text.c_str(), font_color);
	if (textSF == NULL) {
		return;
	}
	if (text.size() > 0) {
		text_pos.x = RES_W/2 - textSF->w/2;
	}
    SDL_Surface* textSF_format = SDL_DisplayFormat(textSF);
    SDL_FreeSurface(textSF);
    SDL_BlitSurface(textSF_format, 0, surface.get_surface(), &text_pos);
    SDL_FreeSurface(textSF_format);
}



Uint8 graphicsLib::getColorNumber(Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(game_screen->format, r, g, b);
}


void graphicsLib::drawCursor(st_position pos) {

    //std::cout << "drawCursor - x: " << pos.x << ", y: " << pos.y << std::endl;
	draw_text(pos.x, pos.y, ">");
}

void graphicsLib::eraseCursor(st_position pos) {
	blank_area(pos.x, pos.y, CURSOR_SPACING, CURSOR_SPACING);
}

void graphicsLib::blink_screen(Uint8 r, Uint8 g, Uint8 b) {
	int i;
    struct graphicsLib_gSurface screen_copy;

	initSurface(st_size(gameScreen.width, gameScreen.height), &screen_copy);
	copyArea(st_position(0, 0), &gameScreen, &screen_copy);

	for (i=0; i<4; i++) {
        SDL_FillRect(game_screen, 0, SDL_MapRGB(game_screen->format, r, g, b));

		updateScreen();
		input.waitTime(80);

        SDL_BlitSurface(screen_copy.get_surface(), 0, game_screen, 0);
		updateScreen();
		input.waitTime(80);
    }
}

void graphicsLib::blink_surface_into_screen(struct graphicsLib_gSurface &surface)
{
    st_color color_white(235, 235, 235);
    st_color color_black(0, 0, 0);

    Uint32 key_n = SDL_MapRGB(game_screen->format, color_black.r, color_black.g, color_black.b);
    for (int i=0; i<5; i++) {
        change_surface_color(key_n, color_white, &surface);
        showSurface(&surface);
        wait_and_update_screen(100);
        change_surface_color(key_n, color_black, &surface);
        showSurface(&surface);
        wait_and_update_screen(100);
    }
}

void graphicsLib::load_icons()
{
	struct graphicsLib_gSurface tmp;
    std::string filename = FILEPATH + "images/items.png";

	surfaceFromFile(filename, &tmp);
	for (int i=0; i<(tmp.width/16); i++) {
		item_icons.push_back(graphicsLib_gSurface());
		initSurface(st_size(16, 16), &item_icons.at(item_icons.size()-1));
		copyArea(st_rectangle(i*16, 0, 16, 16), st_position(0, 0), &tmp, &(item_icons.at(item_icons.size()-1)));
	}

	// big icon
    filename = FILEPATH + "images/icons.png";
	surfaceFromFile(filename, &tmp);
	for (int i=0; i<(tmp.width/14); i++) {
		weapon_icons.push_back(graphicsLib_gSurface());
		initSurface(st_size(14, 28), &weapon_icons.at(weapon_icons.size()-1));
		copyArea(st_rectangle(i*14, 0, 14, 28), st_position(0, 0), &tmp, &(weapon_icons.at(weapon_icons.size()-1)));
	}

	// small icons
    filename = FILEPATH + "images/icons_small.png";
	surfaceFromFile(filename, &tmp);
	for (int i=0; i<(tmp.width/8); i++) {
		small_weapon_icons.push_back(graphicsLib_gSurface());
		initSurface(st_size(8, 8), &small_weapon_icons.at(small_weapon_icons.size()-1));
		copyArea(st_rectangle(i*8, 0, 8, 8), st_position(0, 0), &tmp, &(small_weapon_icons.at(small_weapon_icons.size()-1)));
	}



    filename = FILEPATH + "images/backgrounds/config_fg.png";
	surfaceFromFile(filename, &config_menu);

	_config_menu_pos.x = (RES_W-config_menu.width)*0.5;

    filename = FILEPATH + "images/backgrounds/dialog.png";
	surfaceFromFile(filename, &dialog_surface);

    filename = FILEPATH + "images/backgrounds/weapon_menu.png";
	surfaceFromFile(filename, &ingame_menu);

    filename = FILEPATH + "images/backgrounds/btn_a.png";
    surfaceFromFile(filename, &_btn_a_surface);

    filename = FILEPATH + "images/tilesets/blocks/easymode.png";
    surfaceFromFile(filename, &_easymode_block);

    filename = FILEPATH + "images/tilesets/blocks/hardmode.png";
    surfaceFromFile(filename, &_hardmode_block);


}


void graphicsLib::draw_item_icon(enum ITEM_ICONS_ENUM item_n, st_position pos)
{
	showSurfaceAt(&(item_icons.at(item_n)), pos);
}


void graphicsLib::draw_weapon_icon(short wpn_n, st_position point, bool active)
{
    st_position pos;

    if (point.x == 0) {
        pos.x = WPN_COLUMN1_X;
    } else {
        pos.x = WPN_COLUMN2_X;
    }

    pos.y = WPN_COLUMN_Y-3+point.y*16;

    //std::cout << "### GRAPH::draw_weapon_icon - wpn_n: " << wpn_n<< ", point.x: " << point.x << ", point.y: " << point.y << ", pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;

    clear_area(pos.x, pos.y, 14, 16, 0, 0, 0);
    if (active == true) {
        showSurfaceRegionAt(&weapon_icons.at(wpn_n), st_rectangle(0, 0, 14, 14), pos);
    } else {
        showSurfaceRegionAt(&weapon_icons.at(wpn_n), st_rectangle(0, 14, 14, 14), pos);
    }
}

void graphicsLib::draw_weapon_tooltip_icon(short weapon_n, st_position position)
{
    showSurfaceRegionAt(&weapon_icons.at(weapon_n), st_rectangle(0, 0, 14, 14), position);
}

void graphicsLib::draw_menu_item(int x_pos)
{
	graphicsLib_gSurface* spriteCopy;
    int x;

	if (x_pos == 0) {
        spriteCopy = &e_tank[0];
        x = 93;
	} else if (x_pos == 1) {
        spriteCopy = &w_tank[0];
        x = 117;
	} else {
        spriteCopy = &s_tank[0];
        x = 142;
	}
    copyArea(st_rectangle(0, 0, TILESIZE, TILESIZE), st_position(x, 196), spriteCopy, &gameScreen);
    //copyArea(st_position(x, 196), spriteCopy, &gameScreen);
}

void graphicsLib::erase_menu_item(int x_pos)
{
	graphicsLib_gSurface* spriteCopy;
	int x;

	if (x_pos == 0) {
		spriteCopy = &e_tank[1];
        x = 93;
	} else if (x_pos == 1) {
		spriteCopy = &w_tank[1];
        x = 117;
	} else {
		spriteCopy = &s_tank[1];
        x = 142;
	}
    copyArea(st_rectangle(0, 0, TILESIZE, TILESIZE), st_position(x, 196), spriteCopy, &gameScreen);
    //copyArea(st_position(x, 196), spriteCopy, &gameScreen);
}

void graphicsLib::draw_weapon_menu_bg(Uint8 current_hp, graphicsLib_gSurface* player_frame, short max_hp) {
	showSurfaceAt(&ingame_menu, st_position((RES_W-ingame_menu.width)*0.5, (RES_H-ingame_menu.height)*0.5));

	showSurfaceRegionAt(&weapon_icons.at(0), st_rectangle(0, 14, 14, 14), st_position(WPN_COLUMN1_X, 50));
    draw_horizontal_hp_bar(WPN_COLUMN_Y, 2, current_hp, 0, max_hp);

    for (int i=1; i<6; i++) {
        if (game_save.stages[i] == 1) {
            //std::cout << ">> #1 graphicsLib::draw_weapon_menu_bg - stage[" << i << "]: " << game_save.stages[i] << std::endl;
            showSurfaceRegionAt(&weapon_icons.at(i), st_rectangle(0, 14, 14, 14), st_position(WPN_COLUMN1_X, 50+(i)*16));
            draw_horizontal_hp_bar(WPN_COLUMN_Y+(i)*WEAPON_SPACING, 2, game_save.items.weapons[i], 0, max_hp);
		}
    }

    for (int i=6; i<=9; i++) {
        if (game_save.stages[i] == 1) {
            //std::cout << ">> #3 graphicsLib::draw_weapon_menu_bg - stage[" << i << "]: " << game_save.stages[i] << std::endl;
            showSurfaceRegionAt(&weapon_icons.at(i), st_rectangle(0, 14, 14, 14), st_position(182, 50+(i-5)*16));
            draw_horizontal_hp_bar(WPN_COLUMN_Y+(i-5)*WEAPON_SPACING, 3, game_save.items.weapons[i], 0, max_hp);
		}
	}

    // coil and jet
    if (game_save.stages[COIL_GOT_STAGE] == 1) {
        int wpn_icon_n = 9;
        int menu_row = 4;
        showSurfaceRegionAt(&weapon_icons.at(wpn_icon_n), st_rectangle(0, 14, 14, 14), st_position(182, 50+(menu_row)*16));
        draw_horizontal_hp_bar(WPN_COLUMN_Y+(menu_row)*WEAPON_SPACING, 3, game_save.items.weapons[wpn_icon_n], 0, max_hp);
    }
    if (game_save.stages[JET_GOT_STAGE] == 1) {
        int wpn_icon_n = 10;
        int menu_row = 5;
        showSurfaceRegionAt(&weapon_icons.at(wpn_icon_n), st_rectangle(0, 14, 14, 14), st_position(182, 50+(menu_row)*16));
        draw_horizontal_hp_bar(WPN_COLUMN_Y+(menu_row)*WEAPON_SPACING, 3, game_save.items.weapons[wpn_icon_n], 0, max_hp);
    }



	copyArea(st_position(26, 190), player_frame, &gameScreen);

	std::stringstream ss;
	int item_text_pos = 217;

	ss.str(std::string());
    ss << "0" << (short)game_save.items.lifes;
    draw_text(58, item_text_pos, ss.str());


	ss.str(std::string());
    ss << "0" << (short)game_save.items.energy_tanks;
    draw_text(93, item_text_pos, ss.str());

	ss.str(std::string());
    ss << "0" << (short)game_save.items.weapon_tanks;
    draw_text(117, item_text_pos, ss.str());

	ss.str(std::string());
    ss << "0" << (short)game_save.items.special_tanks;
    draw_text(142, item_text_pos, ss.str());


	ss.str(std::string());
    ss << "0" << (int)game_save.items.bolts;
    draw_text(273, item_text_pos, ss.str());

	//if (item_ref->balancer > 0) {
	if (true) {
        copyArea(st_position(245, 196), &energy_balancer, &gameScreen);
	}

    if (game_save.armor_pieces[ARMOR_ARMS] == true) {
        copyArea(st_position(198, 197), &armor_icon_arms, &gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_BODY] == true) {
        copyArea(st_position(221, 197), &armor_icon_body, &gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_LEGS] == true) {
        copyArea(st_position(175, 197), &armor_icon_legs, &gameScreen);
    }

    draw_text(212, 23, strings_map::get_instance()->get_ingame_string(strings_ingame_config) + std::string(" (R)"));
    draw_text(37, 185, strings_map::get_instance()->get_ingame_string(strings_ingame_life));
    draw_text(111, 185, strings_map::get_instance()->get_ingame_string(strings_ingame_item));

	updateScreen();
}


void graphicsLib::scale2x(SDL_Surface* surface, SDL_Surface* dest, bool smooth_scale) const
{
    register int i, j;
	int b, h;
	int bpp = surface->format->BytesPerPixel;
	if (SDL_MUSTLOCK(dest) != 0)
	{
	 if (SDL_LockSurface(dest) < 0)
	 {
	   fprintf(stderr, "dest locking failedn");
	   return;
	 }
	}

    //std::cout << "scale2x::smooth_scale: " << smooth_scale << ", bpp: " << bpp << std::endl;

	const int wd = ((dest->w / 2) < (surface->w)) ? (dest->w / 2) : (surface->w);
	const int hg = ((dest->h) < (surface->h*2)) ? (dest->h / 2) : (surface->h);

	switch (bpp)
	{
	case 1:
	{
		int tpitch = dest->pitch;
		int spitch = surface->pitch;
		Uint8* tp = (Uint8*) dest->pixels;
		Uint8* sp = (Uint8*) surface->pixels;

		for (j = 0; j < hg; ++j)
		{
			b = j>0?spitch:0;
			h = j<hg?spitch:0;

			for (i = 0; i < wd; ++i)
			{
				if (B != H && D != F) {
					if (smooth_scale == true) {
						E0 = D == B ? D : E;
						E1 = B == F ? F : E;
						E2 = D == H ? D : E;
						E3 = H == F ? F : E;
					} else {
						E0 = E;
						E1 = E;
						E2 = E;
						E3 = E;
					}
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
				}
			}
			tp += 2*tpitch;
			sp += spitch;
		}

		break;
	}

	 case 2:
	 {
	   int tpitch = dest->pitch / 2;
	   int spitch = surface->pitch / 2;
	   Uint16* tp = (Uint16*) dest->pixels;
	   Uint16* sp = (Uint16*) surface->pixels;

       for (j = 0; j < hg; ++j)
       {
           b = j>0?spitch:0;
           h = j<hg?spitch:0;

           for (i = 0; i < wd; ++i)
           {
               if (B != H && D != F) {
                   if (smooth_scale == true) {
                       E0 = D == B ? D : E;
                       E1 = B == F ? F : E;
                       E2 = D == H ? D : E;
                       E3 = H == F ? F : E;
                   } else {
                       E0 = E;
                       E1 = E;
                       E2 = E;
                       E3 = E;
                   }
               } else {
                   E0 = E;
                   E1 = E;
                   E2 = E;
                   E3 = E;
               }
           }
           tp += 2*tpitch;
           sp += spitch;
       }

	   break;
	 }
	 case 4:
	 {
	   int tpitch = dest->pitch / 4;
	   int spitch = surface->pitch / 4;
	   Uint32* tp = (Uint32*) dest->pixels;
	   Uint32* sp = (Uint32*) surface->pixels;

       for (j = 0; j < hg; ++j)
       {
           b = j>0?spitch:0;
           h = j<hg?spitch:0;

           for (i = 0; i < wd; ++i)
           {
               if (B != H && D != F) {
                   if (smooth_scale == true) {
                       E0 = D == B ? D : E;
                       E1 = B == F ? F : E;
                       E2 = D == H ? D : E;
                       E3 = H == F ? F : E;
                   } else {
                       E0 = E;
                       E1 = E;
                       E2 = E;
                       E3 = E;
                   }
               } else {
                   E0 = E;
                   E1 = E;
                   E2 = E;
                   E3 = E;
               }
           }
           tp += 2*tpitch;
           sp += spitch;
       }


	   break;
	 }
	 default:
	   std::cout << "Unsupported bitdepth.n(" << bpp << ")" << std::endl;
	   break;
	}


	if (SDL_MUSTLOCK(dest) != 0)
	{
	 SDL_UnlockSurface(dest);
	}
}




void graphicsLib::draw_hp_bar(short int hp, short int player_n, short int weapon_n, short int max_hp)
{
	short int y, i;
    int hp_percent = (100 * hp) / max_hp;
    int graph_lenght = (int)(hp_percent/2);


    //std::cout << "GRAPH::DRAW_HP_BAR::graph_lenght: " << graph_lenght << std::endl;

    int id = weapon_n;
    if (weapon_n == -1) {
        id = 0;
    }

    st_color color1(game_data.weapon_menu_colors[id].r, game_data.weapon_menu_colors[id].g, game_data.weapon_menu_colors[id].b);    // player color or weapon color
    st_color color2(188, 188, 188); // dark grey
    st_color color3(235, 235, 235); // light grey

	st_position bar_pos(1, 2);
	if (player_n == -1) {
		bar_pos.x = RES_W-15;
	} else if (weapon_n != WEAPON_DEFAULT) {
		bar_pos.x = 10;
	}

    clear_area(bar_pos.x, bar_pos.y, 8, 52, color1.r, color1.g, color1.b);

	if (weapon_n != -1) {
        showSurfaceRegionAt(&small_weapon_icons.at(weapon_n), st_rectangle(0, 0, 8, 8), st_position(bar_pos.x, 54));
	} else {
        showSurfaceRegionAt(&small_weapon_icons.at(WEAPON_COUNT), st_rectangle(0, 0, 8, 8), st_position(bar_pos.x, 54));
	}

    /*
	for (i=0; i<hp; i++) {
        y = ((max_hp-i)*2+1)+bar_pos.y-2;
		clear_area(bar_pos.x+1, y, 2, 1, color2.r, color2.g, color2.b);
		clear_area(bar_pos.x+3, y, 2, 1, color3.r, color3.g, color3.b);
		clear_area(bar_pos.x+5, y, 2, 1, color2.r, color2.g, color2.b);
	}
    */
    y = bar_pos.y + 1 + (50 - graph_lenght);
    clear_area(bar_pos.x+1, y, 2, graph_lenght, color2.r, color2.g, color2.b);
    clear_area(bar_pos.x+3, y, 2, graph_lenght, color3.r, color3.g, color3.b);
    clear_area(bar_pos.x+5, y, 2, graph_lenght, color2.r, color2.g, color2.b);

}

void graphicsLib::clear_area(short int x, short int y, short int w, short int h, short int r, short int g, short int b) {
    //std::cout << ">> graphicsLib::clear_area - x: " << x << ", w: " << w << std::endl;
	SDL_Rect dest;
    dest.x = x + _screen_resolution_adjust.x;
    dest.y = y + _screen_resolution_adjust.y;
	dest.w = w;
	dest.h = h;
    SDL_FillRect(game_screen, &dest, SDL_MapRGB(game_screen->format, r, g, b));
}

void graphicsLib::clear_area_no_adjust(short x, short y, short w, short h, short r, short g, short b)
{
    //std::cout << ">> graphicsLib::clear_area - x: " << x << ", w: " << w << std::endl;
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;
    SDL_FillRect(game_screen, &dest, SDL_MapRGB(game_screen->format, r, g, b));
}

void graphicsLib::clear_surface_area(short int x, short int y, short int w, short int h, short int r, short int g, short int b, struct graphicsLib_gSurface& surface) const {
	SDL_Rect dest;
    if (surface.get_surface() == gameScreen.get_surface()) {
        dest.x = x + _screen_resolution_adjust.x;
        dest.y = y + _screen_resolution_adjust.y;
    } else {
        dest.x = x;
        dest.y = y;
    }
	dest.w = w;
	dest.h = h;
    //int color_n = SDL_MapRGB(surface.gSurface->format, r, g, b);
    SDL_FillRect(surface.get_surface(), &dest, SDL_MapRGB(surface.get_surface()->format, r, g, b));
}


void graphicsLib::show_config_bg(Uint8 position) // 0 - centered, 1 - top, 2 - bottom
{
	if (position == 0) {
		_config_menu_pos.y = (RES_H-config_menu.height)*0.5;
	} else if (position == 1) {
		_config_menu_pos.y = 3;
	} else {
		_config_menu_pos.y = RES_H - config_menu.height - 25;
	}

	st_position bg_pos(_config_menu_pos.x, _config_menu_pos.y);

	copyArea(bg_pos, &config_menu, &gameScreen);
}


// position - 0: center, 1: top, 2: bottom
void graphicsLib::show_dialog(Uint8 position, bool show_btn)
{
	int posX = (RES_W-dialog_surface.width)*0.5;
	int posY;

	if (position == 0) {
		posY = (RES_H-dialog_surface.height)*0.5;
	} else if (position == 1) {
		posY = 3;
	} else {
		posY = RES_H - dialog_surface.height - 25;
	}

	_dialog_pos.x = posX;
	_dialog_pos.y = posY;

	st_position bg_pos(posX, posY);
	copyArea(bg_pos, &dialog_surface, &gameScreen);
    if (show_btn == true) {
        show_btn_a(st_position(posX+dialog_surface.width-_btn_a_surface.width-2-TILESIZE, posY+dialog_surface.height-_btn_a_surface.height-TILESIZE/2));
    }
}

st_position graphicsLib::get_dialog_pos() const {
	return _dialog_pos;
}

void graphicsLib::draw_horizontal_hp_bar(short int y_adjust, short int right, short int hp, short int player_n, short max_hp) {
	short int x;
	short int r, g, b;
	// armas - coluna1: 42, 26, 182, coluna2: 26

	if (right == 0) {
		x = WPN_COLUMN1_X+18;
		r = 127;
		g = 11;
		b = 0;
	} else if (right == 1) {
		x = WPN_COLUMN2_X+18;
		r = 255;
		g = 102;
		b = 0;
	} else if (right == 2) {
		x = WPN_COLUMN1_X+18;
		r = 0;
		g = 0;
		b = 0;
	} else {
		x = WPN_COLUMN2_X+18;
		r = 0;
		g = 0;
		b = 0;
	}
	// border
	/*
	clear_area(weapon_menu_surface, x+1, y_adjust, 61, 2, 188, 188, 188);
	clear_area(weapon_menu_surface, x+1, y_adjust+7, 61, 2, 188, 188, 188);

	clear_area(weapon_menu_surface, x+1, y_adjust, 2, 9, 188, 188, 188);
	clear_area(weapon_menu_surface, x+60, y_adjust, 2, 9, 188, 188, 188);
	*/


	if (right < 2) {
		clear_area(x+1, 1+y_adjust, 61, 9, r, g, b);
	}

    draw_horizontal_hp_bar(st_position(x, y_adjust), hp, player_n, max_hp);

}

void graphicsLib::draw_explosion(st_position pos)
{
	st_size explosion_size(56, 56);
	int max_frames = 7;

	if (_explosion_animation_pos >= max_frames) {
		_explosion_animation_pos = 0;
	}

	int anim_pos = _explosion_animation_pos*explosion_size.width;

    showSurfaceRegionAt(&bomb_explosion_surface, st_rectangle(anim_pos, 0, explosion_size.width, explosion_size.height), pos);

	if (_explosion_animation_timer < timer.getTimer()) {
		_explosion_animation_pos++;
		_explosion_animation_timer = timer.getTimer() + PROJECTILE_DEFAULT_ANIMATION_TIME;
	}

}

void graphicsLib::show_debug_msg(string msg)
{
    std::cout << "show_debug_msg - msg: " << msg << std::endl;
    std::fflush(stdout);

    clear_area(0, 0, RES_W, 50, 50, 50, 50);
    draw_text(10, _debug_msg_pos*12+10, msg, gameScreen);
    updateScreen();
    input.waitTime(3000);
}

void graphicsLib::draw_path(st_position initial_point, st_position final_point, short duration)
{
    short move_step = 1;
    // calculate distance
    int distance = 0;
    int mode = 0;

    // add four because of the curve ahead
    if (initial_point.x > final_point.x) {
        final_point.x -= 4;
    } else if (initial_point.x < final_point.x) {
        final_point.x += 4;
    }

    if (initial_point.x != final_point.x) {
        distance = final_point.x - initial_point.x;
    } else {
        distance = initial_point.y - final_point.y;
        mode = 1;
    }
    // calculate time for each part, being  (move_step)2 pixels each
    int part_duration = duration / (distance/move_step*2);
    if (distance == 0) {
        return;
    }
    if (distance < 0) {
        move_step = -move_step;
    }
    if (duration > 0) {
        for (int i=0; i<abs(distance/move_step); i++) {
            if (mode == 0) {
                clear_area(initial_point.x + i*move_step, initial_point.y, abs(move_step), 4, 255, 255, 255);
            } else {
                clear_area(initial_point.x, initial_point.y - (i+1)*move_step, 4, abs(move_step), 255, 255, 255);
            }
            updateScreen();
            input.waitTime(part_duration);
        }
    } else {
        if (mode == 0) {
            clear_area(initial_point.x, initial_point.y, abs(distance), 4, 255, 255, 255);
        } else {
            clear_area(initial_point.x, initial_point.y-distance, 4, abs(distance), 255, 255, 255);
        }
        updateScreen();
    }
}

void graphicsLib::draw_horizontal_hp_bar(st_position pos, short int hp, short player_n, short max_hp) {
    int hp_percent = (100 * hp) / max_hp;
    int graph_lenght = (int)(hp_percent/2);


    int y = (-28)*2+1;
    short int pos_x = pos.x + y + 58 + (50 - graph_lenght);

    st_color bar_color1(188, 188, 188);
    st_color bar_color2(255, 255, 255);

    if (player_n == 0) {
        bar_color1 = st_color(27, 63, 95);
        bar_color2 = st_color(0, 131, 139);
    } else if (player_n == 1) {
        bar_color1 = st_color(203, 79, 15);
        bar_color2 = st_color(255, 155, 59);
    }

    clear_area(pos_x, 2+pos.y, graph_lenght, 2, bar_color1.r, bar_color1.g, bar_color1.b);
    clear_area(pos_x, 4+pos.y, graph_lenght, 2, bar_color2.r, bar_color2.g, bar_color2.b);
    clear_area(pos_x, 6+pos.y, graph_lenght, 2, bar_color1.r, bar_color1.g, bar_color1.b);

    /*
    for (int i=0; i<hp; i++) {
		int y = ((i-28)*2+1);
		short int pos_x = pos.x+y+58;
		if (player_n == 0) {
			clear_area(pos_x, 2+pos.y, 1, 2, 27, 63, 95);
			clear_area(pos_x, 4+pos.y, 1, 2, 0, 131, 139);
			clear_area(pos_x, 6+pos.y, 1, 2, 27, 63, 95);
		} else if (player_n == 1) {
			clear_area(pos_x, 2+pos.y, 1, 2, 203, 79, 15);
			clear_area(pos_x, 4+pos.y, 1, 2, 255, 155, 59);
			clear_area(pos_x, 6+pos.y, 1, 2, 203, 79, 15);
		} else {
			clear_area(pos_x, 2+pos.y, 1, 2, 188, 188, 188);
			clear_area(pos_x, 4+pos.y, 1, 2, 255, 255, 255);
			clear_area(pos_x, 6+pos.y, 1, 2, 188, 188, 188);
		}
	}
    */
}





void graphicsLib::draw_weapon_cursor(st_position pos, short int hp, short int player_n, short max_hp)
{
    //std::cout << "&&GRAPH::draw_weapon_cursor - pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;
	int pos_y, pos_x;

	if (pos.x == 0) {
		pos_x = WPN_COLUMN1_X + 18;
	} else {
		pos_x = WPN_COLUMN2_X + 18;
	}



	if (pos.y == 0) {
		pos_y = WPN_COLUMN_Y;
	} else if (pos.y == 6 && pos.x == 0) {
		pos_y = 134;
		pos_x = 196;
	} else if (pos.y == 6 && pos.x == 1) {
		pos_y = 159;
		pos_x = 196;
	} else if (pos.y == 6 && pos.x == 2) {
		pos_y = 183;
		pos_x = 196;
	} else {
		pos_y = WEAPON_SPACING*pos.y + 53;
	}
	if (pos.y != 6) {
        draw_horizontal_hp_bar(st_position(pos_x, pos_y), hp, player_n, max_hp);
	}
	updateScreen();
}


void graphicsLib::draw_star(short int x, short int y, int size) {
	clear_area(x, y, size, size, 188, 188, 188);
}

void graphicsLib::erase_star(short int x, short int y, int size) {
	clear_area(x, y, size, size, 0, 0, 0);
}



// random a position for each star
void graphicsLib::init_stars() {
	int i;
	for (i=0; i<INTRO_STARS_NUMBER; i++) {
		star_list[i].x = rand() % 320;
		if (i % 2 == 0) {
            star_list[i].y = rand() % TOP_STARS_LIMIT;
		} else {
            star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
		}
	}
	for (i=0; i<INTRO_STARS_NUMBER; i++) {
		small_star_list[i].x = rand() % 320;
		if (i % 2 == 0) {
            small_star_list[i].y = rand() % TOP_STARS_LIMIT;
		} else {
            small_star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
		}
	}
	for (i=0; i<INTRO_STARS_NUMBER/2; i++) {
		big_star_list[i].x = rand() % 320;
		if (i % 2 == 0) {
            big_star_list[i].y = rand() % TOP_STARS_LIMIT;
		} else {
            big_star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
		}
	}
}

void graphicsLib::anim_stars() {
	int i;

	/// @TODO - add timer to control stars speed
	if (stars_timer > timer.getTimer()) {
		return;
	}

    for (i=0; i<INTRO_STARS_NUMBER; i++) {
		erase_star(star_list[i].x, star_list[i].y, 2);
		erase_star(small_star_list[i].x, small_star_list[i].y, 1);
		star_list[i].x -= ANIM_STARS_STEP;
		if (star_list[i].x <= 0) {
			star_list[i].x = 320+ANIM_STARS_STEP;
			if (i % 2 == 0) {
                star_list[i].y = rand() % TOP_STARS_LIMIT;
			} else {
                star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
			}
		}
		small_star_list[i].x -= ANIM_STARS_SMALL_STEP;
		if (small_star_list[i].x <= 0) {
			small_star_list[i].x = 320+ANIM_STARS_SMALL_STEP;
			if (i % 2 == 0) {
                small_star_list[i].y = rand() % TOP_STARS_LIMIT;
			} else {
                small_star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
			}
		}
	}

	for (i=0; i<INTRO_STARS_NUMBER/2; i++) {
		erase_star(big_star_list[i].x, big_star_list[i].y, 3);
		big_star_list[i].x -= ANIM_STARS_BIG_STEP;
		if (big_star_list[i].x <= 0) {
			big_star_list[i].x = 320+ANIM_STARS_BIG_STEP;
			if (i % 2 == 0) {
                big_star_list[i].y = rand() % TOP_STARS_LIMIT;
			} else {
                big_star_list[i].y = rand() % TOP_STARS_LIMIT + BOTTOM_STARS_LIMIT;
			}
		}
	}

	for (i=0; i<INTRO_STARS_NUMBER; i++) {
		draw_star(star_list[i].x, star_list[i].y, 2);
		draw_star(small_star_list[i].x, small_star_list[i].y, 1);
	}
	for (i=0; i<INTRO_STARS_NUMBER/2; i++) {
		draw_star(big_star_list[i].x, big_star_list[i].y, 3);
	}
    stars_timer = timer.getTimer()+10;
}

void graphicsLib::set_video_mode()
{
#ifdef DINGUX
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_SWSURFACE);
#elif defined(OPEN_PANDORA)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF);
#elif defined(ANDROID)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_SWSURFACE);
#elif defined(WII)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE);
#elif defined(PSP)
    if (game_config.video_fullscreen == false) {
        game_screen = SDL_SetVideoMode(480, 272, VIDEO_MODE_COLORS, SDL_SWSURFACE | SDL_FULLSCREEN | SDL_RESIZABLE);
    } else {
        game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_SWSURFACE | SDL_FULLSCREEN | SDL_RESIZABLE);
    }
#elif defined(DREAMCAST)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
#elif defined(PLAYSTATION2)
    game_screen = SDL_SetVideoMode(640, 480, VIDEO_MODE_COLORS, SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
    /*
    256, 224 - good
    288, 224 - good (strange colors?)
    256x256 -
    320x200 - distortion
    320x240 - distortion
    320x256 - distortion (small)
    400x256 - distortion
    512x448 - good but small
    */
#else

    if (_video_filter == VIDEO_FILTER_NOSCALE) {
        if (game_config.video_fullscreen == false) {
            game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF);
        } else {
            game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
        }
        //game_screen = SDL_SetVideoMode(480, 272, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF); // used for testing centered screen
    } else {
        /// @TODO - do we need scale on fullscreen if no filter?
        if (game_config.video_fullscreen == false) {
            game_screen_scaled = SDL_SetVideoMode(RES_W*2, RES_H*2, VIDEO_MODE_COLORS, SDL_HWSURFACE|SDL_DOUBLEBUF);
        } else {
            game_screen_scaled = SDL_SetVideoMode(RES_W*2, RES_H*2, VIDEO_MODE_COLORS, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
        }
        if (game_screen != NULL) {
            SDL_FreeSurface(game_screen);
        }
        game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, RES_W, RES_H, VIDEO_MODE_COLORS, 0, 0, 0, 255);
    }
#endif

	if (!game_screen) {
        std::cout << "FATAL-ERROR::initGraphics Could not create game_screen" << std::endl;
        show_debug_msg("EXIT #13");
		exit(-1);
	}
    gameScreen.set_surface(game_screen);
    gameScreen.video_screen = true;
	screen_pixel_format = *game_screen->format;

    gameScreen.width = game_screen->w;
    gameScreen.height = game_screen->h;
    _screen_resolution_adjust.x = (game_screen->w - RES_W)/2;
    _screen_resolution_adjust.y = (game_screen->h - RES_H)/2;

}

void graphicsLib::preload_images()
{
	// explision used in death and bosses
    std::string filename = FILEPATH + "images/animations/explosion_boss.png";
	surfaceFromFile(filename, &small_explosion);

	// projectile images
    for (int i=0; i<GameMediator::get_instance()->get_projectile_list_size(); i++) {
        std::string filename(GameMediator::get_instance()->get_projectile(i).graphic_filename);
        filename = FILEPATH + "images/projectiles/" + filename;
		if (filename.length() > 0 && filename.find(".png") != std::string::npos) {
            projectile_surface.push_back(graphicsLib_gSurface());
            surfaceFromFile(filename, &projectile_surface.at(i));
		}
	}

	// bomb explosion
    filename = FILEPATH + std::string("/images/animations/big_boss_explosion.png");
	surfaceFromFile(filename, &bomb_explosion_surface);

	// --- ITEMS --- //
    filename = FILEPATH + "images/sprites/objects/energy_tank.png";
	surfaceFromFile(filename, &e_tank[0]);
    filename = FILEPATH + "images/sprites/objects/weapons_tank.png";
	surfaceFromFile(filename, &w_tank[0]);
    filename = FILEPATH + "images/sprites/objects/special_tank.png";
	surfaceFromFile(filename, &s_tank[0]);
    filename = FILEPATH + "images/sprites/objects/gray_energy_tank.png";
	surfaceFromFile(filename, &e_tank[1]);
    filename = FILEPATH + "images/sprites/objects/gray_weapons_tank.png";
	surfaceFromFile(filename, &w_tank[1]);
    filename = FILEPATH + "images/sprites/objects/gray_special_tank.png";
	surfaceFromFile(filename, &s_tank[1]);
    filename = FILEPATH + "images/sprites/objects/energy_balancer.png";
	surfaceFromFile(filename, &energy_balancer);

    filename = FILEPATH + "images/animations/explosion_32.png";
	surfaceFromFile(filename, &explosion32);
    filename = FILEPATH + "images/animations/explosion_16.png";
	surfaceFromFile(filename, &explosion16);

    filename = FILEPATH + "images/animations/dash_dust.png";
	surfaceFromFile(filename, &dash_dust);
    filename = FILEPATH + "images/animations/hit.png";
	surfaceFromFile(filename, &hit);

    filename = FILEPATH + "images/animations/water_splash.png";
	surfaceFromFile(filename, &water_splash);

    filename = FILEPATH + "images/sprites/objects/armor_arms.png";
    surfaceFromFile(filename, &armor_icon_arms);

    filename = FILEPATH + "images/sprites/objects/armor_body.png";
    surfaceFromFile(filename, &armor_icon_body);

    filename = FILEPATH + "images/sprites/objects/armor_legs.png";
    surfaceFromFile(filename, &armor_icon_legs);

}

void graphicsLib::preload_anim_tiles()
{
    int max = GameMediator::get_instance()->anim_tile_list.size();
    std::cout << "graphicsLib::preload_anim_tiles - max: " << max << std::endl;
    for (int i=0; i<max; i++) {
        std::string file(GameMediator::get_instance()->anim_tile_list.at(i).filename);
        if (file.length() < 1) {
            std::cout << "### graphicsLib::preload_anim_tiles::STOP, file: " << file << std::endl;
            break;
        } else {
            std::string filename = FILEPATH + std::string("images/tilesets/anim/") + file;

            ANIM_TILES_SURFACES.push_back(graphicsLib_gSurface());
            surfaceFromFile(filename, &ANIM_TILES_SURFACES.at(ANIM_TILES_SURFACES.size()-1));

            int frames_n = ANIM_TILES_SURFACES.at(ANIM_TILES_SURFACES.size()-1).width / TILESIZE;
            anim_tile_timer anim_timer(frames_n, timer.getTimer() + GameMediator::get_instance()->anim_tile_list.at(i).frame_delay[0]);

            for (int j=0; j<FS_ANIM_TILE_MAX_FRAMES; j++) {
                anim_timer.frames_delay[j] = GameMediator::get_instance()->anim_tile_list.at(i).frame_delay[j];
            }

            ANIM_TILES_TIMERS.push_back(anim_timer);
        }
    }
}

graphicsLib_gSurface graphicsLib::flip_image(graphicsLib_gSurface original, e_flip_type flip_mode)
{
    //Pointer to the soon to be flipped surface
    SDL_Surface *flipped = NULL;
    SDL_Surface *surface = original.get_surface();
    graphicsLib_gSurface res = original;
    //initSurface(st_size(original.width, original.height), &res);

    //If the image is color keyed
    if (SDL_SRCCOLORKEY) {
        flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0 );
    } else {
        flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask );
    }

    //If the surface must be locked
    if (SDL_MUSTLOCK( surface )) {
        //Lock the surface
        SDL_LockSurface( surface );
    }

    //Go through columns
    for (int x = 0, rx = flipped->w - 1; x < flipped->w; x++, rx-- ) {
        //Go through rows
        for (int y = 0, ry = flipped->h - 1; y < flipped->h; y++, ry-- ) {
            //Get pixel
            Uint32 pixel = original.get_pixel(x, y);

            //Copy pixel
            if ((flip_mode == flip_type_both)) {
                res.put_pixel(rx, ry, pixel);
            } else if (flip_mode == flip_type_horizontal) {
                res.put_pixel(rx, y, pixel );
            } else if(flip_mode == flip_type_vertical) {
                res.put_pixel(x, ry, pixel );
            }
        }
    }
    return res;
}




void graphicsLib::show_btn_a(st_position btn_pos)
{
    showSurfaceAt(&_btn_a_surface, btn_pos, false);
}

void graphicsLib::set_screen_adjust(st_position adjust)
{
    _screen_adjust.x = adjust.x;
    _screen_adjust.y = adjust.y;
}





// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void graphicsLib::preload_faces() const {

	/// @TODO: do this from directory listing
	/*
	string filename;
	int i;

	for (i=0; i<FACES_COUNT; i++) {
        filename = FILEPATH + "images/faces/" + face_name[i];
		surfaceFromFile(filename, FACES_SURFACES[i]);
	}
	*/
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void graphicsLib::unload_faces() const {
	/*
	std::map<std::string, graphicsLib_gSurface>::iterator it;

	for (it = FACES_SURFACES.begin(); it != FACES_SURFACES.end(); it++) {
		delete (*it).second;
		FACES_SURFACES.erase(it);
	}
	*/
}



//void stage_select::place_face(int face_n, char botname[20], short int posx, short int posy) {
void graphicsLib::place_face(std::string face_file, st_position pos) {
	if (face_file == "") {
		return;
	}
	std::map<std::string, graphicsLib_gSurface>::iterator it;

	it = FACES_SURFACES.find(face_file);

	if (it == FACES_SURFACES.end()) {
        std::string filename = FILEPATH + "images/faces/" + face_file;
		surfaceFromFile(filename, &FACES_SURFACES[face_file]);
	}
	copyArea(st_position(pos.x, pos.y), &FACES_SURFACES[face_file], &gameScreen);
}


void graphicsLib::change_surface_color(Sint8 colorkey_n, st_color new_color, graphicsLib_gSurface *surface)
{
    if (surface == NULL) {
        return;
    }
    if (surface->get_surface() == NULL) {
        return;
    }

    //std::cout << "change_surface_color::colorkey_n: " << (int)colorkey_n << ", new_color.rgb: " << new_color.r << ", " << new_color.g << ", " << new_color.b << std::endl;
    surface->change_colorkey_color(colorkey_n, new_color);
}



void graphicsLib::wait_and_update_screen(int period)
{
	unsigned int wait_time = timer.getTimer()+period;
	while (timer.getTimer() < wait_time) {
		updateScreen();
		timer.delay(10);
	}
}

st_position graphicsLib::get_config_menu_pos() const
{
	return _config_menu_pos;
}

st_size graphicsLib::get_config_menu_size()
{
	return st_size(config_menu.width, config_menu.height);
}




void graphicsLib::start_stars_animation()
{
    init_stars();
    _show_stars = true;
}


void graphicsLib::stop_stars_animation()
{
    _show_stars = false;
}

