#include <iostream>
using namespace std;

#include "file/file_io.h"
#include "stage.h"

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::file_map map_data[FS_STAGE_MAX_MAPS];
extern CURRENT_FILE_FORMAT::file_io fio;

extern struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
stage::stage(int setStageN, std::vector<classPlayer> &set_player_list)
{
    //std::cout << "STAGE::CONSTRUCTOR - setStageN: " << setStageN << std::endl;
    std::fflush(stdout);
	number = -1;
	currentMap = 0;
	setNumber(setStageN);
    fio.read_stage(stage_data, setStageN);
    fio.read_stage_maps(setStageN, map_data);

    std::string tileset_name(stage_data.tileset_filename);
    if (tileset_name.length() == 0) {
        tileset_name = std::string("default.png");
    }
    graphLib.loadTileset(tileset_name);

    for (int i=0; i<PRELOAD_MAP_N; i++) {
        maps[i] = NULL;
    }

    std::vector<classPlayer>::iterator it;
    int n = 0;
	for (it=set_player_list.begin(); it != set_player_list.end(); it++) {
        //std::cout << "p[" << n << "] added to stage" << std::endl;
        n++;
        _player_list.push_back(&(*it));
	}
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
stage::~stage()
{
    std::cout << "stage[" << (int)number << "] destructor" << std::endl;
    clean_stage();
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
        std::cout << "ERROR::stage::loadStage - stage number[" << number << "] is greater than the MAX_STAGES[" << MAX_STAGES << "].\n";
        return;
	}

	//std::cout << "stage::loadStage - number: " << number << std::endl;

	// load stage maps
    for (int i=0; i<STAGE_MAX_MAPS; i++) {
        classMap *new_map = new classMap();
        new_map->setMapNumber(i);
        new_map->setStageNumber(number);
        new_map->loadMap();
        new_map->set_player_list(_player_list);
        maps[i] = new_map;
    }
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::showStage() const
{
    maps[currentMap]->showMap();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::showAbove(int scroll_y) const
{
    maps[currentMap]->showAbove(scroll_y);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 stage::getMapPointLock(st_position pos)
{
    return maps[currentMap]->getMapPointLock(pos);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void stage::changeScrolling(st_position pos, bool check_lock) const
{
    // debug for autoscrolling test
    if (stage_data.autoscroll == true) {
        pos.x = 1;
    }
    maps[currentMap]->changeScrolling(pos, check_lock);
}

void stage::set_scrolling(st_position pos) const
{
    maps[currentMap]->set_scrolling(pos);
}

void stage::reset_scrolling()
{
    maps[currentMap]->reset_scrolling();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_position stage::getMapScrolling() const
{
    //std::cout << ">> stage::getMapScrolling[" << currentMap << "].x: " << maps[currentMap]->getMapScrolling().x << std::endl;
    return maps[currentMap]->getMapScrolling();
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classMap* stage::get_current_map()
{
    //std::cout << "**** stage::get_current_map - number: " << maps[currentMap]->number << std::endl;
    return maps[currentMap];
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
Uint8 stage::get_current_map_number() const
{
	return currentMap;
}

void stage::set_current_map(int new_map_n)
{
	//std::cout << "------- stage::set_current_map to " << new_map_n << "-------" << std::endl;
	currentMap = new_map_n;
}

Uint8 stage::get_current_map_n() const
{
	return currentMap;
}

void stage::move_map(const short int move_x, const short int move_y) const
{
    maps[currentMap]->move_map(move_x, move_y);
}

void stage::reload_stage()
{
    clean_stage();
    loadStage();
}

void stage::print_map_objects_number()
{
    maps[currentMap]->print_objects_number();
}

void stage::reset_current_map()
{
	currentMap = checkpoint.map;
    //std::cout << "STAGE::reset_current_map - currentMap: " << currentMap << std::endl;
    if (currentMap > PRELOAD_MAP_N) {
        return;
    }
    if (maps[currentMap] != NULL) {
        maps[currentMap]->set_scrolling(st_position(checkpoint.map_scroll_x, 0));
    }
}

void stage::reset_current_map_objects() const
{
    maps[currentMap]->reset_map();
    maps[currentMap]->reset_map_npcs();
}

void stage::reset_stage_objects() const
{
    //std::cout << "================== stage::reset_stage_objects" << std::endl;
    for (int i=0; i<PRELOAD_MAP_N; i++) {
        maps[i]->reset_map();
    }
}


void stage::clean_stage()
{
    for (int i=0; i<PRELOAD_MAP_N; i++) {
        if (maps[i] != NULL) {
            maps[i]->clean_map();
            delete maps[i];
            maps[i] = NULL;
        }
    }
}

void stage::redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number) const
{
	//std::cout << "stage::redraw_boss_door - is_close: " << is_close << std::endl;
    maps[currentMap]->redraw_boss_door(is_close, nTiles, tileX, tileY, player_number);
}

void stage::move_npcs() const
{
    maps[currentMap]->move_npcs();
}

void stage::show_npcs() const
{
    maps[currentMap]->show_npcs();
}

void stage::move_objects(bool paused) const
{
    maps[currentMap]->move_objects(paused);
}

void stage::show_objects(int adjust) const
{
    maps[currentMap]->show_objects(adjust);
}

bool stage::boss_hit_ground() const
{
    return maps[currentMap]->boss_hit_ground();
}

void stage::reset_stage_maps() const
{
    std::cout << "================== stage::reset_stage_maps" << std::endl;
    reset_stage_npcs();
    reset_stage_objects();
}




void stage::reset_stage_npcs() const
{
	for (int i=0; i<PRELOAD_MAP_N; i++) {
        if (maps[i] != NULL) {
            maps[i]->reset_map_npcs();
		}
	}
}



int stage::get_first_lock_on_left(int x_pos) const
{
    return maps[currentMap]->get_first_lock_on_left(x_pos);
}

int stage::get_teleport_minimal_y(int xpos) const
{
    int tilex = xpos/TILESIZE;
    for (int i=MAP_H-1; i>=4; i--) { // ignore here 3 first tiles, as we need to test them next
        int map_lock = maps[currentMap]->getMapPointLock(st_position(tilex, i));
        bool found_bad_point = false;
        if (map_lock != TERRAIN_UNBLOCKED && map_lock != TERRAIN_WATER) { // found a stop point, now check above ones
            for (int j=i-1; j>=i-3; j--) {
                int map_lock2 = maps[currentMap]->getMapPointLock(st_position(tilex, j));
                if (map_lock2 != TERRAIN_UNBLOCKED && map_lock2 != TERRAIN_WATER) { // found a stop point, now check above ones
                    found_bad_point = true;
                    break;
                }
            }
            if (found_bad_point == false) {
                return i;
            }
        }
    }
    return 0;
}

void stage::reset_objects_timers() const
{
    //std::cout << "================== stage::reset_objects_timers" << std::endl;
    maps[currentMap]->reset_objects_timers();
}

bool stage::subboss_alive_on_left(short tileX)
{
    return maps[currentMap]->subboss_alive_on_left(tileX);
}

void stage::activate_final_boss_teleporter()
{
    for (int i=0; i<PRELOAD_MAP_N; i++) {
        std::cout << "stage::activate_final_boss_teleporter - currentMap: " << currentMap << std::endl;
        maps[currentMap]->activate_final_boss_teleporter();
    }
}

