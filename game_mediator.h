#ifndef GAMEMEDIATOR_H
#define GAMEMEDIATOR_H

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <SDL/SDL_mixer.h>

#include "file/format.h"
#include "file/fio_common.h"
#include "file/fio_strings.h"

class GameMediator
{
public:
    static GameMediator* get_instance();
    Mix_Chunk *get_sfx(std::string filename);
    CURRENT_FILE_FORMAT::file_projectilev3 get_projectile(int n);
    int get_projectile_list_size();

    CURRENT_FILE_FORMAT::file_npc_v3_1_2* get_enemy(int n);
    int get_enemy_list_size();
#ifdef WII
    void short_to_little_endian(short &s);
    void u_short_to_little_endian(unsigned short &s);
    void sint16_to_little_endian(int16_t &i);
    void uint16_to_little_endian(Uint16 &i);
    void int_to_little_endian(int &i);
#endif

private:
    GameMediator();
    GameMediator(GameMediator const&){};             // copy constructor is private
    GameMediator& operator=(GameMediator const&){return *this;};  // assignment operator is private
#ifdef WII
    void wii_convert_npc_list();
    void wii_convert_object_list();
    void wii_convert_ai_list();
    void wii_convert_projectile_list();
    void wii_convert_player_list();
    void wii_convert_anim_tile_list();
#endif

public:
    std::vector<CURRENT_FILE_FORMAT::file_object> object_list;
    std::vector<CURRENT_FILE_FORMAT::file_artificial_inteligence> ai_list;
    std::vector<CURRENT_FILE_FORMAT::file_anim_block> anim_tile_list;
    std::vector<CURRENT_FILE_FORMAT::file_player_v3_1_1> player_list_v3_1;
    CURRENT_FILE_FORMAT::file_map_v2 map_data[FS_STAGE_MAX_MAPS];           // map data
    std::vector<CURRENT_FILE_FORMAT::file_map_npc_v2> map_npc_data;         // map enemy list
    std::vector<CURRENT_FILE_FORMAT::file_map_object_v2> map_object_data;   // map object list
    std::vector<CURRENT_FILE_FORMAT::file_projectilev3> projectile_list;
    std::map<std::string, graphicsLib_gSurface> character_graphic_map;      // character graphics

private:
    static GameMediator* _instance;
    std::map<std::string, Mix_Chunk*> sfx_map;                                               // map of sfx files loaded, used in projectiles to avoid loading from disk every time
    fio_common fio_cmm;

    std::vector<CURRENT_FILE_FORMAT::file_npc_v3_1_2> enemy_list;


};

#endif // GAMEMEDIATOR_H
