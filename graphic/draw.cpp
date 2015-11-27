#include <stdio.h>
#include <stdlib.h>
#include "draw.h"

#include "game_mediator.h"

#define RAIN_DELAY 160
#define FLASH_DELAY 260
#define FLASH_IMG_SIZE 8
#define SNOW_DELAY 40

extern graphicsLib graphLib;

#include "timerlib.h"
extern timerLib timer;

extern CURRENT_FILE_FORMAT::file_game game_data;

extern std::string FILEPATH;

#include "game.h"
extern game gameControl;

#include "inputlib.h"
extern inputLib input;


draw::draw() : _rain_pos(0), _effect_timer(0), _flash_pos(0), _flash_timer(0), screen_gfx(SCREEN_GFX_NONE), flash_effect_enabled(false)
{
    for (int i=0; i<FLASH_POINTS_N; i++) {
        flash_points[i].x = rand() % RES_W;
        flash_points[i].y = rand() % RES_H;
    }

}

void draw::preload()
{
    std::string filename = FILEPATH + "images/animations/ready.png";
    graphLib.surfaceFromFile(filename, &ready_message);

    filename = FILEPATH + "images/sprites/teleport_small.png";
    graphLib.surfaceFromFile(filename, &_teleport_small_gfx);

    // DROPABLE OBJECT GRAPHICS
    for (int i=0; i<GameMediator::get_instance()->object_list.size(); i++) {
        for (int j=0; j<DROP_ITEM_COUNT; j++) {
            short obj_type_n = gameControl.get_drop_item_id(j);
            if (obj_type_n != -1) {
                get_object_graphic(obj_type_n);
            }
        }
    }
}

void draw::show_gfx()
{
    if (screen_gfx == SCREEN_GFX_RAIN) {
        show_rain();
    } else if (screen_gfx == SCREEN_GFX_SNOW) {
        show_snow_effect();
    }
    if (flash_effect_enabled == true || screen_gfx == SCREEN_GFX_FLASH) {
        show_flash();
    }
}

void draw::update_screen()
{
    graphLib.updateScreen();
}

void draw::set_gfx(Uint8 gfx)
{
    screen_gfx = gfx;
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
    if (rain_obj.get_surface() == NULL) {
        // load rain
        std::string filename = FILEPATH + "/images/tilesets/rain.png";
        graphLib.surfaceFromFile(filename, &rain_obj);
    }
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
        std::string filename = FILEPATH + "/images/tilesets/flash.png";
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

void draw::show_boss_intro_sprites(short boss_id, bool show_fall)
{
    UNUSED(show_fall);
    unsigned int intro_frames_n = 0;
    //int intro_frames_rollback = 0;
    st_position boss_pos(20, -37);
    st_position sprite_size;
    graphicsLib_gSurface bgCopy, boss_graphics;

    std::string graph_filename = FILEPATH + "images/sprites/enemies/" + std::string(GameMediator::get_instance()->get_enemy(boss_id).graphic_filename);
    sprite_size.x = GameMediator::get_instance()->get_enemy(boss_id).frame_size.width;
    sprite_size.y = GameMediator::get_instance()->get_enemy(boss_id).frame_size.height;
    graphLib.surfaceFromFile(graph_filename.c_str(), &boss_graphics);

    graphLib.initSurface(st_size(RES_W, RES_H), &bgCopy);
    graph_filename = FILEPATH + "images/backgrounds/stage_boss_intro.png";
    graphLib.surfaceFromFile(graph_filename.c_str(), &bgCopy);
    st_position bg_pos(0, (RES_H/2)-(bgCopy.height/2));
    graphLib.copyArea(bg_pos, &bgCopy, &graphLib.gameScreen);

    update_screen();

    int sprite_pos_y = RES_H/2 - sprite_size.y/2;

    for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
        if (GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].used == true) {
            intro_frames_n++;
        }
    }

    // fall into position
    while (boss_pos.y < sprite_pos_y) {
        boss_pos.y += 4;
        graphLib.copyArea(bg_pos, &bgCopy, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(0, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
        graphLib.wait_and_update_screen(5);
    }
    graphLib.wait_and_update_screen(500);


    // show intro sprites
    if (intro_frames_n > 1) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) {
            if (GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].used == true) {
                //std::cout << "i: " << i << ", used: " << GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].used << ", duration: " << GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].duration << std::endl;
                graphLib.copyArea(bg_pos, &bgCopy, &graphLib.gameScreen);
                graphLib.copyArea(st_rectangle(sprite_size.x * GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].sprite_graphic_pos_x, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
                graphLib.wait_and_update_screen(GameMediator::get_instance()->get_enemy(boss_id).sprites[ANIM_TYPE_INTRO][i].duration);
            }
        }
    } else { // just frow first sprite
        graphLib.copyArea(bg_pos, &bgCopy, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(0, 0, sprite_size.x, sprite_size.y), st_position(boss_pos.x, boss_pos.y), &boss_graphics, &graphLib.gameScreen);
        graphLib.wait_and_update_screen(200);
    }
}

void draw::show_ready()
{
    st_position dest_pos((RES_W/2)-26, (RES_H/2)-6);
    graphLib.copyArea(dest_pos, &ready_message, &graphLib.gameScreen);
    graphLib.updateScreen();
}

void draw::show_bubble(int x, int y)
{
    if (_bubble_gfx.get_surface() == NULL) {
        std::string filename = FILEPATH + "images/tilesets/bubble.png";
        graphLib.surfaceFromFile(filename, &_bubble_gfx);
    }
    graphLib.showSurfaceAt(&_bubble_gfx, st_position(x, y), false);
}

void draw::set_teleport_small_colors(st_color color1, st_color color2)
{
    /// @TODO new colormap
}

void draw::show_teleport_small(int x, int y)
{
    graphLib.showSurfaceAt(&_teleport_small_gfx, st_position(x+_teleport_small_gfx.width/2, y+_teleport_small_gfx.height/2), false);
}

void draw::show_credits()
{
    int line_n=0;
    unsigned int scrolled=0;
    int posY = -RES_H;
    st_rectangle dest;
    graphicsLib_gSurface credits_surface;

    graphLib.initSurface(st_size(RES_W, RES_H+12), &credits_surface);
    graphLib.blank_surface(credits_surface);
    graphLib.blank_screen();


    // add the initial lines to screen
    create_credits_text(credits_surface);

    update_screen();

    // scroll the lines
    while (scrolled < (credits_list.size()*12)+RES_H/2+46) {
        graphLib.copyArea(st_rectangle(0, posY, RES_W, RES_H), st_position(0, 0), &credits_surface, &graphLib.gameScreen);
        update_screen();
        timer.delay(60);
        posY++;
        scrolled++;
        if (posY > 12) {
            graphLib.copyArea(st_rectangle(0, posY, credits_surface.width, RES_H), st_position(0, 0), &credits_surface, &credits_surface);
            // scroll the lines
            dest.x = 0;
            dest.y = RES_H;
            dest.w = RES_W;
            dest.h = 12;
            graphLib.blank_area(dest.x, dest.y, dest.w, dest.h, credits_surface);
            draw_credit_line(credits_surface, line_n+21);
            posY = 0;
            line_n++;
        }
    }
    update_screen();
}

void draw::create_credits_text(graphicsLib_gSurface &surface)
{
    if (credits_list.size() > 0) {
        return;
    }
    credits_list.push_back("*** ROCKBOT CREDITS ****");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");


    credits_list.push_back("--- PLANNER ---");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("");


    credits_list.push_back("--- CHARACTER DESIGN ---");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("FABIANO \"CHIKAGO\" SACCOL");
    credits_list.push_back("GIULIANO \"KOBOLD\" SACCOL");
    credits_list.push_back("IVAN FIEDORUK");
    credits_list.push_back("");

    credits_list.push_back("--- PIXEL GRAPHICS ---");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("GIOVANI \"AKIRA\" FAGANELLO");
    credits_list.push_back("RODRIGO M. HAHN");
    credits_list.push_back("HUNTER TRAMMELL");
    credits_list.push_back("BOBERATU");
    credits_list.push_back("HFBN2");
    credits_list.push_back("CAPT. CHRIS & KB");
    credits_list.push_back("--- OPENGAMEART.ORG ---");
    credits_list.push_back("SURT");
    credits_list.push_back("--- DEVIANTART.COM ---");
    credits_list.push_back("AVERAGE-HANZO");
    credits_list.push_back("");

    credits_list.push_back("--- TEXT REVIEW ---");
    credits_list.push_back("NELSON ROSENBERG");
    credits_list.push_back("");


    credits_list.push_back("--- ILLUSTRATION ---");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("");

    credits_list.push_back("--- MUSIC COMPOSE ---");
    credits_list.push_back("MODARCHIVE.ORG");
    credits_list.push_back("");

    credits_list.push_back("--- SOUND EFFECTS ---");
    credits_list.push_back("CAPCOM");
    credits_list.push_back("");


    credits_list.push_back("--- PROGRAMMER ---");
    credits_list.push_back("IURI FIEDORUK");
    credits_list.push_back("DEMETRIO NETO");
    credits_list.push_back("FELIPE ZACANI");
    credits_list.push_back("");

    credits_list.push_back("--- PORTING ---");
    credits_list.push_back("DINGUX: SHIN-NIL");
    credits_list.push_back("PANDORA: SIGMA NL");
    credits_list.push_back("PS2: RAGNAROK2040");
    credits_list.push_back("PS2: WOON-YUNG LIU");
    credits_list.push_back("PS2: SP193");
    credits_list.push_back("ANDROID: PELYA");
    credits_list.push_back("");

    credits_list.push_back("--- REVIEW & TESTING ---");
    credits_list.push_back("ARISMEIRE KUMMER SILVA FIEDORUK");
    credits_list.push_back("LUIS AGUIRRE");
    credits_list.push_back("BENOITREN (PSP)");
    credits_list.push_back("BATANEN (PSP)");
    credits_list.push_back("SAIYAN X (PSP)");
    credits_list.push_back("AGENT 13 (PS2)");
    credits_list.push_back("MK2ESCORT (PS2)");
    credits_list.push_back("FOUADTJUHMASTER (ANDROID)");
    credits_list.push_back("SHINRYUU82");
    credits_list.push_back("");

    credits_list.push_back("--- SPECIAL THANKS ---");
    credits_list.push_back("FREE SDK DEVELOPERS");
    credits_list.push_back("DEVIANTART.COM COMMUNITY");
    credits_list.push_back("PIXELJOINT.COM COMMUNITY");
    credits_list.push_back("OPENGAMEART.ORG COMMUNITY");
    credits_list.push_back("VENOM");
    credits_list.push_back("JERONIMO");
    credits_list.push_back("");

    credits_list.push_back("--- DEVELOPMENT TOOLS ---");
    credits_list.push_back("LIBSDL.ORG");
    credits_list.push_back("DIGIA QT");
    credits_list.push_back("UBUNTU LINUX");
    credits_list.push_back("MINT LINUX");
    credits_list.push_back("GIMP EDITOR");
    credits_list.push_back("PAINT TOOL SAI");
    credits_list.push_back("COREL DRAW");
    credits_list.push_back("SIMPLE SCREEN RECORD");
    credits_list.push_back("AUDACIOUS PLAYER");
    credits_list.push_back("AUDACITY EDITOR");
    credits_list.push_back("BFXR.NET");

    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("");
    credits_list.push_back("--- PRESENTED BY ---");
    credits_list.push_back("UPPERLAND STUDIOS");
    credits_list.push_back("GREENNEKO WEB & DESIGN");

    for (unsigned int i=0; i<=RES_H/12 && i<credits_list.size(); i++) {
        std::size_t found = credits_list.at(i).find("--- ");
        if (found != std::string::npos) {
            graphLib.draw_centered_text(12*i, credits_list.at(i), surface, st_color(95, 151, 255));
        } else {
            graphLib.draw_centered_text(12*i, credits_list.at(i), surface, st_color(235, 235, 235));
        }
    }
}


void draw::draw_credit_line(graphicsLib_gSurface &surface, Uint8 initial_line)
{
    if (initial_line < credits_list.size()) {
        std::size_t found = credits_list.at(initial_line).find("--- ");
        if (found != std::string::npos) {
            graphLib.draw_centered_text(RES_H, credits_list.at(initial_line), surface, st_color(95, 151, 255));
        } else {
            graphLib.draw_centered_text(RES_H, credits_list.at(initial_line), surface, st_color(235, 235, 235));
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
    graphLib.show_dialog(0, true);
    st_position dialog_pos = graphLib.get_dialog_pos();
    for (unsigned int i=0; i<message.size(); i++) {
        graphLib.draw_text(dialog_pos.x+20, dialog_pos.y+16+(12*i), message[i]);
    }
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

    for (int i=255; i>=0; i--) {
        graphLib.showSurface(&screen_copy);
        graphLib.set_surface_alpha(i, transparent_area);
        graphLib.showSurface(&transparent_area);
        graphLib.updateScreen();
        timer.delay(1);
    }
}

void draw::fade_out_screen(int r, int g, int b)
{
    graphicsLib_gSurface screen_copy;
    graphLib.initSurface(st_size(RES_W, RES_H), &screen_copy);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &screen_copy);

    graphicsLib_gSurface transparent_area;
    graphLib.initSurface(st_size(RES_W, RES_H), &transparent_area);
    graphLib.clear_surface_area(0, 0, RES_W, RES_H, r, g, b, transparent_area);

    for (int i=0; i<=255; i++) {
        graphLib.showSurface(&screen_copy);
        graphLib.set_surface_alpha(i, transparent_area);
        graphLib.showSurface(&transparent_area);
        graphLib.updateScreen();
        timer.delay(1);
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


    //std::cout << "timer.getTimer(): " << timer.getTimer() << ", _effect_timer: " << _effect_timer << std::endl;
    if (timer.getTimer() > _effect_timer) {
        for (it=_snow_particles.begin(); it!=_snow_particles.end(); it++) {
            st_snow_particle *temp_particle = &(*it);
            //std::cout << "SNOW.MOVE" << std::endl;
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
            graphLib.clear_area(temp_particle->position.x, temp_particle->position.y, 3, 3, 230, 230, 255);
        }
        _effect_timer = timer.getTimer() + SNOW_DELAY;
    } else {
        for (it=_snow_particles.begin(); it!=_snow_particles.end(); it++) {
            st_snow_particle *temp_particle = &(*it);
            graphLib.clear_area(temp_particle->position.x, temp_particle->position.y, 3, 3, 230, 230, 255);
        }
    }
}


