#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

using namespace std;

#include "classmap.h"
#include "objects/object.h"
#include "character/character.h"
#include "collision_detection.h"
#include "file/file_io.h"
#include "game_mediator.h"
#include "aux_tools/exception_manager.h"

extern string FILEPATH;
extern graphicsLib graphLib;

#include "game.h"
extern game gameControl;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#ifdef ANDROID
#include <android/log.h>
#endif

#include "file/file_io.h"
extern CURRENT_FILE_FORMAT::file_io fio;



extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap::classMap() : stage_number(-1), number(-1), bg_scroll(st_float_position(0.0, 0.0)), fg_layer_scroll(st_float_position(0.0, 0.0)), _platform_leave_counter(0)
{

    for (int i=0; i<MAP_W; i++) {
		wall_scroll_lock[i] = false;
	}
    _water_bubble.pos.x = -1;
    _water_bubble.pos.y = -1;
    _water_bubble.x_adjust = 0;
    _water_bubble.timer = 0;
    _water_bubble.x_adjust_direction = ANIM_DIRECTION_LEFT;
    _3rd_level_ignore_area = st_rectangle(-1, -1, -1, -1);
    _level3_tiles = std::vector<struct st_level3_tile>();
    _show_map_pos_x = -1;

    graphLib.initSurface(st_size(RES_W+(TILESIZE*2), RES_H), &map_screen);
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap::~classMap()
{
}


void classMap::reset_map()
{
    // reset objects
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        // if object is a player item, remove it
        if (temp_obj.get_id() == game_data.player_items[0] || temp_obj.get_id() == game_data.player_items[1]) {
            temp_obj.set_finished(true);
        } else {
            temp_obj.reset();
        }
    }
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::setStageNumber(int setStageN) {
	stage_number = setStageN;
}


short classMap::get_number() const
{
	return number;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::setMapNumber(int setMapN) {
	number = setMapN;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::loadMap()
{

	if (stage_number == -1) {
        graphLib.show_debug_msg("ERROR::loadStage invalid number[-1]");
                cout << "ERROR::map::loadMap - stage number was not set, can't load it before setting the number.\n";
		return;
	}
	if (number == -1) {
        graphLib.show_debug_msg("ERROR::loadStage invalid number[>MAX]");
                cout << "ERROR::map::loadMap - map number was not set, can't load it before setting the number.\n";
		return;
    }


    object_list.clear();

    _npc_list.clear();
    animation_list.clear();





    _level3_tiles.clear();

    for (int i=0; i<MAP_W; i++) {
        for (int j=0; j<MAP_H; j++) {
            int lvl3_x = GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.x;
            int lvl3_y = GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.y;
            if (lvl3_x != -1 && lvl3_y != -1) {
                struct st_level3_tile temp_tile(st_position(lvl3_x, lvl3_y), st_position(i, j));
                _level3_tiles.push_back(temp_tile);
            }
        }
    }



	load_map_npcs();

    load_map_objects();

    bool column_locked = true;
    for (int i=0; i<MAP_W; i++) {
        column_locked = true;
        for (int j=0; j<MAP_H; j++) {
            // check if a object-door ocuppies the position
            std::vector<object>::iterator object_it;
            bool obj_locked = false;
            for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
                object *obj_item = &(*object_it);

                if (obj_item->get_type() == OBJ_BOSS_DOOR) {
                    if ((i >= obj_item->get_start_position().x/TILESIZE && i < obj_item->get_start_position().x/TILESIZE+obj_item->get_size().width/TILESIZE) && (j >= obj_item->get_start_position().y/TILESIZE && j < obj_item->get_start_position().y/TILESIZE+obj_item->get_size().height/TILESIZE)) {
                        obj_locked = true;
                    }
                }
            }
            if (obj_locked != true && GameMediator::get_instance()->map_data[number].tiles[i][j].locked != TERRAIN_SOLID && GameMediator::get_instance()->map_data[number].tiles[i][j].locked != TERRAIN_DOOR && GameMediator::get_instance()->map_data[number].tiles[i][j].locked != TERRAIN_SCROLL_LOCK && GameMediator::get_instance()->map_data[number].tiles[i][j].locked != TERRAIN_ICE && GameMediator::get_instance()->map_data[number].tiles[i][j].locked != TERRAIN_SPIKE) {
                column_locked = false;
                break;
            }
        }
        wall_scroll_lock[i] = column_locked;
    }

    create_dynamic_background_surfaces();

    init_animated_tiles();


#ifdef HANDLELD // portable consoles aren't strong enought for two dynamic backgrounds
GameMediator::get_instance()->map_data[number].backgrounds[0].speed = 0;
#endif

}


void classMap::show_map()
{
    draw_dynamic_backgrounds();
    if (get_map_gfx_mode() == SCREEN_GFX_MODE_BACKGROUND) {
        draw_lib.show_gfx();
    }
    // TODO: optimization, build a list of visible enemies and one for ghost and other for non-ghost,
    // and use those instead in the show-loop
    show_ghost_npcs();

    // redraw screen, if needed
    if (_show_map_pos_x == -1 || abs(_show_map_pos_x - scroll.x) > TILESIZE) {
        draw_map_tiles();
    // use memory screen
    }
    int diff_scroll_x = scroll.x - _show_map_pos_x;
    graphLib.copyArea(st_rectangle(diff_scroll_x+TILESIZE, 0, RES_W, RES_H), st_position(0, 0), &map_screen, &graphLib.gameScreen);

    // draw animated tiles
    draw_animated_tiles(graphLib.gameScreen);

    if (get_map_gfx_mode() == SCREEN_GFX_MODE_FULLMAP) {
        draw_lib.show_gfx();
    }

    show_objects();
    show_npcs();

}

void classMap::get_map_area_surface(graphicsLib_gSurface& mapSurface)
{
    graphLib.initSurface(st_size(RES_W, RES_H), &mapSurface);

    if (!mapSurface.get_surface()) {
        graphLib.show_debug_msg("EXIT #21.MALLOC");
        exception_manager::throw_general_exception(std::string("classMap::get_map_area_surface"), "Could not init map surface");
    }

    graphLib.clear_surface_area(0, 0, RES_W, RES_H, GameMediator::get_instance()->map_data[number].background_color.r, GameMediator::get_instance()->map_data[number].background_color.g, GameMediator::get_instance()->map_data[number].background_color.b, mapSurface);

    draw_dynamic_backgrounds_into_surface(mapSurface);

    // redraw screen, if needed
    if (_show_map_pos_x == -1 || abs(_show_map_pos_x - scroll.x) > TILESIZE) {
        draw_map_tiles();
    // use memory screen
    }
    int diff_scroll_x = scroll.x - _show_map_pos_x;
    graphLib.copyArea(st_rectangle(diff_scroll_x+TILESIZE, 0, RES_W, RES_H), st_position(0, 0), &map_screen, &mapSurface);

    // draw animated tiles
    draw_animated_tiles(mapSurface);
}

void classMap::draw_map_tiles()
{

    _show_map_pos_x = scroll.x;

    int tile_x_ini = scroll.x/TILESIZE-1;
    if (tile_x_ini < 0) {
        tile_x_ini = 0;
    }


    graphLib.clear_surface(map_screen);

    // draw the tiles of the screen region
    struct st_position pos_origin;
    struct st_position pos_destiny;
    int n = -1;
    for (int i=tile_x_ini; i<tile_x_ini+(RES_W/TILESIZE)+3; i++) {
        int diff = scroll.x - (tile_x_ini+1)*TILESIZE;
        pos_destiny.x = n*TILESIZE - diff + TILESIZE;
        for (int j=0; j<MAP_H; j++) {

            // don't draw easy-mode blocks if game difficulty not set to easy

            if (GameMediator::get_instance()->map_data[number].tiles[i][j].locked == TERRAIN_EASYMODEBLOCK && game_save.difficulty == DIFFICULTY_EASY) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_easymode_block_tile(pos_destiny, map_screen);
            } else if (GameMediator::get_instance()->map_data[number].tiles[i][j].locked == TERRAIN_HARDMODEBLOCK && game_save.difficulty == DIFFICULTY_HARD) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_hardmode_block_tile(pos_destiny, map_screen);
            } else {
                pos_origin.x = GameMediator::get_instance()->map_data[number].tiles[i][j].tile1.x;
                pos_origin.y = GameMediator::get_instance()->map_data[number].tiles[i][j].tile1.y;

                if (pos_origin.x >= 0 && pos_origin.y >= 0) {
                    pos_destiny.y = j*TILESIZE;
                    graphLib.placeTile(pos_origin, pos_destiny, &map_screen);
                }
            }
        }
        n++;
    }
}

void classMap::draw_animated_tiles(graphicsLib_gSurface &surface)
{
    //scroll.x - dest.x
    for (int i=0; i<anim_tile_list.size(); i++) {

        int pos_x = anim_tile_list.at(i).dest_x-scroll.x;
        if (pos_x >= -TILESIZE && pos_x <= RES_W+1) {
            st_position dest_pos(pos_x, anim_tile_list.at(i).dest_y);
            graphLib.place_anim_tile(anim_tile_list.at(i).anim_tile_id, dest_pos, &surface);
        }
    }

    graphLib.update_anim_tiles_timers();
}

void classMap::init_animated_tiles()
{
    // draw the tiles of the screen region
    struct st_position pos_origin;
    struct st_position pos_destiny;
    for (int i=0; i<MAP_W; i++) {
        pos_destiny.x = i*TILESIZE;
        for (int j=0; j<MAP_H; j++) {
            pos_origin.x = GameMediator::get_instance()->map_data[number].tiles[i][j].tile1.x;
            pos_origin.y = GameMediator::get_instance()->map_data[number].tiles[i][j].tile1.y;

            if (pos_origin.x < -1 && pos_origin.y == 0) {
                int anim_tile_id = (pos_origin.x * -1) - 2;
                pos_destiny.y = j*TILESIZE;
                anim_tile_list.push_back(anim_tile_desc(anim_tile_id, pos_destiny));
            }
        }
    }
}


// ********************************************************************************************** //
// show the third level of tiles                                                                  //
// ********************************************************************************************** //
void classMap::showAbove(int scroll_y, int temp_scroll_x, bool show_fg)
{
    int scroll_x = scroll.x;
    if (temp_scroll_x != -99999) {
        scroll_x = temp_scroll_x;
    }
	// only show pieces that in current screen position
    short start_point = scroll_x/TILESIZE;
	if (start_point > 0) { start_point--; }
    short end_point = (scroll_x+RES_W)/TILESIZE;
	if (end_point < MAP_W-1) { end_point++; }


	// draw 3rd tile level
    std::vector<st_level3_tile>::iterator tile3_it;
    for (tile3_it = _level3_tiles.begin(); tile3_it != _level3_tiles.end(); tile3_it++) {

        if (_3rd_level_ignore_area.x != -1 && _3rd_level_ignore_area.w > 0 && ((*tile3_it).map_position.x >= _3rd_level_ignore_area.x && (*tile3_it).map_position.x < _3rd_level_ignore_area.x+_3rd_level_ignore_area.w && (*tile3_it).map_position.y >= _3rd_level_ignore_area.y && (*tile3_it).map_position.y < _3rd_level_ignore_area.y+_3rd_level_ignore_area.h)) {
            continue;
        }
        int pos_x = (*tile3_it).tileset_pos.x;
        int pos_y = (*tile3_it).tileset_pos.y;
        // only show tile if it is on the screen range

        graphLib.place_3rd_level_tile(pos_x, pos_y, ((*tile3_it).map_position.x*TILESIZE)-scroll_x, ((*tile3_it).map_position.y*TILESIZE)+scroll_y);
    }

    if (_water_bubble.pos.x != -1) {
        draw_lib.show_bubble(_water_bubble.pos.x+_water_bubble.x_adjust, _water_bubble.pos.y);
        int water_lock = getMapPointLock(st_position((_water_bubble.pos.x+2+scroll_x)/TILESIZE, _water_bubble.pos.y/TILESIZE));
        _water_bubble.pos.y -= 2;
        if (_water_bubble.x_adjust_direction == ANIM_DIRECTION_LEFT) {
            _water_bubble.x_adjust -= 0.5;
            if (_water_bubble.x_adjust < -4) {
                _water_bubble.x_adjust_direction = ANIM_DIRECTION_RIGHT;
            }
        } else {
            _water_bubble.x_adjust += 0.5;
            if (_water_bubble.x_adjust >= 0) {
                _water_bubble.x_adjust_direction = ANIM_DIRECTION_LEFT;
            }
        }
        if (water_lock != TERRAIN_WATER || _water_bubble.timer < timer.getTimer()) {
            _water_bubble.pos.x = -1;
            _water_bubble.pos.y = -1;
        }
    }

    // animations
    /// @TODO: remove "finished" animations
    std::vector<animation>::iterator animation_it;
    for (animation_it = animation_list.begin(); animation_it != animation_list.end(); animation_it++) {
        if ((*animation_it).finished() == true) {
            animation_list.erase(animation_it);
            break;
        } else {
            (*animation_it).execute(); // TODO: must pass scroll map to npcs somwhow...
        }
    }

    if (show_fg) {
        draw_foreground_layer(scroll_x, scroll_y);
    }


}

bool classMap::is_point_solid(st_position pos) const
{
	short int lock_p = getMapPointLock(pos);

    if (lock_p == TERRAIN_UNBLOCKED || lock_p != TERRAIN_WATER || lock_p == TERRAIN_CHECKPOINT || lock_p == TERRAIN_SCROLL_LOCK || (lock_p == TERRAIN_EASYMODEBLOCK && game_save.difficulty != 0)) {
        return false;
	}
    return true;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
int classMap::getMapPointLock(st_position pos) const
{
    if (pos.x < 0 || pos.y < 0 || pos.y >= MAP_H || pos.x >= MAP_W) {
		return TERRAIN_UNBLOCKED;
	}
    return GameMediator::get_instance()->map_data[number].tiles[pos.x][pos.y].locked;
}

st_position_int8 classMap::get_map_point_tile1(st_position pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.y > RES_H/TILESIZE || pos.x > MAP_W) {
        return st_position_int8(-1, -1);
    }
    return GameMediator::get_instance()->map_data[number].tiles[pos.x][pos.y].tile1;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::changeScrolling(st_float_position pos, bool check_lock)
{
    float bg1_speed = (float)GameMediator::get_instance()->map_data[number].backgrounds[0].speed/10 * SharedData::get_instance()->get_movement_multiplier();
    float foreground_layer_speed = (float)GameMediator::get_instance()->map_data[number].backgrounds[1].speed/10 * SharedData::get_instance()->get_movement_multiplier();

	// moving player to right, screen to left
    if (pos.x > 0 && ((scroll.x/TILESIZE+RES_W/TILESIZE)-1 < MAP_W-1)) {
        int x_change = pos.x;
		if (pos.x >= TILESIZE) { // if change is too big, do not update (TODO: must check all wall until lock)
            x_change = 1;
		}
		int tile_x = (scroll.x+RES_W-TILESIZE+2)/TILESIZE;
		if (check_lock == false || wall_scroll_lock[tile_x] == false) {
            scroll.x += x_change;
            if (GameMediator::get_instance()->map_data[number].backgrounds[0].auto_scroll == BG_SCROLL_MODE_NONE) {
                bg_scroll.x -= ((float)x_change*bg1_speed);
            }
            fg_layer_scroll.x -= ((float)x_change*foreground_layer_speed);
            adjust_dynamic_background_position();
            adjust_foreground_position();
		}
	} else if (pos.x < 0) {
        int x_change = pos.x;
		if (pos.x < -TILESIZE) {
            x_change = -1;
		}
		if (scroll.x/TILESIZE >= 0) { // if change is too big, do not update (TODO: must check all wall until lock)
			int tile_x = (scroll.x+TILESIZE-2)/TILESIZE;
			if (check_lock == false || wall_scroll_lock[tile_x] == false) {
                scroll.x += x_change;
                bg_scroll.x -= ((float)x_change*bg1_speed);
                fg_layer_scroll.x -= ((float)x_change*foreground_layer_speed);
                adjust_dynamic_background_position();
                adjust_foreground_position();
            }
		}
	}

	scroll.y += pos.y;
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::set_scrolling(st_float_position pos)
{
	scrolled = pos;
	scroll.x = pos.x;
	scroll.y = pos.y;
}

void classMap::reset_scrolling()
{
    scrolled = st_position(0, 0);
    scroll.x = 0;
    scroll.y = 0;
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_float_position classMap::getMapScrolling() const
{
    return scroll;
}

st_float_position *classMap::get_map_scrolling_ref()
{
    return &scroll;
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::load_map_npcs()
{

    // remove all elements currently in the list
    if (_npc_list.size() > 0) {
        _npc_list.back().clean_character_graphics_list();
    }
    while (!_npc_list.empty()) {
        _npc_list.pop_back();
    }


    for (int i=0; i<GameMediator::get_instance()->map_npc_data.size(); i++) {
        if (GameMediator::get_instance()->map_npc_data[i].difficulty_mode == DIFFICULTY_MODE_GREATER && GameMediator::get_instance()->map_npc_data[i].difficulty_level > game_save.difficulty) {
            continue;
        } else if (GameMediator::get_instance()->map_npc_data[i].difficulty_mode == DIFFICULTY_MODE_EQUAL && GameMediator::get_instance()->map_npc_data[i].difficulty_level != game_save.difficulty) {
            continue;
        }

        int npc_ic = GameMediator::get_instance()->map_npc_data[i].id_npc;

        if (npc_ic != -1 && GameMediator::get_instance()->map_npc_data[i].stage_id == stage_number && GameMediator::get_instance()->map_npc_data[i].map_id == number) {
            classnpc new_npc = classnpc(stage_number, number, npc_ic, i);


            if (stage_data.boss.id_npc == npc_ic) {
                new_npc.set_stage_boss(true);
            } else if (GameMediator::get_instance()->get_enemy(npc_ic)->is_boss == true) {
                new_npc.set_is_boss(true);
            // adjust NPC position to ground, if needed
            } else if (new_npc.is_able_to_fly() == false && new_npc.hit_ground() == false) {
                new_npc.initialize_boss_position_to_ground();
            }
            new_npc.init_animation();

            _npc_list.push_back(new_npc); // insert new npc at the list-end
        }


	}
}


void classMap::draw_dynamic_backgrounds()
{
    // only draw solid background color, if map-heigth is less than RES_H
    graphicsLib_gSurface* surface_bg = get_dynamic_bg();
    if (surface_bg == NULL || surface_bg->width <= 0) {
        graphLib.clear_surface_area(0, 0, RES_W, RES_H, GameMediator::get_instance()->map_data[number].background_color.r, GameMediator::get_instance()->map_data[number].background_color.g, GameMediator::get_instance()->map_data[number].background_color.b, graphLib.gameScreen);
        draw_static_background();
        return;
    }
    // if there is no background or it does not cover the whole screen, draw solid color
    if (surface_bg->width <= 0 || surface_bg->height < RES_H || GameMediator::get_instance()->map_data[number].backgrounds[0].adjust_y != 0) {
        graphLib.clear_surface_area(0, 0, RES_W, RES_H, GameMediator::get_instance()->map_data[number].background_color.r, GameMediator::get_instance()->map_data[number].background_color.g, GameMediator::get_instance()->map_data[number].background_color.b, graphLib.gameScreen);
    }

    float bg1_speed = (float)GameMediator::get_instance()->map_data[number].backgrounds[0].speed/10 * SharedData::get_instance()->get_movement_multiplier();
    int bg1_scroll_mode = GameMediator::get_instance()->map_data[number].backgrounds[0].auto_scroll;
    // dynamic background won't work in low-end graphics more
    if (SharedData::get_instance()->game_config.graphics_performance_mode != PERFORMANCE_MODE_LOW) {
        if (bg1_scroll_mode == BG_SCROLL_MODE_LEFT) {
            bg_scroll.x -= bg1_speed;
            adjust_dynamic_background_position();
        } else if (bg1_scroll_mode == BG_SCROLL_MODE_RIGHT) {
            bg_scroll.x += bg1_speed;
            adjust_dynamic_background_position();
        } else if (bg1_scroll_mode == BG_SCROLL_MODE_UP) {
            bg_scroll.y -= bg1_speed;
            adjust_dynamic_background_position();
        } else if (bg1_scroll_mode == BG_SCROLL_MODE_DOWN) {
            bg_scroll.y += bg1_speed;
            adjust_dynamic_background_position();
        }
    }

    float x1 = bg_scroll.x;
    if (x1 > 0.0) { // moving to right
        x1 = (RES_W - x1) * -1;
    }

    float y1 = bg_scroll.y + GameMediator::get_instance()->map_data[number].backgrounds[0].adjust_y;

    if (surface_bg->width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(x1, y1), surface_bg, &graphLib.gameScreen);

        // draw rightmost part, if needed
        if (abs(bg_scroll.x) > RES_W) {
            float bg_pos_x = RES_W - (abs(x1)-RES_W);
            graphLib.copyAreaWithAdjust(st_position(bg_pos_x, y1), surface_bg, &graphLib.gameScreen);
        }  else if (surface_bg->width - abs(bg_scroll.x) < RES_W) {
            float bg_pos_x = surface_bg->width - (int)abs(bg_scroll.x);
            graphLib.copyAreaWithAdjust(st_position(bg_pos_x, y1), surface_bg, &graphLib.gameScreen);
        }
    }

    draw_static_background();
}

void classMap::draw_static_background()
{
    if (static_bg.is_null() == false && static_bg_pos.x >= scroll.x-1 && static_bg_pos.x < scroll.x+RES_W) {
        st_position adjusted_static_bg_pos((static_bg_pos.x-scroll.x), static_bg_pos.y);
        graphLib.copyAreaWithAdjust(adjusted_static_bg_pos, &static_bg, &graphLib.gameScreen);
        return;
    }
}

void classMap::draw_foreground_layer(int scroll_x, int scroll_y)
{

    if (strlen(GameMediator::get_instance()->map_data[number].backgrounds[1].filename) > 0) {
        float foreground_speed = (float)GameMediator::get_instance()->map_data[number].backgrounds[1].speed/10 * SharedData::get_instance()->get_movement_multiplier();
        int scroll_mode = GameMediator::get_instance()->map_data[number].backgrounds[1].auto_scroll;
        // dynamic background won't work in low-end graphics more
        if (SharedData::get_instance()->game_config.graphics_performance_mode != PERFORMANCE_MODE_LOW) {
            if (scroll_mode == BG_SCROLL_MODE_LEFT) {
                fg_layer_scroll.x -= ((float)1*foreground_speed);
                adjust_foreground_position();
            } else if (scroll_mode == BG_SCROLL_MODE_RIGHT) {
                fg_layer_scroll.x += ((float)1*foreground_speed);
                adjust_foreground_position();
            } else if (scroll_mode == BG_SCROLL_MODE_UP) {
                fg_layer_scroll.y -= ((float)1*foreground_speed);
                adjust_foreground_position();
            } else if (scroll_mode == BG_SCROLL_MODE_DOWN) {
                fg_layer_scroll.y += ((float)1*foreground_speed);
                adjust_foreground_position();
            }
        }

        int x1 = fg_layer_scroll.x;
        if (x1 > 0) { // moving to right
            x1 = (RES_W - x1) * -1;
        }

        int y1 = fg_layer_scroll.y + GameMediator::get_instance()->map_data[number].backgrounds[1].adjust_y;

        if (get_dynamic_foreground() != NULL && get_dynamic_foreground()->width > 0) {
            // draw leftmost part
            graphLib.copyAreaWithAdjust(st_position(x1, y1), get_dynamic_foreground(), &graphLib.gameScreen);

            // draw rightmost part, if needed
            if (abs(fg_layer_scroll.x) > RES_W) {
                int bg_pos_x = RES_W - (abs(x1)-RES_W);
                graphLib.copyAreaWithAdjust(st_position(bg_pos_x, y1), get_dynamic_foreground(), &graphLib.gameScreen);
            }  else if (get_dynamic_foreground()->width - abs(fg_layer_scroll.x) < RES_W) {
                int foreground_pos_x = get_dynamic_foreground()->width - (int)abs(fg_layer_scroll.x);
                // test if there isn't a overlap, so we need to add +1
                graphLib.copyAreaWithAdjust(st_position(foreground_pos_x, y1), get_dynamic_foreground(), &graphLib.gameScreen);
            }

        }
    }
}

void classMap::adjust_dynamic_background_position()
{
    if (get_dynamic_bg() == NULL) {
        return;
    }

    //int bg_limit = get_dynamic_bg()->width-RES_W;
    int bg_limit = get_dynamic_bg()->width;

    // esq -> direita: #1 bg_limt[640], scroll.x[-640.799]

    if (bg_scroll.x < -bg_limit) {
        bg_scroll.x = 0;
    } else if (bg_scroll.x > bg_limit) {
        bg_scroll.x = 0;
    } else if (bg_scroll.x > 0) {
        bg_scroll.x = -(get_dynamic_bg()->width); // erro aqui
    }


    if (bg_scroll.y < -RES_H) {
        bg_scroll.y = 0;
    } else if (bg_scroll.y > RES_H) {
        bg_scroll.y = 0;
    }
}

void classMap::adjust_foreground_position()
{
    // no need to adjust if no foreground
    if (get_dynamic_foreground() == NULL || get_dynamic_foreground()->width == 0) {
        return;
    }
    //int bg_limit = get_dynamic_foreground()->width-RES_W;
    int foreground_limit = get_dynamic_foreground()->width;

    // esq -> direita: #1 bg_limt[640], scroll.x[-640.799]

    if (fg_layer_scroll.x < -foreground_limit) {
        fg_layer_scroll.x = 0;
    } else if (fg_layer_scroll.x > foreground_limit) {
        fg_layer_scroll.x = 0;
    } else if (fg_layer_scroll.x > 0) {
        fg_layer_scroll.x = -(get_dynamic_foreground()->width); // erro aqui
    }


    if (fg_layer_scroll.y < -RES_H) {
        fg_layer_scroll.y = 0;
    } else if (fg_layer_scroll.y > RES_H) {
        fg_layer_scroll.y = 0;
    }
}

st_float_position classMap::get_foreground_postion()
{
    return fg_layer_scroll;
}

void classMap::set_foreground_postion(st_float_position pos)
{
    fg_layer_scroll = pos;
}

bool classMap::must_show_static_bg()
{
    if (static_bg.is_null() == false && static_bg_pos.x >= scroll.x-1 && static_bg_pos.x < scroll.x+RES_W) {
        return true;
    }
    return false;
}


void classMap::draw_dynamic_backgrounds_into_surface(graphicsLib_gSurface &surface)
{

    graphLib.clear_surface_area(0, 0, surface.width, surface.height, GameMediator::get_instance()->map_data[number].background_color.r, GameMediator::get_instance()->map_data[number].background_color.g, GameMediator::get_instance()->map_data[number].background_color.b, surface);

    if (get_dynamic_bg() == NULL) {
        return;
    }

    int x1 = bg_scroll.x;
    if (x1 > 0) { // moving to right
        x1 = (RES_W - x1) * -1;
    }

    int y1 = bg_scroll.y + GameMediator::get_instance()->map_data[number].backgrounds[0].adjust_y;


    if (get_dynamic_bg()->width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(x1, y1), get_dynamic_bg(), &surface);

        // draw rightmost part, if needed
        if (abs(bg_scroll.x) > RES_W) {
            int bg_pos_x = RES_W - (abs(x1)-RES_W);
            graphLib.copyAreaWithAdjust(st_position(bg_pos_x, y1), get_dynamic_bg(), &surface);
        }  else if (get_dynamic_bg()->width - abs(bg_scroll.x) < RES_W) {
            int bg_pos_x = get_dynamic_bg()->width - abs(bg_scroll.x);
            graphLib.copyAreaWithAdjust(st_position(bg_pos_x, y1), get_dynamic_bg(), &surface);
        }

    }

}

void classMap::add_object(object& obj)
{
    object_list.push_back(obj);
}

st_position classMap::get_first_lock_in_direction(st_position pos, st_size max_dist, int direction)
{
    st_position res;
    st_position x_limit_pos;

    switch (direction) {

    case ANIM_DIRECTION_LEFT:
    {
        res.y = pos.y;
        res.x = pos.x - max_dist.width;
        int scroll_lock_left = get_first_lock_on_left(pos.x/TILESIZE);
        if (scroll_lock_left < pos.x-max_dist.width) {
            for (int pos_i=pos.x; pos_i>(pos.x-max_dist.width); pos_i--) {
                int map_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(pos_i/TILESIZE, pos.y/TILESIZE));
                if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
                    res.x = pos_i+1;
                    break;
                }
            }
        } else {
            res.x = scroll_lock_left+TILESIZE;
        }
        break;
    }

    case ANIM_DIRECTION_RIGHT:
    {
        res.y = pos.y;
        res.x = pos.x + max_dist.width;
        int scroll_lock_right = get_first_lock_on_right(pos.x/TILESIZE);
        if (scroll_lock_right > pos.x+max_dist.width) {
            for (int pos_i=pos.x; pos_i<(pos.x+max_dist.width); pos_i++) {
                int map_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(pos_i/TILESIZE, pos.y/TILESIZE));
                if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
                    res.x = pos_i-1;
                    break;
                }
            }
        } else {
            res.x = scroll_lock_right-TILESIZE;
        }
        break;
    }
    case ANIM_DIRECTION_UP:
    {
        res.y = pos.y - max_dist.height;
        res.x = pos.x;
        for (int pos_i=pos.y; pos_i>(pos.y-max_dist.height); pos_i--) {
            int map_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(pos.x/TILESIZE, pos_i/TILESIZE));
            if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
                res.y = pos_i+1;
                break;
            }
        }
        break;
    }

    case ANIM_DIRECTION_DOWN:
        res.y = pos.y + max_dist.height;
        res.x = pos.x;
        for (int pos_i=pos.y; pos_i<(pos.y+max_dist.height); pos_i++) {
            int map_lock = gameControl.get_current_map_obj()->getMapPointLock(st_position(pos.x/TILESIZE, pos_i/TILESIZE));
            if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
                res.y = pos_i-1;
                break;
            }
        }
        break;

    case ANIM_DIRECTION_UP_LEFT:
        x_limit_pos = get_first_lock_in_direction(pos, max_dist, ANIM_DIRECTION_LEFT);
        res = get_first_lock_in_direction(x_limit_pos, max_dist, ANIM_DIRECTION_UP);
        break;

    case ANIM_DIRECTION_UP_RIGHT:
        x_limit_pos = get_first_lock_in_direction(pos, max_dist, ANIM_DIRECTION_RIGHT);
        res = get_first_lock_in_direction(x_limit_pos, max_dist, ANIM_DIRECTION_UP);
        break;

    case ANIM_DIRECTION_DOWN_LEFT:
        x_limit_pos = get_first_lock_in_direction(pos, max_dist, ANIM_DIRECTION_LEFT);
        res = get_first_lock_in_direction(x_limit_pos, max_dist, ANIM_DIRECTION_DOWN);
        break;

    case ANIM_DIRECTION_DOWN_RIGHT:
        x_limit_pos = get_first_lock_in_direction(pos, max_dist, ANIM_DIRECTION_RIGHT);
        res = get_first_lock_in_direction(x_limit_pos, max_dist, ANIM_DIRECTION_DOWN);
        break;

    default:
        break;
    }

    return res;
}

int classMap::get_first_lock_on_left(int x_tile_pos) const
{
    for (int i=x_tile_pos; i>= 0; i--) {
        if (wall_scroll_lock[i] == true) {
            return i*TILESIZE;
        }
    }
    return -1;
}

int classMap::get_first_lock_on_right(int x_tile_pos) const
{
    int limit = (scroll.x+RES_W)/TILESIZE;
    x_tile_pos += 1;
    for (int i=x_tile_pos; i<=limit; i++) {
        if (wall_scroll_lock[i] == true) {
            return i*TILESIZE;
        }
    }
    return -1;
}

// gets the first tile locked that have at least 3 tiles unlocked above it
int classMap::get_first_lock_on_bottom(int x_pos, int y_pos)
{
    return get_first_lock_on_bottom(x_pos, y_pos, TILESIZE, TILESIZE*3);
}

int classMap::get_first_lock_on_bottom(int x_pos, int y_pos, int w, int h)
{
    int tilex = x_pos/TILESIZE;
    int above_tiles_to_test = h/TILESIZE;
    if (above_tiles_to_test < 2) { // at least two tiles above even for small npcs
        above_tiles_to_test = 2;
    }
    int right_tiles_to_test = w/TILESIZE;
    if (right_tiles_to_test < 1) {
        right_tiles_to_test = 1;
    }

    int initial_y = MAP_H-1;
    if (y_pos >= 0) {
        initial_y = y_pos/TILESIZE;
    }

    for (int i=initial_y; i>=above_tiles_to_test+1; i--) { // ignore here first tiles, as we need to test them next
        int map_lock = getMapPointLock(st_position(tilex, i));
        bool found_bad_point = false;
        if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER && map_lock != TERRAIN_EASYMODEBLOCK && map_lock != TERRAIN_HARDMODEBLOCK) {
            // found a stop point, now check above tiles
            for (int j=i-1; j>=i-above_tiles_to_test; j--) {
                for (int k=0; k<right_tiles_to_test; k++) {
                    int map_lock2 = getMapPointLock(st_position(tilex+k, j));

                    if (map_lock2 != TERRAIN_UNBLOCKED && map_lock2 != TERRAIN_WATER) { // found a stop point, now check above ones
                        found_bad_point = true;
                        break;
                    }
                }
                if (found_bad_point) {
                    break;
                }
            }
            if (found_bad_point == false) {
                return i-1;
            }
        }
    }
    return 0;
}

void classMap::drop_item(classnpc* npc_ref)
{
    if (npc_ref == nullptr) {
        return;
    }
    st_float_position position = st_float_position(npc_ref->getPosition().x + npc_ref->get_size().width/2, npc_ref->getPosition().y + npc_ref->get_size().height/2);
    // dying out of screen should not drop item
    if (position.y > RES_H) {
        return;
    }
    srand(static_cast<unsigned int>(timer.getTimer()));
    int rand_n = static_cast<int> (100.0 * (rand() / (RAND_MAX + 1.0)));
    DROP_ITEMS_LIST obj_type;

    // sub-bosses always will drop energy big
    if (npc_ref->is_subboss()) {
        obj_type = DROP_ITEM_ENERGY_BIG;
    } else {
        // Big Energy (3%), Big Weapon (2%), Small Energy (15)%, Small Weapon (15%), Score Pearl (53%)
        int drop_ratio[5];
        if (game_save.difficulty == DIFFICULTY_EASY) {
            // 5%, 10%, 10%, 20%, 20%, 20% //
            drop_ratio[0] = 95;
            drop_ratio[1] = 85;
            drop_ratio[2] = 75;
            drop_ratio[3] = 55;
            drop_ratio[4] = 35;
        } else if (game_save.difficulty == DIFFICULTY_HARD) {
            // 1%, 1%, 1%, 10%, 10%, 10% //
            drop_ratio[0] = 99;
            drop_ratio[1] = 98;
            drop_ratio[2] = 97;
            drop_ratio[3] = 87;
            drop_ratio[4] = 77;
        } else {
            // .byt 99, 97, 95, 80, 65, 12 (http://tasvideos.org/RandomGenerators.html)
            drop_ratio[0] = 97;
            drop_ratio[1] = 95;
            drop_ratio[2] = 80;
            drop_ratio[3] = 65;
            drop_ratio[4] = 50;
        }

        if (rand_n >= drop_ratio[0]) {
            obj_type = DROP_ITEM_ENERGY_BIG;
        } else if (rand_n >= drop_ratio[1]) {
            obj_type = DROP_ITEM_WEAPON_BIG;
        } else if (rand_n >= drop_ratio[2]) {
            obj_type = DROP_ITEM_ENERGY_SMALL;
        } else if (rand_n >= drop_ratio[3]) {
            obj_type = DROP_ITEM_WEAPON_SMALL;
        } else if (rand_n >= drop_ratio[4]) {
            obj_type = DROP_ITEM_COIN;
        } else {
            return;
        }
    }
    st_position obj_pos;
    obj_pos.y = static_cast<short>(position.y/TILESIZE);
    obj_pos.x = static_cast<short>((position.x - TILESIZE)/TILESIZE);

    short obj_type_n = gameControl.get_drop_item_id(obj_type);
    if (obj_type_n == -1) {
        return;
    }


    object temp_obj(obj_type_n, this, obj_pos, st_position(-1, -1), -1);
    temp_obj.set_position(st_position(static_cast<int>(position.x), static_cast<int>(position.y)));
    temp_obj.set_duration(4500);
    add_object(temp_obj);
}

void classMap::set_bg_scroll(int scrollx)
{
    bg_scroll.x = scrollx;
}

int classMap::get_bg_scroll() const
{
    return static_cast<int>(bg_scroll.x);
}


void classMap::reset_map_timers()
{
    //reset_objects_timers();
    reset_objects_anim_timers();
    reset_enemies_timers();
}

void classMap::reset_enemies_timers()
{
    std::vector<classnpc>::iterator enemy_it;
    for (enemy_it = _npc_list.begin(); enemy_it != _npc_list.end(); enemy_it++) {
        (*enemy_it).reset_timers(); // TODO: must pass scroll map to npcs somwhow...
    }
}


void classMap::reset_objects_timers()
{
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        (*object_it).reset_timers(); // TODO: must pass scroll map to npcs somwhow...
    }
}

void classMap::reset_objects_anim_timers()
{
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        (*object_it).reset_obj_anim_timer(); // TODO: must pass scroll map to npcs somwhow...
    }
}

void classMap::reset_objects()
{
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        if ((*object_it).get_id() == game_data.player_items[0] || (*object_it).get_id() == game_data.player_items[1]) {
            (*object_it).set_finished(true);
        } else {
            (*object_it).reset();
        }
    }
}

void classMap::add_bubble_animation(st_position pos)
{
    if (_water_bubble.timer > timer.getTimer()) {
        return;
    }
    _water_bubble.timer = timer.getTimer()+3000;
    _water_bubble.pos.x = pos.x;
    _water_bubble.pos.y = pos.y;
    _water_bubble.x_adjust = 0;
}

// checks if player have any special object in screen
bool classMap::have_player_object()
{
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        int item_id = temp_obj.get_id();
        if (item_id == game_data.player_items[0] || item_id == game_data.player_items[1]) {
            return true;
        }
    }
    return false;
}

void classMap::remove_player_objects()
{
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        int item_id = temp_obj.get_id();
        if (item_id == game_data.player_items[0] || item_id == game_data.player_items[1]) {
            temp_obj.set_finished(true);
        }
    }
}

bool classMap::subboss_alive_on_left(short tileX)
{
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if ((npc_ref->is_boss() == true || npc_ref->is_subboss() == true) && npc_ref->is_dead() == false) {
            int dist_door_npc = tileX*TILESIZE - npc_ref->getPosition().x;
            if (npc_ref->getPosition().x >= (tileX-20)*TILESIZE && npc_ref->getPosition().x <= tileX*TILESIZE) { // 20 tiles is the size of a visible screen
                return true;
            }
        }
    }
    return false;
}

void classMap::finish_object_teleporter(int number)
{
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        if (temp_obj.get_obj_map_id() == number) {
            temp_obj.set_direction(ANIM_DIRECTION_RIGHT);
        }
    }
}

void classMap::activate_final_boss_teleporter()
{
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        if (temp_obj.get_type() == OBJ_FINAL_BOSS_TELEPORTER) {
            temp_obj.start();
        }
    }
}

Uint8 classMap::get_map_gfx()
{
    return GameMediator::get_instance()->map_data[number].backgrounds[0].gfx;
}

Uint8 classMap::get_map_gfx_mode()
{
    return GameMediator::get_instance()->map_data[number].backgrounds[1].auto_scroll;
}

st_float_position classMap::get_bg_scroll()
{
    return bg_scroll;
}

void classMap::set_bg_scroll(st_float_position pos)
{
    bg_scroll = pos;
}

st_rectangle classMap::get_player_hitbox()
{
    return _player_ref->get_hitbox();
}





// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::load_map_objects() {
	std::map<std::string, object>::iterator it;

	// remove all elements currently in the list
    while (object_list.size() > 0) {
        object_list.pop_back();
    }

    while (animation_list.size() > 0) {
        animation_list.pop_back();
    }

    for (int i=0; i<GameMediator::get_instance()->map_object_data.size(); i++) {
        if (GameMediator::get_instance()->map_object_data[i].difficulty_mode == DIFFICULTY_MODE_GREATER && GameMediator::get_instance()->map_object_data[i].difficulty_level > game_save.difficulty) {
            continue;
        } else if (GameMediator::get_instance()->map_object_data[i].difficulty_mode == DIFFICULTY_MODE_EQUAL && GameMediator::get_instance()->map_object_data[i].difficulty_level != game_save.difficulty) {
            continue;
        }

        if (GameMediator::get_instance()->map_object_data[i].id_object != -1 && GameMediator::get_instance()->map_object_data[i].stage_id == stage_number && GameMediator::get_instance()->map_object_data[i].map_id == number) {
            object temp_obj(GameMediator::get_instance()->map_object_data[i].id_object, this, GameMediator::get_instance()->map_object_data[i].start_point, GameMediator::get_instance()->map_object_data[i].link_dest, GameMediator::get_instance()->map_object_data[i].map_dest);
            temp_obj.set_obj_map_id(i);
            temp_obj.set_direction(GameMediator::get_instance()->map_object_data[i].direction);
			object_list.push_back(temp_obj);
		}
	}
}





st_float_position classMap::get_last_scrolled() const
{
	return scrolled;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::reset_scrolled()
{
	scrolled.x = 0;
	scrolled.y = 0;
}

bool classMap::value_in_range(int value, int min, int max) const
{
    return (value >= min) && (value <= max);
}

void classMap::create_dynamic_background_surfaces()
{
    std::string bg_filename = std::string(GameMediator::get_instance()->map_data[number].backgrounds[0].filename);
    if (bg_filename.length() > 0) {
        draw_lib.add_dynamic_background(bg_filename, GameMediator::get_instance()->map_data[number].backgrounds[0].auto_scroll, GameMediator::get_instance()->map_data[number].background_color);
    }
    // foreground image
    if (strlen(GameMediator::get_instance()->map_data[number].backgrounds[1].filename) > 0) {
        // if image already exists, no need to add or set alpha
        // @NOTE: alpha must be the same for three maps if using same image
        if (draw_lib.get_dynamic_foreground(std::string(GameMediator::get_instance()->map_data[number].backgrounds[1].filename)) == NULL) {
            draw_lib.add_dynamic_background(std::string(GameMediator::get_instance()->map_data[number].backgrounds[1].filename), GameMediator::get_instance()->map_data[number].backgrounds[1].auto_scroll, st_color(COLORKEY_R, COLORKEY_G, COLORKEY_B));
            if (GameMediator::get_instance()->map_data[number].backgrounds[1].gfx != 100 && SharedData::get_instance()->game_config.graphics_performance_mode != PERFORMANCE_MODE_LOW) {
                int fg_alpha = (255 * GameMediator::get_instance()->map_data[number].backgrounds[1].gfx)/100;
                draw_lib.set_dynamic_bg_alpha(GameMediator::get_instance()->map_data[number].backgrounds[1].filename, fg_alpha);
            }
        }
    }
}

graphicsLib_gSurface *classMap::get_dynamic_bg()
{
    return draw_lib.get_dynamic_background(GameMediator::get_instance()->map_data[number].backgrounds[0].filename);
}

graphicsLib_gSurface *classMap::get_dynamic_foreground()
{
    return draw_lib.get_dynamic_foreground(GameMediator::get_instance()->map_data[number].backgrounds[1].filename);
}



int classMap::collision_rect_player_obj(st_rectangle player_rect, object* temp_obj, const short int x_inc, const short int y_inc, const short obj_xinc, const short obj_yinc)
{
    int blocked = 0;
    int obj_y_reducer = 1;
    collision_detection rect_collision_obj;

// used to give char a small amount of pixels that he can enter inside object image

    st_position temp_obj_pos = temp_obj->get_position();

    st_rectangle obj_rect(temp_obj_pos.x+obj_xinc, temp_obj_pos.y+obj_yinc+obj_y_reducer, temp_obj->get_size().width, temp_obj->get_size().height);
    st_rectangle p_rect(player_rect.x+x_inc, player_rect.y+y_inc, player_rect.w, player_rect.h);

    // if only moving up/down, give one extra pivel free (otherwise in won't be able to jump next an object)

    if (x_inc == 0 && y_inc != 0) {
        p_rect.x++;
        p_rect.w -= 2;
    }

    if (temp_obj->get_type() == OBJ_ITEM_JUMP) {
        obj_rect.y += OBJ_JUMP_Y_ADJUST;
    } else {
        obj_rect.y += 1;
    }

    bool xObjOver = value_in_range(obj_rect.x, p_rect.x, p_rect.x + p_rect.w);
    bool xPlayerOver = value_in_range(p_rect.x, obj_rect.x, obj_rect.x + obj_rect.w);
    bool xOverlap = xObjOver == true || xPlayerOver == true;
    bool yOverlap = value_in_range(obj_rect.y, p_rect.y, p_rect.y + p_rect.h) || value_in_range(p_rect.y, obj_rect.y, obj_rect.y + obj_rect.h);

    // check if X is blocked
    bool before_collision = rect_collision_obj.rect_overlap(obj_rect, p_rect);
    if (before_collision == true && temp_obj->get_collision_mode() != COLlISION_MODE_Y) {
        blocked = BLOCK_X;
    }



    if (xOverlap == true && yOverlap == true) {
        if (blocked == 0) {
            blocked = BLOCK_Y;
        } else {
            blocked = BLOCK_XY;
        }
    }

    if (blocked != 0 && temp_obj->get_type() == OBJ_ACTIVE_OPENING_SLIM_PLATFORM) {
        if (abs(p_rect.y + p_rect.h - obj_rect.y) > y_inc || y_inc < 0) {
            blocked = 0;
        }
    }

    return blocked;
}


// this method is used for npcs to ignore certain objects
bool classMap::is_obj_ignored_by_enemies(Uint8 obj_type)
{
    if (obj_type == OBJ_ENERGY_TANK) {
        return true;
    }
    if (obj_type == OBJ_WEAPON_TANK) {
        return true;
    }
    if (obj_type == OBJ_ENERGY_PILL_BIG) {
        return true;
    }
    if (obj_type == OBJ_WEAPON_PILL_BIG) {
        return true;
    }
    if (obj_type == OBJ_ENERGY_PILL_SMALL) {
        return true;
    }
    if (obj_type == OBJ_WEAPON_PILL_SMALL) {
        return true;
    }
    if (obj_type == OBJ_ITEM_FLY) {
        return true;
    }
    if (obj_type == OBJ_ITEM_JUMP) {
        return true;
    }
    if (obj_type == OBJ_ARMOR_ARMS) {
        return true;
    }
    if (obj_type == OBJ_ARMOR_BODY) {
        return true;
    }
    if (obj_type == OBJ_ARMOR_LEGS) {
        return true;
    }
    if (obj_type == OBJ_BOSS_TELEPORTER) {
        return true;
    }
    if (obj_type == OBJ_STAGE_BOSS_TELEPORTER) {
        return true;
    }
    if (obj_type == OBJ_PLATFORM_TELEPORTER) {
        return true;
    }
    if (obj_type == OBJ_SPECIAL_TANK) {
        return true;
    }
    if (obj_type == OBJ_FINAL_BOSS_TELEPORTER) {
        return true;
    }
    if (obj_type == OBJ_CHECKPOINT) {
        return true;
    }
    return false;
}


void classMap::collision_char_object(character* charObj, const float x_inc, const short int y_inc)
{
    int blocked = 0;
    object* res_obj = NULL;

    // ignore collision if teleporting
    if (charObj->get_anim_type() == ANIM_TYPE_TELEPORT) {
        return;
    }

    st_rectangle char_rect = charObj->get_hitbox();

    /// @TODO: isso aqui deveria mesmo estar aqui?
    if (charObj->get_platform() == NULL) {
        for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
            object& temp_obj = (*it);

            if (temp_obj.is_hidden() == true) {
                continue;
            }

            if (temp_obj.is_on_screen() == false) {
                continue;
            }

            if (temp_obj.finished() == true) {
                continue;
            }

            if (charObj->is_player() == false && is_obj_ignored_by_enemies(temp_obj.get_type())) {
                continue;
            }

            // slim platform won't collide if movement is from bottom to top
            if (temp_obj.get_type() == OBJ_ACTIVE_OPENING_SLIM_PLATFORM && y_inc < 0) {
                continue;
            }

            if (temp_obj.is_teleporting()) {
                continue;
            }

            // jumping from inside item-coil must not block player
            if (temp_obj.get_type() == OBJ_ITEM_JUMP && y_inc < 0) {
                continue;
            }

            st_rectangle stopped_char_rect = charObj->get_hitbox();
            stopped_char_rect.x+= CHAR_OBJ_COLlISION_KILL_ADJUST/2;
            stopped_char_rect.y+= CHAR_OBJ_COLlISION_KILL_ADJUST;
            stopped_char_rect.w-= CHAR_OBJ_COLlISION_KILL_ADJUST;
            stopped_char_rect.h-= CHAR_OBJ_COLlISION_KILL_ADJUST*2;

            // check if, without moving, player is inside object
            int no_move_blocked = collision_rect_player_obj(stopped_char_rect, &temp_obj, 0, 0, 0, 0);

            // some platforms can kill the player if he gets stuck inside it
            if (charObj->is_player() == true && (temp_obj.get_type() == OBJ_MOVING_PLATFORM_UPDOWN || temp_obj.get_type() == OBJ_FLY_PLATFORM)) {
                if (no_move_blocked == BLOCK_XY) {
                    _obj_collision = object_collision(BLOCK_INSIDE_OBJ, &temp_obj);
                    return;
                }
            }

            // usar TEMP_BLOCKED aqui, para não zerar o blocked anterior, fazer merge dos valores
            int temp_blocked = 0;
            temp_blocked = collision_rect_player_obj(char_rect, &temp_obj, x_inc, y_inc, 0, 0);

            int temp_obj_y = temp_obj.get_position().y;
            if (temp_obj.get_type() == OBJ_ITEM_JUMP) {
                temp_obj_y += OBJ_JUMP_Y_ADJUST;
            }


            // to enter platform, player.x+player.h must not be much higher than obj.y
            if (temp_blocked != 0) {
                if (temp_obj.get_type() == OBJ_CHECKPOINT) {
                    if (temp_obj.is_started() == false) {
                        temp_obj.start();
                    }
                    checkpoint.x = temp_obj.get_position().x;
                    checkpoint.y = temp_obj.get_position().y + temp_obj.get_size().height + TILESIZE/2;
                    checkpoint.map = gameControl.get_current_map_obj()->get_number();
                    checkpoint.map_scroll_x = gameControl.get_current_map_obj()->getMapScrolling().x;
                    //return;
                    continue;
                } else if (temp_obj.get_type() == OBJ_BOSS_DOOR && charObj->is_player()) {
                    if (temp_obj.is_started() == false && subboss_alive_on_left(temp_obj.get_position().x/TILESIZE) == false) { // check for sub-boss alive on the left
                        // check if player position is not under door //
                        if ((char_rect.y + char_rect.h) <= (temp_obj.get_position().y+temp_obj.get_size().height+2) && char_rect.y >= temp_obj.get_position().y) {
                            temp_obj.start();
                            if (charObj->get_int_position().x > temp_obj.get_position().x + temp_obj.get_size().width) {
                                temp_obj.set_direction(ANIM_DIRECTION_LEFT);
                            } else {
                                temp_obj.set_direction(ANIM_DIRECTION_RIGHT);
                            }
                        }
                    }
                }


                if (temp_blocked == BLOCK_Y || temp_blocked == BLOCK_XY) {

                    bool entered_platform = false;

                    if (temp_obj.get_state() != 0 && temp_obj.get_type() == OBJ_TRACK_PLATFORM) {
                        continue;
                    }

                    if (charObj->is_player() == true && temp_obj.get_state() != 0 && (temp_obj.get_type() == OBJ_RAY_VERTICAL || temp_obj.get_type() == OBJ_RAY_HORIZONTAL)) {
                        charObj->damage(TOUCH_DAMAGE_BIG, false);
                        continue;
                    } else if (charObj->is_player() == true && temp_obj.get_state() != 0 && (temp_obj.get_type() == OBJ_DEATHRAY_VERTICAL || temp_obj.get_type() == OBJ_DEATHRAY_HORIZONTAL)) {
                        charObj->damage(999, false);
                        continue;
                    }

                    if (charObj->is_player() == true && temp_obj.get_type() == OBJ_CRUSHER && temp_obj.check_player_crushed()) {
                        charObj->damage(999, false);
                        continue;
                    }

                    // if inside object and is disappearing block, move char above it
                    if (no_move_blocked == BLOCK_XY && temp_blocked == BLOCK_XY && (charObj->is_player() && temp_obj.get_type() == OBJ_DISAPPEARING_BLOCK || charObj->is_player() && temp_obj.get_type() == OBJ_ACTIVE_DISAPPEARING_BLOCK)) {
                        charObj->set_position(st_position(charObj->get_int_position().x, temp_obj.get_position().y - charObj->get_size().height));
                    }

                    if (y_inc > 0 && char_rect.y <= temp_obj_y) {
                        entered_platform = true;
                    }


                    if (entered_platform == true) {

                        if (temp_obj.is_hidden() == false && (temp_obj.get_type() == OBJ_MOVING_PLATFORM_UPDOWN || temp_obj.get_type() == OBJ_MOVING_PLATFORM_LEFTRIGHT || temp_obj.get_type() == OBJ_DISAPPEARING_BLOCK)) {
                            if (charObj->get_platform() == NULL && (temp_blocked == 2 || temp_blocked == 3)) {


                                charObj->set_platform(&temp_obj);
                                if (temp_obj.get_type() == OBJ_FALL_PLATFORM) {
                                    temp_obj.set_direction(ANIM_DIRECTION_LEFT);
                                }
                            } else if (charObj->get_platform() == NULL && temp_blocked == 1) {
                                charObj->set_platform(&temp_obj);
                            }
                            if (temp_blocked != 0) {
                                _obj_collision = object_collision(temp_blocked, &(*it));
                                return;
                            }
                        } else if (temp_obj.get_type() == OBJ_ITEM_FLY) {
                            if (charObj->get_platform() == NULL && (temp_blocked == 2 || temp_blocked == 3) && y_inc > 0) {
                                charObj->set_platform(&temp_obj);
                                if (temp_obj.get_distance() == 0) {
                                    temp_obj.start();
                                    temp_obj.set_distance(1);
                                    temp_obj.set_timer(timer.getTimer()+30);
                                }
                            }
                            if (temp_blocked != 0) {
                                _obj_collision = object_collision(temp_blocked, &(*it));
                                return;
                            }
                        } else if (temp_obj.get_type() == OBJ_ITEM_JUMP) {
                            if (charObj->get_platform() == NULL && (temp_blocked == 2 || temp_blocked == 3) && y_inc > 0 && charObj->getPosition().y+charObj->get_size().height <= temp_obj_y+1) {
                                charObj->activate_super_jump();
                                charObj->activate_force_jump();
                                temp_obj.start();
                            }
                            if (temp_blocked != 0) {
                                if (y_inc > 0) {
                                    _obj_collision = object_collision(temp_blocked, &(*it));
                                    return;
                                } else {
                                    temp_blocked = 0;
                                }
                            }
                        } else if (temp_obj.is_hidden() == false && temp_obj.is_started() == false && (temp_obj.get_type() == OBJ_ACTIVE_DISAPPEARING_BLOCK || temp_obj.get_type() == OBJ_ACTIVE_OPENING_SLIM_PLATFORM)) {
                            temp_obj.start();
                        } else if (temp_obj.get_type() == OBJ_FALL_PLATFORM || temp_obj.get_type() == OBJ_FLY_PLATFORM) {
                            if (charObj->get_platform() == NULL) {
                                charObj->set_platform(&temp_obj);
                                if (temp_obj.get_state() == OBJ_STATE_STAND) {
                                    temp_obj.set_state(OBJ_STATE_MOVE);
                                    temp_obj.start();
                                }
                                temp_obj.set_timer(timer.getTimer()+30);
                                _obj_collision = object_collision(temp_blocked, &(*it));
                                return;
                            }
                        } else if (temp_obj.get_type() == OBJ_TRACK_PLATFORM) {
                            if (charObj->get_platform() == NULL) {
                                charObj->set_platform(&temp_obj);
                                _obj_collision = object_collision(temp_blocked, &(*it));
                                return;
                            }
                        } else if (temp_obj.get_type() == OBJ_DAMAGING_PLATFORM) {
                            if (charObj->get_platform() == NULL) {
                                charObj->set_platform(&temp_obj);
                                _obj_collision = object_collision(temp_blocked, &(*it));
                                temp_obj.start();
                                return;
                            }
                        }
                    }


                }

                if (temp_blocked != 0) {
                    res_obj = &(*it);
                }

            }



            // merge blocked + temp_blocked
            if (temp_blocked == BLOCK_X) {
                if (blocked == 0) {
                    blocked = BLOCK_X;
                } else if (blocked == BLOCK_Y) {
                    blocked = BLOCK_XY;
                }
            } else if (temp_blocked == BLOCK_Y) {
                if (blocked == 0) {
                    blocked = BLOCK_Y;
                } else if (blocked == BLOCK_X) {
                    blocked = BLOCK_XY;
                }
            } else if (temp_blocked == BLOCK_XY) {
                blocked = BLOCK_XY;
            }
        }




    // this part seems to be OK
    } else {
        object* temp_obj = charObj->get_platform();
        if (temp_obj->is_hidden() == true) {
            charObj->set_platform(NULL);
        } else if (temp_obj->get_type() == OBJ_TRACK_PLATFORM && temp_obj->get_state() != 0) {
            charObj->set_platform(NULL);
        } else {
            blocked = collision_rect_player_obj(char_rect, temp_obj, x_inc, y_inc, 0, 0);
            if (blocked != 0) {
                res_obj = temp_obj;
            }
        }
    }


    // got out of platform
    if (blocked == 0 && charObj->get_platform() != NULL) {
        //  for player item, platform must only be removed only if the item was already adtivated
        if (charObj->get_platform()->get_type() == OBJ_ITEM_FLY || charObj->get_platform()->get_type() == OBJ_ITEM_JUMP) {
            if (charObj->get_platform()->get_distance() > 0 && y_inc != 0) {
                charObj->set_platform(NULL);
            } else {
                _obj_collision = object_collision(0, NULL);
                return;
            }
        } else if (charObj->get_platform()->is_hidden() == true) {
            charObj->set_platform(NULL);
        } else {
            _platform_leave_counter++;
            if (_platform_leave_counter > 2) {
                charObj->set_platform(NULL);
                _platform_leave_counter = 0;
            }
        }
    } else if (blocked != 0 && charObj->get_platform() != NULL) {
        _platform_leave_counter = 0;
    }


    _obj_collision = object_collision(blocked, res_obj);
}

object_collision classMap::get_obj_collision()
{
    return _obj_collision;
}




void classMap::clean_map_npcs_projectiles()
{
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        npc_ref->clean_projectiles();
    }
}

void classMap::reset_beam_objects()
{
    // reset objects
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        short obj_type = temp_obj.get_type();
        if (obj_type == OBJ_DEATHRAY_VERTICAL || obj_type == OBJ_DEATHRAY_HORIZONTAL || obj_type == OBJ_RAY_VERTICAL || obj_type == OBJ_RAY_HORIZONTAL) {
            temp_obj.reset();
        }
    }
}

void classMap::remove_temp_objects()
{
    // reset objects
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        if (temp_obj.get_is_dropped() == true) {
            temp_obj.set_finished(true);
        }
        for (int i=0; i<FS_PLATER_ITEMS_N; i++) {
            if (temp_obj.get_id() == game_data.player_items[i]) {
                temp_obj.set_finished(true);
            }
        }
    }
    clean_finished_objects();
}



bool classMap::get_map_point_wall_lock(int x) const
{
    return wall_scroll_lock[x/TILESIZE];
}

void classMap::move_map(const short int move_x, const short int move_y)
{
    set_scrolling(st_float_position(scroll.x+move_x, scroll.y+move_y));
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classnpc* classMap::collision_player_npcs(character* playerObj, const short int x_inc, const short int y_inc)
{
    UNUSED(x_inc);
    UNUSED(y_inc);
	struct st_rectangle p_rect, npc_rect;

    p_rect = playerObj->get_hitbox();

    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_player_friend() == true) {
			continue;
		}
        if (npc_ref->is_dead() == true) {
			continue;
		}
        if (npc_ref->is_invisible() == true) {
			continue;
		}

        if (npc_ref->is_on_visible_screen() == false) {
            continue;
        }

        if (npc_ref->is_intangible() == true) {
            continue;
        }


        npc_rect = npc_ref->get_hitbox();

        collision_detection rect_collision_obj;
        if (rect_collision_obj.rect_overlap(npc_rect, p_rect) == true) {
            return npc_ref;
        }
    }
    return NULL;
}


// kills any NPC that touches player during player's special attack
void classMap::collision_player_special_attack(character* playerObj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y)
{
    UNUSED(x_inc);
    UNUSED(y_inc);
    struct st_rectangle p_rect, npc_rect;

    //reduce = abs((float)16-playerObj->sprite->w)*0.5;

    // ponto 3, topo/esquerda
    if (playerObj->get_direction() == ANIM_DIRECTION_LEFT) {
        p_rect.x = playerObj->getPosition().x + reduce_x;
        p_rect.w = playerObj->get_size().width;
    } else {
        p_rect.x = playerObj->getPosition().x;
        p_rect.w = playerObj->get_size().width - reduce_x;
    }
    p_rect.y = playerObj->getPosition().y + reduce_y;
    p_rect.h = playerObj->get_size().height;

    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_player_friend() == true) {
            continue;
        }
        if (npc_ref->is_dead() == true) {
            continue;
        }
        if (npc_ref->is_invisible() == true) {
            continue;
        }

        if (npc_ref->is_on_visible_screen() == false) {
            continue;
        }


        npc_rect.x = npc_ref->getPosition().x;
        npc_rect.w = npc_ref->get_size().width;
        npc_rect.y = npc_ref->getPosition().y;
        npc_rect.h = npc_ref->get_size().height;

        if (npc_ref->get_size().width >= TILESIZE) { // why is this here??? O.o
            npc_rect.x = npc_ref->getPosition().x+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.w = npc_ref->get_size().width-PLAYER_NPC_COLLISION_REDUTOR;
        }
        if (npc_ref->get_size().height >= TILESIZE) {
            npc_rect.y = npc_ref->getPosition().y+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.h = npc_ref->get_size().height-PLAYER_NPC_COLLISION_REDUTOR;
        }
        collision_detection rect_collision_obj;
        if (rect_collision_obj.rect_overlap(npc_rect, p_rect) == true) {
            npc_ref->damage(12, false);
        }
    }
}

classnpc* classMap::find_nearest_npc(st_position pos)
{
    int min_dist = 9999;
    classnpc* min_dist_npc = NULL;

    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_player_friend() == true) {
            continue;
        }
        if (npc_ref->is_dead() == true) {
            continue;
        }
        if (npc_ref->is_invisible() == true) {
            continue;
        }
        if (npc_ref->is_on_visible_screen() == false) {
            continue;
        }
        float dist = sqrt(pow((pos.x - npc_ref->getPosition().x), 2) + pow((pos.y - npc_ref->getPosition().y), 2));
        if (dist < min_dist) {
            min_dist_npc = npc_ref;
            min_dist = dist;
        }
    }
    return min_dist_npc;
}

classnpc *classMap::find_nearest_npc_on_direction(st_position pos, int direction)
{
    int lower_dist = 9999;
    classnpc* ret = NULL;

    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_on_visible_screen() == false) {
            continue;
        }
        if (npc_ref->is_dead() == true) {
            continue;
        }

        st_position npc_pos(npc_ref->getPosition().x*TILESIZE, npc_ref->getPosition().y*TILESIZE);
        npc_pos.x = (npc_pos.x + npc_ref->get_size().width/2)/TILESIZE;
        npc_pos.y = (npc_pos.y + npc_ref->get_size().height)/TILESIZE;

        // if facing left, ignore enemies with X greater than player x
        if (direction == ANIM_DIRECTION_LEFT && npc_pos.x > pos.x) {
            continue;
        }
        // if facing right, ignore enemies with X smaller than player x+width
        if (direction == ANIM_DIRECTION_RIGHT && npc_pos.x < pos.x) {
            continue;
        }

        // pitagoras: raiz[ (x2-x1)^2 + (y2-y1)^2 ]
        int dist = sqrt(pow((float)(pos.x - npc_pos.x), (float)2) + pow((float)(pos.y - npc_pos.y ), (float)2));
        if (dist < lower_dist) {
            lower_dist = dist;
            ret = npc_ref;
        }
    }
    return ret;
}

/// @TODO: fix animation. investigate a better way for drawing it (code is way too confusing)
void classMap::redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number) {
    timer.delay(10);
	for (int k=0; k<nTiles; k++) {
		// redraw screen
        show_map();

        _3rd_level_ignore_area = st_rectangle(tileX, tileY-5, 1, nTiles+5);
        showAbove();
        draw_lib.update_screen();
		int tiles_showed;
		if (is_close == false) {
			tiles_showed = k;
		} else {
			tiles_showed = 0;
		}
		for (int i=0; i<MAP_W; i++) {
			for (int j=0; j<MAP_H; j++) {
                if (GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.x != -1 && GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.y != -1) {
                        if (i == tileX && GameMediator::get_instance()->map_data[number].tiles[i][j].locked == TERRAIN_DOOR) {
							if (is_close == false) {
								if (tiles_showed < nTiles) {

                                    if (!graphLib.gameScreen.get_surface()) {
                                        graphLib.show_debug_msg("EXIT #21.C");
                                        exception_manager::throw_general_exception(std::string("classMap::redraw_boss_door"), "Gamescreen is null #1");
                                    }


                                    graphLib.placeTile(st_position(GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.x, GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)-scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
                                    draw_lib.update_screen();
									tiles_showed++;
								}
							} else {
								if (tiles_showed < k) {

                                    if (!graphLib.gameScreen.get_surface()) {
                                        graphLib.show_debug_msg("EXIT #21.D");
                                        exception_manager::throw_general_exception(std::string("classMap::redraw_boss_door"), "Gamescreen is null #2");
                                    }

                                    graphLib.placeTile(st_position(GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.x, GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)-scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
                                    draw_lib.update_screen();
									tiles_showed++;
								}
							}
						} else {

                            if (!graphLib.gameScreen.get_surface()) {
                                graphLib.show_debug_msg("EXIT #21.E");
                                exception_manager::throw_general_exception(std::string("classMap::redraw_boss_door"), "Gamescreen is null #3");
                            }


                            graphLib.placeTile(st_position(GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.x, GameMediator::get_instance()->map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)+scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
						}
				}
			}
		}
        _player_ref->show();
        draw_lib.show_hud(_player_ref->get_current_hp(), 1, _player_ref->get_selected_weapon(), _player_ref->get_selected_weapon_value());
        showAbove();
        draw_lib.update_screen();
        timer.delay(100);
	}
    if (is_close == true) {
        _3rd_level_ignore_area = st_rectangle(-1, -1, -1, -1);
        showAbove();
        draw_lib.update_screen();
    }
    timer.delay(100);
}


void classMap::add_animation(ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize)
{
    animation_list.push_back(animation(pos_type, surface, pos, adjust_pos, frame_time, repeat_times, direction, framesize, &scroll));
}

void classMap::add_animation(animation anim)
{
    animation_list.push_back(anim);
}

void classMap::clear_animations()
{
	animation_list.erase(animation_list.begin(), animation_list.end());
}

void classMap::set_player(classPlayer *player_ref)
{
    _player_ref = player_ref;
}

classnpc* classMap::spawn_map_npc(short npc_id, st_position npc_pos, short int direction, bool player_friend, bool progressive_span)
{

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "MAP::spawn_map_npc, id[%d]", npc_id);
#endif

    classnpc new_npc(stage_number, number, npc_id, npc_pos, direction, player_friend);

    if (progressive_span == true) {
        new_npc.set_progressive_appear_pos(new_npc.get_size().height);
    }
    _npc_spawn_list.push_back(new_npc); // insert new npc at the list-end

    classnpc* npc_ref = &(_npc_spawn_list.back());

    int id = npc_ref->get_number();
    std::string npc_name = npc_ref->get_name();

    return npc_ref;
}

int classMap::child_npc_count(int parent_id)
{
    int count = 0;
    std::vector<classnpc>::iterator npc_it;
    int n = 0;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_dead() == false && npc_ref->get_parent_id() == parent_id) {
            count++;
        }
        n++;
    }
    for (npc_it = _npc_spawn_list.begin(); npc_it != _npc_spawn_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_dead() == false && npc_ref->get_parent_id() == parent_id) {
            count++;
        }
        n++;
    }
    return count;
}


void classMap::move_npcs() /// @TODO - check out of screen
{

    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {

        classnpc* npc_ref = &(*npc_it);
        // check if NPC is outside the visible area
        st_position npc_pos = npc_ref->get_real_position();
        short dead_state = npc_ref->get_dead_state();

        std::string name(npc_ref->get_name());


        if (npc_ref->is_on_screen() != true) {
            if (dead_state == 2 && npc_ref->is_boss() == false && npc_ref->is_subboss()) {
                npc_ref->revive();
            }
            npc_ref->move_projectiles();
            continue; // no need for moving NPCs that are out of sight
        } else if (dead_state == 2 && npc_ref->auto_respawn() == true && npc_ref->is_boss() == false) {
            npc_ref->reset_position();
            npc_ref->revive();
            continue;
        } else if (dead_state == 1 && npc_ref->is_spawn() == false && npc_ref->is_boss() == false) { // drop item
            drop_item(npc_ref);
        }

        npc_ref->execute(); // TODO: must pass scroll map to npcs somwhow...

		if (dead_state == 1) {
            if (npc_ref->is_stage_boss() == false) {
                npc_ref->execute_ai(); // to ensure death-reaction is run

                // sub-boss have a different explosion
                if (npc_ref->is_subboss()) {
                    soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 1);
                    st_float_position pos1(npc_ref->getPosition().x+2, npc_ref->getPosition().y+20);
                    add_animation(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos1, st_position(-8, -8), 80, 2, npc_ref->get_direction(), st_size(56, 56));
                    st_float_position pos2(pos1.x+50, pos1.y-30);
                    add_animation(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos2, st_position(-8, -8), 80, 2, npc_ref->get_direction(), st_size(56, 56));
                } else if (npc_ref->getPosition().y < RES_H) { // don't add death explosion when dying out of screen
                    add_animation(ANIMATION_STATIC, &graphLib.explosion32, npc_ref->getPosition(), st_position(-8, -8), 80, 2, npc_ref->get_direction(), st_size(32, 32));
                }
                // check if boss flag wasn't passed to a spawn on dying reaction AI
                if (npc_ref->is_boss()) {
                    gameControl.check_player_return_teleport();
                }

                // all kinds of bosses need to remove projectiles once dying
                if (npc_ref->is_boss() || npc_ref->is_subboss() || npc_ref->is_stage_boss()) {
                    npc_ref->clean_projectiles();
                // regular enemies only remove effect-type projectiles (quake, wind, freeze, etc)
                } else {
                    npc_ref->clean_effect_projectiles();
                }
            } else {
                // run npc move one more time, so reaction is executed to test if it will spawn a new boss (replace-itself)
                for (int i=0; i<2; i++) {
                    npc_ref->execute_ai(); // to ensure death-reaction is run
                }


                if (npc_ref->is_stage_boss() == false) { // if now the NPC is not the stage boss anymore, continue
                    gameControl.draw_explosion(npc_pos, true);
                    soundManager.play_boss_music();
                    graphLib.blink_screen(255, 255, 255);
                    continue;
                } else {
                    gameControl.remove_all_projectiles();
                    graphLib.set_screen_adjust(st_position(0, 0));
                    /// @TODO - replace with game_data.final_boss_id
                    if (game_data.final_boss_id == npc_ref->get_number()) {
                        soundManager.stop_music();
                        gameControl.draw_explosion(npc_pos, true);
                        graphLib.blink_screen(255, 255, 255);
                        graphLib.blank_screen();
                        graphLib.updateScreen();
                        timer.delay(1000);
                        gameControl.show_ending();
                        return;
                    } else {
                        gameControl.draw_explosion(npc_pos, true);
                        gameControl.got_weapon();
                    }
                }
			}
			return;
		}
    }

    if (_npc_spawn_list.size() > 0) {
        std::vector<classnpc>::iterator npc_it;
        for (npc_it = _npc_spawn_list.begin(); npc_it != _npc_spawn_list.end(); npc_it++) {
            _npc_list.push_back(*npc_it);
        }
        _npc_spawn_list.clear();
    }
}

void classMap::show_npcs() /// @TODO - check out of screen
{
    bool has_boss = false;
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->npc_is_ghost()) {
            continue;
        }
        if (gameControl.must_show_boss_hp() && npc_ref->is_boss() && npc_ref->is_on_visible_screen() == true) {
            has_boss = true;
            draw_lib.set_boss_hp(npc_ref->get_current_hp());
		}
        if (npc_ref->is_dead() == false) {
            npc_ref->show();
        }
        npc_ref->show_projectiles();
    }
    if (has_boss == false) {
        draw_lib.set_boss_hp(-99);
    }
}

void classMap::show_ghost_npcs()
{
    bool has_boss = false;
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (!npc_ref->npc_is_ghost()) {
            continue;
        }
        if (gameControl.must_show_boss_hp() && npc_ref->is_boss() && npc_ref->is_on_visible_screen() == true) {
            has_boss = true;
            draw_lib.set_boss_hp(npc_ref->get_current_hp());
        }
        if (npc_ref->is_dead() == false) {
            npc_ref->show();
        }
        npc_ref->show_projectiles();
    }
    if (has_boss == false) {
        draw_lib.set_boss_hp(-99);
    }
}

void classMap::show_npcs_to_left(int x)
{
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_dead() == false && npc_ref->is_on_visible_screen() && npc_ref->getPosition().x <= x) {
            npc_ref->show();
        }
        npc_ref->show_projectiles();
    }
    draw_lib.set_boss_hp(-99);
}

void classMap::move_objects(bool paused)
{
    /// @TODO - update timers
    std::vector<object>::iterator object_it;
	for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
		if ((*object_it).finished() == true) {
			object_list.erase(object_it);
			break;
		} else {
            (*object_it).execute(paused); /// @TODO: must pass scroll map to npcs somwhow...
		}
    }
}

void classMap::clean_finished_objects()
{
    std::vector<object>::iterator object_it;
    std::vector<object> kept_object_list;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        if ((*object_it).finished() == false) {
            kept_object_list.push_back(*object_it);
        }
    }
    object_list = kept_object_list;
}

std::vector<object*> classMap::check_collision_with_objects(st_rectangle collision_area)
{
    std::vector<object*> res;

    for (unsigned int i=0; i<object_list.size(); i++) {
        object* temp_obj = &object_list.at(i);
        collision_detection rect_collision_obj;
        bool res_collision = rect_collision_obj.rect_overlap(temp_obj->get_area(), collision_area);
        if (res_collision == true) {
            res.push_back(temp_obj);
        }
    }
    return res;
}

void classMap::show_objects(int adjust_y, int adjust_x)
{
    /// @TODO - update timers
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        if ((*object_it).get_type() != OBJ_STAGE_BOSS_TELEPORTER && (*object_it).get_type() != OBJ_BOSS_TELEPORTER && (*object_it).get_type() != OBJ_FINAL_BOSS_TELEPORTER) { // teleporters are shown above
            (*object_it).show(adjust_y, adjust_x); // TODO: must pass scroll map to objects somwhow...
        }
    }
}

void classMap::show_above_objects(int adjust_y, int adjust_x)
{
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        if ((*object_it).get_type() == OBJ_STAGE_BOSS_TELEPORTER || (*object_it).get_type() == OBJ_BOSS_TELEPORTER || (*object_it).get_type() == OBJ_FINAL_BOSS_TELEPORTER || (*object_it).get_type() == OBJ_BOSS_DOOR) { // teleporters are shown above
            (*object_it).show(adjust_y, adjust_x); // TODO: must pass scroll map to objects somwhow...
        }
    }
}

bool classMap::boss_hit_ground(classnpc* npc_ref)
{
    if (npc_ref->is_boss() == true && npc_ref->is_on_visible_screen() == true) {
        int limit_y = npc_ref->get_start_position().y - TILESIZE;

        if (limit_y > RES_H/2) {
            limit_y = RES_H/2;
        }
        if (npc_ref->get_can_fly()) {
            limit_y = RES_H/2 - npc_ref->get_size().height/2;
        }

        if (npc_ref->getPosition().y >= limit_y) {
            // flying boss can stop on middle of the screen
            if (npc_ref->get_can_fly() == true) {
                npc_ref->set_animation_type(ANIM_TYPE_WALK_AIR);
                return true;
            // non-flying bosses need to hit gound to stop
            } else if (npc_ref->hit_ground() == true) {
                npc_ref->set_animation_type(ANIM_TYPE_STAND);
                return true;
            }
        }
    }
    return false;
}

// return false when still playing and true when unable or done
bool classMap::boss_show_intro_sprites(classnpc *npc_ref)
{
    if (npc_ref->have_frame_graphic(npc_ref->get_direction(), ANIM_TYPE_INTRO, 0) == false) {
        return true;
    }
    if (npc_ref->get_anim_type() != ANIM_TYPE_INTRO) {
        npc_ref->set_animation_type(ANIM_TYPE_INTRO);
        return false;
    }
    if (!npc_ref->is_on_last_animation_frame()) {
        return false;
    }
    if (npc_ref->get_can_fly() == true) {
        npc_ref->set_animation_type(ANIM_TYPE_WALK_AIR);
    } else {
        npc_ref->set_animation_type(ANIM_TYPE_STAND);
    }
    return true;
}

classnpc *classMap::get_near_boss()
{
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_boss() == true && npc_ref->is_on_visible_screen() == true) {
            return npc_ref;
        }
    }
    return NULL;
}

bool classMap::is_boss_on_extended_screen()
{
    std::vector<classnpc>::iterator npc_it;
    for (npc_it = _npc_list.begin(); npc_it != _npc_list.end(); npc_it++) {
        classnpc* npc_ref = &(*npc_it);
        if (npc_ref->is_boss() == true && npc_ref->is_on_screen() == true && !npc_ref->is_dead()) {
            return true;
        }
    }
    return false;
}

void classMap::reset_map_npcs()
{
	load_map_npcs();
}







