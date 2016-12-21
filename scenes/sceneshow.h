#ifndef SCENESHOW_H
#define SCENESHOW_H

#include "defines.h"
#include "file/fio_scenes.h"
#include "file/fio_strings.h"

class sceneShow
{
public:
    sceneShow();

    void show_scene(int n);
    void show_image(int n);
    void show_text(int n);
    void clear_area(int n);
    void clear_screen();
    void play_sfx(int n);
    void play_music(int n);
    void show_viewpoint(int n);
    void show_animation(int n, int repeat_n, int repeat_mode);



private:
    void run_image_scene(CURRENT_FILE_FORMAT::file_scene_show_image scene_image);
    void run_text(CURRENT_FILE_FORMAT::file_scene_show_text text);
    void run_viewpoint_scene(CURRENT_FILE_FORMAT::file_scene_show_viewpoint viewpoint);

private:
    CURRENT_FILE_FORMAT::fio_scenes fio_scn;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    float speed_x;
    float speed_y;
    float inc_x;
    float inc_y;
    int total_dist;

    // lists
    std::vector<CURRENT_FILE_FORMAT::file_scene_list> scene_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_image> image_scenes;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_text> text_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_clear_area> cleararea_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_play_sfx> playsfx_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_play_music> playmusic_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_viewpoint> viewpoint_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_animation> animation_list;

    bool _interrupt_scene;

};

#endif // SCENESHOW_H
