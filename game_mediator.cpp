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

CURRENT_FILE_FORMAT::file_projectilev2 GameMediator::get_projectile(int n)
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

CURRENT_FILE_FORMAT::file_npc* GameMediator::get_enemy(int n)
{
    // boss not yet set
    if (n < 0 || n >= enemy_list.size()) {
        std::cout << "ERROR: GameMediator::get_enemy - invalid enemy position[" << n << "], list-size: " << enemy_list.size() << std::endl;
        // return first NPC to avoid errors
        n = 0;
    }
    return &enemy_list.at(n);
}

int GameMediator::get_enemy_list_size()
{
    return enemy_list.size();
}

#ifdef WII
void GameMediator::short_to_little_endian(short &s)
{
     s = (s>>8) | (s<<8);
}

void GameMediator::u_short_to_little_endian(unsigned short &s)
{
    short temp_short = s;
    short_to_little_endian(temp_short);
    s = temp_short;
}

void GameMediator::sint16_to_little_endian(int16_t &i)
{
    i = (i << 8) | ((i >> 8) & 0xFF);
}


void GameMediator::int_to_little_endian(int &i)
{
    i = (i<<24)|((i<<8)&0xFF0000)|((i>>8)&0xFF00)|(i>>24);
}

void GameMediator::uint16_to_little_endian(Uint16 &i)
{
    int16_t temp = i;
    sint16_to_little_endian(temp);
    i = temp;
}


void GameMediator::wii_convert_npc_list() {
    for (int i=0; i<enemy_list.size(); i++) {
        short_to_little_endian(enemy_list.at(i).hp.total);
        short_to_little_endian(enemy_list.at(i).hp.current);

        printf("#convert.npc[%s].range.OLD[%d] #\n", enemy_list.at(i).name, enemy_list.at(i).walk_range);
        sint16_to_little_endian(enemy_list.at(i).walk_range);
        printf("@convert.npc[%s].range.NEW[%d] #\n", enemy_list.at(i).name, enemy_list.at(i).walk_range);

        sint16_to_little_endian(enemy_list.at(i).start_point.x);
        sint16_to_little_endian(enemy_list.at(i).start_point.y);
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                uint16_to_little_endian(enemy_list.at(i).sprites[j][k].duration);
                sint16_to_little_endian(enemy_list.at(i).sprites[j][k].collision_rect.x);
                sint16_to_little_endian(enemy_list.at(i).sprites[j][k].collision_rect.y);
                sint16_to_little_endian(enemy_list.at(i).sprites[j][k].collision_rect.w);
                sint16_to_little_endian(enemy_list.at(i).sprites[j][k].collision_rect.h);
            }
        }
        sint16_to_little_endian(enemy_list.at(i).sprites_pos_bg.x);
        sint16_to_little_endian(enemy_list.at(i).sprites_pos_bg.y);
        sint16_to_little_endian(enemy_list.at(i).walk_range);
        int_to_little_endian(enemy_list.at(i).respawn_delay);

    }
}

void GameMediator::wii_convert_object_list() {
    for (int i=0; i<object_list.size(); i++) {
        sint16_to_little_endian(object_list.at(i).distance);
        sint16_to_little_endian(object_list.at(i).frame_duration);
        int_to_little_endian(object_list.at(i).timer);

    }
}

void GameMediator::wii_convert_ai_list() {
    for (int i=0; i<ai_list.size(); i++) {
        for (int j=0; j<AI_MAX_STATES; j++) {
            int_to_little_endian(ai_list.at(i).states[j].chance);
            int_to_little_endian(ai_list.at(i).states[j].action);
            int_to_little_endian(ai_list.at(i).states[j].go_to);
            int_to_little_endian(ai_list.at(i).states[j].go_to_delay);
            int_to_little_endian(ai_list.at(i).states[j].extra_parameter);
        }
        for (int j=0; j<MAX_AI_REACTIONS; j++) {
            int_to_little_endian(ai_list.at(i).reactions[j].action);
            int_to_little_endian(ai_list.at(i).reactions[j].go_to);
            int_to_little_endian(ai_list.at(i).reactions[j].go_to_delay);
            int_to_little_endian(ai_list.at(i).reactions[j].extra_parameter);
        }
    }
}


void GameMediator::wii_convert_projectile_list() {

    printf("$ size(PROJECTILE_TRAJECTORIES)[%d] $\n", sizeof(PROJECTILE_TRAJECTORIES));

    for (int i=0; i<projectile_list.size(); i++) {
        sint16_to_little_endian(projectile_list.at(i).size.width);
        sint16_to_little_endian(projectile_list.at(i).size.height);


        printf("# trajectory[%d].old[%d] #\n", i, projectile_list.at(i).trajectory);
        Uint16 trajectory = projectile_list.at(i).trajectory;
        uint16_to_little_endian(trajectory);
        printf("# trajectory[%d].NEW[%d] #\n", i, trajectory);
    }
}

void GameMediator::wii_convert_player_list() {
    for (int i=0; i<player_list.size(); i++) {
        sint16_to_little_endian(player_list.at(i).sprite_hit_area.x);
        sint16_to_little_endian(player_list.at(i).sprite_hit_area.y);
        sint16_to_little_endian(player_list.at(i).sprite_hit_area.w);
        sint16_to_little_endian(player_list.at(i).sprite_hit_area.h);
        for (int j=0; j<ANIM_TYPE_COUNT; j++) {
            for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                uint16_to_little_endian(player_list.at(i).sprites[j][k].duration);
                sint16_to_little_endian(player_list.at(i).sprites[j][k].collision_rect.x);
                sint16_to_little_endian(player_list.at(i).sprites[j][k].collision_rect.y);
                sint16_to_little_endian(player_list.at(i).sprites[j][k].collision_rect.w);
                sint16_to_little_endian(player_list.at(i).sprites[j][k].collision_rect.h);
            }
        }
        for (int j=0; j<MAX_WEAPON_N; j++) {
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color1.r);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color1.g);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color1.b);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color2.r);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color2.g);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color2.b);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color3.r);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color3.g);
            sint16_to_little_endian(player_list.at(i).weapon_colors[j].color3.b);
        }
    }
}

void GameMediator::wii_convert_anim_tile_list() {
    for (int i=0; i<anim_tile_list.size(); i++) {
        for (int j=0; j<FS_ANIM_TILE_MAX_FRAMES; j++) {
            int_to_little_endian(anim_tile_list.at(i).frame_delay[j]);
        }
    }
}
#endif

GameMediator::GameMediator()
{
    enemy_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_npc>("game_enemy_list.dat");
    object_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_object>("game_object_list.dat");
    ai_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_artificial_inteligence>("game_ai_list.dat");
    projectile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_projectilev2>("data/game_projectile_list_v2.dat");

    anim_tile_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_anim_block>("anim_block_list.dat");
    player_list = fio_cmm.load_from_disk<CURRENT_FILE_FORMAT::file_player>("player_list.dat");

#ifdef WII
    wii_convert_npc_list();
    wii_convert_object_list();
    wii_convert_ai_list();
    wii_convert_projectile_list();
    wii_convert_player_list();
    wii_convert_anim_tile_list();
#endif
    // add some dummy data for game not to crash
    if (projectile_list.size() == 0) {
        projectile_list.push_back(CURRENT_FILE_FORMAT::file_projectilev2());
    }
    if (player_list.size() == 0) {
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            player_list.push_back(CURRENT_FILE_FORMAT::file_player(i));
        }
    }
}

