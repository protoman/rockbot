#ifndef SCENESHOW_H
#define SCENESHOW_H

#include "defines.h"
#include "file/fio_scenes.h"
#include "file/v3/3_0_1/file_scene.h"

class sceneShow
{
public:
    sceneShow();

    void show_scene(int n);
    void show_image(int n);
    void show_text(int n);

private:
    void run_image_scene(CURRENT_FILE_FORMAT::file_scene_show_image scene_image);
    void run_text(CURRENT_FILE_FORMAT::file_scene_show_text text);

private:
    CURRENT_FILE_FORMAT::fio_scenes fio_scn;
    float speed_x;
    float speed_y;
    float inc_x;
    float inc_y;
    int total_dist;

};

#endif // SCENESHOW_H
