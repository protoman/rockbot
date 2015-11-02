#ifndef GAMEMEDIATOR_H
#define GAMEMEDIATOR_H

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <SDL/SDL_mixer.h>

#include "file/format.h"
#include "file/fio_common.h"

class GameMediator
{
public:
    static GameMediator* get_instance();
    Mix_Chunk *get_sfx(std::string filename);

private:
    GameMediator();
    GameMediator(GameMediator const&){};             // copy constructor is private
    GameMediator& operator=(GameMediator const&){};  // assignment operator is private


public:
    std::vector<CURRENT_FILE_FORMAT::file_npc> enemy_list;
    std::vector<CURRENT_FILE_FORMAT::file_object> object_list;
    std::vector<CURRENT_FILE_FORMAT::file_artificial_inteligence> ai_list;
    std::vector<CURRENT_FILE_FORMAT::file_projectile> projectile_list;
    std::vector<CURRENT_FILE_FORMAT::st_anim_map_tile> anim_tile_list;

private:
    static GameMediator* _instance;
    std::map<std::string, Mix_Chunk*> sfx_map;                                               // map of sfx files loaded, used in projectiles to avoid loading from disk every time
    fio_common fio_cmm;


};

#endif // GAMEMEDIATOR_H
