#include <stdio.h>
#include <stdlib.h>
#ifdef ANDROID
#include <android/log.h>
#endif

#include "../graphic/draw.h"
#include "../game_mediator.h"
#include "../timerlib.h"
#include "../game.h"
#include "../inputlib.h"
#include "../strings_map.h"
#include "../file/file_io.h"


#define RAIN_DELAY 160
#define FLASH_DELAY 260
#define FLASH_IMG_SIZE 8
#define SNOW_DELAY 40
#define LIGHTINGBOLT_DELAY1 4600
#define LIGHTINGBOLT_DELAY2 80
#define TRAIN_DELAY 2000
#define TRAIN_EFFECT_DELAY 80
#define TRAIN_EFFECT_SCREEN_MOVE 1
#define INFERNO_ALPHA_STEP 3
#define STARS_DELAY 50
#define CASTLE_PATH_DURATION 1000

#define ENERGY_BALL_IMG_SIZE 10
#define ENERGY_BALL_PERCENT_SLICE 5

extern graphicsLib graphLib;

extern timerLib timer;

extern CURRENT_FILE_FORMAT::file_game game_data;

extern std::string FILEPATH;


extern game gameControl;

extern inputLib input;


extern soundLib soundManager;

#define FADE_INC 2

extern CURRENT_FILE_FORMAT::file_io fio;
extern CURRENT_FILE_FORMAT::st_save game_save;

#define ARMOR_PIECES_SPRITE_W 28
#define ARMOR_PIECES_SPRITE_H 29

#define ARMOR_PIECES_SPRITE_POS_X (RES_W - 36)

#include "data/datautil.h"

draw::draw() : _rain_pos(0), _effect_timer(0), _flash_pos(0), _flash_timer(0), screen_gfx(SCREEN_GFX_NONE), flash_effect_enabled(false)
{
    for (int i=0; i<FLASH_POINTS_N; i++) {
        flash_points[i].x = rand() % RES_W;
        flash_points[i].y = rand() % RES_H;
    }
    _weapon_tooltip_timer = 0;
    _weapon_tooltip_n = 0;
    _weapon_tooltip_pos_ref = NULL;
    _weapon_tooltip_direction_ref = NULL;
    _train_effect_timer = 0;
    _train_effect_state = 0;
    _train_sfx = NULL;
    _lightingbolt_effect_timer = 0;
    _lightingbolt_effect_state = 0;
    // INFERNO EFFECT //
    _inferno_alpha = 0;
    _inferno_alpha_mode = 0;
    _boss_current_hp = -99;
    current_alpha = -1;
    teleport_small_frame_count = 1;
    teleport_small_frame = 0;
    teleport_small_frame_timer = 0;

}

void draw::preload()
{
    std::string filename = GAMEPATH + "shared/images/teleport_small.png";
    graphLib.surfaceFromFile(filename, &_teleport_small_gfx);
    _teleport_small_gfx.init_colorkeys();
    teleport_small_frame_count = _teleport_small_gfx.width/_teleport_small_gfx.height;
    if (teleport_small_frame_count <1) {
        teleport_small_frame_count = 1;
    }

    filename = GAMEPATH + "shared/images/snowflacke.png";
    graphLib.surfaceFromFile(filename, &snow_flacke);

    filename = GAMEPATH + "shared/images/rain.png";
    graphLib.surfaceFromFile(filename, &rain_obj);

    filename = GAMEPATH + "shared/images/black_line.png";
    graphLib.surfaceFromFile(filename, &shadow_line);

    filename = GAMEPATH + "shared/images/boss_intro_bg.png";
    graphLib.surfaceFromFile(filename, &boss_intro_bg);

    // DROPABLE OBJECT GRAPHICS
    for (int i=0; i<GameMediator::get_instance()->object_list.size(); i++) {
        for (int j=0; j<DROP_ITEM_COUNT; j++) {
            short obj_type_n = gameControl.get_drop_item_id(j);
            if (obj_type_n != -1) {
                get_object_graphic(obj_type_n);
            }
        }
    }

    filename = GAMEPATH + "shared/images/hp_ball.png";
    graphLib.surfaceFromFile(filename, &hud_player_hp_ball);

    filename = GAMEPATH + "shared/images/wpn_ball.png";
    graphLib.surfaceFromFile(filename, &hud_player_wpn_ball);
    hud_player_wpn_ball.init_colorkeys();

    filename = GAMEPATH + "shared/images/boss_hp_ball.png";
    graphLib.surfaceFromFile(filename, &hud_boss_hp_ball);

    filename = GAMEPATH + "shared/images/hp_ball_disabled.png";
    graphLib.surfaceFromFile(filename, &hp_ball_disabled);

    filename = GAMEPATH + "/shared/images/buttons/d_pad.png";

    for (int i=0; i<INPUT_IMAGES_COUNT; i++) {
        input_images_map[i] = graphicsLib_gSurface();
    }

    graphLib.surfaceFromFile(filename, &input_images_map[INPUT_IMAGES_DPAD_LEFTRIGHT]);

    filename = GAMEPATH + "/shared/images/buttons/btn_a.png";
    graphLib.surfaceFromFile(filename, &input_images_map[INPUT_IMAGES_A]);

    filename = GAMEPATH + "/shared/images/buttons/btn_b.png";
    graphLib.surfaceFromFile(filename, &input_images_map[INPUT_IMAGES_B]);

    filename = GAMEPATH + "/shared/images/buttons/btn_x.png";
    graphLib.surfaceFromFile(filename, &input_images_map[INPUT_IMAGES_X]);

    filename = GAMEPATH + "/shared/images/buttons/btn_y.png";
    graphLib.surfaceFromFile(filename, &input_images_map[INPUT_IMAGES_Y]);

    filename = GAMEPATH + "shared/images/death_animation.png";
    graphLib.surfaceFromFile(filename, &_death_animation);
    _death_animation_frames_n = _death_animation.width/_death_animation.height;

    filename = FILEPATH + "images/backgrounds/weapon_menu.png";
    graphLib.surfaceFromFile(filename, &ingame_menu_bg_img);

    filename = FILEPATH + "/images/backgrounds/player_armor_pieces.png";
    graphLib.surfaceFromFile(filename, &ingame_menu_player_armor_pieces);

}

void draw::show_gfx()
{
    if (screen_gfx == SCREEN_GFX_RAIN) {
        show_rain();
    } else if (screen_gfx == SCREEN_GFX_SNOW) {
        show_snow_effect();
    } else if (screen_gfx == SCREEN_GFX_TRAIN) {
        show_train_effect();
    } else if (screen_gfx == SCREEN_GFX_LIGHTINGBOLT) {
        show_lightingbolt_effect();
    } else if (screen_gfx == SCREEN_GFX_SHADOW_TOP) {
        show_shadow_top_effect();
    } else if (screen_gfx == SCREEN_GFX_INFERNO) {
        show_inferno_effect();
    }

    if (flash_effect_enabled == true || screen_gfx == SCREEN_GFX_FLASH) {
        show_flash();
    }
}

graphicsLib_gSurface *draw::get_input_surface(e_INPUT_IMAGES input)
{
    return &input_images_map[input];
}

void draw::update_screen()
{
    if (current_alpha != -1) {
        if (current_alpha < 254) {
            current_alpha += 2;
        }
        if (current_alpha_surface.width < RES_W) {
            graphLib.initSurface(st_size(RES_W, RES_H), &current_alpha_surface);
            graphLib.clear_surface_area(0, 0, RES_W, RES_H, current_alpha_color.r, current_alpha_color.g, current_alpha_color.b, current_alpha_surface);
        } else {
            graphLib.set_surface_alpha(current_alpha, current_alpha_surface);
            graphLib.showSurface(&current_alpha_surface);
        }
    }
    graphLib.updateScreen();
}

void draw::set_gfx(Uint8 gfx, short mode)
{
    screen_gfx = gfx;
    screen_gfx_mode = mode;
    // free train sfx if not using it
    if (_train_sfx != NULL && screen_gfx != SCREEN_GFX_TRAIN) {
        Mix_FreeChunk(_train_sfx);
        _train_sfx = NULL;
    }
    if (screen_gfx == SCREEN_GFX_INFERNO) {
        if (_inferno_surface.is_null()) {
            graphLib.initSurface(st_size(RES_W, RES_H), &_inferno_surface);
            graphLib.clear_surface_area(0, 0, RES_W, RES_H, 180, 0, 0, _inferno_surface);
        }
    } else {
        free_inferno_surface();
    }
}

Uint8 draw::get_gfx()
{
    return screen_gfx;
}

void draw::set_flash_enabled(bool enabled)
{
    flash_effect_enabled = enabled;
}


void draw::show_rain()
{
    for (int i=0; i<MAP_W; i++) {
        for (int j=0; j<MAP_H; j++) {
            graphLib.showSurfaceRegionAt(&rain_obj, st_rectangle(_rain_pos*TILESIZE, 0, TILESIZE, TILESIZE), st_position(i*TILESIZE, j*TILESIZE));
        }
    }
    if (timer.getTimer() > _effect_timer) {
        _rain_pos++;
        if (_rain_pos > 2) {
            _rain_pos = 0;
        }
        _effect_timer = timer.getTimer() + RAIN_DELAY;
    }
}

void draw::show_flash()
{
    if (flash_obj.get_surface() == NULL) {
        // load rain
        std::string filename = GAMEPATH + "/shared/images/flash.png";
        graphLib.surfaceFromFile(filename, &flash_obj);
    }
    for (int i=0; i<FLASH_POINTS_N; i++) {
        graphLib.showSurfaceRegionAt(&flash_obj, st_rectangle(_flash_pos*FLASH_IMG_SIZE, 0, FLASH_IMG_SIZE, FLASH_IMG_SIZE), st_position(flash_points[i].x, flash_points[i].y));
    }
    if (timer.getTimer() > _flash_timer) {
        _flash_pos++;
        if (_flash_pos > 2) {
            _flash_pos = 0;
        }
        _flash_timer = timer.getTimer() + FLASH_DELAY;
    }
}

void draw::show_boss_intro_sprites(int boss_id, bool show_fall)
{
    unsigned int intro_frames_n = 0;
    //int intro_frames_rollback = 0;
    st_position sprite_size;
    graphicsLib_gSurface bgCopy, boss_graphics;


    // TODO: background
    std::string bg_filename_str = std::string(GameMediator::get_instance()->get_enemy(boss_id)->bg_graphic_filename);
    std::string bg_filename = FILEPATH + "images/sprites/enemies/backgrounds/" + bg_filename_str;
    std::string graph_filename = FILEPATH + "images/sprites/enemies/" + std::string(GameMediator::get_instance()->get_enemy(boss_id)->graphic_filename);

    //std::cout << "### graph_filename[" << graph_filename << "], bg_filename[" << bg_filename << "]" << std::endl;

    graphicsLib_gSurface bg_img;
    int bg_x = 0;
    int bg_y = 0;
    int bg_w = 0;
    int bg_h = 0;
    int dest_x = 0;
    int dest_y = 0;
    bool show_bg = false;
    if (bg_filename_str.length() > 0 && fio.file_exists(bg_filename)) {
        //std::cout << "### HAS_BG" << std::endl;
        show_bg = true;
        graphLib.surfaceFromFile(bg_filename.c_str(), &bg_img);
        // available space is 320x117
        bg_x = (RES_W - bg_img.width) / 2;
        bg_y = -(117 - bg_img.height) / 2;
        bg_w = bg_img.width;
        if (bg_w > RES_W) {
            bg_w = RES_W;
        }
        bg_h = bg_img.height;
        if (bg_h > 117) {
            bg_h = 117;
        }
        dest_x = 0;
        dest_y = 34;
        if (bg_w < RES_W) {
            dest_x = (RES_W-bg_w)/2;
        }
        if (bg_h < 117) {
            dest_y = (117-bg_h)/2;
        }

        bg_x = 0;

        //std::cout << "bg_x[" << bg_x << "], bg_y[" << bg_y << "], bg_w[" << bg_w << "], bg_h[" << bg_h << "], dest_x[" << dest_x << "], dest_y[" << dest_y << "]" << std::endl;
        graphLib.showSurfaceRegionAt(&bg_img, st_rectangle(bg_x, bg_y, bg_w, bg_h), st_position(dest_x, dest_y));


    //} else {
        //std::cout << "### NO_BG" << std::endl;
    }


    sprite_size.x = GameMediator::get_instance()->get_enemy(boss_id)->frame_size.width;
    sprite_size.y = GameMediator::get_instance()->get_enemy(boss_id)->frame_size.height;

    graphLib.surfaceFromFile(graph_filename.c_str(), &boss_graphics);
    int sprite_pos_y = BOSS_INTRO_BG_POS_Y-sprite_size.y/2;
    st_position boss_pos(RES_W/2-sprite_size.x/2, sprite_pos_y);

    if (bg_filename_str.length() > 0 && fio.file_exists(bg_filename)) {
        boss_pos.x -= 8;
        //boss_pos.y = 160;
    }

    graphLib.showSurface(&boss_intro_bg);

    for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
        if (GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i].used == true) {
            intro_frames_n++;
        }
    }

    // fall into position
    if (show_fall == true) {
        while (boss_pos.y < sprite_pos_y) {
            boss_pos.y += 4;
            graphLib.showSurface(&boss_intro_bg);
            if (show_bg) {
                graphLib.showSurfaceRegionAt(&bg_img, st_rectangle(bg_x, bg_y, bg_w, bg_h), st_position(dest_x, dest_y));
            }
            graphLib.copyArea(st_rectangle(0, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
            graphLib.updateScreen();
            timer.delay(5);
        }
        graphLib.updateScreen();
        timer.delay(500);
    } else {
        boss_pos.y = sprite_pos_y;
    }

    if (bg_filename_str.length() > 0 && fio.file_exists(bg_filename)) {
        boss_pos.y += 26;
    }


    // show intro sprites
    if (intro_frames_n > 1) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
            if (GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i].used == true) {
                graphLib.showSurface(&boss_intro_bg);
                if (show_bg) {
                    graphLib.showSurfaceRegionAt(&bg_img, st_rectangle(bg_x, bg_y, bg_w, bg_h), st_position(dest_x, dest_y));
                }
                graphLib.copyArea(st_rectangle(sprite_size.x * GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i].sprite_graphic_pos_x, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
                // only wait if not last frame
                if (i<ANIM_FRAMES_COUNT-1 && GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i+1].used) {
                    graphLib.updateScreen();
                    timer.delay(GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i].duration);
                }
            }
        }
    } else { // just frow first sprite
        graphLib.showSurface(&boss_intro_bg);
        if (show_bg) {
            graphLib.showSurfaceRegionAt(&bg_img, st_rectangle(bg_x, bg_y, bg_w, bg_h), st_position(dest_x, dest_y));
        }
        graphLib.copyArea(st_rectangle(0, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
        graphLib.updateScreen();
        timer.delay(200);
    }
}

void draw::show_ready()
{
    st_position dest_pos((RES_W/2)-26, (RES_H/2)-6);
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &screen_copy);

    for (int i=0; i<6; i++) {
        graphLib.draw_centered_text(dest_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_ready_message));
        update_screen();
        timer.delay(200);
        graphLib.copyArea(st_position(0, 0), &screen_copy, &graphLib.gameScreen);
        update_screen();
        timer.delay(200);
    }
}

void draw::show_bubble(int x, int y)
{
    if (_bubble_gfx.get_surface() == NULL) {
        std::string filename = GAMEPATH + "shared/images/bubble.png";
        graphLib.surfaceFromFile(filename, &_bubble_gfx);
    }
    graphLib.showSurfaceAt(&_bubble_gfx, st_position(x, y), false);
}

void draw::set_teleport_small_colors(st_color color1, st_color color2)
{
    graphLib.change_surface_color(0, color1, &_teleport_small_gfx);
    graphLib.change_surface_color(1, color2, &_teleport_small_gfx);
}

void draw::show_teleport_small(int x, int y)
{
    if (teleport_small_frame_count > 0) {
        int x_origin = teleport_small_frame * _teleport_small_gfx.height;
        if (timer.getTimer() > teleport_small_frame_timer) {
            teleport_small_frame++;
            teleport_small_frame_timer = timer.getTimer()+100;
        }
        if (teleport_small_frame >= teleport_small_frame_count) {
            teleport_small_frame = 0;
        }
        st_rectangle origin_rect(x_origin, 0, _teleport_small_gfx.height, _teleport_small_gfx.height);
        graphLib.showSurfaceRegionAt(&_teleport_small_gfx, origin_rect, st_position(x, y));
    } else {
        graphLib.showSurfaceAt(&_teleport_small_gfx, st_position(x, y), false);
    }
}

int draw::show_credits_text(bool can_leave, std::vector<std::string> credit_text, bool show_new_characters_warning)
{
    unsigned int scrolled = 0;
    int posY = RES_H;
    st_rectangle dest;
    float bg1_speed = 0.5;
    float bg2_speed = 3.5;
    float bg1_pos = 0;
    float bg2_pos = 0;
    graphicsLib_gSurface bg1;
    graphicsLib_gSurface bg2;

    graphLib.surfaceFromFile(GAMEPATH + "/shared/images/star_field1.png", &bg1);
    graphLib.surfaceFromFile(GAMEPATH + "/shared/images/star_field2.png", &bg2);

    graphLib.blank_screen();

    // add the initial lines to screen
    create_engine_credits_text();

    timer.delay(200);
    input.clean();
    timer.delay(200);

    update_screen();

    // scroll the lines
    unsigned int limit = (credit_text.size()*12)+RES_H/2;
    while (scrolled < limit) {

        //@TODO: draw stars fields //
        graphLib.blank_screen();
        graphLib.copyArea(st_position(0, bg1_pos), &bg1, &graphLib.gameScreen);
        if (bg1_pos > 0) {
            graphLib.copyArea(st_rectangle(0, RES_H-bg1_pos, RES_W, bg1_pos), st_position(0, 0), &bg1, &graphLib.gameScreen);
        }
        graphLib.copyArea(st_position(0, bg2_pos), &bg2, &graphLib.gameScreen);
        if (bg2_pos > 0) {
            graphLib.copyArea(st_rectangle(0, RES_H-bg2_pos, RES_W, bg2_pos), st_position(0, 0), &bg2, &graphLib.gameScreen);
        }


        // @TODO: calculate min and max to trestrain loop number
        for (unsigned int i=0; i<credit_text.size(); i++) {
            draw_credits_line(i, credit_text, posY);
        }


        if (can_leave) {
            input.read_input();
            if (input.wait_scape_time(STARS_DELAY) == 1 || input.p1_input[BTN_JUMP] == 1) {
                return 1;
            }
        } else {
            timer.delay(STARS_DELAY);
        }
        posY--;
        scrolled++;
        // need to copy next line and reposition

        bg1_pos += bg1_speed;
        bg2_pos += bg2_speed;

        if (bg1_pos >= RES_H) {
            bg1_pos = 0;
        }
        if (bg2_pos >= RES_H) {
            bg2_pos = 0;
        }
        update_screen();
    }

    // keep showing stars and text until user press a key
    timer.delay(50);
    input.clean();
    if (can_leave == false) {
        bool keep_running = true;
        while (keep_running) {
            graphLib.blank_screen();
            graphLib.copyArea(st_position(0, bg1_pos), &bg1, &graphLib.gameScreen);
            if (bg1_pos > 0) {
                graphLib.copyArea(st_rectangle(0, RES_H-bg1_pos, RES_W, bg1_pos), st_position(0, 0), &bg1, &graphLib.gameScreen);
            }
            graphLib.copyArea(st_position(0, bg2_pos), &bg2, &graphLib.gameScreen);
            if (bg2_pos > 0) {
                graphLib.copyArea(st_rectangle(0, RES_H-bg2_pos, RES_W, bg2_pos), st_position(0, 0), &bg2, &graphLib.gameScreen);
            }
            bg1_pos += bg1_speed;
            bg2_pos += bg2_speed;
            if (bg1_pos >= RES_H) {
                bg1_pos = 0;
            }
            if (bg2_pos >= RES_H) {
                bg2_pos = 0;
            }

            draw_credits_line(credit_text.size()-2, credit_text, posY);
            draw_credits_line(credit_text.size()-1, credit_text, posY);
            if (show_new_characters_warning == true) {
                graphLib.draw_centered_text(RES_H-34, strings_map::get_instance()->get_ingame_string(strings_ending_NEW_CHARACTERS_AVAILABLE));
            }
            graphLib.draw_text(40, RES_H-22, strings_map::get_instance()->get_ingame_string(strings_ending_NEW_CHARACTERS_PRESS_TO_CONTINUE));
            graphLib.showSurfaceAt(&input_images_map[INPUT_IMAGES_A], st_position(80, RES_H-24), false);

            if (input.wait_scape_time(STARS_DELAY) == 1 || input.p1_input[BTN_JUMP] == 1) {
                keep_running = false;
                return 1;
            }

            update_screen();
        }

    }

    update_screen();
    return 0;
}

void draw::draw_credits_line(int i, std::vector<string> credit_text, int posY)
{
    int text_pos = posY+12*i;

    if (text_pos >= -12 && text_pos <= RES_H+12) {
        std::size_t found_title_blue = credit_text.at(i).find("- ");
        std::size_t found_title_red = credit_text.at(i).find("# ");
        if (credit_text.at(i)[0] == '@') { // section
            std::string text_out = credit_text.at(i);
            text_out = text_out.substr(1, text_out.length()-1);
            graphLib.draw_centered_text(text_pos, text_out, graphLib.gameScreen, st_color(102, 255, 181));
        } else if (found_title_red != std::string::npos) { // main title
            graphLib.draw_centered_text(text_pos, credit_text.at(i), graphLib.gameScreen, st_color(249, 98, 98));
        } else if (found_title_blue != std::string::npos) { // sub-title
            graphLib.draw_centered_text(text_pos, credit_text.at(i), graphLib.gameScreen, st_color(95, 151, 255));
        } else {
            graphLib.draw_centered_text(text_pos, credit_text.at(i));
        }
    }
}

int draw::show_credits(bool can_leave, bool show_new_characters_warning)
{
    soundManager.stop_music();
    soundManager.load_music("rockbot_endcredits.mod");
    soundManager.play_music();

    int res = show_credits_text(can_leave, create_engine_credits_text(), show_new_characters_warning);

    if (res == 1) {
        soundManager.stop_music();
        soundManager.load_music(game_data.game_start_screen_music_filename);
        soundManager.play_music();
        return 1;
    }
    //graphLib.blank_screen();// should leave presented by on the screen
    graphLib.updateScreen();
    timer.delay(1000);
    input.clean();
    timer.delay(100);
    input.wait_keypress();
    return 0;
}

void draw::show_about()
{
    // show informations about the gameproject
    graphLib.clear_area(0, 0, RES_W, RES_H, 0, 0, 0);
    graphLib.draw_centered_text(10, strings_map::get_instance()->get_ingame_string(strings_about_site), st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
    graphLib.draw_centered_text(22, "http://rockdot.upperland.net", st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));

    graphLib.draw_centered_text(60, strings_map::get_instance()->get_ingame_string(strings_about_bugs), st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
    graphLib.draw_centered_text(72, "bugs@upperland.net", st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));

    graphLib.draw_centered_text(110, strings_map::get_instance()->get_ingame_string(strings_about_twitter), st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
    graphLib.draw_centered_text(122, "@RockbotGame", st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));

    graphLib.draw_centered_text(160, strings_map::get_instance()->get_ingame_string(strings_about_facebook), st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
    graphLib.draw_centered_text(172, "facebook.com/rockbotgame", st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));

    graphLib.draw_centered_text(220, strings_map::get_instance()->get_ingame_string(strings_ingame_config_presstorestart), st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));

    graphLib.updateScreen();
    timer.delay(1000);

    input.clean();
    timer.delay(100);
    input.wait_keypress();
}


std::vector<string> draw::create_engine_credits_text()
{

    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<string> credits_list;
    credits_list = fio_str.get_string_list_from_file(FILEPATH + "/game_credits.txt");

    if (credits_list.size() > 0) {
        for (int i=0; i<6; i++) {
            credits_list.push_back("");
        }
    }
    std::vector<string> credits_engine = fio_str.get_game_engine_credits();
    for (unsigned int i=0; i<credits_engine.size(); i++) {
        credits_list.push_back(credits_engine.at(i));
    }

    return credits_list;
}


void draw::draw_credit_line(graphicsLib_gSurface &surface, Uint8 initial_line,std::vector<std::string> credit_text)
{
    if (initial_line < credit_text.size()) {
        std::size_t found = credit_text.at(initial_line).find("- ");
        if (found != std::string::npos) {
            graphLib.draw_centered_text(RES_H, credit_text.at(initial_line), surface, st_color(95, 151, 255));
        } else {
            graphLib.draw_centered_text(RES_H, credit_text.at(initial_line), surface, st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
        }
    }
}






graphicsLib_gSurface *draw::get_object_graphic(int obj_id)
{
    std::map<unsigned int, graphicsLib_gSurface>::iterator it;
    graphicsLib_gSurface temp_sprite;

    it = objects_sprite_list.find(obj_id);
    if (it == objects_sprite_list.end()) { // there is no graphic with this key yet, add it
        std::string graphic_filename(GameMediator::get_instance()->object_list.at(obj_id).graphic_filename);
        if (graphic_filename.length() > 0) {
            std::string complete_filename(FILEPATH + "images/sprites/objects/" + graphic_filename);
            graphLib.surfaceFromFile(complete_filename, &temp_sprite);
            objects_sprite_list.insert(std::pair<unsigned int, graphicsLib_gSurface>(obj_id, temp_sprite));
            it = objects_sprite_list.find(obj_id);
        } else {
            std::cout << "ERROR: Invalid object graphic. Object_ID: '" + obj_id << "'" << std::endl;
            return NULL;
        }
    }
    return &(*it).second;
}

void draw::remove_object_graphic(int obj_id)
{
    std::map<unsigned int, graphicsLib_gSurface>::iterator it;
    it = objects_sprite_list.find(obj_id);
    if (it != objects_sprite_list.end()) {
        objects_sprite_list.erase(obj_id);
    }
}

void draw::show_ingame_warning(std::vector<std::string> message)
{
    graphLib.show_dialog(0);
    st_position dialog_pos = graphLib.get_dialog_pos();
    for (unsigned int i=0; i<message.size(); i++) {
        graphLib.draw_text(dialog_pos.x+20, dialog_pos.y+16+(12*i), message[i]);
    }
    graphLib.show_dialog_button(0);
    input.clean();
    input.wait_keypress();
}

void draw::fade_in_screen(int r, int g, int b, int total_delay)
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &screen_copy);

    graphicsLib_gSurface transparent_area;
    graphLib.initSurface(st_size(RES_W, RES_H), &transparent_area);
    graphLib.clear_surface_area(0, 0, RES_W, RES_H, r, g, b, transparent_area);

    float step = 255.0/20.0;
    float alpha_value = 255;
    float delay = (total_delay / 25)-10;

    for (float i=0; i<=20; i++) {
        graphLib.showSurface(&screen_copy);
        graphLib.set_surface_alpha((int)alpha_value, transparent_area);
        graphLib.showSurface(&transparent_area);
        alpha_value -= step;
        graphLib.updateScreen();
        timer.delay(delay);
    }
}

void draw::fade_out_screen(int r, int g, int b, int total_delay)
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &screen_copy);

    graphicsLib_gSurface transparent_area;
    graphLib.initSurface(st_size(RES_W, RES_H), &transparent_area);
    graphLib.clear_surface_area(0, 0, RES_W, RES_H, r, g, b, transparent_area);

    float step = 255.0/20.0;
    float alpha_value = 0;
    float delay = (total_delay / 25)-10;

    for (float i=0; i<=20; i++) {
        graphLib.showSurface(&screen_copy);
        graphLib.set_surface_alpha((int)alpha_value, transparent_area);
        alpha_value += step;
        graphLib.showSurface(&transparent_area);
        graphLib.updateScreen();
        if (delay >= 1) {
            timer.delay(delay);
        }
    }
}

// executes the fade-out each time update screen is called
void draw::add_fade_out_effect(int r, int g, int b)
{
    current_alpha = 0;
    current_alpha_color = st_color(r, g, b);
}

void draw::remove_fade_out_effect()
{
    current_alpha = -1;
    current_alpha_surface.freeGraphic();
}

void draw::pixelate_screen()
{
    graphicsLib_gSurface res_surface;
    graphLib.initSurface(st_size(RES_W, RES_H), &res_surface);

    int width = graphLib.gameScreen.width;
    int height = graphLib.gameScreen.height;


    for (int pixelationAmount=2; pixelationAmount<8; pixelationAmount++) {
        for (int x = 0; x < width; x+= pixelationAmount) { // do the whole image
            for (int y = 0; y < height; y+= pixelationAmount) {

                int avR = 0;
                int avG = 0;
                int avB =0;


                int pointsCount = 0;
                for (int i=0; i<pixelationAmount; i++) {
                    for (int j=0; j<pixelationAmount; j++) {
                        st_color pt_color = graphLib.gameScreen.get_point_color(x+i, y+j);
                        //res_surface.set_point_color((i+x), (j+y), pt_color.r, pt_color.g, pt_color.b);

                        avR += (int) (pt_color.r);
                        avG+= (int) (pt_color.g);
                        avB += (int) (pt_color.b);
                        if (pt_color.r != 0 && pt_color.g != 0 && pt_color.g != 0) {
                            pointsCount++;
                        }
                    }
                }
                if (pointsCount != 0) {
                    avR = avR/pointsCount; //divide all by the amount of samples taken to get an average
                    avG = avG/pointsCount;
                    avB = avB/pointsCount;
                }

                for (int i=0; i<pixelationAmount; i++) {
                    for (int j=0; j<pixelationAmount; j++) {
                        res_surface.set_point_color(x+i, y+j, avR, avG, avB);
                    }
                }

            }
        }
        graphLib.copyArea(st_position(0, 0), &res_surface, &graphLib.gameScreen);
        graphLib.updateScreen();
        timer.delay(20);
    }
    res_surface.freeGraphic();

}

void draw::add_weapon_tooltip(short weapon_n, const st_position &player_pos, const Uint8 &direction)
{
    _weapon_tooltip_n = weapon_n;
    _weapon_tooltip_pos_ref = &player_pos;
    _weapon_tooltip_timer = timer.getTimer() + 1500;
    _weapon_tooltip_direction_ref = &direction;
}

void draw::show_weapon_tooltip()
{
    if (timer.getTimer() < _weapon_tooltip_timer) {
        int adjust_x = 5;
        int adjust_y = 4-TILESIZE;
        Uint8 direction_value = *_weapon_tooltip_direction_ref;
        if (direction_value == ANIM_DIRECTION_LEFT) {
            adjust_x = 10;
        }
        graphLib.draw_weapon_changed_tooltip(_weapon_tooltip_n);
        //graphLib.draw_weapon_tooltip_icon(_weapon_tooltip_n, st_position(20, RES_H-60), true);
    }
}

void draw::draw_in_game_menu(graphicsLib_gSurface *character_sprite, short selected_weapon_n)
{
    graphLib.copyArea(st_rectangle(0, 0, ingame_menu_bg_img.width, ingame_menu_bg_img.height), st_position(0, 0), &ingame_menu_bg_img, &graphLib.gameScreen);

    int config_text_pos_x = RES_W - 10 - (strings_map::get_instance()->get_ingame_string(strings_ingame_config).length()+4)*8;
    graphLib.draw_text(20, 22, "GAME MENU");
    graphLib.draw_text(config_text_pos_x, 22, strings_map::get_instance()->get_ingame_string(strings_ingame_config) + std::string(" [R]"));

    draw_weapon_menu_weapon(selected_weapon_n);
    weapon_menu_show_player(character_sprite);

    graphLib.updateScreen();
}

void draw::draw_game_menu_weapon_bar(int selected_weapon_n, int weapon_n, int percent, int value)
{
    if (weapon_n >= WEAPON_ITEM_ETANK && weapon_n < WEAPON_COUNT && value == 0) {
        return;
    }

    int row_n = weapon_n/WEAPON_MENU_COL_N;
    int col_n = 0;
    int adjusted_weapon_n = 0;
    if (weapon_n != 0) {
        adjusted_weapon_n = (weapon_n - row_n*WEAPON_MENU_COL_N);
        col_n = adjusted_weapon_n;
    }

    int x = 25 + col_n*76;
    int y = 60 + row_n*20;

    graphLib.draw_small_weapon_icon_at(weapon_n, st_position(x-10, y+1), selected_weapon_n == weapon_n);

    if (selected_weapon_n == weapon_n && weapon_n < WEAPON_ITEM_ETANK) {
        hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[weapon_n].color1);
    }


    if (weapon_n != WEAPON_DEFAULT && weapon_n >= WEAPON_ITEM_ETANK) {
        //std::cout << "TANK[" << weapon_n << "], value[" << value << "]" << std::endl;
        char error_msg[10];
        if (value < 10) {
            sprintf(error_msg, "[0%d]", value);
        } else {
            sprintf(error_msg, "[%d]", value);
        }
        graphLib.draw_text(x, y+1, std::string(error_msg));
        return;
    }

    // 5 balls, each have 4 possible stages
    // so each slice of energy is 100 / (5*4) = 5%
    for (int i=0; i<5; i++) {
        // less than min1 means black ball
        int min1 = ENERGY_BALL_PERCENT_SLICE*4*i + 5;     // 1/4
        int min2 = ENERGY_BALL_PERCENT_SLICE*4*i + 10;    // 2/4
        int min3 = ENERGY_BALL_PERCENT_SLICE*4*i + 15;    // 3/4
        int min4 = ENERGY_BALL_PERCENT_SLICE*4*i + 20;    // full

        int img_origin_x = ENERGY_BALL_IMG_SIZE*4;

        if (percent >= min4) {
            img_origin_x = 0;
        } else if (percent >= min3) {
            img_origin_x = ENERGY_BALL_IMG_SIZE;
        } else if (percent >= min2) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*2;
        } else if (percent >= min1) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*3;
        }

        if (selected_weapon_n == weapon_n) {
            graphLib.copyArea(st_rectangle(img_origin_x, 0, hud_player_wpn_ball.height, hud_player_wpn_ball.height), st_position(x+i*10, y), &hud_player_wpn_ball, &graphLib.gameScreen);
        } else {
            graphLib.copyArea(st_rectangle(img_origin_x, 0, hp_ball_disabled.height, hp_ball_disabled.height), st_position(x+i*10, y), &hp_ball_disabled, &graphLib.gameScreen);
        }
    }
}


void draw::draw_weapon_menu_weapon(short selected_weapon_n)
{
    for (int i=0; i<WEAPON_COUNT; i++) {
        int selected_weapon_value = game_save.items.weapons[i];
        if (i < WEAPON_ITEM_ETANK) {
            // draw weapon
            hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[i].color1);
            int wpn_percent = (100 * selected_weapon_value) / fio.get_heart_pieces_number(game_save);
            //std::cout << "draw::draw_weapon_menu_weapon - wpn_percent[" << i << "][" << wpn_percent << "]" << std::endl;
            if (dataUtil::get_instance()->has_weapon(i) == true) {
                draw_game_menu_weapon_bar(selected_weapon_n, i, wpn_percent, game_save.items.weapons[i]);
            }
        } else if (i != WEAPON_DEFAULT && i >= WEAPON_ITEM_ETANK) {
            hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, st_color(250, 250, 250));
            int wpn_percent = 0;
            if (i == WEAPON_ITEM_ETANK) {
                wpn_percent = game_save.items.energy_tanks*10;
                draw_game_menu_weapon_bar(selected_weapon_n, i, wpn_percent, game_save.items.energy_tanks);
            } else if (i == WEAPON_ITEM_WTANK) {
                wpn_percent = game_save.items.weapon_tanks*10;
                draw_game_menu_weapon_bar(selected_weapon_n, i, wpn_percent, game_save.items.weapon_tanks);
            } else if (i == WEAPON_ITEM_STANK) {
                wpn_percent = game_save.items.special_tanks*10;
                draw_game_menu_weapon_bar(selected_weapon_n, i, wpn_percent, game_save.items.special_tanks);
            }

        }
    }

    graphLib.draw_text(18, 214,  strings_map::get_instance()->get_ingame_string(strings_stage_select_select));
    graphLib.draw_text(180, 214,  "OK");
    graphLib.draw_text(220, 214,  strings_map::get_instance()->get_ingame_string(strings_ingame_leavestage));

    graphLib.draw_text(104, 165,  strings_map::get_instance()->get_ingame_string(strings_weapon_selected)+std::string("/")+strings_map::get_instance()->get_ingame_string(strings_ingame_item)+std::string(":"));
    graphLib.draw_text(104, 179,  get_selected_weapon_name(selected_weapon_n));

}

string draw::get_selected_weapon_name(int selected_weapon_n)
{
    std::string weapon_name = "";
    if (selected_weapon_n < WEAPON_ITEM_COIL) {
        weapon_name = std::string(game_data.weapons[selected_weapon_n].name);
    }
    if (selected_weapon_n == WEAPON_ITEM_COIL) {
        weapon_name = strings_map::get_instance()->get_ingame_string(strings_weapon_name_COIL);
    } else if (selected_weapon_n == WEAPON_ITEM_JET) {
        weapon_name = strings_map::get_instance()->get_ingame_string(strings_weapon_name_JET);
    } else if (selected_weapon_n == WEAPON_ITEM_ETANK) {
        char crystal_msg[50];
        sprintf(crystal_msg, "%s [%d]", strings_map::get_instance()->get_ingame_string(strings_weapon_name_ETANK).c_str(), game_save.items.energy_tanks);
        weapon_name = std::string(crystal_msg);
    } else if (selected_weapon_n == WEAPON_ITEM_WTANK) {
        char crystal_msg[50];
        sprintf(crystal_msg, "%s [%d]", strings_map::get_instance()->get_ingame_string(strings_weapon_name_WTANK).c_str(), game_save.items.weapon_tanks);
        weapon_name = std::string(crystal_msg);
    } else if (selected_weapon_n == WEAPON_ITEM_STANK) {
        char crystal_msg[50];
        sprintf(crystal_msg, "%s [%d]", strings_map::get_instance()->get_ingame_string(strings_weapon_name_STANK).c_str(), game_save.items.special_tanks);
        weapon_name = std::string(crystal_msg);
    }
    return weapon_name;
}

void draw::weapon_menu_show_player(graphicsLib_gSurface *character_sprite)
{
    graphLib.copyArea(st_position(6, 157), character_sprite, &graphLib.gameScreen);
    char player_lifes[20];
    sprintf(player_lifes, "x[%d]", game_save.items.lifes);
    graphLib.draw_text(38, 178, player_lifes);

    // show armor pieces
    graphLib.copyArea(st_rectangle(0, ARMOR_PIECES_SPRITE_H*game_save.selected_player, ARMOR_PIECES_SPRITE_W, ARMOR_PIECES_SPRITE_H), st_position(ARMOR_PIECES_SPRITE_POS_X, 157), &ingame_menu_player_armor_pieces, &graphLib.gameScreen);
    if (game_save.armor_pieces[ARMOR_TYPE_ARMS]) {
        graphLib.copyArea(st_rectangle(ARMOR_PIECES_SPRITE_W, ARMOR_PIECES_SPRITE_H*game_save.selected_player, ARMOR_PIECES_SPRITE_W, ARMOR_PIECES_SPRITE_H), st_position(ARMOR_PIECES_SPRITE_POS_X, 157), &ingame_menu_player_armor_pieces, &graphLib.gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_TYPE_BODY]) {
        graphLib.copyArea(st_rectangle(ARMOR_PIECES_SPRITE_W*2, ARMOR_PIECES_SPRITE_H*game_save.selected_player, ARMOR_PIECES_SPRITE_W, ARMOR_PIECES_SPRITE_H), st_position(ARMOR_PIECES_SPRITE_POS_X, 157), &ingame_menu_player_armor_pieces, &graphLib.gameScreen);
    }
    if (game_save.armor_pieces[ARMOR_TYPE_LEGS]) {
        graphLib.copyArea(st_rectangle(ARMOR_PIECES_SPRITE_W*3, ARMOR_PIECES_SPRITE_H*game_save.selected_player, ARMOR_PIECES_SPRITE_W, ARMOR_PIECES_SPRITE_H), st_position(ARMOR_PIECES_SPRITE_POS_X, 157), &ingame_menu_player_armor_pieces, &graphLib.gameScreen);
    }

}


st_float_position draw::get_radius_point(st_position center_point, int radius, float angle)
{
    st_float_position res;
    // x = r * cos(x0), y = r * sin(y0)
    res.x = radius * cos(angle) + center_point.x;
    res.y = radius * sin(angle) + center_point.y;
    return res;
}


void draw::draw_explosion(st_position center_point, int radius, int angle_inc)
{
    // 8 initial points
    int points_n = 12;
    st_float_position points[points_n];
    float angle_diff = 360 / points_n;
    int frame = 0;

    for (int j=0; j<points_n; j++) {
        float angle = (j*angle_diff)+angle_inc;
        float angle_rad = (angle * 3.14)/180;
        points[j] = get_radius_point(center_point, radius, angle_rad);
        graphLib.copyArea(st_rectangle(frame*32, 0, 32, 32), st_position(points[j].x, points[j].y), graphLib.get_preloaded_image(PRELOADED_IMAGES_EXPLOSION_BUBBLE), &graphLib.gameScreen);
    }
    update_screen();
}

void draw::draw_player_death(st_position center_point, int frame_n)
{
    int frame_size = _death_animation.height;
    graphLib.copyArea(st_rectangle(frame_n*frame_size, 0, frame_size, frame_size), st_position(center_point.x-frame_size/2, center_point.y-frame_size/2), &_death_animation, &graphLib.gameScreen);
}

int draw::get_death_animation_frames_n()
{
    return _death_animation_frames_n;
}


graphicsLib_gSurface *draw::get_dynamic_background(string filename)
{
    std::map<std::string, graphicsLib_gSurface>::iterator it;

    it = maps_dynamic_background_list.find(filename);
    if (it == maps_dynamic_background_list.end()) {
        return NULL;
    }

    return &maps_dynamic_background_list.find(filename)->second;
}

graphicsLib_gSurface *draw::get_dynamic_foreground(string filename)
{
    std::map<std::string, graphicsLib_gSurface>::iterator it;

    it = maps_dynamic_background_list.find(filename);
    if (it == maps_dynamic_background_list.end()) {
        return NULL;
    }

    return &maps_dynamic_background_list.find(filename)->second;
}

void draw::set_dynamic_bg_alpha(string filename, int alpha)
{
    std::map<std::string, graphicsLib_gSurface>::iterator it;

    it = maps_dynamic_background_list.find(filename);
    if (it == maps_dynamic_background_list.end()) {
        return;
    }
    graphLib.set_surface_alpha(alpha, &maps_dynamic_background_list.find(filename)->second);
}


void draw::show_hud(int hp, int player_n, int selected_weapon, int selected_weapon_value)
{
    if (game_data.game_style == GAME_STYLE_VINTAGE) {
        show_hud_vintage(hp, player_n, selected_weapon, selected_weapon_value);
        return;
    }
    // player HP
    int hp_percent = (100 * hp) / fio.get_heart_pieces_number(game_save);
    draw_enery_ball(hp_percent, 3, hud_player_hp_ball);


    if (selected_weapon != WEAPON_DEFAULT && selected_weapon < WEAPON_ITEM_ETANK) {
        // draw weapon
        hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[selected_weapon].color1);
        int wpn_percent = (100 * selected_weapon_value) / fio.get_heart_pieces_number(game_save);
        draw_enery_ball(wpn_percent, 62, hud_player_wpn_ball);
    } else if (selected_weapon != WEAPON_DEFAULT && selected_weapon >= WEAPON_ITEM_ETANK) {
        hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, st_color(250, 250, 250));
        int wpn_percent = 0;
        if (selected_weapon == WEAPON_ITEM_ETANK) {
            wpn_percent = game_save.items.energy_tanks*10;
        } else if (selected_weapon == WEAPON_ITEM_WTANK) {
            wpn_percent = game_save.items.weapon_tanks*10;
        } else if (selected_weapon == WEAPON_ITEM_STANK) {
            wpn_percent = game_save.items.special_tanks*10;
        }
        draw_enery_ball(wpn_percent, 62, hud_player_wpn_ball);
    }

    // boss HP

    if (gameControl.must_show_boss_hp() && _boss_current_hp != -99) {
        int boss_hp_percent = (100 * _boss_current_hp) / BOSS_INITIAL_HP;
        int adjust_x = strings_map::get_instance()->get_ingame_string(strings_stage_select_boss).length()*FONT_SIZE;
        graphLib.draw_text(RES_W-55-4-adjust_x, 4, strings_map::get_instance()->get_ingame_string(strings_stage_select_boss));
        draw_enery_ball(boss_hp_percent, RES_W-55, hud_boss_hp_ball);
    }
}

void draw::show_hud_vintage(int hp, int player_n, int selected_weapon, int selected_weapon_value)
{
    // player HP
    int hp_percent = (100 * hp) / fio.get_heart_pieces_number(game_save);
    //draw_energy_bar(short hp, short player_n, short weapon_n, short max_hp)
    draw_energy_bar(hp, player_n, selected_weapon, fio.get_heart_pieces_number(game_save));
    if (selected_weapon != WEAPON_DEFAULT) {
        draw_energy_bar(hp, player_n, WEAPON_DEFAULT, fio.get_heart_pieces_number(game_save));
    }

    // boss HP
    if (gameControl.must_show_boss_hp() && _boss_current_hp != -99) {
        int boss_hp_percent = (100 * _boss_current_hp) / BOSS_INITIAL_HP;
        graphLib.draw_text(RES_W-95, 10, strings_map::get_instance()->get_ingame_string(strings_stage_select_boss));
        draw_enery_ball(boss_hp_percent, RES_W-55, hud_boss_hp_ball);
    }
}


void draw::draw_enery_ball(int value, int x_pos, graphicsLib_gSurface& ball_surface)
{
    // 5 balls, each have 4 possible stages
    // so each slice of energy is 100 / (5*4) = 5%
    for (int i=0; i<5; i++) {
        // less than min1 means black ball
        int min1 = ENERGY_BALL_PERCENT_SLICE*4*i + 5;     // 1/4
        int min2 = ENERGY_BALL_PERCENT_SLICE*4*i + 10;    // 2/4
        int min3 = ENERGY_BALL_PERCENT_SLICE*4*i + 15;    // 3/4
        int min4 = ENERGY_BALL_PERCENT_SLICE*4*i + 20;    // full

        int img_origin_x = ENERGY_BALL_IMG_SIZE*4;

        if (value >= min4) {
            img_origin_x = 0;
        } else if (value >= min3) {
            img_origin_x = ENERGY_BALL_IMG_SIZE;
        } else if (value >= min2) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*2;
        } else if (value >= min1) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*3;
        }

        graphLib.copyArea(st_rectangle(img_origin_x, 0, ball_surface.height, ball_surface.height), st_position(x_pos+(10*i), 3), &ball_surface, &graphLib.gameScreen);
    }
}


void draw::draw_energy_bar(short hp, short player_n, short weapon_n, short max_hp)
{
    short int y;
    int hp_percent = (100 * hp) / max_hp;
    int graph_lenght = (int)(hp_percent/2);


    st_color color0(0, 0, 0); // black
    st_color color1(10, 10, 10);
    st_color color2(10, 10, 10);
    if (player_n != -1) {
        st_color weapon_color1 = GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[weapon_n].color1;
        st_color weapon_color2 = GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[weapon_n].color2;
        color1 = st_color(weapon_color1.r, weapon_color1.g, weapon_color1.b);    // player color or weapon color
        color2 = st_color(weapon_color2.r, weapon_color2.g, weapon_color2.b);    // player color or weapon color
    }

    st_position bar_pos(1, 2);
    if (player_n == -1) {
        bar_pos.x = RES_W-15;
    } else if (weapon_n != WEAPON_DEFAULT) {
        bar_pos.x = 10;
    }

    int weapon_n_adjusted = weapon_n;
    if (weapon_n == -1) {
        weapon_n_adjusted = WEAPON_COUNT;
    }
    if (weapon_n <= WEAPON_ITEM_JET) {
        graphLib.draw_small_weapon_icon(weapon_n_adjusted, st_position(bar_pos.x, 54), true);
        y = bar_pos.y + 1 + (50 - graph_lenght);
        graphLib.clear_area(bar_pos.x, y, 2, graph_lenght, color1.r, color1.g, color1.b);
        graphLib.clear_area(bar_pos.x+2, y, 5, graph_lenght, color2.r, color2.g, color2.b);
        graphLib.clear_area(bar_pos.x+6, y, 2, graph_lenght, color1.r, color1.g, color1.b);
        for (int i=1; i<graph_lenght; i+=2) {
            graphLib.clear_area(bar_pos.x, y+i, 8, 1, color0.r, color0.g, color0.b);
        }
    }
}

void draw::set_boss_hp(int hp)
{
    _boss_current_hp = hp;
}

void draw::show_boss_intro_bg()
{
    graphLib.showSurface(&boss_intro_bg);
    graphLib.updateScreen();
}

void draw::clear_maps_dynamic_background_list()
{
    maps_dynamic_background_list.clear();
}

void draw::add_dynamic_background(string filename, int auto_scroll_mode, st_color bg_color)
{
    // only add if not existing in map
    if (maps_dynamic_background_list.find(filename) == maps_dynamic_background_list.end()) {
        maps_dynamic_background_list.insert(std::pair<std::string,graphicsLib_gSurface>(filename, graphicsLib_gSurface()));
        std::string bg1_filename(FILEPATH+"images/map_backgrounds/" + filename);
        graphicsLib_gSurface temp_surface;
        graphLib.surfaceFromFile(bg1_filename, &temp_surface);
        graphLib.initSurface(st_size(temp_surface.width, temp_surface.height), &maps_dynamic_background_list.find(filename)->second);
        graphLib.clear_surface_area(0, 0, temp_surface.width, temp_surface.height, bg_color.r, bg_color.g, bg_color.b, maps_dynamic_background_list.find(filename)->second);
        graphLib.copyArea(st_position(0, 0), &temp_surface, &maps_dynamic_background_list.find(filename)->second);
    }
}



void draw::generate_snow_particles()
{
    for (int i=0; i<SNOW_PARTICLES_NUMBER; i++) {
        int rand_x = rand() % RES_W;
        int rand_y = rand() % RES_H;
        int rand_speed = rand() % 5;
        if (rand_speed < 1) {
            rand_speed = 1;
        }
        _snow_particles.push_back(st_snow_particle(st_float_position(rand_x, rand_y), rand_speed));
    }
}

void draw::show_snow_effect()
{
    if (_snow_particles.size() == 0) {                  // generate snow particles, if needed
        generate_snow_particles();
    }
    std::vector<st_snow_particle>::iterator it;

    if (timer.getTimer() > _effect_timer) {

        for (it=_snow_particles.begin(); it!=_snow_particles.end(); it++) {
            st_snow_particle *temp_particle = &(*it);
            temp_particle->position.y += temp_particle->speed;
            if (temp_particle->direction == ANIM_DIRECTION_LEFT) {
                temp_particle->position.x--;
            } else {
                temp_particle->position.x++;
            }
            temp_particle->x_dist++;
            if (temp_particle->x_dist > 8) {
                temp_particle->x_dist = 0;
                temp_particle->direction = !temp_particle->direction;
            }
            if (temp_particle->position.y > RES_H) {
                temp_particle->position.x = rand() % RES_W;
                temp_particle->speed = rand() % 5;
                if (temp_particle->speed < 1) {
                    temp_particle->speed = 1;
                }
                temp_particle->position.y = 0;

            }
            graphLib.showSurfaceRegionAt(&snow_flacke, st_rectangle(0, 0, snow_flacke.width, snow_flacke.height), st_position(temp_particle->position.x, temp_particle->position.y));
        }
        _effect_timer = timer.getTimer() + SNOW_DELAY;
    } else {
        for (it=_snow_particles.begin(); it!=_snow_particles.end(); it++) {
            st_snow_particle *temp_particle = &(*it);
            graphLib.showSurfaceRegionAt(&snow_flacke, st_rectangle(0, 0, snow_flacke.width, snow_flacke.height), st_position(temp_particle->position.x, temp_particle->position.y));
        }
    }
}

void draw::show_train_effect()
{
    if (_train_effect_timer == 0) {
        _train_effect_timer = timer.getTimer() + TRAIN_DELAY;
        _train_effect_state = 0;
        if (_train_sfx == NULL) {
            _train_sfx = soundManager.sfx_from_file("train.wav");
        }
    } else {
        if (_train_effect_timer < timer.getTimer()) {
            if (_train_effect_state == 0) {
                graphLib.set_screen_adjust(st_position(0, -TRAIN_EFFECT_SCREEN_MOVE));
                _train_effect_timer = timer.getTimer() + TRAIN_EFFECT_DELAY;
                _train_effect_state++;
                soundManager.play_sfx_from_chunk(_train_sfx, 1);
            } else {
                graphLib.set_screen_adjust(st_position(0, 0));
                _train_effect_timer = timer.getTimer() + TRAIN_DELAY;
                _train_effect_state = 0;
                soundManager.play_sfx_from_chunk(_train_sfx, 1);
            }
        }
    }
    //graphLib.set_screen_adjust(st_position(-QUAKE_SCREEN_MOVE, 0));
}

void draw::show_lightingbolt_effect()
{
    if (_lightingbolt_effect_timer == 0) {
        _lightingbolt_effect_timer = timer.getTimer() + LIGHTINGBOLT_DELAY1;
        return;
    }
    if (_lightingbolt_effect_timer < timer.getTimer()) {
        if (_lightingbolt_effect_state <= 5) {
            _lightingbolt_effect_timer = timer.getTimer() + LIGHTINGBOLT_DELAY2;
            _lightingbolt_effect_state++;
        } else {
            _lightingbolt_effect_timer = timer.getTimer() + LIGHTINGBOLT_DELAY1;
            _lightingbolt_effect_state = 0;
        }
    } else {
        if (_lightingbolt_effect_state % 2 != 0) {
            if (_lightingbolt_effect_state == 1) {
                soundManager.play_shared_sfx("thunder.wav");
            }
            graphicsLib_gSurface transparent_area;
            graphLib.initSurface(st_size(RES_W, RES_H), &transparent_area);
            graphLib.clear_surface_area(0, 0, RES_W, RES_H, 250, 250, 158, transparent_area);
            graphLib.set_surface_alpha(80, transparent_area);
            graphLib.showSurface(&transparent_area);

            graphLib.clear_area(0, 0, RES_W, RES_H, 250, 250, 158);
        }
    }
}

void draw::show_shadow_top_effect()
{

    int max = 100;
    int alpha = 255;
    int alpha_step = alpha/12;
    for (int i=0; i<max; i+=10) {
        graphLib.set_surface_alpha(alpha, &shadow_line);
        graphLib.copyArea(st_rectangle(0, 0, shadow_line.width, shadow_line.height), st_position(0, i), &shadow_line, &graphLib.gameScreen);
        alpha -= alpha_step;
    }
}

void draw::show_inferno_effect()
{
    if (_inferno_surface.is_null()) {
        graphLib.initSurface(st_size(RES_W, RES_H), &_inferno_surface);
        graphLib.clear_surface_area(0, 0, RES_W, RES_H, 180, 0, 0, _inferno_surface);
    }
    graphLib.set_surface_alpha(_inferno_alpha, _inferno_surface);
    graphLib.showSurface(&_inferno_surface);
    if (_inferno_alpha_mode == 0) {
        _inferno_alpha += INFERNO_ALPHA_STEP;
        if (_inferno_alpha >= 180) {
            _inferno_alpha_mode = 1;
        }
    } else {
        _inferno_alpha -= INFERNO_ALPHA_STEP;
        if (_inferno_alpha <= 50) {
            _inferno_alpha_mode = 0;
        }
    }
}

void draw::free_inferno_surface()
{
    _inferno_surface.freeGraphic();
}



