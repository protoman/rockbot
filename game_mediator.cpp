#include "game_mediator.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

// Global static pointer used to ensure a single instance of the class.
GameMediator* GameMediator::_instance = NULL;


GameMediator *GameMediator::get_instance()
{
    if (!_instance) {
        _instance = new GameMediator();
    }
    return _instance;

}

GameMediator::GameMediator()
{
    enemy_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat");
    object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat");
    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat");
    anim_tile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::st_anim_map_tile>("anim_tiles.dat");

    // add some dummy data for game not to crash
    if (projectile_list.size() == 0) {
        projectile_list.push_back(CURRENT_FILE_FORMAT::file_projectile());
    }
}

