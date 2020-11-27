#ifndef FILE_MAP_H
#define FILE_MAP_H

#include "../format/st_common.h"
#include "defines.h"

#define MAP_BACKGROUND_COUNT 2

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


// ======================= V2 ========================= //
struct file_map_npc_v2 {
    Sint8 id_npc;
    struct st_position start_point;
    Uint8 direction;
    Uint8 stage_id;
    Uint8 map_id;
    Uint8 difficulty_level;
    Uint8 difficulty_mode;
    file_map_npc_v2() {
        id_npc = -1;
        direction = 0;
        stage_id = INTRO_STAGE;
        map_id = 0;
        difficulty_level = DIFFICULTY_EASY;
        difficulty_mode = DIFFICULTY_MODE_GREATER;
    }
    file_map_npc_v2(file_map_npc origin, int stage, int map) {
        id_npc = origin.id_npc;
        start_point = origin.start_point;
        direction = origin.direction;
        stage_id = stage;
        map_id = map;
        difficulty_level = DIFFICULTY_EASY;
        difficulty_mode = DIFFICULTY_MODE_GREATER;
    }
};

struct file_map_object_v2 {
    Sint8 id_object;
    struct st_position start_point;
    Uint8 direction;
    st_position link_dest;                          // used for teleporter
    Sint8 map_dest;
    Uint8 stage_id;
    Uint8 map_id;
    Uint8 difficulty_level;
    Uint8 difficulty_mode;

    file_map_object_v2() {
        id_object = -1;
        direction = 0;
        map_dest = -1;
        link_dest.x = -1;
        link_dest.y = -1;
        stage_id = INTRO_STAGE;
        map_id = 0;
        difficulty_level = DIFFICULTY_EASY;
        difficulty_mode = DIFFICULTY_MODE_GREATER;
    }

    file_map_object_v2(file_map_object origin, int stage, int map) {
        id_object = origin.id_object;
        start_point = origin.start_point;
        direction = origin.direction;
        link_dest = origin.link_dest;
        map_dest = origin.map_dest;
        stage_id = stage;
        map_id = map;
        difficulty_level = DIFFICULTY_EASY;
        difficulty_mode = DIFFICULTY_MODE_GREATER;
    }
};

struct file_map_v2 {
    st_map_background backgrounds[MAP_BACKGROUND_COUNT];
    st_color background_color;
    map_tile tiles[MAP_W][MAP_H];		// map tiles

    file_map_v2() {
        // add ground to new map
        for (int i=0; i<MAP_W; i++) {
            tiles[i][MAP_H-2].locked = TERRAIN_SOLID;
            tiles[i][MAP_H-1].locked = TERRAIN_SOLID;
        }
    }

    file_map_v2(file_map original) {
        for (int i=0; i<MAP_BACKGROUND_COUNT; i++) {
            backgrounds[i] = original.backgrounds[i];
        }
        background_color = original.background_color;
        for (int i=0; i<MAP_W; i++) {
            for (int j=0; j<MAP_H; j++) {
                tiles[i][j] = original.tiles[i][j];
            }
        }
    }
};

}

#endif // FILE_MAP_H
