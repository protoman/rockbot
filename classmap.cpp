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
#include "colision_detection.h"
#include "file/file_io.h"
#include "game_mediator.h"

extern string FILEPATH;
extern graphicsLib graphLib;

#include "game.h"
extern game gameControl;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "file/file_io.h"
extern CURRENT_FILE_FORMAT::file_io fio;


extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::st_save game_save;
extern CURRENT_FILE_FORMAT::file_map map_data[FS_STAGE_MAX_MAPS];

extern CURRENT_FILE_FORMAT::st_save game_save;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap::classMap() : stage_number(-1), number(-1), bg1_scroll(st_float_position(0.0, 0.0)), bg2_scroll(st_float_position(0.0, 0.0)), _platform_leave_counter(0)
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
    _break_npc_loop = false;
    bg1_surface.width = 0;
    bg2_surface.width = 0;

}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap::~classMap()
{
    //std::cout << "map[" << number << "] destructor" << std::endl;
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
	//std::cout << "classMap::setStageNumber - setStageN: " << setStageN << std::endl;
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
		cout << "ERROR::map::loadMap - stage number was not set, can't load it before setting the number.\n";
		return;
	}
	if (number == -1) {
		cout << "ERROR::map::loadMap - map number was not set, can't load it before setting the number.\n";
		return;
    }

    _level3_tiles.clear();
    object_list.clear();

    _npc_list.clear();
    animation_list.clear();

    for (int i=0; i<MAP_W; i++) {
        for (int j=0; j<MAP_H; j++) {
            int lvl3_x = map_data[number].tiles[i][j].tile3.x;
            int lvl3_y = map_data[number].tiles[i][j].tile3.y;
            if (lvl3_x > -1 && lvl3_y != -1) {
                struct st_level3_tile temp_tile(st_position(lvl3_x, lvl3_y), st_position(i, j));
                _level3_tiles.push_back(temp_tile);
            } else if (lvl3_x < -1 && lvl3_y == 0) { // anim tiles
                lvl3_y = j*TILESIZE;
                struct st_level3_tile temp_tile(st_position(lvl3_x, lvl3_y), st_position(i, j));
                _level3_tiles.push_back(temp_tile);
            }
        }
    }

	bool column_locked = true;
	for (int i=0; i<MAP_W; i++) {
		column_locked = true;
		for (int j=0; j<MAP_H; j++) {
            if (map_data[number].tiles[i][j].locked != TERRAIN_SOLID && map_data[number].tiles[i][j].locked != TERRAIN_DOOR && map_data[number].tiles[i][j].locked != TERRAIN_SCROLL_LOCK && map_data[number].tiles[i][j].locked != TERRAIN_ICE && map_data[number].tiles[i][j].locked != TERRAIN_SPIKE) {
				column_locked = false;
				break;
			}
		}
		wall_scroll_lock[i] = column_locked;
	}

    _level3_tiles.clear();

    for (int i=0; i<MAP_W; i++) {
        for (int j=0; j<MAP_H; j++) {
            int lvl3_x = map_data[number].tiles[i][j].tile3.x;
            int lvl3_y = map_data[number].tiles[i][j].tile3.y;
            if (lvl3_x != -1 && lvl3_y != -1) {
                struct st_level3_tile temp_tile(st_position(lvl3_x, lvl3_y), st_position(i, j));
                _level3_tiles.push_back(temp_tile);
            }
        }
    }

	load_map_npcs();

    load_map_objects();

    create_dynamic_background_surfaces();

#ifdef HANDLELD // portable consoles aren't strong enought for two dynamic backgrounds
map_data[number].backgrounds[0].speed = 0;
#endif

}


void classMap::showMap()
{
    graphLib.clear_surface_area(0, 0, RES_W, RES_H, map_data[number].background_color.r, map_data[number].background_color.g, map_data[number].background_color.b, graphLib.gameScreen);
    draw_dynamic_backgrounds();
    int tile_x_ini = scroll.x/TILESIZE-1;
    if (tile_x_ini < 0) {
        tile_x_ini = 0;
    }

    // draw the tiles of the screen region
    struct st_position pos_origin;
    struct st_position pos_destiny;
    int n = -1;
    for (int i=tile_x_ini; i<tile_x_ini+(RES_W/TILESIZE)+2; i++) {
        int diff = scroll.x - (tile_x_ini+1)*TILESIZE;
        pos_destiny.x = n*TILESIZE - diff;
        for (int j=0; j<MAP_H; j++) {

            // don't draw easy-mode blocks if game difficulty not set to easy
            game_save.difficulty = DIFFICULTY_HARD;

            if (map_data[number].tiles[i][j].locked == TERRAIN_EASYMODEBLOCK && game_save.difficulty == DIFFICULTY_EASY) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_easymode_block_tile(pos_destiny, graphLib.gameScreen);
            } else if (map_data[number].tiles[i][j].locked == TERRAIN_HARDCODEBLOCK && game_save.difficulty == DIFFICULTY_HARD) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_hardmode_block_tile(pos_destiny, graphLib.gameScreen);
            } else {
                pos_origin.x = map_data[number].tiles[i][j].tile1.x;
                pos_origin.y = map_data[number].tiles[i][j].tile1.y;

                if (pos_origin.x >= 0 && pos_origin.y >= 0) {
                    pos_destiny.y = j*TILESIZE;
                    graphLib.placeTile(pos_origin, pos_destiny, &graphLib.gameScreen);
                } else if (pos_origin.x < -1 && pos_origin.y == 0) {
                    int anim_tile_id = (pos_origin.x * -1) - 2;
                    pos_destiny.y = j*TILESIZE;
                    //std::cout << "MAP::showMap::place_anim_tile[" << i << "][" << j << "]" << std::endl;
                    graphLib.place_anim_tile(anim_tile_id, pos_destiny, &graphLib.gameScreen);
                }
            }
        }
        n++;
    }


    graphLib.update_anim_tiles_timers();
    draw_lib.show_gfx();
}


// ********************************************************************************************** //
// show the third level of tiles                                                                  //
// ********************************************************************************************** //
void classMap::showAbove(int scroll_y, int temp_scroll_x)
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
    //std::cout << "showAbove - start_point: " << start_point << ", end_point: " << end_point << std::endl;


	// draw 3rd tile level
    std::vector<st_level3_tile>::iterator tile3_it;
    for (tile3_it = _level3_tiles.begin(); tile3_it != _level3_tiles.end(); tile3_it++) {

        if (_3rd_level_ignore_area.x != -1 && _3rd_level_ignore_area.w > 0 && ((*tile3_it).map_position.x >= _3rd_level_ignore_area.x && (*tile3_it).map_position.x < _3rd_level_ignore_area.x+_3rd_level_ignore_area.w && (*tile3_it).map_position.y >= _3rd_level_ignore_area.y && (*tile3_it).map_position.y < _3rd_level_ignore_area.y+_3rd_level_ignore_area.h)) {
            continue;
        }
        graphLib.place_3rd_level_tile((*tile3_it).tileset_pos.x, (*tile3_it).tileset_pos.y, ((*tile3_it).map_position.x*TILESIZE)-scroll_x, ((*tile3_it).map_position.y*TILESIZE)+scroll_y);
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
            //std::cout << ">> MAP::showAbove::HIDE_BUBBLE <<" <<std::endl;
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
	if (pos.x < 0 || pos.y < 0 || pos.y > RES_H/TILESIZE || pos.x > MAP_W) {
		return TERRAIN_UNBLOCKED;
	}
    return map_data[number].tiles[pos.x][pos.y].locked;
}

st_position_int8 classMap::get_map_point_tile1(st_position pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.y > RES_H/TILESIZE || pos.x > MAP_W) {
        return st_position_int8(-1, -1);
    }
    return map_data[number].tiles[pos.x][pos.y].tile1;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::changeScrolling(st_float_position pos, bool check_lock)
{

    if (timer.is_paused() == true) {
        return;
    }

    //std::cout << "MAP::changeScrolling::timer: " << timer.getTimer() << ", pos.x: " << pos.x << std::endl;

    float bg1_speed = (float)map_data[number].backgrounds[0].speed/10;
    float bg2_speed = (float)map_data[number].backgrounds[1].speed/10;

	// moving player to right, screen to left
	if (pos.x > 0 && ((scroll.x/TILESIZE+RES_W/TILESIZE)-1 < MAP_W)) {
        int x_change = pos.x;
		if (pos.x >= TILESIZE) { // if change is too big, do not update (TODO: must check all wall until lock)
            x_change = 1;
		}
		int tile_x = (scroll.x+RES_W-TILESIZE+2)/TILESIZE;
		if (check_lock == false || wall_scroll_lock[tile_x] == false) {
            scroll.x += x_change;
            if (map_data[number].backgrounds[0].auto_scroll == BG_SCROLL_MODE_NONE) {
                bg1_scroll.x -= ((float)x_change*bg1_speed);
            }
            if (map_data[number].backgrounds[0].auto_scroll == BG_SCROLL_MODE_NONE) {
                bg2_scroll.x -= ((float)x_change*bg2_speed);
            }
            adjust_dynamic_background_position();
		}
	} else if (pos.x < 0) {
		int x_chance = pos.x;
		if (pos.x < -TILESIZE) {
            x_chance = -1;
		}
		if (scroll.x/TILESIZE >= 0) { // if change is too big, do not update (TODO: must check all wall until lock)
			int tile_x = (scroll.x+TILESIZE-2)/TILESIZE;
            //std::cout << "#2 LEFT changeScrolling - scroll.x: " << scroll.x << ", testing tile_x: " << tile_x << std::endl;
			if (check_lock == false || wall_scroll_lock[tile_x] == false) {
				//std::cout << "classMap::changeScrolling - 2" << std::endl;
                scroll.x += x_chance;
                bg1_scroll.x -= ((float)x_chance*bg1_speed);
                if (bg1_scroll.x > 0) {
                    bg1_scroll.x = -RES_W; // erro aqui
                }
                bg2_scroll.x -= ((float)x_chance*bg2_speed);
                if (bg2_scroll.x > 0) {
					//std::cout << "LEFT RESET BG2POS" << bg2_scroll << std::endl;
                    bg2_scroll.x = -RES_W;
				}
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
    //std::cout << "------- classMap::set_scrolling - map: " << number << ", pos.x: " << pos.x << "-------" << std::endl;
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
    //std::cout << "getMapScrolling, x: " << scroll.x << ", y: " << scroll.y << std::endl;
    return scroll;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classMap::load_map_npcs()
{
    // remove all elements currently in the list
    while (!_npc_list.empty()) {
        _npc_list.back().clean_character_graphics_list();
        _npc_list.pop_back();
    }

    //std::cout << "classmap::load_map_npcs - stage: " << stage_number << ", map: " << number << std::endl;

	for (int i=0; i<MAX_MAP_NPC_N; i++) {
        if (map_data[number].map_npcs[i].id_npc != -1) {
            classnpc new_npc = classnpc(stage_number, number, map_data[number].map_npcs[i].id_npc, i);
            new_npc.set_map(this);

            if (stage_data.boss.id_npc == map_data[number].map_npcs[i].id_npc) {
                new_npc.set_stage_boss(true);
            } else if (GameMediator::get_instance()->get_enemy(map_data[number].map_npcs[i].id_npc).is_boss == true) {
                new_npc.set_is_boss(true);
            }

            _npc_list.push_back(new_npc); // insert new npc at the list-end
        }
	}
}


void classMap::draw_dynamic_backgrounds()
{

    float bg1_speed = (float)map_data[number].backgrounds[0].speed/10;
    int bg1_scroll_mode = map_data[number].backgrounds[0].auto_scroll;
    if (bg1_scroll_mode == BG_SCROLL_MODE_LEFT) {
        bg1_scroll.x -= ((float)1*bg1_speed);
    } else if (bg1_scroll_mode == BG_SCROLL_MODE_RIGHT) {
        bg1_scroll.x += ((float)1*bg1_speed);
    } else if (bg1_scroll_mode == BG_SCROLL_MODE_UP) {
        bg1_scroll.y -= ((float)1*bg1_speed);
    } else if (bg1_scroll_mode == BG_SCROLL_MODE_DOWN) {
        bg1_scroll.y += ((float)1*bg1_speed);
    }

    float bg2_speed = (float)map_data[number].backgrounds[1].speed/10;
    int bg2_scroll_mode = map_data[number].backgrounds[1].auto_scroll;
    if (bg2_scroll_mode == BG_SCROLL_MODE_LEFT) {
        bg2_scroll.x -= ((float)1*bg2_speed);
    } else if (bg2_scroll_mode == BG_SCROLL_MODE_RIGHT) {
        bg2_scroll.x += ((float)1*bg2_speed);
    } else if (bg2_scroll_mode == BG_SCROLL_MODE_UP) {
        bg2_scroll.y -= ((float)1*bg2_speed);
    } else if (bg2_scroll_mode == BG_SCROLL_MODE_DOWN) {
        bg2_scroll.y += ((float)1*bg2_speed);
    }

    adjust_dynamic_background_position();

    int x1 = bg1_scroll.x;
    if (x1 > 0) { // moving to right
        x1 = (RES_W - x1) * -1;
    }
    int x2 = bg2_scroll.x;
    if (x2 > 0) { // moving to right
        x2 = (RES_W - x2) * -1;
    }

    //std::cout << "## bg1_scroll.y: " << bg1_scroll.y << std::endl;

    int y1 = bg1_scroll.y + map_data[number].backgrounds[0].adjust_y;
    int y2 = bg2_scroll.y + map_data[number].backgrounds[1].adjust_y;

    if (bg1_surface.width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(x1, y1), &bg1_surface, &graphLib.gameScreen);
    }


    if (bg2_surface.width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(bg2_scroll.x, y2), &bg2_surface, &graphLib.gameScreen);
    }
}

void classMap::adjust_dynamic_background_position()
{
    if (bg1_scroll.x < -RES_W) {
        bg1_scroll.x = 0;
    } else if (bg1_scroll.x > RES_W) {
        bg1_scroll.x = 0;
    }
    if (bg2_scroll.x < -RES_W) {
        bg2_scroll.x = 0;
    } else if (bg2_scroll.x > RES_W) {
        bg2_scroll.x = 0;
    }

    if (bg1_scroll.y < -RES_H) {
        bg1_scroll.y = 0;
    } else if (bg1_scroll.y > RES_H) {
        bg1_scroll.y = 0;
    }
    if (bg2_scroll.y < -RES_H) {
        bg2_scroll.y = 0;
    } else if (bg2_scroll.y > RES_H) {
        bg2_scroll.y = 0;
    }
}


void classMap::draw_dynamic_backgrounds_into_surface(graphicsLib_gSurface &surface)
{
    //std::cout << "MAP::draw_dynamic_backgrounds_into_surface - color: (" << map_data[number].background_color.r << ", " << map_data[number].background_color.g << ", " << map_data[number].background_color.b << ")" << std::endl;
    graphLib.clear_surface_area(0, 0, surface.width, surface.height, map_data[number].background_color.r, map_data[number].background_color.g, map_data[number].background_color.b, surface);
    if (bg1_surface.width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(bg1_scroll.x, bg1_scroll.y+map_data[number].backgrounds[0].adjust_y), &bg1_surface, &surface);
    }


    if (bg2_surface.width > 0) {
        // draw leftmost part
        graphLib.copyAreaWithAdjust(st_position(bg2_scroll.x, bg2_scroll.y+map_data[number].backgrounds[1].adjust_y), &bg2_surface, &surface);
    }
}

void classMap::add_object(object obj)
{
    object_list.push_back(obj);
}

int classMap::get_first_lock_on_left(int x_pos) const
{
    for (int i=x_pos; i>= 0; i--) {
        if (wall_scroll_lock[i] == true) {
            return i*TILESIZE;
        }
    }
    return -1;
}

int classMap::get_first_lock_on_right(int x_pos) const
{
    int limit = (scroll.x+RES_W)/TILESIZE;
    x_pos += 1;
    std::cout << "classMap::get_first_lock_on_right - x_pos: " << x_pos << ", limit: " << limit << std::endl;
    for (int i=x_pos; i<=limit; i++) {
        if (wall_scroll_lock[i] == true) {
            std::cout << "classMap::get_first_lock_on_right - found lock at: " << i << std::endl;
            return i*TILESIZE;
        }
    }
    return -1;
}

void classMap::drop_item(st_position position)
{
    int rand_n = rand() % 100;
    //std::cout << ">>>>>>> classMap::drop_item - rand_n: " << rand_n << std::endl;
    if (rand_n <= 35) {
        st_position obj_pos;
        obj_pos.y = position.y/TILESIZE;
        obj_pos.x = (position.x - TILESIZE)/TILESIZE;
        DROP_ITEMS_LIST obj_type;
        if (rand_n < 15) {
            //std::cout << ">>>>>>> classMap::drop_item - DROP_ITEM_ENERGY_SMALL" << std::endl;
            obj_type = DROP_ITEM_ENERGY_SMALL;
        } else if (rand_n < 25) {
            //std::cout << ">>>>>>> classMap::drop_item - DROP_ITEM_WEAPON_SMALL" << std::endl;
            obj_type = DROP_ITEM_WEAPON_SMALL;
        } else if (rand_n < 30) {
            //std::cout << ">>>>>>> classMap::drop_item - DROP_ITEM_ENERGY_BIG" << std::endl;
            obj_type = DROP_ITEM_ENERGY_BIG;
        } else {
            //std::cout << ">>>>>>> classMap::drop_item - DROP_ITEM_WEAPON_BIG" << std::endl;
            obj_type = DROP_ITEM_WEAPON_BIG;
        }
        short obj_type_n = gameControl.get_drop_item_id(obj_type);
        if (obj_type_n == -1) {
            std::cout << ">>>>>>>>> obj_type_n(" << obj_type_n << ") invalid for obj_type(" << obj_type << ")" << std::endl;
            return;
        }
        object temp_obj(obj_type_n, this, obj_pos, st_position(-1, -1), -1);
        temp_obj.set_duration(4500);
        add_object(temp_obj);
    }
}

void classMap::set_bg1_scroll(int scrollx)
{
    bg1_scroll.x = scrollx;
}

void classMap::set_bg2_scroll(int scrollx)
{
    bg2_scroll.x = scrollx;
}

int classMap::get_bg1_scroll() const
{
    return bg1_scroll.x;
}

int classMap::get_bg2_scroll() const
{
    return bg2_scroll.x;
}

void classMap::reset_objects_timers()
{
    std::cout << ">>>>>> MAP::reset_objects_timers - object_list.size: " << object_list.size() << std::endl;
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        (*object_it).reset_timers(); // TODO: must pass scroll map to npcs somwhow...
    }
}

void classMap::reset_objects()
{
    //std::cout << ">>>>>> MAP::reset_objects - object_list.size: " << object_list.size() << std::endl;
    std::vector<object>::iterator object_it;
    for (object_it = object_list.begin(); object_it != object_list.end(); object_it++) {
        (*object_it).reset();
    }
}

void classMap::print_objects_number()
{
    //std::cout << ">>>>>> MAP::print_objects_number - n: " << object_list.size() << std::endl;
}

void classMap::add_bubble_animation(st_position pos)
{
    if (_water_bubble.timer > timer.getTimer()) {
        //std::cout << ">> MAP::add_bubble::CANT_ADD <<" <<std::endl;
        return;
    }
    //std::cout << ">> MAP::add_bubble::ADDED <<" <<std::endl;
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

bool classMap::subboss_alive_on_left(short tileX)
{
    for (int i=0; i<_npc_list.size(); i++) {
        if (_npc_list.at(i).is_subboss() == true && _npc_list.at(i).is_dead() == false) {
            std::cout << "Opa, achou um sub-boss!" << std::endl;
            std::cout << "pos.x: " << _npc_list.at(i).getPosition().x << ", tileX*TILESIZE: " << tileX*TILESIZE << std::endl;
            if (_npc_list.at(i).getPosition().x >= (tileX-20)*TILESIZE && _npc_list.at(i).getPosition().x<= tileX*TILESIZE) { // 20 tiles is the size of a visible screen
                std::cout << "Opa, achou um sub-boss NA ESQUERDA!!" << std::endl;
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
        std::cout << "number: " << number << ", obj.id: " << temp_obj.get_obj_map_id() << std::endl;
        if (temp_obj.get_obj_map_id() == number) {
            temp_obj.set_direction(ANIM_DIRECTION_RIGHT);
        }
    }
}

void classMap::activate_final_boss_teleporter()
{
    for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
        object& temp_obj = (*it);
        std::cout << "number: " << number << ", obj.id: " << temp_obj.get_obj_map_id() << ", type: " << temp_obj.get_type() << ", OBJ_FINAL_BOSS_TELEPORTER: " << OBJ_FINAL_BOSS_TELEPORTER << std::endl;
        if (temp_obj.get_type() == OBJ_FINAL_BOSS_TELEPORTER) {
            temp_obj.start();
        }
    }
}

Uint8 classMap::get_map_gfx()
{
    //std::cout << ">> MAP::get_map_gfx::number: " << number << std::endl;
    return map_data[number].backgrounds[0].gfx;
}

st_float_position classMap::get_bg1_scroll()
{
    return bg1_scroll;
}

st_float_position classMap::get_bg2_scroll()
{
    return bg2_scroll;
}

void classMap::set_bg1_scroll(st_float_position pos)
{
    bg1_scroll = pos;
}

void classMap::set_bg2_scroll(st_float_position pos)
{
    bg2_scroll = pos;
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

	for (int i=0; i<MAX_MAP_NPC_N; i++) {
        if (map_data[number].map_objects[i].id_object != -1) {
            //int temp_id = map_data[number].map_objects[i].id_object;
            object temp_obj(map_data[number].map_objects[i].id_object, this, map_data[number].map_objects[i].start_point, map_data[number].map_objects[i].link_dest, map_data[number].map_objects[i].map_dest);
            temp_obj.set_obj_map_id(i);
            temp_obj.set_direction(map_data[number].map_objects[i].direction);
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
    graphicsLib_gSurface temp_surface;
    if (strlen(map_data[number].backgrounds[0].filename) > 0) {
        std::string bg1_filename(FILEPATH+"images/map_backgrounds/" + map_data[number].backgrounds[0].filename);

        std::cout << "bg1_filename: '" << bg1_filename << "'" << std::endl;

        graphLib.surfaceFromFile(bg1_filename, &temp_surface);
        create_dynamic_background_surface(bg1_surface, temp_surface);
    } else {
        bg1_surface.freeGraphic();
    }

    if (strlen(map_data[number].backgrounds[1].filename) > 0) {
        std::string bg2_filename(FILEPATH+"images/map_backgrounds/"+ map_data[number].backgrounds[1].filename);
        graphLib.surfaceFromFile(bg2_filename, &temp_surface);
        create_dynamic_background_surface(bg2_surface, temp_surface);
    } else {
        bg2_surface.freeGraphic();
    }
}

void classMap::create_dynamic_background_surface(graphicsLib_gSurface &dest_surface, graphicsLib_gSurface &image_surface) const
{
    // initialize dest_surface
    int n = 0;

    //map_data[number].backgrounds[0].
    if (map_data[number].backgrounds[0].auto_scroll == BG_SCROLL_MODE_UP || map_data[number].backgrounds[0].auto_scroll == BG_SCROLL_MODE_DOWN) {
        graphLib.initSurface(st_size(RES_W, RES_H*2), &dest_surface);
        int total_h = 0;
        while (total_h <= RES_H*2) {
            graphLib.copyArea(st_position(0, total_h), &image_surface, &dest_surface);
            total_h += image_surface.height;
            n++;
        }
    } else {
        graphLib.initSurface(st_size(RES_W*2, RES_H), &dest_surface);
        int total_w = 0;
        while (total_w <= RES_W*2) {
            graphLib.copyArea(st_position(total_w, 0), &image_surface, &dest_surface);
            total_w += image_surface.width;
            n++;
        }
    }
}


int classMap::colision_rect_player_obj(character* playerObj, object* temp_obj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y, const short obj_xinc, const short obj_yinc)
{
    int blocked = 0;
    int obj_y_reducer = 2;
    colision_detection rect_colision_obj;

    st_rectangle obj_rect(temp_obj->get_position().x+obj_xinc, temp_obj->get_position().y+obj_yinc+obj_y_reducer, temp_obj->get_size().width, temp_obj->get_size().height);
    st_rectangle p_rect(playerObj->getPosition().x+x_inc+reduce_x/2-1, playerObj->getPosition().y+reduce_y+y_inc, playerObj->get_size().width-reduce_x/2-1, playerObj->get_size().height-reduce_y);

    bool xObjOver = value_in_range(obj_rect.x, p_rect.x, p_rect.x + p_rect.w);
    bool xPlayerOver = value_in_range(p_rect.x, obj_rect.x, obj_rect.x + obj_rect.w);
    bool xOverlap = xObjOver == true || xPlayerOver == true;
    bool yOverlap = value_in_range(obj_rect.y, p_rect.y, p_rect.y + p_rect.h) || value_in_range(p_rect.y, obj_rect.y, obj_rect.y + obj_rect.h);

    //std::cout << "x_inc: " << x_inc << ", y_inc: " << y_inc << ", xOverlap: " << xOverlap << ", yOverlap: " << yOverlap << std::endl;
    //std::cout << "obj_rect.x: " << obj_rect.x << ", p.x+p.w: " << (p_rect.x + p_rect.w) << ", xOverlap: " << xOverlap << ", yOverlap: " << yOverlap << std::endl;


    // check if X is blocked
    bool before_colision = rect_colision_obj.rect_overlap(obj_rect, p_rect);
    if (before_colision == true && temp_obj->get_colision_mode() != COLISION_MODE_Y) {
        //std::cout << ">>>>>>>>> BLOCK 2 - classMap::colision_player_object - py: " << (playerObj->getPosition().y+reduce_y) << ", objy+h: " << (obj_rect.y+obj_rect.h) << std::endl;
        blocked = BLOCK_X;
    }

    if (xOverlap == true && yOverlap == true) {
        if (blocked == 0) {
            blocked = BLOCK_Y;
        } else {
            blocked = BLOCK_XY;
        }

    }
    return blocked;
}


void classMap::colision_char_object(character* charObj, const float x_inc, const short int y_inc, short int reduce_x, short int reduce_y)
{
    int blocked = 0;
    object* res_obj = NULL;

    //if (y_inc < 0) std::cout << "MAP::colision_player_object - y_inc: " << y_inc << std::endl;


    if (charObj->get_platform() == NULL) {
        for (std::vector<object>::iterator it=object_list.begin(); it!=object_list.end(); it++) {
            object& temp_obj = (*it);

            if (temp_obj.is_hidden() == true) {
                //std::cout << "obj[" << temp_obj.get_name() << "] - leave #1" << std::endl;
                continue;
            }

            if (temp_obj.is_on_screen() == false) {
                //std::cout << "obj[" << temp_obj.get_name() << "] - leave #2" << std::endl;
                continue;
            }

            if (temp_obj.finished() == true) {
                continue;
            }

            //if (y_inc < 0) std::cout << "MAP::colision_player_object - DEBUG #1" << std::endl;

            // usar TEMP_BLOCKED aqui, para não zerar o blocked anterior, fazer merge dos valores
            int temp_blocked = 0;
            if (y_inc >= 0 || (temp_obj.get_type() != OBJ_ITEM_FLY && temp_obj.get_type() != OBJ_ITEM_JUMP)) { // jumping up on items does not block
                temp_blocked = colision_rect_player_obj(charObj, &temp_obj, x_inc, y_inc, reduce_x, reduce_y, 0, 0);
                //std::cout << "temp_blocked[" << temp_obj.get_name() << "]: " << temp_blocked << std::endl;
            }

            // to enter platform, player.x+player.h must not be much higher than obj.y
            if (temp_obj.get_type() == OBJ_ITEM_FLY || temp_obj.get_type() == OBJ_ITEM_JUMP || temp_obj.get_type() == OBJ_ACTIVE_DISAPPEARING_BLOCK || temp_obj.get_type() == OBJ_FALL_PLATFORM || temp_obj.get_type() == OBJ_FLY_PLATFORM || temp_obj.get_type() == OBJ_MOVING_PLATFORM_LEFTRIGHT || temp_obj.get_type() == OBJ_MOVING_PLATFORM_UPDOWN) {
                if (charObj->getPosition().y+charObj->get_size().height-2 > temp_obj.get_position().y) {
                    temp_blocked = 0;
                }
            }


            if (temp_blocked == BLOCK_Y || temp_blocked == BLOCK_XY) {

                bool entered_platform = false;

                if (temp_obj.get_state() != 0 && temp_obj.get_type() == OBJ_TRACK_PLATFORM) {
                    temp_blocked = 0;
                    continue;
                }

                if (temp_obj.get_state() != 0 && (temp_obj.get_type() == OBJ_RAY_VERTICAL || temp_obj.get_type() == OBJ_RAY_HORIZONTAL)) {
                    charObj->damage(TOUCH_DAMAGE_BIG, false);
                    temp_blocked = 0;
                    continue;
                } else if (temp_obj.get_state() != 0 && (temp_obj.get_type() == OBJ_DEATHRAY_VERTICAL || temp_obj.get_type() == OBJ_DEATHRAY_HORIZONTAL)) {
                    std::cout << "DEATHRAY(damage) - player.x: " << charObj->getPosition().x << ", map.scroll_x: " << scroll.x << ", pos.x: " << temp_obj.get_position().x << ", size.w: " << temp_obj.get_size().width << std::endl;
                    charObj->damage(999, false);
                    temp_blocked = 0;
                    continue;
                }

                if (y_inc > 0 && charObj->getPosition().y <= temp_obj.get_position().y) {
                    //std::cout << ">>>>>>>> entered_platform!!!!!!! <<<<< classmap::colision_player_object - obj_rect.x: " << obj_rect.x << ", obj_rect.y: " << obj_rect.y << ", obj_rect.w: " << obj_rect.w << ", obj_rect.h: " << obj_rect.h << std::endl;
                    entered_platform = true;
                }


                if (entered_platform == true) {
                    //std::cout << "player.platform: " << playerObj->get_platform() << std::endl;


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
                            _obj_colision = object_colision(temp_blocked, &(*it));
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
                            _obj_colision = object_colision(temp_blocked, &(*it));
                            return;
                        }
                    } else if (temp_obj.get_type() == OBJ_ITEM_JUMP) {
                        if (charObj->get_platform() == NULL && (temp_blocked == 2 || temp_blocked == 3) && y_inc > 0 && charObj->getPosition().y+charObj->get_size().height <= temp_obj.get_position().y+1) {
                            charObj->activate_super_jump();
                            charObj->activate_force_jump();
                            temp_obj.start();
                        }
                        if (temp_blocked != 0) {
                            if (y_inc > 0) {
                                //std::cout << ">>>> temp_blocked: " << temp_blocked << ", y_inc: " << y_inc << std::endl;
                                _obj_colision = object_colision(temp_blocked, &(*it));
                                return;
                            } else {
                                std::cout << ">>>> RESET BLOCKED" <<  std::endl;
                                temp_blocked = 0;
                            }
                        }
                    } else if (temp_obj.is_hidden() == false && temp_obj.is_started() == false && temp_obj.get_type() == OBJ_ACTIVE_DISAPPEARING_BLOCK) {
                        temp_obj.start();
                    } else if (temp_obj.get_type() == OBJ_FALL_PLATFORM || temp_obj.get_type() == OBJ_FLY_PLATFORM) {
                        if (charObj->get_platform() == NULL) {
                            charObj->set_platform(&temp_obj);
                            if (temp_obj.get_state() == OBJ_STATE_STAND) {
                                temp_obj.set_state(OBJ_STATE_MOVE);
                                temp_obj.start();
                            }
                            temp_obj.set_timer(timer.getTimer()+30);
                            _obj_colision = object_colision(temp_blocked, &(*it));
                            return;
                        }
                    } else if (temp_obj.get_type() == OBJ_TRACK_PLATFORM) {
                        if (charObj->get_platform() == NULL) {
                            charObj->set_platform(&temp_obj);
                            _obj_colision = object_colision(temp_blocked, &(*it));
                            return;
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
            blocked = colision_rect_player_obj(charObj, temp_obj, x_inc, y_inc, reduce_x, reduce_y, 0, 0);
        }
    }


    // got out of platform
    if (blocked == 0 && charObj->get_platform() != NULL) {
        //  for player item, platform must only be removed only if the item was already adtivated
        if (charObj->get_platform()->get_type() == OBJ_ITEM_FLY || charObj->get_platform()->get_type() == OBJ_ITEM_JUMP) {
            //std::cout << "y_inc: " << y_inc << ", playerObj->get_platform()->get_distance(): " << playerObj->get_platform()->get_distance() << std::endl;
            if (charObj->get_platform()->get_distance() > 0 && y_inc != 0) {
                charObj->set_platform(NULL);
            } else {
                _obj_colision = object_colision(0, NULL);
                return;
            }
        } else if (charObj->get_platform()->is_hidden() == true) {
            //std::cout << ">> OUT OF PLATFORM #2" << std::endl;
            charObj->set_platform(NULL);
        } else {
            _platform_leave_counter++;
            if (_platform_leave_counter > 2) {
                //std::cout << ">> OUT OF PLATFORM #3" << std::endl;
                charObj->set_platform(NULL);
                _platform_leave_counter = 0;
            }
        }
    } else if (blocked != 0 && charObj->get_platform() != NULL) {
        _platform_leave_counter = 0;
    }

    _obj_colision = object_colision(blocked, res_obj);
}

object_colision classMap::get_obj_colision()
{
    return _obj_colision;
}




void classMap::clean_map_npcs_projectiles()
{
    for (int i=0; i<_npc_list.size(); i++) {
        _npc_list.at(i).clean_projectiles();
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

graphicsLib_gSurface classMap::get_map_area_surface()
{
    //@TODO - create a new surface, because we need that only for transition screen
    graphicsLib_gSurface mapSurface;
    graphLib.initSurface(st_size(RES_W, RES_H), &mapSurface);

    if (!mapSurface.get_surface()) {
        graphLib.show_debug_msg("EXIT #21.MALLOC");
        exit(-1);
    }

    graphLib.clear_surface_area(0, 0, RES_W, RES_H, map_data[number].background_color.r, map_data[number].background_color.g, map_data[number].background_color.b, mapSurface);

    draw_dynamic_backgrounds_into_surface(mapSurface);

    int tile_x_ini = scroll.x/TILESIZE-1;

    // draw the tiles of the screen region
    struct st_position pos_origin;
    struct st_position pos_destiny;
    int n = -1;
    for (int i=tile_x_ini; i<tile_x_ini+(RES_W/TILESIZE)+2; i++) {
        int diff = scroll.x - (tile_x_ini+1)*TILESIZE;
        pos_destiny.x = n*TILESIZE - diff;
        for (int j=0; j<MAP_H; j++) {
            /*
            pos_origin.x = map_data[number].tiles[i][j].tile1.x;
            pos_origin.y = map_data[number].tiles[i][j].tile1.y;
            pos_destiny.y = j*TILESIZE;
            graphLib.placeTile(pos_origin, pos_destiny, &mapSurface);
            */


            // don't draw easy-mode blocks if game difficulty not set to easy
            game_save.difficulty = DIFFICULTY_HARD;

            if (map_data[number].tiles[i][j].locked == TERRAIN_EASYMODEBLOCK && game_save.difficulty == DIFFICULTY_EASY) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_easymode_block_tile(pos_destiny, mapSurface);
            } else if (map_data[number].tiles[i][j].locked == TERRAIN_HARDCODEBLOCK && game_save.difficulty == DIFFICULTY_HARD) {
                pos_destiny.y = j*TILESIZE;
                graphLib.place_hardmode_block_tile(pos_destiny, mapSurface);
            } else {
                pos_origin.x = map_data[number].tiles[i][j].tile1.x;
                pos_origin.y = map_data[number].tiles[i][j].tile1.y;

                if (pos_origin.x >= 0 && pos_origin.y >= 0) {
                    pos_destiny.y = j*TILESIZE;
                    graphLib.placeTile(pos_origin, pos_destiny, &mapSurface);
                } else if (pos_origin.x < -1 && pos_origin.y == 0) {
                    int anim_tile_id = (pos_origin.x * -1) - 2;
                    pos_destiny.y = j*TILESIZE;
                    graphLib.place_anim_tile(anim_tile_id, pos_destiny, &mapSurface);
                }
            }


        }
        n++;
    }


    return mapSurface;
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
short int classMap::colision_player_npcs(character* playerObj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y)
{
    UNUSED(x_inc);
    UNUSED(y_inc);
	struct st_rectangle p_rect, npc_rect;
    std::vector<classnpc*>::iterator npc_it;

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

    //std::cout << "colision_player_npcs - p1.x: " << p1.x << ", p1.y: " << p1.y << std::endl;

    for (int i=0; i<_npc_list.size(); i++) {
        if (_npc_list.at(i).is_player_friend() == true) {
			//std::cout << "colision_player_npcs - FRIEND" << std::endl;
			continue;
		}
        if (_npc_list.at(i).is_dead() == true) {
			//std::cout << "colision_player_npcs - DEAD" << std::endl;
			continue;
		}
        if (_npc_list.at(i).is_invisible() == true) {
			//std::cout << "colision_player_npcs - INVISIBLE" << std::endl;
			continue;
		}

        if (_npc_list.at(i).is_on_visible_screen() == false) {
            continue;
        }


        npc_rect.x = _npc_list.at(i).getPosition().x;
        npc_rect.w = _npc_list.at(i).get_size().width;
        npc_rect.y = _npc_list.at(i).getPosition().y;
        npc_rect.h = _npc_list.at(i).get_size().height;

        if (_npc_list.at(i).get_size().width >= TILESIZE) { // why is this here??? O.o
            npc_rect.x = _npc_list.at(i).getPosition().x+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.w = _npc_list.at(i).get_size().width-PLAYER_NPC_COLLISION_REDUTOR;
		}
        if (_npc_list.at(i).get_size().height >= TILESIZE) {
            npc_rect.y = _npc_list.at(i).getPosition().y+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.h = _npc_list.at(i).get_size().height-PLAYER_NPC_COLLISION_REDUTOR;
		}

		colision_detection rect_colision_obj;
		if (rect_colision_obj.rect_overlap(npc_rect, p_rect) == true) {
            //std::cout << "colision_player_npcs[" << temp_obj->getName() << "] - COLISION!" << std::endl;
            if (npc_rect.h > p_rect.h) {
				return 2;
			} else {
				return 1;
			}
		}
    }
    return 0;
}


// kills any NPC that touches player during player's special attack
void classMap::colision_player_special_attack(character* playerObj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y)
{
    UNUSED(x_inc);
    UNUSED(y_inc);
    struct st_rectangle p_rect, npc_rect;
    std::vector<classnpc*>::iterator npc_it;

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

    for (int i=0; i<_npc_list.size(); i++) {
        if (_npc_list.at(i).is_player_friend() == true) {
            continue;
        }
        if (_npc_list.at(i).is_dead() == true) {
            continue;
        }
        if (_npc_list.at(i).is_invisible() == true) {
            continue;
        }

        if (_npc_list.at(i).is_on_visible_screen() == false) {
            continue;
        }


        npc_rect.x = _npc_list.at(i).getPosition().x;
        npc_rect.w = _npc_list.at(i).get_size().width;
        npc_rect.y = _npc_list.at(i).getPosition().y;
        npc_rect.h = _npc_list.at(i).get_size().height;

        if (_npc_list.at(i).get_size().width >= TILESIZE) { // why is this here??? O.o
            npc_rect.x = _npc_list.at(i).getPosition().x+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.w = _npc_list.at(i).get_size().width-PLAYER_NPC_COLLISION_REDUTOR;
        }
        if (_npc_list.at(i).get_size().height >= TILESIZE) {
            npc_rect.y = _npc_list.at(i).getPosition().y+PLAYER_NPC_COLLISION_REDUTOR;
            npc_rect.h = _npc_list.at(i).get_size().height-PLAYER_NPC_COLLISION_REDUTOR;
        }
        colision_detection rect_colision_obj;
        if (rect_colision_obj.rect_overlap(npc_rect, p_rect) == true) {
            _npc_list.at(i).damage(12, false);
        }
    }
}

classnpc *classMap::find_nearest_npc(st_position pos)
{
    std::vector<classnpc*>::iterator npc_it;
    int min_dist = 9999;
    classnpc* min_dist_npc = NULL;

    for (int i=0; i<_npc_list.size(); i++) {
        if (_npc_list.at(i).is_player_friend() == true) {
            //std::cout << "colision_player_npcs - FRIEND" << std::endl;
            continue;
        }
        if (_npc_list.at(i).is_dead() == true) {
            //std::cout << "colision_player_npcs - DEAD" << std::endl;
            continue;
        }
        if (_npc_list.at(i).is_invisible() == true) {
            //std::cout << "colision_player_npcs - INVISIBLE" << std::endl;
            continue;
        }
        if (_npc_list.at(i).is_on_visible_screen() == false) {
            continue;
        }
        float dist = sqrt(pow((pos.x - _npc_list.at(i).getPosition().x), 2) + pow((pos.y - _npc_list.at(i).getPosition().y), 2));
        if (dist < min_dist) {
            min_dist_npc = (*npc_it);
            min_dist = dist;
        }
    }
    return min_dist_npc;
}

/// @TODO: fix animation. investigate a better way for drawing it (code is way too confusing)
void classMap::redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number) {
	//is_close = false; // THIS IS A TEMPORARY FIX

	//std::cout << "classMap::redraw_boss_door - is_close: " << is_close << std::endl;

	input.waitTime(10);
	for (int k=0; k<nTiles; k++) {
		//if (is_close == false) { std::cout << "classMap::redraw_boss_door - nTiles: " << nTiles << ", tilePieces: " << tilePieces << ", tileCount: " << tileCount << std::endl; }
		// redraw screen
		showMap();

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
                if (map_data[number].tiles[i][j].tile3.x != -1 && map_data[number].tiles[i][j].tile3.y != -1) {
                        if (i == tileX && map_data[number].tiles[i][j].locked == TERRAIN_DOOR) {
							//std::cout << "****** redraw_boss_door - k: " << k << ", tiles_showed: " << tiles_showed << ", nTiles: " << nTiles << std::endl;
							if (is_close == false) {
								if (tiles_showed < nTiles) {

                                    if (!graphLib.gameScreen.get_surface()) {
                                        graphLib.show_debug_msg("EXIT #21.C");
                                        exit(-1);
                                    }


                                    graphLib.placeTile(st_position(map_data[number].tiles[i][j].tile3.x, map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)-scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
                                    draw_lib.update_screen();
									tiles_showed++;
								}
							} else {
								if (tiles_showed < k) {

                                    if (!graphLib.gameScreen.get_surface()) {
                                        graphLib.show_debug_msg("EXIT #21.D");
                                        exit(-1);
                                    }


                                    graphLib.placeTile(st_position(map_data[number].tiles[i][j].tile3.x, map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)-scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
                                    draw_lib.update_screen();
									tiles_showed++;
								}
							}
						} else {

                            if (!graphLib.gameScreen.get_surface()) {
                                graphLib.show_debug_msg("EXIT #21.E");
                                exit(-1);
                            }


                            graphLib.placeTile(st_position(map_data[number].tiles[i][j].tile3.x, map_data[number].tiles[i][j].tile3.y), st_position((i*TILESIZE)+scroll.x, (j*TILESIZE)-scroll.y), &graphLib.gameScreen);
						}
				}
			}
		}
        _player_ref->show();
        graphLib.draw_hp_bar(_player_ref->get_current_hp(), player_number, WEAPON_DEFAULT, fio.get_heart_pieces_number(game_save));
        showAbove();
        draw_lib.update_screen();
		input.waitTime(100);
	}
    if (is_close == true) {
        _3rd_level_ignore_area = st_rectangle(-1, -1, -1, -1);
        showAbove();
        draw_lib.update_screen();
    }
    input.waitTime(100);
}


void classMap::add_animation(ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize)
{
    //std::cout << ">>>>> classMap::add_animation - repeat_times: " << repeat_times << std::endl;
	animation_list.push_back(animation(pos_type, surface, pos, adjust_pos, frame_time, repeat_times, direction, framesize, &scroll));
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
    classnpc new_npc(stage_number, number, npc_id, npc_pos, direction, player_friend);
    new_npc.set_map(this);
    if (progressive_span == true) {
        new_npc.set_progressive_appear_pos(new_npc.get_size().height);
    }
    _npc_list.push_back(new_npc); // insert new npc at the list-end
    _break_npc_loop = true;
    return &(_npc_list.back());
}


void classMap::move_npcs() /// @TODO - check out of screen
{
    //std::cout << "*************** classMap::showMap - npc_list.size: " << _npc_list.size() << std::endl;
    for (int i=0; i<_npc_list.size(); i++) {
        if (_break_npc_loop == true) {
            _break_npc_loop = false;
            break;
        }
        // check if NPC is outside the visible area
        st_position npc_pos = _npc_list.at(i).get_real_position();
        short dead_state = _npc_list.at(i).get_dead_state();

        std::string name(_npc_list.at(i).getName());


        if (_npc_list.at(i).is_on_screen() != true) {
            if (dead_state == 2 && _npc_list.at(i).is_boss() == false && _npc_list.at(i).is_subboss()) {
                _npc_list.at(i).revive();
            }
            continue; // no need for moving NPCs that are out of sight
        } else if (dead_state == 2 && _npc_list.at(i).auto_respawn() == true && _npc_list.at(i).is_boss() == false) {
            _npc_list.at(i).reset_position();
            _npc_list.at(i).revive();
            continue;
        } else if (dead_state == 1 && _npc_list.at(i).is_spawn() == false && _npc_list.at(i).is_boss() == false) {// drop item
            drop_item(st_position(_npc_list.at(i).getPosition().x + _npc_list.at(i).get_size().width/2, _npc_list.at(i).getPosition().y + _npc_list.at(i).get_size().height/2));
        }

        // if is showing stage boss on a stage already finished, just teleport out, victory is yours!
        if (_npc_list.at(i).is_stage_boss() == true && _npc_list.at(i).is_on_visible_screen() == true && game_save.stages[gameControl.currentStage] == 1 && gameControl.currentStage <= 8) {
            gameControl.got_weapon();
            return;
        }

        if (dead_state == 0) {
            _npc_list.at(i).execute(); // TODO: must pass scroll map to npcs somwhow...
        }




		if (dead_state == 1) {
            if (_npc_list.at(i).is_stage_boss() == false) {
                _npc_list.at(i).execute_ai(); // to ensure death-reaction is run
                // sub-boss have a different explosion
                if (_npc_list.at(i).is_subboss()) {
                    soundManager.play_repeated_sfx(SFX_BIG_EXPLOSION, 1);
                    st_float_position pos1(_npc_list.at(i).getPosition().x+2, _npc_list.at(i).getPosition().y+20);
                    add_animation(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos1, st_position(-8, -8), 80, 2, _npc_list.at(i).get_direction(), st_size(56, 56));
                    st_float_position pos2(pos1.x+50, pos1.y-30);
                    add_animation(ANIMATION_STATIC, &graphLib.bomb_explosion_surface, pos2, st_position(-8, -8), 80, 2, _npc_list.at(i).get_direction(), st_size(56, 56));
                } else {
                    add_animation(ANIMATION_STATIC, &graphLib.explosion32, _npc_list.at(i).getPosition(), st_position(-8, -8), 80, 2, _npc_list.at(i).get_direction(), st_size(32, 32));
                }
                // check if boss flag wasn't passed to a spawn on dying reaction AI
                if (_npc_list.at(i).is_boss()) {
                    gameControl.check_player_return_teleport();
                }
                _npc_list.at(i).clean_projectiles();
            } else {
                // run npc move one more timer, so reaction is executed to test if it will spawn a new boss
                _npc_list.at(i).execute_ai(); // to ensure death-reaction is run
                _npc_list.at(i).execute_ai(); // to ensure death-reaction is run
                if (_npc_list.at(i).is_stage_boss() == false) { // if now the NPC is not the stage boss anymore, continue
                    gameControl.draw_explosion(npc_pos.x, npc_pos.y, true);
                    soundManager.play_boss_music();
                    graphLib.blink_screen(255, 255, 255);
                    gameControl.fill_boss_hp_bar();
                    continue;
                } else {
                    _npc_list.at(i).clean_projectiles();
                    gameControl.remove_all_projectiles();
                    //std::cout << "classMap::showMap - killed boss" << std::endl;
                    graphLib.set_screen_adjust(st_position(0, 0));
                    /// @TODO - replace with game_data.final_boss_id
                    if (stage_number == CASTLE1_STAGE5) {
                        gameControl.show_ending(npc_pos);
                        return;
                    } else {
                        gameControl.draw_explosion(npc_pos.x, npc_pos.y, true);
                        gameControl.got_weapon();
                    }
                }
			}
			return;
		}
    }
}

void classMap::show_npcs() /// @TODO - check out of screen
{
    for (int i=0; i<_npc_list.size(); i++) {
        if (gameControl.must_show_boss_hp() && _npc_list.at(i).is_boss() && _npc_list.at(i).is_on_visible_screen() == true) {
            graphLib.draw_hp_bar(_npc_list.at(i).get_current_hp(), -1, -1, PLAYER_INITIAL_HP);
		}
        if (_npc_list.at(i).is_dead() == false) {
            _npc_list.at(i).show();
        }
    }
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

std::vector<object*> classMap::check_collision_with_objects(st_rectangle collision_area)
{
    std::vector<object*> res;

    for (unsigned int i=0; i<object_list.size(); i++) {
        object* temp_obj = &object_list.at(i);
        colision_detection rect_colision_obj;
        bool res_collision = rect_colision_obj.rect_overlap(temp_obj->get_area(), collision_area);
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
        (*object_it).show(adjust_y, adjust_x); // TODO: must pass scroll map to objects somwhow...
    }
}

bool classMap::boss_hit_ground()
{
    for (int i=0; i<_npc_list.size(); i++) {
        if (_npc_list.at(i).is_boss() == true && _npc_list.at(i).is_on_visible_screen() == true) {
            //std::cout << "MAP::boss_hit_ground - move boss to ground - pos.y: " << _npc_list.at(i).getPosition().y << std::endl;

            int limit_y = _npc_list.at(i).get_start_position().y - TILESIZE;
            //std::cout << "#### limit_y: " << limit_y << std::endl;
            if (limit_y > RES_H/2) {
                limit_y = RES_H/2;
            }

            if (_npc_list.at(i).is_able_to_fly() == true) {
                //std::cout << "MAP::boss_hit_ground - pos.y: " << _npc_list.at(i).getPosition().y << ", center: " << (RES_H/2 - _npc_list.at(i).get_size().height/2) << std::endl;
                if (_npc_list.at(i).getPosition().y >= RES_H/2 - _npc_list.at(i).get_size().height/2) {
                    //std::cout << "boss_hit_ground #1" << std::endl;
                    return true;
                }
            } else if (_npc_list.at(i).getPosition().y >= limit_y && _npc_list.at(i).hit_ground()) {
                //std::cout << "boss_hit_ground #2" << std::endl;
                return true;
            }
			break;
		}
    }
	return false;
}

void classMap::reset_map_npcs()
{
	load_map_npcs();
}







