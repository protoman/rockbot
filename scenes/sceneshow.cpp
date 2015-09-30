#include "sceneshow.h"

extern std::string FILEPATH;

#include "soundlib.h"
extern soundLib soundManager;

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
    scene_list = fio_scn.load_scenes();
    image_scenes = fio_scn.load_scenes_show_image();
    text_list = fio_scn.load_scenes_show_text();
    cleararea_list = fio_scn.load_scenes_clear_area();
    playsfx_list = fio_scn.load_scenes_play_sfx();
    playmusic_list = fio_scn.load_scenes_play_music();
}

void sceneShow::show_scene(int n)
{
    if (scene_list.size() <= n) {
        std::cout << "ERROR: Scene List[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        exit(-1);
    }
    CURRENT_FILE_FORMAT::file_scene_list scene = scene_list.at(0);
    for (int i=0; i<SCENE_OBJECTS_N; i++) {
        int scene_seek_n = scene.objects[i].seek_n;
        if (scene_seek_n != -1) {
            int scene_type = scene.objects[i].type;
            if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
                show_text(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
                show_image(scene_seek_n);
            }
        }
    }
}

void sceneShow::show_image(int n)
{
    if (image_scenes.size() <= n) {
        std::cout << "ERROR: Scene image[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
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
    graphLib.surfaceFromFile(FILEPATH + "images/scenes/" + scene_image.filename, &image);
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
}



void sceneShow::show_text(int n)
{
    if (text_list.size() <= n) {
        std::cout << "ERROR: Scene Text[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        exit(-1);
    }
    run_text(text_list.at(n));
}

void sceneShow::clear_area(int n)
{
    if (cleararea_list.size() <= n) {
        std::cout << "ERROR: Scene ClearArea[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        exit(-1);
    }
    graphLib.clear_area(cleararea_list.at(n).x, cleararea_list.at(n).y, cleararea_list.at(n).w, cleararea_list.at(n).h, cleararea_list.at(n).r, cleararea_list.at(n).g, cleararea_list.at(n).b);
    graphLib.updateScreen();
}

void sceneShow::clear_screen()
{
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.updateScreen();
}

void sceneShow::play_sfx(int n)
{
    if (playsfx_list.size() <= n) {
        std::cout << "ERROR: Scene PlaySFX[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        exit(-1);
    }
    soundManager.play_sfx_from_file(playsfx_list.at(n).filename, playsfx_list.at(n).repeat_times);
}

void sceneShow::play_music(int n)
{
    if (playmusic_list.size() <= n) {
        std::cout << "ERROR: Scene PlayMusic[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        exit(-1);
    }
    soundManager.stop_music();
    soundManager.load_music(playmusic_list.at(n).filename);
    soundManager.play_music();
}

void sceneShow::run_text(CURRENT_FILE_FORMAT::file_scene_show_text text)
{

    int lines_n = 0;
    int max_line_w = 0;
    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
        std::string line = std::string(text.text_lines[i]);
        if (line.size() > 0) {
            if (line.size() > max_line_w) {
                max_line_w = line.size();
            }
            lines_n++;
        }
    }

    int center_x = (RES_W * 0.5) - (max_line_w/2 * FONT_SIZE);
    int center_y = (RES_H * 0.5) - (lines_n * (LINE_H_DIFF * 0.5));
    int pos_x = 0;
    int pos_y = 0;

    if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_dialogbottom) {
        pos_x = 10;
        pos_y = 140;
    } else if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_dialogtop) {
        pos_x = 10;
        pos_y = 10;
    } else if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_centered) {
        pos_x = center_x;
        pos_y = center_y;

    } else if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_center_x) {
        pos_x = center_x;
        pos_y = text.y;

    } else if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_center_y) {
        pos_x = text.x;
        pos_y = center_y;

    } else if (text.position_type == CURRENT_FILE_FORMAT::text_position_type_user_defined) {
        pos_x = text.x;
        pos_y = text.y;
    }

    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
        std::string line = std::string(text.text_lines[i]);
        if (line.length() < 1) {
            break;
        }
        int adjusted_y = pos_y+(LINE_H_DIFF*i);
        graphLib.clear_area(pos_x, adjusted_y, strlen(text.text_lines[i])*9, 8, 0, 0, 0);
        graphLib.draw_progressive_text(pos_x, adjusted_y, line, false);
    }
}

