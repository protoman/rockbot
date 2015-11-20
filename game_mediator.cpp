#include "game_mediator.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

#include "soundlib.h"
extern soundLib soundManager;

// Global static pointer used to ensure a single instance of the class.
GameMediator* GameMediator::_instance = NULL;


GameMediator *GameMediator::get_instance()
{
    if (!_instance) {
        _instance = new GameMediator();
    }
    return _instance;

}

Mix_Chunk* GameMediator::get_sfx(std::string filename)
{
    std::map<std::string, Mix_Chunk*>::iterator it = sfx_map.find(filename);
    if (it == sfx_map.end()) {
        Mix_Chunk* sfx = soundManager.sfx_from_file(filename);
        sfx_map.insert(std::pair<std::string, Mix_Chunk*>(filename, sfx));
        return sfx;
    } else {
        return it->second;
    }
}

CURRENT_FILE_FORMAT::file_projectile GameMediator::get_projectile(int n)
{
    if (n < 0 || n >= projectile_list.size()) {
        return projectile_list.at(0);
    }
    return projectile_list.at(n);
}

int GameMediator::get_projectile_list_size()
{
    return projectile_list.size();
}

CURRENT_FILE_FORMAT::file_npc GameMediator::get_enemy(int n)
{
    if (n < 0 || n >= enemy_list.size()) {
        std::cout << "GameMediator::get_enemy - invalid enemy position[" << n << "], list-size: " << enemy_list.size() << std::endl;
        exit(-1);
    }
    return enemy_list.at(n);
}

int GameMediator::get_enemy_list_size()
{
    return enemy_list.size();
}

GameMediator::GameMediator()
{
    enemy_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat");
    object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat");
    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat");
    projectile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_projectile>("game_projectile_list.dat");
    anim_tile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat");

    // add some dummy data for game not to crash
    if (projectile_list.size() == 0) {
        projectile_list.push_back(CURRENT_FILE_FORMAT::file_projectile());
    }
}

