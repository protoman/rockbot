#ifndef FILE_STAGE_v301H
#define FILE_STAGE_v301H

#include "file/format/st_common.h"
#include "defines.h"

namespace format_v4 {


struct file_dialog { // DONE - Stage x2
    char face_graphics_filename[20];
    bool top_side;
    int text1_string_ids[FS_DIALOG_LINES];
    int text2_string_ids[FS_DIALOG_LINES];
    int answer1_string_ids[FS_MAX_PLAYERS][FS_DIALOG_LINES];
    int answer2_string_ids[FS_MAX_PLAYERS][FS_DIALOG_LINES];


    file_dialog() {
        top_side = true;
        face_graphics_filename[0] = '\0';
        for (int i=0; i<FS_DIALOG_LINES; i++) {
            text1_string_ids[i] = -1;
            text2_string_ids[i] = -1;
            for (int j=0; j<FS_MAX_PLAYERS; j++) {
                answer1_string_ids[j][i] = -1;
                answer2_string_ids[j][i] = -1;
            }
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
    file_boss() {
        id_npc = -1;
        id_weapon = -1;
        sprintf(name, "%s", "Boss");
    }
};


struct file_stage {
    char name[FS_CHAR_NAME_SIZE];
    char bgmusic_filename[FS_CHAR_FILENAME_SIZE];
    file_link links[FS_STAGE_MAX_LINKS];
    file_boss boss;
    file_dialog intro_dialog;
    file_dialog boss_dialog;
    char tileset_filename[FS_CHAR_FILENAME_SIZE];
    bool autoscroll[FS_STAGE_MAX_MAPS];
    Sint8 cutscene_pre;
    Sint8 cutscene_pos;

    // CONSTRUCTOR //
    file_stage() {
        for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
            autoscroll[i] = false;
        }
        boss = file_boss();
        sprintf(name, "%s", "Stage");
        bgmusic_filename[0] = '\0';
        tileset_filename[0] = '\0';
        cutscene_pre = -1;
        cutscene_pos = -1;
    }
};




// we have 20 stages:
// 1 intro, 8 bosses, 5 castle #1, 5 castle #2, 1 reserve
struct file_stages {
    file_stage stages[FS_MAX_STAGES];
    file_stages() {
        for (int i=0; i<FS_MAX_STAGES; i++) {
            if (i == 0) {
                sprintf(stages[i].name, "#%d INTRO STAGE", i);
            } else if (i < 9) {
                sprintf(stages[i].name, "#%d 8-Bosses-Stage", i);
            } else if (i <14) {
                sprintf(stages[i].name, "Castle #1 Stage #%d", i);
            } else {
                sprintf(stages[i].name, "Castle #2 Stage #%d", i);
            }

        }
    }
};

}

#endif // FILE_STAGE_H


