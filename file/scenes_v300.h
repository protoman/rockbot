#ifndef SCENES_V300_H
#define SCENES_V300_H

#include "../defines.h"
#include "../file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6

namespace format_v_3_0_0 {

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
        Sint8 colorcyle;                                    // colorcycle used
        int delay;                                          // delay before next scene is executed

        file_scene() {
            deleted = false;
        }
    };





}

#endif // SCENES_V300_H
