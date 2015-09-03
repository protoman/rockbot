#include <iostream>

#include <SDL/SDL.h>				//Include da SDL
#include <SDL/SDL_image.h>		//Include da biblioteca SDL_Image
#include <SDL/SDL_mixer.h>	// Include da biblioteca SDL_Mixer
#include <SDL/SDL_ttf.h>		// Include da biblioteca SDL_ttf
#include <SDL/SDL_endian.h>
#include <SDL/SDL_mixer.h>

// GLOBAL/EXTERN VARIABLES
std::string FILEPATH;
std::string SAVEPATH;

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h>
#include "../file/file_io.h"
#include "savepng.h"

std::string EXEC_NAME("rockbot");

CURRENT_FILE_FORMAT::file_io fio;
CURRENT_FILE_FORMAT::file_game game_data;
CURRENT_FILE_FORMAT::file_stage stage_data;

SDL_Surface *tileset;
SDL_Surface *tileset_beaten;
SDL_Surface *screen;


void get_filepath()
{
    char *buffer = new char[MAXPATHLEN];
    getcwd(buffer, MAXPATHLEN);
    if(buffer != NULL){
        FILEPATH = std::string(buffer);
    }
    FILEPATH += "/";
    delete[] buffer;
    SAVEPATH = FILEPATH;
}


void load_tileset() {
    std::string filename = FILEPATH + "data/images/tilesets/default.png";
    SDL_RWops *rwop;

    rwop = SDL_RWFromFile(filename.c_str(), "rb");
    tileset = IMG_Load_RW(rwop, 1);
    if (!tileset) {
        std::cout << "ERROR: Could not find file '" << filename << "'\n";
        exit(-1);
    }

    filename = FILEPATH + "data/images/tilesets/default_beaten.png";
    /// @TODO - put beaten back
    rwop = SDL_RWFromFile(filename.c_str(), "rb");
    tileset_beaten = IMG_Load_RW(rwop, 1);
    if (!tileset_beaten) {
        std::cout << "ERROR: Could not find file '" << filename << "'\n";
        exit(-1);
    }
}


void copy_surface_area(SDL_Surface *surface, st_position origin, st_position destiny, int w, int h) {
    SDL_Rect src, dest;
    src.x = origin.x;
    src.y = origin.y;
    src.w = w;
    src.h = h;
    dest.x = destiny.x;
    dest.y = destiny.y;
    dest.w = w;
    dest.h = h;

    //std::cout << "placeTile - src.x: " << src.x << ", src.y: " << src.y << ", dest.x: " << dest.x << ", dest.y: " << dest.y << std::endl;

    if (!surface) {
        std::cout << "copySDLArea - ERROR surfaceOrigin is NULL\n";
        exit(-1);
    }
    if (!screen) {
        std::cout << "copySDLPortion - ERROR surfaceDestiny is NULL - ignoring..." << std::endl;
        exit(-1);
    }
    SDL_BlitSurface(surface, &src, screen, &dest);

}



void draw_map(int number, bool beaten) {
    struct st_position pos_origin;
    struct st_position pos_destiny;

    // bg color
    Uint32 color;
    color = SDL_MapRGB(screen->format, stage_data.maps[number].background_color.r, stage_data.maps[number].background_color.g, stage_data.maps[number].background_color.b);
    SDL_Rect dest;
    dest.x = 0;
    dest.y = number*MAP_H*TILESIZE;
    dest.w = MAP_W*TILESIZE;
    dest.h = MAP_H*TILESIZE;
    SDL_FillRect(screen, &dest, color);
    SDL_Surface *myset;

    if (beaten == true) {
        myset = tileset_beaten;
    } else {
        myset = tileset;
    }

    for (int i=0; i<2; i++) {
        if (strlen(stage_data.maps[number].backgrounds[i].filename) > 0) {
            SDL_Surface* bg;

            std::string bg_filename(FILEPATH + "data/images/map_backgrounds/" + stage_data.maps[number].backgrounds[i].filename);
            if (beaten == true) {
                std::string bg_beaten_filename = bg_filename.substr(0, bg_filename.length()-4);
                bg_filename = bg_beaten_filename + "_beaten.png";
            }
            SDL_RWops *rwop;

            rwop = SDL_RWFromFile(bg_filename.c_str(), "rb");
            bg = IMG_Load_RW(rwop, 1);
            if (bg) {
                int total_w = 0;
                while (total_w < MAP_W*TILESIZE) {
                    pos_origin.x = 0;
                    pos_origin.y = 0;
                    pos_destiny.x = total_w;
                    pos_destiny.y = number*MAP_H*TILESIZE + stage_data.maps[number].backgrounds[i].adjust_y;
                    copy_surface_area(bg, pos_origin, pos_destiny, bg->w, bg->h);

                    total_w += bg->w;
                }
                SDL_FreeSurface(bg);
            }
        }
    }

    for (int i=0; i<MAP_W; i++) {
        for (int j=0; j<MAP_H; j++) {
            // level 1
            pos_origin.x = stage_data.maps[number].tiles[i][j].tile1.x*TILESIZE;
            pos_origin.y = stage_data.maps[number].tiles[i][j].tile1.y*TILESIZE;
            pos_destiny.x = i*TILESIZE;
            pos_destiny.y = (j*TILESIZE) + (number*MAP_H*TILESIZE);
            if (pos_origin.x >= 0) {
                copy_surface_area(myset, pos_origin, pos_destiny, TILESIZE, TILESIZE);
            }

            // level 2
            pos_origin.x = stage_data.maps[number].tiles[i][j].tile2.x*TILESIZE;
            pos_origin.y = stage_data.maps[number].tiles[i][j].tile2.y*TILESIZE;
            if (pos_origin.x >= 0) {
                copy_surface_area(myset, pos_origin, pos_destiny, TILESIZE, TILESIZE);
            }

            // level 3
            pos_origin.x = stage_data.maps[number].tiles[i][j].tile3.x*TILESIZE;
            pos_origin.y = stage_data.maps[number].tiles[i][j].tile3.y*TILESIZE;
            if (pos_origin.x >= 0) {
                copy_surface_area(myset, pos_origin, pos_destiny, TILESIZE, TILESIZE);
            }

        }
    }

}

std::string string_number(int number) {
    std::ostringstream convert;

    if (number >= 100) {
        convert << number;
    } else if (number >= 10) {
        convert << "0" << number;
    } else {
        convert << "00" << number;
    }
    return convert.str();
}


void draw_stage(int stage_n) {
    fio.read_stage(stage_data, stage_n);

    if (stage_data.id == -1) {
        return;
    }

    draw_map(0, false);
    draw_map(1, false);
    draw_map(2, false);
    std::string filename = FILEPATH + "/screenshots/shot_" + string_number(stage_n) + ".png";

    SDL_Surface* shot = SDL_PNGFormatAlpha(screen);
    SDL_SavePNG(shot, filename.c_str());
    SDL_FreeSurface(shot);

    draw_map(0, true);
    draw_map(1, true);
    draw_map(2, true);
    filename = FILEPATH + "/screenshots/shot_beaten_" + string_number(stage_n) + ".png";

    shot = SDL_PNGFormatAlpha(screen);
    SDL_SavePNG(shot, filename.c_str());
    SDL_FreeSurface(shot);
}

int main(int argc, char *argv[])
{
    std::string argvString = "";
    argvString = std::string(argv[0]);
    get_filepath();
    // fallback in case getcwd returns null
    if (FILEPATH.size() == 0) {
        std::cout << "Could not read path, fallback to using argv" << std::endl;
        FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    }
    std::cout << "main - argvString: '" << argvString << "', FILEPATH: '" << FILEPATH << "'" << std::endl; std::fflush(stdout);


    if (SDL_Init(SDL_INIT_VIDEO) < 0  || (screen = SDL_SetVideoMode(MAP_W*TILESIZE, MAP_H*TILESIZE*3, 0, 0)) == 0) {
        std::cout << "SDL Error" << std::endl;
        std::cout << "Unable to init SDL. Error: " << SDL_GetError() << std::endl;
        std::fflush(stdout);
        exit(-1);
    }
    std::cout << ">>> GRAPHLIB::init DEBUG #1 <<<" << std::endl; std::fflush(stdout);
    atexit(SDL_Quit);

    // load tileset

	fio.read_game(game_data);
    load_tileset();
    for (int i=0; i<MAX_STAGES; i++) {
        draw_stage(i);
    }


    SDL_FreeSurface(screen);
    SDL_FreeSurface(tileset);
    SDL_FreeSurface(tileset_beaten);
    SDL_Quit();
	return 1;
}

