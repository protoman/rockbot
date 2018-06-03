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

#include "aux_tools/exception_manager.h"

extern CURRENT_FILE_FORMAT::st_game_config game_config;

sceneShow::sceneShow()
{
    _interrupt_scene = false;
    scene_list = fio_scn.load_scenes();
    image_scenes = fio_scn.load_scenes_show_image();
    text_list = fio_scn.load_scenes_show_text();
    cleararea_list = fio_scn.load_scenes_clear_area();
    playsfx_list = fio_scn.load_scenes_play_sfx();
    playmusic_list = fio_scn.load_scenes_play_music();
    viewpoint_list = fio_scn.load_scenes_show_viewpoint();
    animation_list = fio_scn.load_scenes_show_animation();
}

void sceneShow::show_scene(int n)
{
    if (n < 0) {
        return;
    }
    if (scene_list.size() <= n) {
        std::cout << "ERROR: Scene List[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        return;
    }
    CURRENT_FILE_FORMAT::file_scene_list scene = scene_list.at(n);
    input.clean();

    for (int i=0; i<SCENE_OBJECTS_N; i++) {
        input.read_input();
        int scene_seek_n = scene.objects[i].seek_n;
        //std::cout << ">> sceneShow::show_scene - i: " << i << ", scene_seek_n: " << scene_seek_n << std::endl;

        if (_interrupt_scene == true || input.p1_input[BTN_START] == 1) {
            break;
        }

        if (scene_seek_n != -1) {
            int scene_type = scene.objects[i].type;
            //std::cout << "### scene_type[" << scene_type << "]" << std::endl;
            if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
                show_text(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
                clear_area(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_SCREEN) {
                graphLib.clear_area(0 ,0, RES_W, RES_H, 0, 0, 0);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
                show_image(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
                show_viewpoint(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
                play_music(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
                play_sfx(scene_seek_n);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
                show_animation(scene_seek_n, scene.objects[i].repeat_value, scene.objects[i].repeat_type);
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
                soundManager.stop_music();
            } else if (scene_type == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
                show_scene(scene_seek_n);
            } else {
                std::cout << ">> sceneShow::show_scene - unknown scene_type[" << scene_type << "]" << std::endl;
            }
            //std::cout << "show_scene::DELAY[" << i << "][" << scene.objects[i].delay_after << "]" << std::endl;
            if (input.wait_scape_time(scene.objects[i].delay_after) == 1) {
                _interrupt_scene = true;
            }
        } else {
            break;
        }
    }
    std::cout << "show_scene::DONE" << std::endl;
}

void sceneShow::show_image(int n)
{
    std::cout << "sceneShow::show_image::START" << std::endl;
    if (image_scenes.size() <= n) {
        std::cout << "ERROR: Scene image[" << n << "] invalid. List size is " << image_scenes.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.0");
        char number_str[20];
        sprintf(number_str, "%d", image_scenes.size());
        exception_manager::throw_general_exception(std::string("sceneShow::show_image - Invalid list position."), std::string(number_str));
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
        speed_x = (float)((float)diff_x / diff_y);
    }
    if (current_scene_image.ini_x > current_scene_image.dest_x) {
        speed_x = -speed_x;
    }
    if (current_scene_image.ini_y > current_scene_image.dest_y) {
        speed_y = -speed_y;
    }
    if (diff_x >= diff_y) {
        total_dist = diff_x;
    } else {
        total_dist = diff_y;
    }
    run_image_scene(current_scene_image);

}



// @TODO - this should only set some variables in a global and the drawinbg should be handled my show_scene()
// That way we can move the run_XXX methods into threads to run in paralel
void sceneShow::run_image_scene(CURRENT_FILE_FORMAT::file_scene_show_image scene_image)
{
    std::cout << "** sceneShow::run_image_scene::START" << std::endl;
    float x = scene_image.ini_x;
    float y = scene_image.ini_y;
    graphicsLib_gSurface image;
    graphicsLib_gSurface bg_image;
    graphLib.initSurface(st_size(RES_W, RES_H), &bg_image);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &bg_image);
    graphLib.surfaceFromFile(FILEPATH + "images/scenes/" + scene_image.filename, &image);

    std::cout << "** sceneShow::run_image_scene::total_dist: " << total_dist << std::endl;

    while (total_dist > 0) {
        input.read_input();
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
    graphLib.showSurfaceAt(&image, st_position(x, y), false);
    graphLib.updateScreen();
}


void sceneShow::run_viewpoint_scene(CURRENT_FILE_FORMAT::file_scene_show_viewpoint viewpoint)
{
    std::cout << "** sceneShow::run_image_scene::START" << std::endl;
    float x = viewpoint.ini_x;
    float y = viewpoint.ini_y;


    graphicsLib_gSurface image;
    graphLib.surfaceFromFile(FILEPATH + "images/scenes/" + viewpoint.filename, &image);

    //std::cout << "** sceneShow::run_image_scene::total_dist: " << total_dist << std::endl;

    while (total_dist > 0) {
        input.read_input();
        //std::cout << "total_dist: " << total_dist << std::endl;
        timer.delay(viewpoint.move_delay);
        std::cout << "rect - x[" << x << "], .y[" << y << "], w[" << viewpoint.w << "], h[" << viewpoint.h << "]" << std::endl;

        //void graphicsLib::showSurfacePortion(graphicsLib_gSurface *surfaceOrigin, const st_rectangle origin_rect, st_rectangle destiny_rect)
        graphLib.showSurfacePortion(&image, st_rectangle(x, y, viewpoint.w, viewpoint.h), st_rectangle(viewpoint.pos_x, viewpoint.pos_y, viewpoint.w, viewpoint.h));

        graphLib.updateScreen();
        x += speed_x;
        y += speed_y;
        total_dist--;
    }
    graphLib.showSurfacePortion(&image, st_rectangle(x, y, viewpoint.w, viewpoint.h), st_rectangle(viewpoint.pos_x, viewpoint.pos_y, viewpoint.w, image.height));
    timer.delay(viewpoint.move_delay);

    graphLib.updateScreen();
}



void sceneShow::show_text(int n)
{
    if (text_list.size() <= n) {
        std::cout << "ERROR: Scene Text[" << n << "] invalid. List size is " << text_list.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.1");
        char number_str[20];
        sprintf(number_str, "%d", text_list.size());
        exception_manager::throw_general_exception(std::string("sceneShow::show_text - Invalid list position."), std::string(number_str));
    }
    run_text(n);
}

void sceneShow::clear_area(int n)
{
    if (cleararea_list.size() <= n) {
        std::cout << "ERROR: Scene ClearArea[" << n << "] invalid. List size is " << cleararea_list.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.2");
        char number_str[20];
        sprintf(number_str, "%d", cleararea_list.size());
        exception_manager::throw_general_exception(std::string("sceneShow::clear_area - Invalid list position."), std::string(number_str));
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
        std::cout << "ERROR: Scene PlaySFX[" << n << "] invalid. List size is " << playsfx_list.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.3");
        char number_str[20];
        sprintf(number_str, "%d", playsfx_list.size());
        exception_manager::throw_general_exception(std::string("sceneShow::play_sfx - Invalid list position."), std::string(number_str));
    }
    if (playsfx_list.at(n).repeat_times < 1) {
        playsfx_list.at(n).repeat_times = 1;
    }
    soundManager.play_sfx_from_file(playsfx_list.at(n).filename, playsfx_list.at(n).repeat_times);
}

void sceneShow::play_music(int n)
{
    if (playmusic_list.size() <= n) {
        std::cout << "ERROR: Scene PlayMusic[" << n << "] invalid. List size is " << playmusic_list.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.4");
        char number_str[20];
        sprintf(number_str, "%d", playmusic_list.size());
        exception_manager::throw_general_exception(std::string("sceneShow::play_music - Invalid list position."), std::string(number_str));
    }
    soundManager.stop_music();
    soundManager.load_music(playmusic_list.at(n).filename);
    soundManager.play_music();
}



void sceneShow::run_text(int n)
{

    int lines_n = 0;
    int max_line_w = 0;

    // this part is used to calculate text/lines size for positioning
    /// @TODO: optimize using a vector


    std::vector<std::string> text_lines;
    std::vector<std::string> scene_text_list = fio_str.get_string_list_from_scene_text_file(n, game_config.selected_language);
    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {

        std::string line = scene_text_list[i];

        if (line.size() > 0) {
            text_lines.push_back(line);
            if (line.size() > max_line_w) {
                max_line_w = line.size();
            }
            // only count as line if not empty
            lines_n = i+1;
        } else {
            text_lines.push_back(std::string(" "));
        }
    }

    int center_x = (RES_W * 0.5) - (max_line_w/2 * FONT_SIZE);
    int center_y = (RES_H * 0.5) - (lines_n * (SCENES_LINE_H_DIFF * 0.5));
    int pos_x = 0;
    int pos_y = 0;

    if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_dialogbottom) {
        pos_x = 10;
        pos_y = SCENES_TEXT_BOTTOM_POSY;
    } else if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_dialogtop) {
        pos_x = 10;
        pos_y = 10;
    } else if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_centered) {
        pos_x = center_x;
        pos_y = center_y;

    } else if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_center_x) {
        pos_x = center_x;
        pos_y = text_list.at(n).y;

    } else if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_center_y) {
        pos_x = text_list.at(n).x;
        pos_y = center_y;

    } else if (text_list.at(n).position_type == CURRENT_FILE_FORMAT::text_position_type_user_defined) {
        pos_x = text_list.at(n).x;
        pos_y = text_list.at(n).y;
    }

    std::vector<std::string> lines;
    int max_w = 0;
    for (int i=0; i<text_lines.size(); i++) {
        std::string line = std::string(text_lines.at(i));
        if (line.length() < 1) {
            break;
        }
        lines.push_back(line);
        if (line.length() * FONT_SIZE > max_w) {
            max_w = line.length() * (FONT_SIZE+5)+10;
        }
    }
    // clear text area
    int max_h = lines.size()*SCENES_LINE_H_DIFF;
    //std::cout << "lines[" << lines.size() << "], max_w[" << max_w << "], max_h[" << max_h << "]" << std::endl;
    graphLib.clear_area(pos_x, pos_y, RES_W, max_h, 0, 0, 0);


    for (int i=0; i<lines.size(); i++) {
        if (input.p1_input[BTN_START] == 1) {
            _interrupt_scene = true;
            break;
        }
        std::string line = std::string(lines.at(i));
        int adjusted_y = pos_y+(SCENES_LINE_H_DIFF*i);
        graphLib.clear_area(pos_x, adjusted_y, line.length()*9, 8, 0, 0, 0);
        if (graphLib.draw_progressive_text(pos_x, adjusted_y, line, false, 30) == 1) {
            _interrupt_scene = true;
            break;
        }
    }
    std::cout << "run_text::DONE" << std::endl;
}



void sceneShow::show_viewpoint(int n)
{
    std::cout << "sceneShow::show_image::START" << std::endl;
    if (viewpoint_list.size() <= n) {
        std::cout << "ERROR: Scene image[" << n << "] invalid. List size is " << viewpoint_list.size() << "." << std::endl;
        graphLib.show_debug_msg("EXIT #42.5");
        char number_str[20];
        sprintf(number_str, "%d", viewpoint_list.size());
        exception_manager::throw_general_exception(std::string("sceneShow::play_music - Invalid list position."), std::string(number_str));
    }
    speed_x = 1;
    speed_y = 1;
    inc_x = 0;
    inc_y = 0;
    CURRENT_FILE_FORMAT::file_scene_show_viewpoint viewpoint = viewpoint_list.at(n);
    int diff_x = abs(viewpoint.ini_x - viewpoint.dest_x);
    int diff_y = abs(viewpoint.ini_y - viewpoint.dest_y);
    if (diff_x > diff_y) {
        speed_y = (float)((float)diff_y / diff_x);
        std::cout << "speed_y: " << speed_y << ", res: " << ((float)diff_y / diff_x) << std::endl;
    } else if (diff_x < diff_y) {
        speed_x = (float)((float)diff_x / diff_y);
    }
    if (viewpoint.ini_x > viewpoint.dest_x) {
        speed_x = -speed_x;
    }
    if (viewpoint.ini_y > viewpoint.dest_y) {
        speed_y = -speed_y;
    }
    if (diff_x >= diff_y) {
        total_dist = diff_x;
    } else {
        total_dist = diff_y;
    }
    std::cout << "SCENE::VIEWPOINT - speed_y[" << speed_y << "], speed_x[" << speed_x << "]" << std::endl;
    run_viewpoint_scene(viewpoint);
}

void sceneShow::show_animation(int n, int repeat_n, int repeat_mode)
{
    int frame_n = 0;
    CURRENT_FILE_FORMAT::file_scene_show_animation scene = animation_list.at(n);
    long frame_timer = timer.getTimer() + scene.frame_delay;
    long started_timer = timer.getTimer();
    int repeat_times = 0;

    graphicsLib_gSurface image;
    graphLib.surfaceFromFile(FILEPATH + "images/scenes/animations/" + scene.filename, &image);
    int max_frames = image.width / scene.frame_w;

    graphicsLib_gSurface bg_image;
    graphLib.initSurface(st_size(scene.frame_w, scene.frame_h), &bg_image);
    graphLib.copy_gamescreen_area(st_rectangle(scene.x, scene.y, scene.frame_w, scene.frame_h), st_position(0, 0), &bg_image);


    std::cout << "max_frames[" << max_frames << "], image.w[" << image.width << "], scene.frame_w[" << scene.frame_w << "]" << std::endl;

    while (true) {
        input.read_input();
        int x = frame_n*scene.frame_w;


        // stop condition
        if (repeat_times > 0 && repeat_n <= 1) {
            std::cout << "sceneShow::show_animation::LEAVE#1" << std::endl;
            break;
        } else {
            if (repeat_mode == 0) { // time-mode
                if ((timer.getTimer() - started_timer) > repeat_n) {
                    std::cout << "sceneShow::show_animation::LEAVE#2" << std::endl;
                    break;
                }
            } else { // repeat number mode
                if (repeat_times > repeat_n) {
                    std::cout << "sceneShow::show_animation::LEAVE#3" << std::endl;
                    break;
                }
            }
        }
        graphLib.showSurfaceAt(&bg_image, st_position(scene.x, scene.y), false);
        //std::cout << "x[" << x << "], img.w[" << image.width << "], frame.w[" << scene.frame_w << "]" << std::endl;
        graphLib.showSurfaceRegionAt(&image, st_rectangle(x, 0, scene.frame_w, scene.frame_h), st_position(scene.x, scene.y));
        graphLib.updateScreen();
        timer.delay(scene.frame_delay);


        if (frame_timer < timer.getTimer()) {
            frame_n++;
            if (frame_n >= max_frames) {
                frame_n = 0;
                repeat_times++;
            }
            frame_timer = timer.getTimer() + scene.frame_delay;
        }

    }
    // avoid leaving animation image trail if it is a repeating one
    if (repeat_n > 1) {
        graphLib.showSurfaceAt(&bg_image, st_position(scene.x, scene.y), false);
        graphLib.updateScreen();
        timer.delay(scene.frame_delay);
    }
}
