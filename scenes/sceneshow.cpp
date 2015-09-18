#include "sceneshow.h"

extern std::string FILEPATH;
//extern soundLib soundManager;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "graphic/draw.h"
extern draw draw_lib;

#include "timerlib.h"
extern timerLib timer;

sceneShow::sceneShow()
{

}

void sceneShow::show_scene(int n)
{

}

void sceneShow::show_image(int n)
{
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_image> image_scenes = fio_scn.load_scenes_show_image();
    if (image_scenes.size() <= n) {
        std::cout << "ERROR: Scene image[" << n << "] invalid." << std::endl;
        exit(-1);
    }
    speed_x = 1;
    speed_y = 1;
    inc_x = 0;
    inc_y = 0;
    CURRENT_FILE_FORMAT::file_scene_show_image current_scene_image = image_scenes.at(n);
    int diff_x = abs(current_scene_image.ini_x - current_scene_image.dest_x);
    int diff_y = abs(current_scene_image.ini_y - current_scene_image.dest_y);
    if (diff_x > diff_y) {
        speed_y = (float)((float)diff_y / diff_x);
        std::cout << "speed_y: " << speed_y << ", res: " << ((float)diff_y / diff_x) << std::endl;
    } else if (diff_x < diff_y) {
        speed_x = (float)((float)diff_y / diff_x);
    }
    if (current_scene_image.ini_x > current_scene_image.dest_x) {
        speed_x = -speed_x;
    }
    if (current_scene_image.ini_y > current_scene_image.dest_y) {
        speed_y = -speed_y;
    }
    total_dist = diff_x;
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.updateScreen();
    run_image_scene(current_scene_image);

}



// @TODO - this should only set some variables in a global and the drawinbg should be handled my show_scene()
// That way we can move the run_XXX methods into threads to run in paralel
void sceneShow::run_image_scene(CURRENT_FILE_FORMAT::file_scene_show_image scene_image)
{
    float x = scene_image.ini_x;
    float y = scene_image.ini_y;
    graphicsLib_gSurface image;
    graphicsLib_gSurface bg_image;
    graphLib.initSurface(st_size(RES_W, RES_H), &bg_image);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &bg_image);
    graphLib.surfaceFromFile(FILEPATH + "/data/images/scenes/" + scene_image.filename, &image);
    while (total_dist > 0) {
        //std::cout << "total_dist: " << total_dist << std::endl;
        timer.delay(scene_image.move_delay);
        // @TODO - copy background, but should be done in a smarter way as there can be several moving elements
        graphLib.showSurfaceAt(&bg_image, st_position(0, 0), false);
        graphLib.showSurfaceAt(&image, st_position(x, y), false);
        graphLib.updateScreen();
        x += speed_x;
        y += speed_y;
        total_dist--;
    }
    input.wait_keypress();
}



void sceneShow::show_text(int n)
{
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_text> text_list = fio_scn.load_scenes_show_text();

    CURRENT_FILE_FORMAT::file_scene_show_text test;
    test.x = 30;
    test.y = RES_H - 90;
    sprintf(test.text_lines[0], "%s", "LINE #1");
    sprintf(test.text_lines[1], "%s", "LINE #2");
    sprintf(test.text_lines[2], "%s", "LINE #3");
    run_text(test);
}

void sceneShow::run_text(CURRENT_FILE_FORMAT::file_scene_show_text text)
{
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.updateScreen();
    timer.delay(500);
    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
        std::string line = std::string(text.text_lines[i]);
        if (line.length() < 1) {
            break;
        }
        graphLib.draw_progressive_text(text.x, text.y+(10*i), line, false);
    }
    input.wait_keypress();
}

