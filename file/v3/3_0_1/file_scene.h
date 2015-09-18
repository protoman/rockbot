#ifndef FILE_SCENE_H
#define FILE_SCENE_H


#include "defines.h"
#include "file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6

namespace format_v_3_0_1 {

    struct st_scene_area {
        int x;
        int y;
        int w;
        int h;
        st_scene_area() {
            x = 0;
            y = 0;
            w = 0;
            h = 0;
        }
        st_scene_area(int set_x, int set_y, int set_w, int set_h) {
            x = set_x;
            y = set_y;
            w = set_w;
            h = set_h;
        }
    };

    enum e_SCENETYPE {
        SCENETYPE_CLEAR_SCREEN,
        SCENETYPE_CLEAR_AREA,
        SCENETYPE_SHOW_IMAGE,
        SCENETYPE_PLAY_SFX,
        SCENETYPE_PLAY_MUSIC,
        SCENETYPE_STOP_MUSIC,
        SCENETYPE_REPEAT_SCROLLBG,              // keeps scrolling a background (like ninja gaiden's opening's grass)
        SCENETYPE_SHOW_TEXT,
        SCENETYPE_CHANGE_COLORCYCLE,
        SCENETYPE_MOVE_IMAGE,                   // shows an image that moves from point "place" to point destiny
        SCENETYPE_SHOW_ANIMATION                // shows an animation sequence
    };

    /// @TODO - convert some int to uint8/short

    struct file_scene_clear_area {
        char name[FS_NAME_SIZE];
        int x;
        int y;
        int w;
        int h;
        int r;
        int g;
        int b;

        file_scene_clear_area() {
            x = 0;
            y = 0;
            w = 0;
            h = 0;
            r = 0;
            g = 0;
            b = 0;
        }
    };

    struct file_scene_show_image {
        char name[FS_NAME_SIZE];
        char filename[FS_CHAR_FILENAME_SIZE];
        int ini_x;
        int ini_y;
        int dest_x;
        int dest_y;
        st_scene_area copy_area;
        int move_delay;
        bool blocking;                              // to be used, if false, to run multiple scene-types in parallel. defaults to true.
        int loop_mode;
        file_scene_show_image() {
            name[0] = '\0';
            filename[0] = '\0';
            ini_x = 0;
            ini_y = 0;
            dest_x = 0;
            dest_y = 0;
            move_delay = 100;
            blocking = true;
            loop_mode = 0;
        }
    };

    struct file_scene_show_animation {
        char name[FS_NAME_SIZE];
        char filename[FS_CHAR_FILENAME_SIZE];
        int x;
        int y;
        int frame_w;
        int frame_h;
        int frame_delay;
        bool repeat;                                // if true, will keep running until the scene is over
        bool blocking;                              // to be used, if false, to run multiple scene-types in parallel. defaults to true.

        file_scene_show_animation() {
            name[0] = '\0';
            filename[0] = '\0';
            frame_w = 0;
            frame_h = 0;
            frame_delay = 0;
            repeat = false;
        }

    };

    struct file_scene_show_text {
        char name[FS_NAME_SIZE];
        char text_lines[SCENE_TEXT_LINES_N][FS_INI_KEY_SIZE]; // points to a key in a XML file that holds the text
        int x;
        int y;
        int transition_type; ///@TODO: convert to enum

        file_scene_show_text() {
            name[0] = '\0';
            for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
                text_lines[i][0] = '\0';
            }
            x = 0;
            y = 0;
            transition_type = 0;
        }
    };

    struct file_scene_play_sfx {
        char name[FS_NAME_SIZE];
        char filename[FS_CHAR_FILENAME_SIZE];
        int repeat_times;

        file_scene_play_sfx() {
            name[0] = '\0';
            filename[0] = '\0';
            repeat_times = 0;
        }
    };

    struct file_scene_play_music {
        char name[FS_NAME_SIZE];
        char filename[FS_CHAR_FILENAME_SIZE];

        file_scene_play_music() {
            name[0] = '\0';
            filename[0] = '\0';
        }
    };

    // file_scene holds the type and a "pointer" to the entry number in file-seeker for that entry in the respective type file
    struct file_scene_list {
        e_SCENETYPE type;
        int seek_n;
        char name[FS_NAME_SIZE];
        int delay_after;

        file_scene_list() {
            type = SCENETYPE_CLEAR_SCREEN;
            seek_n = -1;
            name[0] = '\0';
            delay_after = 0;
        }
    };


}

#endif // FILE_SCENE_H

