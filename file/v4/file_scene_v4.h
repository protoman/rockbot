#ifndef FILE_SCENE_H
#define FILE_SCENE_H


#include "defines.h"
#include "file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6
#define SCENE_OBJECTS_N 20

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
        char text_lines[SCENE_TEXT_LINES_N][FS_INI_KEY_SIZE]; // points to a key in a XML file that holds the text
        int x;
        int y;
        int transition_type; ///@TODO: convert to enum
        int position_type;

        file_scene_show_text() {
            name[0] = '\0';
            for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
                text_lines[i][0] = '\0';
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
        int type;
        int seek_n;
        int delay_after;
        int repeat_type;
        int repeat_value;

        file_scene_object() {
            type = SCENETYPE_CLEAR_SCREEN;
            seek_n = -1;
            delay_after = 0;
            repeat_type = 1;
            repeat_value = 1;
        }

    };

    // header of one scene_list file
    struct file_scene_list {
        char name[FS_NAME_SIZE];
        file_scene_object objects[SCENE_OBJECTS_N];
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
        scenetype_change_colorcycle,
        scenetype_move_image,                   // shows an image that moves from point "place" to point destiny
        scenetype_show_animation                // shows an animation sequence
    };

    // a scene is a sequence of several little actions
    // the actions are stored in a different file and its type and IDare stored
    struct file_scene_sequence {
        int action_id;

        // tipo[texto, imagem, som, animação, limpa-tela]/id
    };

    // multiple options that are unique to text are stored in this scruct
    struct st_scene_text_options {
        bool center_align;                                  // horizontal align
        bool progressive_text;                              // each character appears by itself until full text is shown
        bool fade_in_out;

        st_scene_text_options() {
            center_align = false;
            progressive_text = false;
            fade_in_out = true;
        }
    };

    // holds options for all types of scene, except animations
    struct file_scene {
        bool deleted;                                             // if logically deleted
        e_scenes_types type;                                // type of the scene
        int repeat_times;                                   // how many times this action will repeat until finished
        unsigned int repeat_timer;                          // how many milisseconds this action will play until finished
        st_rectangle rect;                                  // this is used for clear-area
        st_position place;                                  // used by show functions: image, text, repeat_scrollbg, move-image start point
        st_position destiny;                                // for moving image
        enum SFX_LIST sfx;                                  // what is the SFX the will be played
        char bgmusic_filename[FS_CHAR_FILENAME_SIZE];       // what is the music that will be played
        char bgimage_filename[FS_CHAR_FILENAME_SIZE];       // what is the filename of the background that will repeat
        Uint8 move_speed;                                   // how fast the repeat-background will move
        char text[SCENE_TEXT_LINES_N][DIALOG_LINE_LIMIT];   // string that will be show as text
        st_scene_text_options text_options;                 // options for text
        int delay;                                          // delay before next scene is executed

        file_scene() {
            deleted = false;
        }
    };

}

#endif // FILE_SCENE_H

