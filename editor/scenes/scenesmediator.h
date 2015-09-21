#ifndef SCENESMEDIATOR_H
#define SCENESMEDIATOR_H

#include <vector>
#include "file/v3/3_0_1/file_scene.h"

class ScenesMediator
{
public:
    ScenesMediator();
    static ScenesMediator* get_instance();

public:
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_image> image_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_viewpoint> viewpoint_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_text> text_list;

    // @TODO
    std::vector<CURRENT_FILE_FORMAT::file_scene_clear_area> cleararea_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_animation> animation_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_play_sfx> playsfx_list;
    std::vector<CURRENT_FILE_FORMAT::file_scene_play_music> playmusic_list;

    std::vector<CURRENT_FILE_FORMAT::file_scene_list> scenes_list;

    int selected_scene;

private:
    static ScenesMediator*_instance;
};

#endif // SCENESMEDIATOR_H
