#ifndef FILE_SCENE_H
#define FILE_SCENE_H


#include "defines.h"
#include "file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6
#define SCENE_OBJECTS_MAX 20

namespace format_v4 {

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
        SCENETYPE_MOVE_IMAGE,                   // shows an image that moves from point "place" to point destiny
        SCENETYPE_MOVE_VIEWPOINT,
        SCENETYPE_SHOW_ANIMATION,                // shows an animation sequence
        SCENETYPE_PLAY_SFX,
        SCENETYPE_PLAY_MUSIC,
        SCENETYPE_STOP_MUSIC,
        SCENETYPE_SHOW_TEXT,
        SCENETYPE_SUBSCENE                      // you can add another scene to the scene, to it is possible to repeat
        //SCENETYPE_REPEAT_SCROLLBG,              // keeps scrolling a background (like ninja gaiden's opening's grass)
    };


    enum e_text_position_type {
        text_position_type_dialogbottom,
        text_position_type_dialogtop,
        text_position_type_centered,
        text_position_type_center_x,
        text_position_type_center_y,
        text_position_type_user_defined
    };

    /// @TODO - convert some int to uint8/short

    struct file_scene_clear_area {
        char name[FS_NAME_SIZE];
        Uint16 x;
        Uint16 y;
        Uint16 w;
        Uint16 h;
        Uint16 r;
        Uint16 g;
        Uint16 b;

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
        int move_type;                              // in the future we can use to move as arc, sin, linear, etc
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
            move_type = 0;
        }
    };


    struct file_scene_show_viewpoint {
        char name[FS_NAME_SIZE];
        char filename[FS_CHAR_FILENAME_SIZE];
        int ini_x;
        int ini_y;
        int dest_x;
        int dest_y;
        int w;
        int h;
        int move_delay;
        bool blocking;                              // to be used, if false, to run multiple scene-types in parallel. defaults to true.
        int loop_mode;
        file_scene_show_viewpoint() {
            name[0] = '\0';
            filename[0] = '\0';
            ini_x = 0;
            ini_y = 0;
            w = 0;
            h = 0;
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
        int x;                                      // TBD
        int y;                                      // TBD
        int frame_w;
        int frame_h;
        int frame_delay;
        bool repeat;                                // if true, will keep running until the scene is over
        bool blocking;                              // to be used, if false, to run multiple scene-types in parallel. defaults to true.

        file_scene_show_animation() {
            name[0] = '\0';
            filename[0] = '\0';
            x = 0;
            y = 0;
            frame_w = 0;
            frame_h = 0;
            frame_delay = 0;
            repeat = false;
        }

    };

    struct file_scene_show_text {
        char name[FS_NAME_SIZE];
        int line_string_id[SCENE_TEXT_LINES_N]; // points to a key in a dat file that holds the text
        int x;
        int y;
        int transition_type; ///@TODO: convert to enum
        int position_type;

        file_scene_show_text() {
            name[0] = '\0';
            for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
                line_string_id[i] = -1;
            }
            x = 0;
            y = 0;
            transition_type = 0;
            position_type = 0;
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

    enum e_scene_repeat_types {
        scene_repeat_time,
        scene_repeat_number
    };



    // file_scene_object holds the type and a "pointer" to the entry number in file-seeker for that entry in the respective type file
    struct file_scene_object {
        Sint8 type;
        int seek_n;
        int delay_after;
        int repeat_type;
        int repeat_value;
        bool run_in_background;

        file_scene_object() {
            type = SCENETYPE_CLEAR_SCREEN;
            seek_n = -1;
            delay_after = 0;
            repeat_type = 1;
            repeat_value = 1;
            run_in_background = false;
        }

    };

    // header of one scene_list file
    struct file_scene_list {
        char name[FS_NAME_SIZE];
        file_scene_object objects[SCENE_OBJECTS_MAX];
        file_scene_list() {
            name[0] = '\0';
        }
    };



    enum e_scenes_types {
        scenetype_clear_screen,
        scenetype_clear_area,
        scenetype_show_image,
        scenetype_play_sfx,
        scenetype_play_music,
        scenetype_stop_music,
        scenetype_repeat_scrollbg,              // keeps scrolling a background (like ninja gaiden's opening's grass)
        scenetype_show_text,
        scenetype_move_image,                   // shows an image that moves from point "place" to point destiny
        scenetype_show_animation                // shows an animation sequence
    };



}

#endif // FILE_SCENE_H

