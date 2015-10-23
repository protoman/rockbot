#ifndef FILE_MAP_H
#define FILE_MAP_H

#include "file/format/st_common.h"
#include "defines.h"

namespace format_v4 {


//file_map maps[FS_MAX_STAGES][FS_STAGE_MAX_MAPS];

struct map_tile {
    Sint8 locked;
    struct st_position_int8 tile1;                  // position of tile (x and y) in the graphic file (each tile is 16*16)
    struct st_position_int8 tile3;                  // tird level, to be shown over player
    map_tile() {
        locked = 0;
        tile1.x = -1;
        tile1.y = -1;
        tile3.x = -1;
        tile3.y = -1;
    }
};


struct st_map_background {
    char filename[FS_CHAR_FILENAME_SIZE];
    Sint16 adjust_y;                             // distance of the background from the screen top
    Sint8 speed;                                // 0 means will scroll in the same speed as foreground (so the bg is used only to ease map-creation)
    Uint8 auto_scroll;                          // automatic move background even if player isn't moving. 0: don't move, 1: horizontal, 2: vertical
    Uint8 gfx;                                  // indicates that the map have an gfx executed by drawlib (rain, snow, train, etc)
    st_map_background()
    {
        filename[0] = '\0';
        adjust_y = 0;
        speed = 0;
        auto_scroll = 0;
        sprintf(filename, "%s", "");
        gfx = 0;
    }
};


struct file_map_npc {
    Sint8 id_npc;
    struct st_position start_point;
    Uint8 direction;
    file_map_npc() {
        id_npc = -1;
        direction = 0;
    }
};

struct file_map_object {
    Sint8 id_object;
    struct st_position start_point;
    Uint8 direction;
    st_position link_dest;                          // used for teleporter
    Sint8 map_dest;
    file_map_object() {
        id_object = -1;
        direction = 0;
        map_dest = -1;
        link_dest.x = -1;
        link_dest.y = -1;
    }
};

struct file_map {
    st_map_background backgrounds[2];
    st_color background_color;
    map_tile tiles[MAP_W][MAP_H];		// map tiles
    file_map_npc map_npcs[FS_MAX_MAP_NPCS];
    file_map_object map_objects[FS_MAX_MAP_OBJECTS];
    file_map() {
        for (int i=0; i<FS_MAX_MAP_NPCS; i++) {
            map_npcs[i].id_npc = -1;
        }
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            map_objects[i].id_object = -1;
        }
        // add ground to new map
        for (int i=0; i<MAP_W; i++) {
            tiles[i][MAP_H-2].locked = TERRAIN_SOLID;
            tiles[i][MAP_H-1].locked = TERRAIN_SOLID;
        }
    }
};

}

#endif // FILE_MAP_H
