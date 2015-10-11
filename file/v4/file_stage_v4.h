#ifndef FILE_STAGE_v301H
#define FILE_STAGE_v301H

#include "file/format/st_common.h"
#include "defines.h"

namespace format_v4 {

struct st_map_background {
    char filename[FS_CHAR_FILENAME_SIZE];
    Sint16 adjust_y;                             // distance of the background from the screen top
    Sint8 speed;                                // 0 means will scroll in the same speed as foreground (so the bg is used only to ease map-creation)
    bool auto_scroll;                           // automatic move background even if player isn't moving
    st_map_background()
    {
        filename[0] = '\0';
        adjust_y = 0;
        speed = 0;
        auto_scroll = false;
        sprintf(filename, "%s", "");
    }
};

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





struct file_dialog { // DONE - Stage x2
    char face_graphics_filename[20];
    bool top_side;
    char line1[FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    char answer1[FS_MAX_PLAYERS][FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    char line2[FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    char answer2[FS_MAX_PLAYERS][FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    file_dialog() {
        top_side = true;
        face_graphics_filename[0] = '\0';
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            for (int j=0; j<3; j++) {
                answer1[i][j][0] = '\0';
                answer2[i][j][0] = '\0';
            }
        }
        for (int j=0; j<3; j++) {
            line1[j][0] = '\0';
            line2[j][0] = '\0';
        }
    }
};

// defines a castle intro and details
struct file_castle {
    st_position points[CASTLE_STAGES_MAX];
    char filename[FS_FILENAME_SIZE];                                        // castle background image
    file_dialog boss_defeated_dialog;                                       // boss defeated dialog
};




struct file_link { // DONE - Stage
    Sint8 id_map_origin;
    Sint8 id_map_destiny;
    st_position_uint8 pos_origin;
    st_position_uint8 pos_destiny;
    Uint8 size;
    bool bidirecional;
    bool is_door;
    Sint8 type;
    file_link() {
        id_map_origin = -1;
        id_map_destiny = -1;
        size = 1;
        bidirecional = false;
        is_door = false;
        type = LINK_VERTICAL;
    }
};

struct file_boss { // DONE - Stage
    Sint8 id_npc;
    Sint8 id_weapon;
    char name[20];
    char face_graphics_filename[20];
    file_boss() {
        id_npc = -1;
        id_weapon = -1;
        sprintf(name, "%s", "Boss");
        face_graphics_filename[0] = '\0';
    }
};


struct file_colorcycle { // DONE - Stage
    st_color color[FS_COLORCYCLE_MAX_ITEMS][COLOR_KEY_LENGTH];
    Sint16 duration[FS_COLORCYCLE_MAX_ITEMS][COLOR_KEY_LENGTH];
};

struct file_stage {
    char name[FS_CHAR_NAME_SIZE];
    char bgmusic_filename[FS_CHAR_FILENAME_SIZE];
    file_map maps[FS_STAGE_MAX_MAPS];
    file_link links[FS_STAGE_MAX_LINKS];
    file_boss boss;
    file_dialog intro_dialog;
    file_dialog boss_dialog;
    file_colorcycle colorcycle;
    char tileset_filename[FS_CHAR_FILENAME_SIZE];

    // CONSTRUCTOR //
    file_stage() {
       boss = file_boss();
        sprintf(name, "%s", "Stage");
        bgmusic_filename[0] = '\0';
        tileset_filename[0] = '\0';
    }
};





/**
 * @brief
 *
 */
struct file_stages {
    file_stage stages[FS_MAX_STAGES];
    file_stages() {
        for (int i=0; i<FS_MAX_STAGES; i++) {
            sprintf(stages[i].name, "Stage [%d]", i);
        }
    }
};

}

#endif // FILE_STAGE_H


