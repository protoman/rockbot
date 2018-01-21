#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;


#include "defines.h"

#define LOAD_FILE_NUMBER_TRIES 4

extern std::string FILEPATH;
extern std::string GAMEPATH;

#include "aux_tools/stringutils.h"

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

#define DEBUG_MSG_DELAY 5000

#include "file/file_io.h"
#include "class_config.h"
#include "strings_map.h"
#include "aux_tools/exception_manager.h"

#ifdef ANDROID
#include <android/log.h>
#include "ports/android/android_game_services.h"
extern android_game_services game_services;
#endif

#define B sp[i - b]
#define D sp[i - (i>0?1:0)]
#define F sp[i + (i<wd?1:0)]
#define H sp[i + h]
#define E  sp[i]
#define E0 tp[i*2]
#define E1 tp[i*2 + 1]
#define E2 tp[i*2 + tpitch]
#define E3 tp[i*2 + 1 + tpitch]

// initialize static member
std::map<std::string, st_char_sprite_data> graphicsLib::character_graphics_list;
std::map<std::string, graphicsLib_gSurface> graphicsLib::character_graphics_background_list;

graphicsLib::graphicsLib() : _show_stars(false), game_screen(NULL), _explosion_animation_timer(0), _explosion_animation_pos(0), _timer(0), font(NULL)
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

    _screen_resolution_adjust = st_position(0, 0);
}

graphicsLib::~graphicsLib()
{
}


bool graphicsLib::initGraphics()
{
	string filename;
    _video_filter = game_config.video_filter;

    printf(">> WII.DEBUG.INIT_GRAPHICS #1 <<\n");
    fflush(stdout);


#ifdef DREAMCAST
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 ) {
        std::cout << "SDL Error" << std::endl;
        std::cout << "Unable to init SDL. Error: " << SDL_GetError() << std::endl;
        std::fflush(stdout);
        SDL_Quit();
		exit(-1);
    }
#else
	// GRAPHIC LIB
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER|SDL_INIT_AUDIO) < 0 ) {
        std::cout << "SDL Error" << std::endl;
        std::cout << "Unable to init SDL. Error: " << SDL_GetError() << std::endl;
        std::fflush(stdout);
        exception_manager::throw_general_exception(std::string("graphicsLib::initGraphics - Unable to init SDL."), SDL_GetError());
    }
#endif
	atexit(SDL_Quit);


    printf("INFO: There are %d joysticks attached\n", SDL_NumJoysticks());
#if defined(PLAYSTATION2) || defined(WII)
    if (SDL_NumJoysticks() <= 0) {
        std::cout << "No joysticks found" << std::endl;

        printf(">> WII.DEBUG.INIT_GRAPHICS.NO_JOYSTICKS <<\n");
        fflush(stdout);

        SDL_Quit();
        exit(-1);
    }
#endif
	input.init_joystick();


    printf(">> WII.DEBUG.INIT_GRAPHICS #2 <<\n");
    fflush(stdout);

    // FONT
    TTF_Init();
    filename = GAMEPATH + "/fonts/pressstart2p.ttf";

	char *buffer = new char[filename.size()+1];
	std::strcpy(buffer, filename.c_str());
	SDL_RWops *fileRW;
	fileRW = SDL_RWFromFile(buffer, "rb");
	if (!fileRW) {
		printf("ERROR::initGraphics - could not open '%s' font\n", buffer);
        fflush(stdout);
        delete[] buffer;
		return false;
	} else {
        font = TTF_OpenFontRW(fileRW, 1, FONT_SIZE);
    }
    delete[] buffer;

    printf(">> WII.DEBUG.INIT_GRAPHICS #3 <<\n");
    fflush(stdout);


	// GAME SCREEN
	SDL_ShowCursor( SDL_DISABLE );
#ifdef PC
    SDL_WM_SetCaption("Rockbot Game", "Rockbot Game");
#endif
    set_video_mode();
#ifdef PC
    std::string icon_filename = FILEPATH + "shared/images/window_icon.png";
    SDL_RWops *rwop = SDL_RWFromFile(icon_filename.c_str(), "rb");
    if (rwop) {
        SDL_Surface* icon_img = IMG_Load_RW(rwop, 1);
        if (icon_img != NULL) {
            SDL_WM_SetIcon(icon_img, NULL);
        }
    } else {
        std::cout << "graphicsLib::initGraphics(set-window-icon): rwop for [" << icon_filename << "] is NULL " << std::endl;
    }
#endif
	// other loading methods

    printf(">> WII.DEBUG.INIT_GRAPHICS.END <<\n");
    fflush(stdout);

    load_shared_graphics();

    return true;
}

void graphicsLib::load_shared_graphics()
{
    std::string filename = GAMEPATH + "shared/images/config_bg.png";
    surfaceFromFile(filename, &config_menu);

    water_tile = SDLSurfaceFromFile(GAMEPATH + "/shared/images/water_tile.png");
    SDL_SetAlpha(water_tile, SDL_SRCALPHA, 120);
    _config_menu_pos.x = 0;
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

    /*
    // clear non-used areasfor when screen is greater than game-size
    if (game_screen->w > RES_W) {
        clear_area_no_adjust(0, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0);
        clear_area_no_adjust(_screen_resolution_adjust.x+RES_W, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0);
    }
    if (game_screen->h > RES_H) {
        clear_area_no_adjust(0, 0, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0);
        clear_area_no_adjust(0, _screen_resolution_adjust.y + RES_H, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0);
    }
    */

    if (_screen_resolution_adjust.x > 0 || _screen_resolution_adjust.y > 0) {
        copyArea(st_position(-_screen_resolution_adjust.x, -_screen_resolution_adjust.y), &_screen_border, &gameScreen);
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

    filename = StringUtils::clean_filename(filename);
    rwop = SDL_RWFromFile(filename.c_str(), "rb");

    if (!rwop) {
        std::cout << "DEBUG.SDLSurfaceFromFile - Error in graphicsLib::SDLSurfaceFromFile - file '" << filename << "' not found\n";
            return NULL;
    }
    spriteCopy = IMG_Load_RW(rwop, 1);
    if (spriteCopy == NULL) {
        std::cout << "[graphicsLib::SDLSurfaceFromFile] Error on IMG_Load_RW, could not load image '" << filename << "'. Details: " << IMG_GetError() << std::endl;
    }
    if (game_screen == NULL || game_screen->format == NULL) {
        return NULL;
    }

    SDL_Surface *res_surface = SDL_DisplayFormat(spriteCopy);
    SDL_FreeSurface(spriteCopy);
    SDL_SetColorKey(res_surface, SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));

    return res_surface;
}


void graphicsLib::surfaceFromFile(string filename, struct graphicsLib_gSurface* res)
{
    if (res == NULL) {
        return;
    }
    res->freeGraphic();
    // try a few times to load the file to prevent intermitent errors //
    for (int i=0; i<LOAD_FILE_NUMBER_TRIES; i++) {
        res->set_surface(SDLSurfaceFromFile(filename));
        if (res->get_surface() != NULL) {
            break;
        }
    }
    if (res->get_surface() == NULL) {
        std::cout << "surfaceFromFile - error loading file: '" << filename << "'" << std::endl;
        _debug_msg_pos = 1;
        show_debug_msg(filename);
        _debug_msg_pos = 0;
        printf("graphicsLib::surfaceFromFile - error loading SDL surface from file '%s'.\n", filename.c_str());
        fflush(stdout);
        timer.delay(1000);
        show_debug_msg("EXIT #05");
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::surfaceFromFile"), filename);
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
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::loadTileset"), filename);
	}
}


void graphicsLib::copySDLArea(struct st_rectangle origin_rectangle, struct st_position destiny_pos, SDL_Surface* surfaceOrigin, SDL_Surface* surfaceDestiny, bool fix_colors=true)
{
    UNUSED(fix_colors);

    if (!surfaceOrigin) {
        std::cout << "copySDLArea - ERROR surfaceOrigin is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.1.0");
        return;
    }
    if (!surfaceDestiny) {
        std::cout << "copySDLArea - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.1.1");
        return;
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
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "ERROR surfaceOrigin is NULL");
#endif
        show_debug_msg("ERROR #20");
        return;
    }
    if (!surfaceDestiny) {
        std::cout << "copySDLPortion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "ERROR surfaceDestiny is NULL");
#endif
        show_debug_msg("ERROR #21");
        return;
    }

    //std::cout << "GRAPHLIB::copySDLPortion - src.x[" << src.x << "]" << std::endl;
    //std::cout << "GRAPHLIB::copySDLPortion - src.w[" << src.w << "]" << std::endl;
    //std::cout << "GRAPHLIB::copySDLPortion - surfaceOrigin.w[" << surfaceOrigin->w << "]" << std::endl;

    if (src.x >= surfaceOrigin->w || (src.x+src.w) > surfaceOrigin->w) {
        //printf(">> Invalid X portion src.x[%d], src.w[%d] for image.w[%d] <<\n", src.x, src.w, surfaceOrigin->w);
        fflush(stdout);
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "Invalid X portion <<\n");
#endif
        return;
    }
    if (src.y >= surfaceOrigin->h || (src.y+src.h) > surfaceOrigin->h) {
        printf(">> Invalid Y portion[%d] h[%d] for image.w[%d] and image.h[%d] <<\n", src.y, src.h, surfaceOrigin->w, surfaceOrigin->h);
        fflush(stdout);
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "Invalid Y portion <<\n");
#endif
        return;
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
        show_debug_msg("ERROR #21.3");
        return;
    }
    if (surfaceDestiny == &gameScreen) {
        pos.x += _screen_adjust.x;
        pos.y += _screen_adjust.y;
    }
    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}


void graphicsLib::copyArea(struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny)
{
    if (!surfaceDestiny->get_surface()) {
        std::cout << "copyArea - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.4");
        return;
    }
    st_rectangle origin_rectangle(0, 0, surfaceOrigin->width, surfaceOrigin->height);
    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}

void graphicsLib::copyAreaWithAdjust(struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny)
{
    if (!surfaceDestiny->get_surface()) {
        std::cout << "copyAreaWithAdjust - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.4");
        return;
    }
    int w = surfaceOrigin->width;
    int h = surfaceOrigin->height;
    st_rectangle origin_rectangle(0, 0, w, h);
    //pos.x += _screen_adjust.x;
    //pos.y += _screen_adjust.y;

    copySDLArea(origin_rectangle, pos, surfaceOrigin->get_surface(), surfaceDestiny->get_surface());
}

void graphicsLib::placeTile(struct st_position pos_origin, struct st_position pos_destiny, struct graphicsLib_gSurface* gSurface)
{
    if (!gSurface->get_surface()) {
        std::cout << "placeTile - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.5");
        return;
    }
    struct st_rectangle origin_rectangle;

	origin_rectangle.x = pos_origin.x * TILESIZE;
	origin_rectangle.y = pos_origin.y * TILESIZE;

	origin_rectangle.w = TILESIZE;
    origin_rectangle.h = TILESIZE;


    pos_destiny.x += _screen_adjust.x;
    pos_destiny.y += _screen_adjust.y;
    copySDLArea(origin_rectangle, pos_destiny, tileset, gSurface->get_surface());
}

void graphicsLib::place_easymode_block_tile(st_position destiny, graphicsLib_gSurface& surface)
{
    destiny.x += _screen_adjust.x;
    destiny.y += _screen_adjust.y;

    copySDLArea(st_rectangle(0, 0, TILESIZE, TILESIZE), destiny, _easymode_block.get_surface(), surface.get_surface());
}

void graphicsLib::place_hardmode_block_tile(st_position destiny, graphicsLib_gSurface &surface)
{
    destiny.x += _screen_adjust.x;
    destiny.y += _screen_adjust.y;
    copySDLArea(st_rectangle(0, 0, TILESIZE, TILESIZE), destiny, _hardmode_block.get_surface(), surface.get_surface());
}

void graphicsLib::place_anim_tile(int anim_tile_id, st_position pos_destiny, struct graphicsLib_gSurface* dest_surface)
{

    //std::cout << "place_anim_tile - id[" << anim_tile_id << "]" << std::endl;
    if (anim_tile_id >= ANIM_TILES_SURFACES.size()) {
        std::cout << "place_anim_tile - ERROR Invalid anim-tile-id: " << anim_tile_id << " - ignoring..." << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "place_anim_tile - ERROR Invalid anim-tile-id[%d], ignoring.", anim_tile_id);
#endif
        return;
    }

    struct graphicsLib_gSurface* tile_ref = &ANIM_TILES_SURFACES.at(anim_tile_id);


    if (tile_ref->get_surface() == NULL) {
        std::cout << "place_anim_tile - ERROR surfaceDestiny is NULL for id " << anim_tile_id << " - ignoring..." << std::endl;
        char debug_msg[255];
        sprintf(debug_msg, "EXIT:place_anim_tile[%d][%d]", anim_tile_id, ANIM_TILES_SURFACES.size());
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "place_anim_tile - ERROR surfaceDestiny is NULL for id[%d]", anim_tile_id);
#endif
        show_debug_msg(debug_msg);
        return;
    }


    struct st_rectangle origin_rectangle;

    origin_rectangle.x = ANIM_TILES_TIMERS.at(anim_tile_id).frame_pos * TILESIZE;
    origin_rectangle.y = 0;
    origin_rectangle.w = TILESIZE;
    origin_rectangle.h = TILESIZE;

    pos_destiny.x += _screen_adjust.x;
    pos_destiny.y += _screen_adjust.y;

    copySDLArea(origin_rectangle, pos_destiny, tile_ref->get_surface(), dest_surface->get_surface());

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
    st_position pos_destiny(dest_x+_screen_adjust.x, dest_y+_screen_adjust.y);
    if (origin_x < -1) {
        int anim_tile_id = (origin_x * -1) - 2;
        place_anim_tile(anim_tile_id, pos_destiny, &graphLib.gameScreen);
        return;
    }

    struct st_rectangle origin_rectangle(origin_x*TILESIZE, origin_y*TILESIZE, TILESIZE, TILESIZE);

    if (origin_rectangle.x < 0 || origin_rectangle.x > tileset->w) {
        std::cout << "[WARNING] GRAPHLIB::place_tile - invalid position #1, ignoring. origin.x[" << origin_x << "], origin.y[" << origin_y << "], origin.w[" << origin_rectangle.w << "]" << std::endl;
        return;
    } else if (origin_rectangle.y < 0 || origin_rectangle.y> tileset->h) {
        std::cout << "[WARNING] GRAPHLIB::place_tile - invalid position #2, ignoring. origin.x[" << origin_x << "], origin.y[" << origin_y << "], origin.w[" << origin_rectangle.w << "]" << std::endl;
        return;
    //} else {
        //std::cout << "GRAPHLIB::place_3rd_level_tile - origin.x[" << origin_x << "], origin.y[" << origin_y << "]" << std::endl;
    }

    copySDLArea(origin_rectangle, pos_destiny, tileset, game_screen);
}



void graphicsLib::showSurface(struct graphicsLib_gSurface* surfaceOrigin)
{
    if (!game_screen) {
        std::cout << "showSurface - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.6");
        return;
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
        show_debug_msg("ERROR #21.4");
        return;
    }

    if (surfaceOrigin == NULL) {
        std::cout << "showSurfaceRegionAt - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #31.0");
        return;
    }

    copySDLArea(origin_rectangle, pos_destiny, surfaceOrigin->get_surface(), game_screen);
}

void graphicsLib::showSurfacePortion(graphicsLib_gSurface *surfaceOrigin, const st_rectangle origin_rect, st_rectangle destiny_rect)
{
    if (!game_screen) {
        std::cout << "showSurfacePortion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("ERROR #21.2");
        return;
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
        show_debug_msg("ERROR #21.7");
        return;
    }

	struct st_rectangle origin_rectangle;
	struct st_position pos_destiny;

    if (surfaceOrigin->get_surface() == NULL) {
		std::cout << "Error: no data in surfaceOrigin at graphicsLib::showSurfaceAt." << std::endl;
        show_debug_msg("ERROR #41.0");
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

    if (surfaceOrigin->get_surface() == NULL) {
        std::cout << "CRITICAL ERROR!" << std::endl;
        show_debug_msg("ERROR #41.1");
        return;
    }

    // create a new surface
    struct graphicsLib_gSurface tmp;
    initSurface(st_size(surfaceOrigin->width, surfaceOrigin->height), &tmp);
    copyArea(st_position(0, 0), surfaceOrigin, &tmp);
    //struct st_position pos, struct graphicsLib_gSurface* surfaceOrigin, struct graphicsLib_gSurface* surfaceDestiny
    showSurfaceAt(&tmp, pos, false);
}

void graphicsLib::initSurface(struct st_size size, struct graphicsLib_gSurface* gSurface)
{
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

    gSurface->freeGraphic();
    SDL_Surface* temp_surface = NULL;
    SDL_Surface* rgb_surface = SDL_CreateRGBSurface(SDL_SWSURFACE , size.width, size.height, VIDEO_MODE_COLORS, 0, 0, 0, 0);
    if (rgb_surface != NULL) {
        temp_surface = SDL_DisplayFormat(rgb_surface);
        if (!temp_surface) {
            show_debug_msg("EXIT #21.INIT #1");
#ifdef PSP
            graphLib.psp_show_available_ram(100);
#endif
            show_debug_msg("EXIT #41.2");
            exception_manager::throw_general_exception(std::string("graphicsLib::initSurface #1"), "NO RAM?");
        }
        SDL_FreeSurface(rgb_surface);
    }



    SDL_FillRect(temp_surface, NULL, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
    SDL_SetColorKey(temp_surface, SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));

    gSurface->set_surface(temp_surface);


    if (gSurface->get_surface() == NULL) {
        show_debug_msg("EXIT #21.INIT #2");
        exception_manager::throw_general_exception(std::string("graphicsLib::initSurface #2"), "NO RAM?");
    }
	gSurface->width = size.width;
    gSurface->height = size.height;
}

void graphicsLib::clear_surface(graphicsLib_gSurface &surface)
{
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

    SDL_FillRect(surface.get_surface(), NULL, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
    SDL_SetColorKey(surface.get_surface(), SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
}


void graphicsLib::set_surface_alpha(int alpha, graphicsLib_gSurface& surface)
{
    if (surface.width <= 0 || surface.get_surface() == NULL) {
        //std::cout << "[WARNING] GRAPHLIB::set_surface_alpha[&] - invalid surface, ignoring" << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "[WARNING] GRAPHLIB::set_surface_alpha[&] - invalid surface, ignoring");
#endif
        return;
    }
    if (surface.is_rle_enabled == false) {
        SDL_SetColorKey(surface.get_surface(), SDL_RLEACCEL|SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
        surface.is_rle_enabled = true;
    }
    SDL_SetAlpha(surface.get_surface(), SDL_RLEACCEL|SDL_SRCALPHA, alpha);
}

void graphicsLib::set_surface_alpha(int alpha, graphicsLib_gSurface *surface)
{
    if (surface->width <= 0 || surface->get_surface() == NULL) {
        //std::cout << "[WARNING] GRAPHLIB::set_surface_alpha[*] - invalid surface, ignoring" << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "[WARNING] GRAPHLIB::set_surface_alpha[&] - invalid surface, ignoring");
#endif
        return;
    }
    if (surface->is_rle_enabled == false) {
        SDL_SetColorKey(surface->get_surface(), SDL_RLEACCEL|SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
        surface->is_rle_enabled = true;
    }
    SDL_SetAlpha(surface->get_surface(), SDL_RLEACCEL|SDL_SRCALPHA, alpha);
}


struct graphicsLib_gSurface graphicsLib::surfaceFromRegion(struct st_rectangle rect_origin, struct graphicsLib_gSurface& originalSurface)
{

	struct st_position destiny_pos;

	destiny_pos.x = 0;
	destiny_pos.y = 0;
	struct graphicsLib_gSurface res;
	initSurface(st_size(rect_origin.w, rect_origin.h), &res);

    if (res.get_surface() == NULL) {
        std::cout << "surfaceFromRegion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        show_debug_msg("EXIT #21.8");
        exception_manager::throw_general_exception(std::string("graphicsLib::surfaceFromRegion"), "surfaceDestiny is NULL");
    }

    /// @NOTE: removed for optimization test
    copySDLArea(rect_origin, destiny_pos, originalSurface.get_surface(), res.get_surface());
    /// @NOTE: removed for optimization test
	return res;
}



void graphicsLib::blank_screen() {
    blank_screen(0, 0, 0);
}

void graphicsLib::blank_screen(int r, int g, int b)
{
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

    SDL_FillRect(game_screen, NULL, SDL_MapRGB(game_screen->format, r, g, b));
}

void graphicsLib::blank_surface(graphicsLib_gSurface &surface)
{
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

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
    return draw_progressive_text(x, y, text, interrupt, 15);
}

int graphicsLib::draw_progressive_text(short x, short y, string text, bool interrupt, int delay)
{
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
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::draw_progressive_text, fount is NULL"), std::string(TTF_GetError()));
    }

    for (i=0; i<text.size(); i++) {
        input.read_input();
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
        if (input.wait_scape_time(delay) ==1) {
            return 1;
        }
        //timer.delay(delay);
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
    SDL_Color font_color = SDL_Color();
    font_color.r = color.r;
    font_color.g = color.g;
    font_color.b = color.b;
    x += _screen_resolution_adjust.x;
    y += _screen_resolution_adjust.y;
    SDL_Rect text_pos={x, y, 0, 0};
    if (!font) {
        printf("graphicsLib::draw_text - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #10");
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::draw_text, fount is NULL"), std::string(TTF_GetError()));
        // handle error
    }

    //std::cout << "RENDER #1[" << text << "]" << std::endl;
    /*
    if (text.length() == 1) {
        std::cout << "DEBUG #9" << std::endl;
    }
    */
    SDL_Surface* textSF = TTF_RenderUTF8_Solid(font, text.c_str(), font_color);

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
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::draw_text #2, fount is NULL"), std::string(TTF_GetError()));
        // handle error
    }
    text_pos.x += _screen_resolution_adjust.x;
    text_pos.y += _screen_resolution_adjust.y;
    std::cout << "RENDER #2[" << text << "]" << std::endl;
    SDL_Surface* textSF = TTF_RenderUTF8_Solid(font, text.c_str(), font_color);
    SDL_Surface* textSF_format = SDL_DisplayFormat(textSF);
    SDL_FreeSurface(textSF);
    SDL_BlitSurface(textSF_format, 0, surface.get_surface(), &text_pos);
    SDL_FreeSurface(textSF_format);
}

void graphicsLib::draw_centered_text(short y, string text, st_color font_color)
{
    draw_centered_text(y, text, gameScreen, font_color);
}

void graphicsLib::draw_centered_text(short y, string text)
{
    draw_centered_text(y, text, gameScreen, st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
}

void graphicsLib::draw_centered_text(short y, string text, graphicsLib_gSurface &surface, st_color temp_font_color)
{
	SDL_Color font_color;
    font_color.r = temp_font_color.r;
    font_color.g = temp_font_color.g;
    font_color.b = temp_font_color.b;
	SDL_Rect text_pos={0, y, 0, 0};
	if (!font) {
        printf("graphicsLib::draw_centered_text - TTF_OpenFont: %s\n", TTF_GetError());
        show_debug_msg("EXIT #12");
        exception_manager::throw_file_not_found_exception(std::string("graphicsLib::draw_centered_text, fount is NULL"), std::string(TTF_GetError()));
	}
    SDL_Surface* textSF = TTF_RenderUTF8_Solid(font, text.c_str(), font_color);
	if (textSF == NULL) {
		return;
	}
	if (text.size() > 0) {
		text_pos.x = RES_W/2 - textSF->w/2;
	}
    text_pos.y = y;
    text_pos.x += _screen_resolution_adjust.x;
    text_pos.y += _screen_resolution_adjust.y;
    SDL_Surface* textSF_format = SDL_DisplayFormat(textSF);
    SDL_FreeSurface(textSF);
    SDL_BlitSurface(textSF_format, 0, surface.get_surface(), &text_pos);
    SDL_FreeSurface(textSF_format);
}



Uint8 graphicsLib::getColorNumber(Uint8 r, Uint8 g, Uint8 b) {
    if (game_screen == NULL || game_screen->format == NULL) {
        return 0;
    }

    return SDL_MapRGB(game_screen->format, r, g, b);
}


void graphicsLib::drawCursor(st_position pos) {

    //std::cout << "drawCursor - x: " << pos.x << ", y: " << pos.y << std::endl;
	draw_text(pos.x, pos.y, ">");
}

void graphicsLib::eraseCursor(st_position pos) {
    clear_area(pos.x, pos.y, CURSOR_SPACING, CURSOR_SPACING, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);
}

void graphicsLib::blink_screen(Uint8 r, Uint8 g, Uint8 b) {
	int i;
    struct graphicsLib_gSurface screen_copy;

    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

	initSurface(st_size(gameScreen.width, gameScreen.height), &screen_copy);
	copyArea(st_position(0, 0), &gameScreen, &screen_copy);

	for (i=0; i<4; i++) {
        SDL_FillRect(game_screen, 0, SDL_MapRGB(game_screen->format, r, g, b));

		updateScreen();
        timer.delay(80);

        SDL_BlitSurface(screen_copy.get_surface(), 0, game_screen, 0);
		updateScreen();
        timer.delay(80);
    }
}

void graphicsLib::blink_surface_into_screen(struct graphicsLib_gSurface &surface)
{
    st_color color_white(235, 235, 235);
    st_color color_black(0, 0, 0);

    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

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
    std::string filename = FILEPATH + "images/icons.png";

	// big icon
	surfaceFromFile(filename, &tmp);
    int icon_size = tmp.height/2;
    for (int i=0; i<(tmp.width/(icon_size)); i++) {
		weapon_icons.push_back(graphicsLib_gSurface());
        initSurface(st_size(icon_size, icon_size*2), &weapon_icons.at(weapon_icons.size()-1));
        copyArea(st_rectangle(i*icon_size, 0, icon_size, icon_size*2), st_position(0, 0), &tmp, &(weapon_icons.at(weapon_icons.size()-1)));
	}

	// small icons
    filename = FILEPATH + "images/icons_small.png";
	surfaceFromFile(filename, &tmp);
	for (int i=0; i<(tmp.width/8); i++) {
		small_weapon_icons.push_back(graphicsLib_gSurface());
		initSurface(st_size(8, 8), &small_weapon_icons.at(small_weapon_icons.size()-1));
		copyArea(st_rectangle(i*8, 0, 8, 8), st_position(0, 0), &tmp, &(small_weapon_icons.at(small_weapon_icons.size()-1)));
	}


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


void graphicsLib::draw_weapon_icon(short wpn_n, st_position point, bool active)
{
    st_position pos;
    int icon_size = weapon_icons.at(wpn_n).width;
    int spacer_h = icon_size+2;

    if (point.x == 0) {
        pos.x = WPN_COLUMN1_X;
    } else {
        pos.x = WPN_COLUMN2_X;
    }

    pos.y = WPN_COLUMN_Y-3+point.y*spacer_h;

    //std::cout << "### GRAPH::draw_weapon_icon - wpn_n: " << wpn_n<< ", point.x: " << point.x << ", point.y: " << point.y << ", pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;


    clear_area(pos.x, pos.y, icon_size, icon_size, 0, 0, 0);
    if (active == true) {
        showSurfaceRegionAt(&weapon_icons.at(wpn_n), st_rectangle(0, 0, icon_size, icon_size), pos);
    } else {
        showSurfaceRegionAt(&weapon_icons.at(wpn_n), st_rectangle(0, icon_size, icon_size, icon_size), pos);
    }
}

void graphicsLib::draw_weapon_tooltip_icon(short weapon_n, st_position position, bool disabled)
{
    if (weapon_n >= weapon_icons.size()) {
        return;
    }
    int icon_size = weapon_icons.at(weapon_n).width;
    if (disabled == true) {
        showSurfaceRegionAt(&weapon_icons.at(weapon_n), st_rectangle(0, 0, icon_size, icon_size), position);
    } else {
        showSurfaceRegionAt(&weapon_icons.at(weapon_n), st_rectangle(0, icon_size, icon_size, icon_size), position);
    }
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
    copyArea(st_rectangle(0, 0, TILESIZE, TILESIZE), st_position(x, 199), spriteCopy, &gameScreen);
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
    copyArea(st_rectangle(0, 0, TILESIZE, TILESIZE), st_position(x, 199), spriteCopy, &gameScreen);
    //copyArea(st_position(x, 196), spriteCopy, &gameScreen);
}

void graphicsLib::draw_weapon_menu_bg(Uint8 current_hp, graphicsLib_gSurface* player_frame, short max_hp) {
    int icon_size = weapon_icons.at(0).width;
    int spacer_h = icon_size+2;

    showSurfaceAt(&ingame_menu, st_position((RES_W-ingame_menu.width)*0.5, (RES_H-ingame_menu.height)*0.5));

    showSurfaceRegionAt(&weapon_icons.at(0), st_rectangle(0, icon_size, icon_size, icon_size), st_position(WPN_COLUMN1_X, 50));
    draw_horizontal_hp_bar(WPN_COLUMN_Y, 2, current_hp, 3, max_hp);

    for (int i=1; i<6; i++) {
        if (game_save.stages[i] == 1) {
            //std::cout << ">> #1 graphicsLib::draw_weapon_menu_bg - stage[" << i << "]: " << game_save.stages[i] << std::endl;
            showSurfaceRegionAt(&weapon_icons.at(i), st_rectangle(0, icon_size, icon_size, icon_size), st_position(WPN_COLUMN1_X, 50+(i)*spacer_h));
            draw_horizontal_hp_bar(WPN_COLUMN_Y+(i)*spacer_h, 2, game_save.items.weapons[i], 3, max_hp);
		}
    }

    for (int i=6; i<=9; i++) {
        if (game_save.stages[i] == 1) {
            //std::cout << ">> #3 graphicsLib::draw_weapon_menu_bg - stage[" << i << "]: " << game_save.stages[i] << std::endl;
            showSurfaceRegionAt(&weapon_icons.at(i), st_rectangle(0, icon_size, icon_size, icon_size), st_position(182, 50+(i-5)*spacer_h));
            draw_horizontal_hp_bar(WPN_COLUMN_Y+(i-5)*spacer_h, 3, game_save.items.weapons[i], 3, max_hp);
		}
	}

    // coil and jet
    if (game_save.stages[COIL_GOT_STAGE] == 1) {
        int wpn_icon_n = 9;
        int menu_row = 4;
        showSurfaceRegionAt(&weapon_icons.at(wpn_icon_n), st_rectangle(0, icon_size, icon_size, icon_size), st_position(182, 50+(menu_row)*spacer_h));
        draw_horizontal_hp_bar(WPN_COLUMN_Y+(menu_row)*spacer_h, 3, game_save.items.weapons[wpn_icon_n], 3, max_hp);
    }
    if (game_save.stages[JET_GOT_STAGE] == 1) {
        int wpn_icon_n = 10;
        int menu_row = 5;
        showSurfaceRegionAt(&weapon_icons.at(wpn_icon_n), st_rectangle(0, icon_size, icon_size, icon_size), st_position(182, 50+(menu_row)*spacer_h));
        draw_horizontal_hp_bar(WPN_COLUMN_Y+(menu_row)*spacer_h, 3, game_save.items.weapons[wpn_icon_n], 3, max_hp);
    }


    // player life
    copyArea(st_position(16, 195), player_frame, &gameScreen);
    int item_text_pos = 217;
    std::stringstream ss;
	ss.str(std::string());
    ss << "0" << (short)game_save.items.lifes;
    draw_text(48, item_text_pos, ss.str());


    ss.str(std::string());
    ss << "0" << (short)game_save.items.energy_tanks;
    draw_text(93, item_text_pos, ss.str());

	ss.str(std::string());
    ss << "0" << (short)game_save.items.weapon_tanks;
    draw_text(117, item_text_pos, ss.str());

	ss.str(std::string());
    ss << "0" << (short)game_save.items.special_tanks;
    draw_text(142, item_text_pos, ss.str());


    // @TODO: add coins support //
    /*
	ss.str(std::string());
    ss << "0" << (int)game_save.items.bolts;
    draw_text(278, item_text_pos, ss.str());
    */

    // @TODO: add energy balancer support //
    /*
    if (item_ref->balancer > 0) {
        copyArea(st_position(245, 198), &energy_balancer, &gameScreen);
	}
    */

    if (game_save.armor_pieces[ARMOR_TYPE_ARMS] == true) {
        copyArea(st_position(198, 200), &armor_icon_arms, &gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_TYPE_BODY] == true) {
        copyArea(st_position(221, 200), &armor_icon_body, &gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_TYPE_LEGS] == true) {
        copyArea(st_position(175, 200), &armor_icon_legs, &gameScreen);
    }

    int config_text_pos_x = RES_W - 10 - (strings_map::get_instance()->get_ingame_string(strings_ingame_config, game_config.selected_language).length()+4)*8;
    draw_text(config_text_pos_x, 22, strings_map::get_instance()->get_ingame_string(strings_ingame_config, game_config.selected_language) + std::string(" (R)"));
    draw_text(10, 188, strings_map::get_instance()->get_ingame_string(strings_ingame_life, game_config.selected_language));
    draw_text(111, 188, strings_map::get_instance()->get_ingame_string(strings_ingame_item, game_config.selected_language));
    draw_text(187, 188, strings_map::get_instance()->get_ingame_string(strings_ingame_armor,game_config.selected_language));

    // @TODO: add coins support //
    //draw_text(268, 188, strings_map::get_instance()->get_ingame_string(strings_ingame_coin,game_config.selected_language));

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

     case 3:
    {
        int tpitch = dest->pitch;
               int spitch = surface->pitch;

               const int wd = ((dest->w / 2) < (surface->w))
                   ? (dest->w / 2) : (surface->w);

               const int hg = ((dest->h) < (surface->h*2))
                   ? (dest->h) : (surface->h*2);

               Uint8* tp = (Uint8*) dest->pixels;
               Uint8* sp = (Uint8*) surface->pixels;

               for (j = 0; j < hg; ++j)
               {
                 for (i = 0; i < 3 * wd; i += 3)
                 {
                   int i2 = i * 2;
                   tp[i2 + 0] = sp[i];
                   tp[i2 + 1] = sp[i + 1];
                   tp[i2 + 2] = sp[i + 2];
                   tp[i2 + 3] = sp[i];
                   tp[i2 + 4] = sp[i + 1];
                   tp[i2 + 5] = sp[i + 2];
                 }
                 tp += tpitch;
                 if (j % 2 != 0)  sp += spitch;
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


void graphicsLib::clear_area(short int x, short int y, short int w, short int h, short int r, short int g, short int b) {
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }
    //std::cout << ">> graphicsLib::clear_area - x: " << x << ", w: " << w << std::endl;
	SDL_Rect dest;
    dest.x = x + _screen_resolution_adjust.x;
    dest.y = y + _screen_resolution_adjust.y;
	dest.w = w;
	dest.h = h;
    SDL_FillRect(game_screen, &dest, SDL_MapRGB(game_screen->format, r, g, b));
}

void graphicsLib::clear_area_alpha(short x, short y, short w, short h, short r, short g, short b, int alpha)
{
    if (game_screen == NULL || game_screen->format) {
        return;
    }
    SDL_Rect dest;
    dest.x = x + _screen_resolution_adjust.x;
    dest.y = y + _screen_resolution_adjust.y;
    dest.w = w;
    dest.h = h;
    SDL_FillRect(game_screen, &dest, SDL_MapRGBA(game_screen->format, 0xff, 0xff, 0x00, 0xff));
    //SDL_FillRect(game_screen, &dest, SDL_MapRGB(game_screen->format, r, g, b));
}

void graphicsLib::clear_area_no_adjust(short x, short y, short w, short h, short r, short g, short b)
{
    if (game_screen == NULL || game_screen->format == NULL) {
        return;
    }

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

    if (surface.get_surface() == NULL || surface.get_surface()->format == NULL) {
        return;
    }

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

void graphicsLib::clear_surface_area_no_adjust(short x, short y, short w, short h, short r, short g, short b, graphicsLib_gSurface &surface) const
{
    if (surface.get_surface() == NULL || surface.get_surface()->format == NULL) {
        return;
    }
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;
    SDL_FillRect(surface.get_surface(), &dest, SDL_MapRGB(surface.get_surface()->format, r, g, b));
}


void graphicsLib::show_config_bg()
{
    showSurface(&config_menu);
}

void graphicsLib::show_config_bg_animation()
{
    int total = RES_H/2-15+1;
    for (int i=0; i<total; i+=2) {
        // top part
        int y = RES_H/2-i-15;
        int h = 15+i;
        int w = config_menu.width;
        copyArea(st_rectangle(0, 0, w, h), st_position(0, y), &config_menu, &gameScreen);

        // bottom_part
        y = config_menu.height-15-i;
        copyArea(st_rectangle(0, y, w, h), st_position(0, RES_H/2), &config_menu, &gameScreen);


        //std::cout << "y[" << y << "], h[" << h << "]" << std::endl;
        updateScreen();
        timer.delay(3);
    }
}


// position - 0: center, 1: top, 2: bottom
void graphicsLib::show_dialog(Uint8 position)
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
}

void graphicsLib::show_dialog_button(Uint8 position)
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

    show_btn_a(st_position(posX+dialog_surface.width-_btn_a_surface.width-2-TILESIZE, posY+dialog_surface.height-_btn_a_surface.height-TILESIZE/2));
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
#ifdef ANDROID
    game_services.firebase_log(msg);
#endif

    clear_area(0, 0, RES_W, 50, 50, 50, 50);
    draw_text(10, _debug_msg_pos*12+10, msg, gameScreen);
    updateScreen();
    timer.delay(DEBUG_MSG_DELAY);
}

void graphicsLib::draw_path(st_position initial_point, st_position final_point, short duration)
{
    short move_step = 1;
    // calculate distance
    int distance = 0;
    int mode = 0;

    initial_point.x += 2;
    final_point.x += 2;
    int horizontal_adjust = 10;

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
    if (distance == 0) {
        return;
    }
    int part_duration = duration / (distance/move_step*2);
    if (distance < 0) {
        move_step = -move_step;
    }
    if (duration > 0) {
        for (int i=0; i<abs(distance/move_step); i++) {
            if (mode == 0) { // HORIZONTAL
                clear_area(initial_point.x + i*move_step, initial_point.y, abs(move_step), 4, 255, 0, 255);
            } else {
                clear_area(initial_point.x, initial_point.y - (i+1)*move_step, 4, abs(move_step), 255, 255, 0);
            } // VERTICAL
            updateScreen();
            timer.delay(part_duration);
        }
    } else {
        if (mode == 0) { // HORIZONTAL
            if (initial_point.x < final_point.x) {
                clear_area(initial_point.x+horizontal_adjust, initial_point.y+2, abs(distance)-horizontal_adjust, 4, 255, 255, 255);
            } else {
                clear_area(final_point.x+horizontal_adjust, initial_point.y+2, abs(distance)-horizontal_adjust, 4, 255, 255, 255);
            }
        } else { // VERTICAL
            clear_area(initial_point.x, initial_point.y-distance+2, 4, abs(distance)-2, 255, 255, 255);
        }
        updateScreen();
    }
}

void graphicsLib::draw_horizontal_hp_bar(st_position pos, short int hp, short player_n, short max_hp) {
    int hp_percent = (100 * hp) / max_hp;
    int graph_lenght = (int)(hp_percent * 0.7);


    int y = (-28)*2+1;
    short int pos_x = pos.x + y + 58;

    st_color bar_color1(188, 188, 188);
    st_color bar_color2(255, 255, 255);

    //std::cout << "################## draw_horizontal_hp_bar::player_n: " << player_n << std::endl;

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
    int icon_size = weapon_icons.at(0).width;
    int spacer_h = icon_size+2;
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
        pos_y = spacer_h*pos.y + 53;
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
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_SWSURFACE | SDL_DOUBLEBUF);
#elif defined(WII)
    //game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_HWSURFACE);
    _video_filter = VIDEO_FILTER_BITSCALE;
    game_screen_scaled = SDL_SetVideoMode(RES_W*2, RES_H*2, VIDEO_MODE_COLORS, SDL_HWSURFACE);
    if (game_screen != NULL) {
        SDL_FreeSurface(game_screen);
    }
    game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, RES_W, RES_H, VIDEO_MODE_COLORS, 0, 0, 0, 255);
#elif defined(PSP)
    _video_filter = VIDEO_FILTER_NOSCALE;
    game_screen = SDL_SetVideoMode(RES_W, RES_H, VIDEO_MODE_COLORS, SDL_SWSURFACE|SDL_ANYFORMAT|SDL_NOFRAME);

#elif defined(DREAMCAST)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, 24, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
#elif defined(PLAYSTATION2)
    game_screen = SDL_SetVideoMode(RES_W, RES_H, 24, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    _video_filter = VIDEO_FILTER_NOSCALE;
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
            game_screen_scaled = SDL_SetVideoMode(RES_W*2, RES_H*2, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF);
        } else {
            game_screen_scaled = SDL_SetVideoMode(RES_W*2, RES_H*2, VIDEO_MODE_COLORS, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
        }
        if (game_screen != NULL) {
            SDL_FreeSurface(game_screen);
        }
        SDL_Surface *temp_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, RES_W, RES_H, VIDEO_MODE_COLORS, 0, 0, 0, 255);
        game_screen = SDL_DisplayFormat(temp_screen);
        SDL_FreeSurface(temp_screen);
    }
#endif

	if (!game_screen) {
        std::cout << "FATAL-ERROR::initGraphics Could not create game_screen" << std::endl;
        show_debug_msg("EXIT #13");
        exception_manager::throw_general_exception(std::string("graphicsLib::initGraphics, fount is NULL"), std::string("Could not create game_screen"));
	}
    gameScreen.set_surface(game_screen);
    gameScreen.video_screen = true;

    gameScreen.width = game_screen->w;
    gameScreen.height = game_screen->h;
    _screen_resolution_adjust.x = (game_screen->w - RES_W)/2;
    _screen_resolution_adjust.y = (game_screen->h - RES_H)/2;

    // need border
    if (_screen_resolution_adjust.x > 0 || _screen_resolution_adjust.y > 0) {
        initSurface(st_size(game_screen->w, game_screen->h), &_screen_border);
        clear_surface_area_no_adjust(0, 0, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0, _screen_border);
        clear_surface_area_no_adjust(0, game_screen->h-_screen_resolution_adjust.y, game_screen->w, _screen_resolution_adjust.y, 0, 0, 0, _screen_border);


        clear_surface_area_no_adjust(0, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0, _screen_border);
        clear_surface_area_no_adjust(game_screen->w-_screen_resolution_adjust.x, 0, _screen_resolution_adjust.x, game_screen->h, 0, 0, 0, _screen_border);

    }

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
        projectile_surface.push_back(st_surface_with_direction());
        if (filename.length() > 0 && filename.find(".png") != std::string::npos) {
            //std::cout << "GRAPHLIB::preload_images - i[" << i << "], list.size[" << projectile_surface.size() << "]" << std::endl;
            surfaceFromFile(filename, &projectile_surface.at(i).surface[ANIM_DIRECTION_RIGHT]);
            flip_image(projectile_surface.at(i).surface[ANIM_DIRECTION_RIGHT], projectile_surface.at(i).surface[ANIM_DIRECTION_LEFT], flip_type_horizontal);
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
    filename = FILEPATH + "images/animations/player_death_explosion.png";
    surfaceFromFile(filename, &explosion_player_death);

    filename = FILEPATH + "images/animations/dash_dust.png";
	surfaceFromFile(filename, &dash_dust);

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
    //std::cout << "graphicsLib::preload_anim_tiles - max: " << max << std::endl;
    for (int i=0; i<max; i++) {
        std::string file(GameMediator::get_instance()->anim_tile_list.at(i).filename);
        if (file.length() < 1) {
            //std::cout << "### graphicsLib::preload_anim_tiles::STOP, file: " << file << std::endl;
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

void graphicsLib::flip_image(graphicsLib_gSurface original, graphicsLib_gSurface& res, e_flip_type flip_mode)
{


    //std::cout << ">>>>>>>>>>>>>>>>>> GRAPHLIB::flip_image <<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

    //Pointer to the soon to be flipped surface
    res = original;
    //initSurface(st_size(original.width, original.height), &res);

    //If the surface must be locked
    if (SDL_MUSTLOCK( original.get_surface() )) {
        //Lock the surface
        SDL_LockSurface( original.get_surface() );
    }

    //Go through columns
    for (int x = 0, rx = original.get_surface()->w - 1; x < original.get_surface()->w; x++, rx-- ) {
        //Go through rows
        for (int y = 0, ry = original.get_surface()->h - 1; y < original.get_surface()->h; y++, ry-- ) {
            //Get pixel
            Uint32 pixel = original.get_pixel(x, y);

            //Copy pixel
            if ((flip_mode == flip_type_both)) {
                res.put_pixel(rx, ry, pixel);
            } else if (flip_mode == flip_type_horizontal) {
                res.put_pixel(rx, y, pixel );
            } else if(flip_mode == flip_type_vertical) {
                res.put_pixel(x, ry, pixel );
            } else {
                std::cout << "UNKNOWN flip mode [" << flip_mode << "]" << std::endl;
                char enum_str[20];
                sprintf(enum_str, "%d", flip_mode);
                exception_manager::throw_param_exception(std::string("graphicsLib::flip_image, invalid mode"), std::string(enum_str));
            }
        }
    }
}

void graphicsLib::set_spriteframe_surface(st_spriteFrame *frame, graphicsLib_gSurface newSurface)
{
    initSurface(st_size(newSurface.width, newSurface.height), &frame->frameSurface);
    copyArea(st_position(0, 0), &newSurface, &frame->frameSurface);
}

void graphicsLib::place_water_tile(st_position dest)
{
    //copySDLArea(struct st_rectangle origin_rectangle, struct st_position destiny_pos, SDL_Surface* surfaceOrigin, SDL_Surface* surfaceDestiny, bool fix_colors=true)

    //std::cout << "GRAPH::place_water_tile[" << dest.x << "][" << dest.y << "]" << std::endl;

    if (dest.x >= -TILESIZE) {
        copySDLArea(st_rectangle(0, 0, TILESIZE, TILESIZE), dest, water_tile, game_screen);
    }
}

// zoom from a small size to the picture size
void graphicsLib::zoom_image(st_position dest, graphicsLib_gSurface picture, int smooth)
{
    SDL_Surface *rotozoom_picture;

    /*
    // Zoom and display the picture
    framemax=4*360; frameinc=1;
    for (framecount=360; framecount<framemax; framecount += frameinc) {
        if ((framecount % 360)==0) frameinc++;
        input.read_input();
        if (input.p1_input[BTN_JUMP] == 1) {
            SDL_FreeSurface(rotozoom_picture);
            return;
        }
        blank_screen();
        zoomxf=(float)framecount/(float)framemax;
        zoomxf=1.5*zoomxf*zoomxf;
        zoomyf=0.5+fabs(1.0*sin((double)framecount/80.0));
        if ((framecount % 120)==0) {
         printf ("  Frame: %i   Zoom: x=%.2f y=%.2f\n",framecount,zoomxf,zoomyf);
        }
        if ((rotozoom_picture = zoomSurface(picture.get_surface(), zoomxf, zoomyf, smooth))!=NULL) {
            dest.x = (gameScreen.get_surface()->w - rotozoom_picture->w)/2;;
            dest.y = (gameScreen.get_surface()->h - rotozoom_picture->h)/2;
            dest.w = rotozoom_picture->w;
            dest.h = rotozoom_picture->h;
            if (SDL_BlitSurface(rotozoom_picture, NULL, gameScreen.get_surface(), &dest) < 0 ) {
                fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
                break;
            }
            SDL_FreeSurface(rotozoom_picture);
        }

        // Display by flipping screens
        timer.delay(100);
        updateScreen();
    }
    */
    st_position center(dest.x+picture.width/2, dest.y+picture.height/2);
    std::cout << ">>>>>>>>>>> dest[" << dest.x << "][" << dest.y << "]" << std::endl;
    std::cout << "center[" << center.x << "][" << center.y << "]" << std::endl;

    for (float i=0.1; i<1.0; i+=0.03) {
        if ((rotozoom_picture = zoomSurface(picture.get_surface(), i, i, smooth)) != NULL) {
        //double angle = 360*i;
        //if ((rotozoom_picture = rotozoomSurface(picture.get_surface(), angle, 1.0, smooth)) != NULL) {
            std::cout << "GRAPHLIB::ZOOM #1 [" << i << "]" << std::endl;
            struct st_rectangle origin_rectangle(0, 0, rotozoom_picture->w, rotozoom_picture->h);

            st_position dest_zoom(center.x-rotozoom_picture->w/2, center.y-rotozoom_picture->h/2);
            std::cout << "rotozoom_picture[" << rotozoom_picture->w << "][" << rotozoom_picture->h << "]" << std::endl;
            std::cout << "dest_zoom[" << dest_zoom.x << "][" << dest_zoom.y << "]" << std::endl;

            // clear area
            clear_area(dest_zoom.x, dest_zoom.y, rotozoom_picture->w, rotozoom_picture->h, CONFIG_BGCOLOR_R, CONFIG_BGCOLOR_G, CONFIG_BGCOLOR_B);


            copySDLArea(origin_rectangle, dest_zoom, rotozoom_picture, game_screen, false);
            updateScreen();
            timer.delay(20);
            SDL_FreeSurface(rotozoom_picture);
        } else {
            std::cout << "Error creating zoomed surface" << std::endl;
        }
    }

    /* Pause for a sec */
    SDL_Delay(100);
}

void graphicsLib::rotate_image(graphicsLib_gSurface &picture, double angle)
{
    SDL_Surface *rotozoom_picture;

    SDL_Surface *alpha_surface = SDL_DisplayFormatAlpha(picture.get_surface());

    if ((rotozoom_picture = rotozoomSurface(alpha_surface, angle, 1.0, true)) != NULL) {
        SDL_Surface *res_surface = SDL_DisplayFormatAlpha(rotozoom_picture);
        SDL_FreeSurface(rotozoom_picture);
        SDL_SetColorKey(res_surface, SDL_SRCCOLORKEY, SDL_MapRGB(game_screen->format, COLORKEY_R, COLORKEY_G, COLORKEY_B));
        picture.set_surface(res_surface);
        //std::cout << "GRAPHLIB::rotate_image - #3: rotozoom_picture w[" << picture.width << "], h[" << picture.height << "]" << std::endl;
    } else {
        std::cout << "GRAPHLIB::rotate_image - Error generating rotated image" << std::endl;
    }
    SDL_FreeSurface(alpha_surface);
}

void graphicsLib::rotated_from_image(graphicsLib_gSurface *picture, graphicsLib_gSurface &dest, double angle)
{
    SDL_Surface *rotozoom_picture;
    if ((rotozoom_picture = rotozoomSurface(picture->get_surface(), angle, 1.0, true)) != NULL) {
        dest.set_surface(rotozoom_picture);
    } else {
        std::cout << "GRAPHLIB::rotate_image - Error generating rotated image" << std::endl;
    }
}

#ifdef PSP
void graphicsLib::psp_show_available_ram(int n)
{
    char debug_msg[255];
    sprintf(debug_msg, "MEM[%d][%d]", n, (int)_ram_counter.ramAvailable());
    show_debug_msg(std::string(debug_msg));
    //std::cout << "unload_stage::RAM::BF='" << ram_counter.ramAvailable() << "'" << std::endl;
}
#endif



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

    //std::cout << "change_surface_color::colorkey_n[" << (int)colorkey_n << "]" << std::endl;
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

