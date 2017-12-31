#include <stdio.h>
#include <stdlib.h>
#include "draw.h"

#include "game_mediator.h"

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

#define ENERGY_BALL_IMG_SIZE 10
#define ENERGY_BALL_PERCENT_SLICE 5

#ifdef ANDROID
#include <android/log.h>
#endif

extern graphicsLib graphLib;

#include "timerlib.h"
extern timerLib timer;

extern CURRENT_FILE_FORMAT::file_game game_data;

extern std::string FILEPATH;

#include "game.h"
extern game gameControl;

#include "inputlib.h"
extern inputLib input;

#include "strings_map.h"

extern soundLib soundManager;

#define FADE_INC 2

#include "file/file_io.h"
extern CURRENT_FILE_FORMAT::file_io fio;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_game_config game_config;

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


    filename = FILEPATH + "/images/1up_icons.png";
    graphLib.surfaceFromFile(filename, &hud_player_1up);
}

void draw::show_gfx()
{
    //std::cout << "screen_gfx[" << (int)screen_gfx << "]" << std::endl;
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
    } else if (screen_gfx != SCREEN_GFX_NONE) {
        std::cout << "screen_gfx[" << (int)screen_gfx << "] UNKNOWN" << std::endl;
    }

    if (flash_effect_enabled == true || screen_gfx == SCREEN_GFX_FLASH) {
        show_flash();
    }
    show_weapon_tooltip();
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
        std::cout << "## DRAW::SHOW_RAIN::SET-EFFECT-TIMER: " << _effect_timer << std::endl;
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

    std::string graph_filename = FILEPATH + "images/sprites/enemies/" + std::string(GameMediator::get_instance()->get_enemy(boss_id)->graphic_filename);

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "DRAW::show_boss_intro_sprites, id[%d], filename[%s]", boss_id, graph_filename.c_str());
#endif


    sprite_size.x = GameMediator::get_instance()->get_enemy(boss_id)->frame_size.width;
    sprite_size.y = GameMediator::get_instance()->get_enemy(boss_id)->frame_size.height;

    //std::cout << ">>> sprite_size.x[" << sprite_size.x << "], sprite_size.y[" << sprite_size.y << "]" << std::endl;

    graphLib.surfaceFromFile(graph_filename.c_str(), &boss_graphics);
    int sprite_pos_y = BOSS_INTRO_BG_POS_Y-sprite_size.y/2;
    st_position boss_pos(RES_W/2-sprite_size.x/2, sprite_pos_y);

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
            graphLib.copyArea(st_rectangle(0, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
            graphLib.updateScreen();
            timer.delay(5);
        }
        graphLib.updateScreen();
        timer.delay(500);
    } else {
        boss_pos.y = sprite_pos_y;
    }


    // show intro sprites
    if (intro_frames_n > 1) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
            if (GameMediator::get_instance()->get_enemy(boss_id)->sprites[ANIM_TYPE_INTRO][i].used == true) {
                //std::cout << "i: " << i << ", used: " << GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].used << ", duration: " << GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].duration << std::endl;
                graphLib.showSurface(&boss_intro_bg);
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
        graphLib.draw_centered_text(dest_pos.y, strings_map::get_instance()->get_ingame_string(strings_ingame_ready_message, game_config.selected_language));
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
        std::cout << "timer[" << timer.getTimer() << "], frame.timer[" << teleport_small_frame_timer << "], frames[" << teleport_small_frame_count << "], current frame[" << teleport_small_frame << "]" << std::endl;
        if (teleport_small_frame >= teleport_small_frame_count) {
            teleport_small_frame = 0;
        }
        st_rectangle origin_rect(x_origin, 0, _teleport_small_gfx.height, _teleport_small_gfx.height);
        graphLib.showSurfaceRegionAt(&_teleport_small_gfx, origin_rect, st_position(x, y));
    } else {
        graphLib.showSurfaceAt(&_teleport_small_gfx, st_position(x, y), false);
    }
}

int draw::show_credits_text(bool can_leave, std::vector<std::string> credit_text)
{
    int line_n=0;
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


    std::cout << "draw::show_credits_text::START" << std::endl;

    // add the initial lines to screen
    create_engine_credits_text();

    timer.delay(200);
    input.clean();
    timer.delay(200);

    update_screen();

    // scroll the lines
    int limit = (credit_text.size()*12)+RES_H/2;
    std::cout << "credit_text.size[" << credit_text.size() << "]" << std::endl;
    while (scrolled < limit) {

        //std::cout << "scrolled[" << scrolled << "], limit[" << limit << "]" << std::endl;

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
                //std::cout << "text_pos[" << i << "][" << text_pos << "]" << std::endl;
            }
        }


        if (can_leave) {
            input.read_input();
            if (input.wait_scape_time(STARS_DELAY) == 1 || input.p1_input[BTN_START] == 1) {
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
    update_screen();
    return 0;
}

int draw::show_credits(bool can_leave)
{
    soundManager.stop_music();
    soundManager.load_shared_music("sprnova.it");
    soundManager.play_music();
    int res = show_credits_text(can_leave, create_engine_credits_text());
    if (res == 1) {
        soundManager.stop_music();
        soundManager.load_music(game_data.game_start_screen_music_filename);
        soundManager.play_music();
        return 1;
    }
    //graphLib.blank_screen();// should leave presented by on the screen
    graphLib.updateScreen();
    timer.delay(1000);
    if (can_leave == false) {
        graphLib.draw_centered_text(RES_H/2+16, "YOU HAVE UNLOCKED A SECRET.");
        graphLib.draw_centered_text(RES_H/2+28, "SELECT NEW GAME TO PICK");
        graphLib.draw_centered_text(RES_H/2+40, "A NEW AVAILABLE CHARACTER.");

        graphLib.draw_centered_text(RES_H/2+64, "PRESS A BUTTON TO CONTINUE.");
        graphLib.updateScreen();
    }
    input.clean();
    timer.delay(100);
    input.wait_keypress();
}

void draw::show_unlocked_charsMsg()
{

}

std::vector<string> draw::create_engine_credits_text()
{

    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<string> credits_list = fio_str.get_string_list_from_file(FILEPATH + "/game_credits.txt");

    if (credits_list.size() > 0) {
        for (int i=0; i<6; i++) {
            credits_list.push_back("");
        }
    }

    credits_list.push_back("- ROCKBOT/ENGINE CREDITS -");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");


    credits_list.push_back("- PLANNER -");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- HELPER -");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");


    credits_list.push_back("- CHARACTER DESIGNER -");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("BOBERATU");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- OBJECT DESIGNER -");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- GRAPHICS DESIGNER -");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("BOBERATU");
    credits_list.push_back("HFBN2");
    credits_list.push_back("CAPT. CHRIS AND KB");
    credits_list.push_back("SURT.OPENGAMEART");
    credits_list.push_back("AVERAGE-HANZO.DEVIANTART");
    credits_list.push_back("FUNKY96.DEVIANTART");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- DIALOGS EDITOR -");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- TEST PLAYER -");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");


    credits_list.push_back("- ILLUSTRATION DESIGNER -");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- MUSIC COMPOSER -");
    credits_list.push_back("MODARCHIVE.ORG");
    credits_list.push_back("FIREAGE");
    credits_list.push_back("SOLARIS");
    credits_list.push_back("LEON/EXCESS");
    credits_list.push_back("DIOMATIC");
    credits_list.push_back("JASON");
    credits_list.push_back("DEADLOCK/EXLSD");
    credits_list.push_back("4MAT");
    credits_list.push_back("ZANAZAC");
    credits_list.push_back("AMEGA INDUSTRIESQ");
    credits_list.push_back("DEATH ADDR");
    credits_list.push_back("TOBBX");
    credits_list.push_back("HYOCHAN");
    credits_list.push_back("DR.BULLY/MAJIC12");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- SFX DESIGNER -");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");


    credits_list.push_back("- PROGRAMMER -");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("FARLEY KNIGHT");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- PORTING HELPER -");
    credits_list.push_back("DINGUX: SHIN-NIL");
    credits_list.push_back("PANDORA: SIGMA NL");
    credits_list.push_back("PS2: RAGNAROK2040");
    credits_list.push_back("PS2: WOON-YUNG LIU");
    credits_list.push_back("PS2: SP193");
    credits_list.push_back("ANDROID: PELYA");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- REVIEW & TESTING -");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("ANDREW PRZELUCKI");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- SPECIAL THANKS -");
    credits_list.push_back("FELIPE ZACANI (FPS)");
    credits_list.push_back("FREE SDK DEVELOPERS");
    credits_list.push_back("DEVIANTART.COM");
    credits_list.push_back("PIXELJOINT.COM");
    credits_list.push_back("OPENGAMEART.ORG");
    credits_list.push_back("VENOM");
    credits_list.push_back("JERONIMO");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");

    credits_list.push_back("- DEVELOPMENT TOOLS -");
    credits_list.push_back("LIBSDL");
    credits_list.push_back("DIGIA QT");
    credits_list.push_back("GCC G+");
    credits_list.push_back("UBUNTU LINUX");
    credits_list.push_back("MINT LINUX");
    credits_list.push_back("GIMP EDITOR");
    credits_list.push_back("PAINT TOOL SAI");
    credits_list.push_back("COREL DRAW");
    credits_list.push_back("SIMPLE SCREEN RECORD");
    credits_list.push_back("AUDACIOUS PLAYER");
    credits_list.push_back("AUDACITY EDITOR");
    credits_list.push_back("BFXR.NET");

    for (int i=0; i<20; i++) {
        credits_list.push_back("");
    }
    credits_list.push_back("- PRESENTED BY -");
    credits_list.push_back("UPPERLAND STUDIOS");

    return credits_list;
}


void draw::draw_credit_line(graphicsLib_gSurface &surface, Uint8 initial_line,std::vector<std::string> credit_text)
{
    if (initial_line < credit_text.size()) {
        std::cout << "draw::draw_credit_line - text[" << credit_text.at(initial_line) << "]" << std::endl;
        std::size_t found = credit_text.at(initial_line).find("- ");
        if (found != std::string::npos) {
            graphLib.draw_centered_text(RES_H, credit_text.at(initial_line), surface, st_color(95, 151, 255));
        } else {
            graphLib.draw_centered_text(RES_H, credit_text.at(initial_line), surface, st_color(TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE, TEXT_DEFAUL_COLOR_VALUE));
        }
    } else {
        std::cout << "ERROR draw_credit_line, initial_line[" << initial_line << "], credit_text.size()[" << credit_text.size() << "]" << std::endl;
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

void draw::fade_in_screen(int r, int g, int b)
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &screen_copy);

    graphicsLib_gSurface transparent_area;
    graphLib.initSurface(st_size(RES_W, RES_H), &transparent_area);
    graphLib.clear_surface_area(0, 0, RES_W, RES_H, r, g, b, transparent_area);

    for (int i=255; i>=0; i-=FADE_INC) {
        graphLib.showSurface(&screen_copy);
        graphLib.set_surface_alpha(i, transparent_area);
        graphLib.showSurface(&transparent_area);
        graphLib.updateScreen();
        timer.delay(1);
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
        //std::cout << "alpha_value[" << alpha_value << "], i[" << i << "], step[" << step << "]" << std::endl;
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
                    //std::cout << "x[" << x << "], y[" << y << "], xx[" << (i+x) << "]" << std::endl;
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
        //std::cout << "pixelationAmount[" << pixelationAmount << "]" << std::endl;
        graphLib.copyArea(st_position(0, 0), &res_surface, &graphLib.gameScreen);
        graphLib.updateScreen();
        timer.delay(20);
    }
    std::cout << "END" << std::endl;
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
        graphLib.draw_weapon_tooltip_icon(_weapon_tooltip_n, st_position(_weapon_tooltip_pos_ref->x+adjust_x, _weapon_tooltip_pos_ref->y+adjust_y), true);
    }
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
    // lifes
    /*
    graphLib.copyArea(st_rectangle(game_save.selected_player*hud_player_1up.height, 0, hud_player_1up.height, hud_player_1up.height), st_position(6, 20), &hud_player_1up, &graphLib.gameScreen);
    char lifes_text[125];
    sprintf(lifes_text, "x0%d", game_save.items.lifes);
    graphLib.draw_text(26, 24, std::string(lifes_text));
    */


    // player HP
    int hp_percent = (100 * hp) / fio.get_heart_pieces_number(game_save);
    graphLib.draw_text(6, 10, "HP:");
    draw_enery_ball(hp_percent, 30, hud_player_hp_ball);
    /*
    for (int i=0; i<5; i++) {
        int min = 20*i;
        int max2 = min+20;
        //std::cout << "i[" << i << "], hp_percent[" << hp_percent << "], min[" << min << "], max1[" << max1 << "], max2[" << max2 << "]" << std::endl;

        int img_origin_x = 0;
        if (hp_percent < min) {
            img_origin_x = hud_player_hp_ball.height*2;
        } else if (hp_percent < max2) {
            img_origin_x = hud_player_hp_ball.height;
        }

        graphLib.copyArea(st_rectangle(img_origin_x, 0, hud_player_hp_ball.height, hud_player_hp_ball.height), st_position(30+10*i, 9), &hud_player_hp_ball, &graphLib.gameScreen);
    }
    */


    if (selected_weapon != WEAPON_DEFAULT) {
        // draw weapon

        hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN, GameMediator::get_instance()->player_list_v3_1[PLAYER_1].weapon_colors[selected_weapon].color1);
        int wpn_percent = (100 * selected_weapon_value) / fio.get_heart_pieces_number(game_save);
        //std::cout << "selected_weapon_value[" << selected_weapon_value << "]" << std::endl;
        graphLib.draw_text(90, 10, "WPN:");
        draw_enery_ball(wpn_percent, 122, hud_player_wpn_ball);
    }

    // boss HP

    if (gameControl.must_show_boss_hp() && _boss_current_hp != -99) {
        int boss_hp_percent = (100 * _boss_current_hp) / BOSS_INITIAL_HP;
        graphLib.draw_text(RES_W-95, 10, "BOSS:");
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
        //std::cout << "i[" << i << "], hp_percent[" << hp_percent << "], min[" << min << "], max1[" << max1 << "], max2[" << max2 << "]" << std::endl;

        int img_origin_x = ENERGY_BALL_IMG_SIZE*4;

        //std::cout << "value[" << value << "], min1[" << min1 << "], min2[" << min2 << "], min3[" << min3 << "], min4[" << min4 << "]" << std::endl;

        if (value >= min4) {
            img_origin_x = 0;
        } else if (value >= min3) {
            img_origin_x = ENERGY_BALL_IMG_SIZE;
        } else if (value >= min2) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*2;
        } else if (value >= min1) {
            img_origin_x = ENERGY_BALL_IMG_SIZE*3;
        }

        graphLib.copyArea(st_rectangle(img_origin_x, 0, ball_surface.height, ball_surface.height), st_position(x_pos+(10*i), 9), &ball_surface, &graphLib.gameScreen);
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


        std::cout << "DRAW::add_dynamic_background::ADD[" << filename << "]" << std::endl;

        maps_dynamic_background_list.insert(std::pair<std::string,graphicsLib_gSurface>(filename, graphicsLib_gSurface()));
        std::string bg1_filename(FILEPATH+"images/map_backgrounds/" + filename);


        graphicsLib_gSurface temp_surface;
        graphLib.surfaceFromFile(bg1_filename, &temp_surface);

        std::cout << "MAP::add_bg[" << bg1_filename << "], w[" << (int)temp_surface.width << "], h[" << (int)temp_surface.height << "]" << std::endl;


        graphLib.initSurface(st_size(temp_surface.width, temp_surface.height), &maps_dynamic_background_list.find(filename)->second);
        graphLib.clear_surface_area(0, 0, temp_surface.width, temp_surface.height, bg_color.r, bg_color.g, bg_color.b, maps_dynamic_background_list.find(filename)->second);
        graphLib.copyArea(st_position(0, 0), &temp_surface, &maps_dynamic_background_list.find(filename)->second);
        //maps_dynamic_background_list.find(filename)->second
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


    //std::cout << "## DRAW::SHOW_SNOW - timer.getTimer(): " << timer.getTimer() << ", _effect_timer: " << _effect_timer << std::endl;

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
            //graphLib.clear_area(temp_particle->position.x, temp_particle->position.y, 3, 3, 230, 230, 255);
            graphLib.showSurfaceRegionAt(&snow_flacke, st_rectangle(0, 0, snow_flacke.width, snow_flacke.height), st_position(temp_particle->position.x, temp_particle->position.y));
        }
        _effect_timer = timer.getTimer() + SNOW_DELAY;

        //std::cout << "## DRAW::SHOW_SNOW::SET-EFFECT-TIMER: " << _effect_timer << ", current timer: " << timer.getTimer() << std::endl;

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
        std::cout << "TRAIN_EFFECT-RESET" << std::endl;
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
        //std::cout << "shadow.y[" << i << "], alpha[" << alpha << "]" << std::endl;
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



