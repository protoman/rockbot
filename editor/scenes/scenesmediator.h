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

private:
    static ScenesMediator*_instance;
};

#endif // SCENESMEDIATOR_H
