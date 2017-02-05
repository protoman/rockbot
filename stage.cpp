#include <iostream>
using namespace std;

#include "file/file_io.h"
#include "stage.h"

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::file_map map_data[FS_STAGE_MAX_MAPS];
extern CURRENT_FILE_FORMAT::file_io fio;

extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;
extern CURRENT_FILE_FORMAT::st_game_config game_config;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
stage::stage(int setStageN, classPlayer* set_player_ref) : stage_is_loaded(false)
{
    // ignore -1, as it is an unitialized copy
    if (setStageN == -1) {
        return;
    }
    number = -1;
    set_current_map(0);
    setNumber(setStageN);
    fio.read_stage(stage_data, setStageN);
    fio.read_stage_maps(setStageN, map_data);

    std::string tileset_name(stage_data.tileset_filename);
    if (tileset_name.length() == 0) {
        tileset_name = std::string("default.png");
    }
    graphLib.loadTileset(tileset_name);

    autoscroll_timer = 0;

    _player_ref = set_player_ref;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
stage::~stage()
{
}

int stage::get_number()
{
    return number;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::setNumber(int setNumber)
{
	number = setNumber;
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::loadStage() {
    //std::cout << "*************** stage::loadStage(" << number << ") ***************" << std::endl;
    if (number == -1) {
        graphLib.show_debug_msg("ERROR::stage::loadStage - stage number was not set, can't load it before setting the number");
		return;
	}
    if (number >= MAX_STAGES) {
        graphLib.show_debug_msg("ERROR::loadStage invalid number");
        std::cout << "ERROR::stage::loadStage - stage number[" << number << "] is greater than the MAX_STAGES[" << MAX_STAGES << "].\n";
        return;
	}

    draw_lib.clear_maps_dynamic_background_list();

    stage_is_loaded = true;
    //std::cout << "stage::loadStage - number: " << number << std::endl;

	// load stage maps
    for (int i=0; i<STAGE_MAX_MAPS; i++) {
        classMap new_map;
        maps[i] = new_map;
        maps[i].setMapNumber(i);
        maps[i].setStageNumber(number);
        maps[i].loadMap();
        maps[i].set_player(_player_ref);
    }
    check_map_effect();
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::showStage()
{
    maps[currentMap].showMap();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::showAbove(int scroll_y)
{
    maps[currentMap].showAbove(scroll_y);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 stage::getMapPointLock(st_position pos)
{
    return maps[currentMap].getMapPointLock(pos);
}


void stage::change_map_scroll(st_float_position pos, bool check_lock, bool ignore_auto_scroll)
{
    // debug for autoscrolling test
    bool map_autoscroll = static_cast<bool>(stage_data.autoscroll[currentMap]);
    // avoid data error (getting 66 as value from data file)
    if (map_autoscroll > 1) {
        map_autoscroll = false;
    }
    if (ignore_auto_scroll == false && map_autoscroll == true) {
        if (timer.is_paused() == false && autoscroll_timer < timer.getTimer()) {
            autoscroll_timer = timer.getTimer()+20;
            pos.x = 1.5;
        } else {
            pos.x = 0;
        }
    }
    maps[currentMap].changeScrolling(pos, check_lock);
}

void stage::set_scrolling(st_float_position pos)
{
    maps[currentMap].set_scrolling(pos);
}

void stage::reset_scrolling()
{
    maps[currentMap].reset_scrolling();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_float_position stage::getMapScrolling()
{
    //std::cout << ">> stage::getMapScrolling[" << currentMap << "].x: " << maps[currentMap].getMapScrolling().x << std::endl;
    return maps[currentMap].getMapScrolling();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap *stage::get_current_map()
{
    //std::cout << "**** stage::get_current_map - number: " << maps[currentMap].number << std::endl;
    return &maps[currentMap];
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 stage::get_current_map_number()
{
	return currentMap;
}

void stage::set_current_map(int new_map_n)
{
    //std::cout << "------- stage::set_current_map to " << new_map_n << "-------" << std::endl;
    std::fflush(stdout);

	currentMap = new_map_n;

    //std::cout << ">> PASS #1" << std::endl;
    std::fflush(stdout);

    check_map_effect();
    //std::cout << ">> PASS #2" << std::endl;
    std::fflush(stdout);
}

Uint8 stage::get_current_map_n()
{
	return currentMap;
}

void stage::move_map(const short int move_x, const short int move_y)
{
    maps[currentMap].move_map(move_x, move_y);
}

void stage::reload_stage()
{
    loadStage();
}

void stage::print_map_objects_number()
{
    maps[currentMap].print_objects_number();
}

void stage::reset_current_map()
{
    set_current_map(checkpoint.map);
    //std::cout << "STAGE::reset_current_map - currentMap: " << currentMap << std::endl;
    if (currentMap > PRELOAD_MAP_N) {
        return;
    }
    maps[currentMap].set_scrolling(st_float_position(checkpoint.map_scroll_x, 0));
}

void stage::reset_current_map_objects()
{
    maps[currentMap].reset_map();
    maps[currentMap].reset_map_npcs();
}

void stage::reset_stage_objects()
{
    //std::cout << "================== stage::reset_stage_objects" << std::endl;
    for (int i=0; i<PRELOAD_MAP_N; i++) {
        maps[i].reset_map();
    }
}

void stage::redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number)
{
	//std::cout << "stage::redraw_boss_door - is_close: " << is_close << std::endl;
    maps[currentMap].redraw_boss_door(is_close, nTiles, tileX, tileY, player_number);
}

void stage::move_npcs()
{
    maps[currentMap].move_npcs();
}

void stage::show_npcs()
{
    maps[currentMap].show_npcs();
}

void stage::move_objects(bool paused)
{
    maps[currentMap].move_objects(paused);
}

void stage::show_objects(int adjust)
{
    maps[currentMap].show_objects(adjust);
}

bool stage::boss_hit_ground()
{
    return maps[currentMap].boss_hit_ground();
}

void stage::reset_stage_maps()
{
    //std::cout << "================== stage::reset_stage_maps" << std::endl;
    reset_stage_npcs();
    reset_stage_objects();
}




void stage::reset_stage_npcs()
{
	for (int i=0; i<PRELOAD_MAP_N; i++) {
        maps[i].reset_map_npcs();
	}
}



int stage::get_first_lock_on_left(int x_pos)
{
    return maps[currentMap].get_first_lock_on_left(x_pos);
}

int stage::get_first_lock_on_right(int x_pos)
{
    return maps[currentMap].get_first_lock_on_right(x_pos);
}

int stage::get_teleport_minimal_y_tile(int xpos)
{
    int tilex = xpos/TILESIZE;

    for (int i=MAP_H-1; i>=4; i--) { // ignore here 3 first tiles, as we need to test them next

        //std::cout << "STAGE::get_teleport_minimal_y[" << i << "]" << std::endl;

        int map_lock = maps[currentMap].getMapPointLock(st_position(tilex, i));
        bool found_bad_point = false;
        if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) {
            // found a stop point, now check above tiles
            for (int j=i-1; j>=i-3; j--) {
                int map_lock2 = maps[currentMap].getMapPointLock(st_position(tilex, j));
                if (map_lock2 != TERRAIN_UNBLOCKED && map_lock2 != TERRAIN_WATER) { // found a stop point, now check above ones
                    found_bad_point = true;
                    break;
                }
            }
            if (found_bad_point == false) {
                return i-3;
            }
        }
    }
    // if no good point was found, return the top of the screen
    return 0;
}

void stage::reset_objects_timers()
{
    //std::cout << "================== stage::reset_objects_timers" << std::endl;
    maps[currentMap].reset_objects_timers();
}

bool stage::subboss_alive_on_left(short tileX)
{
    return maps[currentMap].subboss_alive_on_left(tileX);
}

void stage::activate_final_boss_teleporter()
{
    for (int i=0; i<PRELOAD_MAP_N; i++) {
        //std::cout << "stage::activate_final_boss_teleporter - currentMap: " << currentMap << std::endl;
        maps[currentMap].activate_final_boss_teleporter();
    }
}

short stage::get_current_map_gfx_mode()
{
    return maps[currentMap].get_map_gfx_mode();
}

void stage::check_map_effect()
{
    if (number == -1 || stage_is_loaded == false) {
        return;
    }
    if (currentMap == -1 || currentMap >= PRELOAD_MAP_N) {
        return;
    }
    // no map effects unless config is set to high-end performance mode
    if (game_config.graphics_performance_mode != PERFORMANCE_MODE_HIGH) {
        return;
    }


    //std::cout << ">> PASS #3 [" << (int)number << "][" << (int)currentMap << "]" << std::endl;
    std::fflush(stdout);
    std::cout << "####### STAGE::check_map_effect - currentMap[" << (int)currentMap << "], map.gfx: " << (int)maps[currentMap].get_map_gfx() << ", draw.gfx: " << (int)draw_lib.get_gfx() << std::endl;
    if (maps[currentMap].get_map_gfx() != draw_lib.get_gfx()) {
        draw_lib.set_gfx(maps[currentMap].get_map_gfx(), maps[currentMap].get_map_gfx_mode());
    }
}

