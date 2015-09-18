#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include "file_io.h"
#include "convert.h"
#include "../file/convert.h"
#ifdef DREAMCAST
#include <kos.h>
#endif

extern std::string FILEPATH;
extern std::string SAVEPATH;

extern CURRENT_FILE_FORMAT::st_game_config game_config;

extern bool GAME_FLAGS[FLAG_COUNT];

namespace format_v_2_1_1 {

    file_io::file_io()
    {
    }

    void file_io::write_game(format_v_2_1_1::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/game_v211.dat";

        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << std::endl;
            return;
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }

        fp.write(reinterpret_cast<char *>(&data_in), sizeof(struct format_v_2_1_1::file_game));
        fp.close();

        // ----------------------- STAGES ----------------------- //

    }

    void file_io::write_all_stages(format_v_2_1_1::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v211.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << std::endl;
            return;
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_2_1_1::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v_2_1_1::file_game& data_out) const {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/game_v211.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save 211" << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v_2_1_1::file_game), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }


    void file_io::read_all_stages(format_v_2_1_1::file_stages& stages_data_out) const
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v211.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR::read_game - could not load file '" << filename << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        unsigned int res = fread(&stages_data_out, sizeof(format_v_2_1_1::file_stages), 1, fp);
        UNUSED(res);
        fclose(fp);
    }

    void file_io::read_stage(format_v_2_0_3::file_stage &stages_data_out, short stage_n) const
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v211.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read stages 211" << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v_2_0_3::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_2_0_3::file_stage), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }


}



namespace format_v_2_1_2 {

    file_io::file_io()
    {

    }




    void file_io::write_game(format_v_2_1_2::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/game_v212.dat";

        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << std::endl;
            return;
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }

        fp.write(reinterpret_cast<char *>(&data_in), sizeof(struct format_v_2_1_2::file_game));
        fp.close();

        // ----------------------- STAGES ----------------------- //

    }

    void file_io::write_all_stages(format_v_2_1_2::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v212.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << std::endl;
            return;
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_2_1_2::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v_2_1_2::file_game& data_out) const {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/game_v212.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v_2_1_2::file_game), 1, fp);
        if (read_result  == -1) {
            std::cout << ">>file_io::read_game - Error reading struct data from game file '" << filename << "'." << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }


    void file_io::read_all_stages(format_v_2_1_2::file_stages& stages_data_out)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v212.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR::read_game - could not load file '" << filename << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        unsigned int res = fread(&stages_data_out, sizeof(format_v_2_1_2::file_stages), 1, fp);
        UNUSED(res);
        fclose(fp);
    }

    void file_io::read_stage(format_v_2_1_2::file_stage &stages_data_out, short stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v212.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v_2_1_2::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_2_1_2::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            std::fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }

    bool file_io::file_exists(std::string filename) const
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }

    // @TODO - mover tudo para dentro da classe convert
    /*
    void file_io::check_conversion() const
    {
        std::string filename_v211 = std::string(FILEPATH) + "data/game_v211.dat";
        std::string filename_v212 = std::string(FILEPATH) + "data/game_v212.dat";
        if (file_exists(filename_v212) == false && file_exists(filename_v211) == true) {
            convert convert_obj;
            convert_obj.v211_to_v212();
        }
    }
    */

    bool file_io::save_exists() const
    {
#ifdef DREAMCAST
    file_t f;
    vmu_pkg_t pkg;
    int filesize;
    uint8 *data;
    printf("Checking if exists save data\n");
    f = fs_open("/vmu/a1/rbsav", O_RDONLY);

    if(f<0) {
        printf("Error reading save data - save not exists\n");
        return false;
    }
    printf("Save data loaded!\n");
    return true;
#else

        std::string filename = std::string(SAVEPATH) + "/game212.sav";
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            fclose(fp);
            return true;
        }
        return false;
#endif
    }

    void file_io::load_config(format_v_2_1_2::st_game_config& config)
    {

#ifdef DREAMCAST
    file_t f;
    vmu_pkg_t pkg;
    int filesize;
    uint8 *data;
    printf("Loading config data\n");
    f = fs_open("/vmu/a1/rbcfg", O_RDONLY);

    if(f<0) {
        printf("Error reading config data\n");
        return;
    }

    filesize = fs_total(f);
    data=(uint8 *)malloc(filesize);
    fs_read(f, data, filesize);
    vmu_pkg_parse(data, &pkg);
    memcpy(&config,pkg.data,sizeof(struct format_v_2_1_2::st_game_config));
    fs_close(f);

#else

        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v212.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
        } else {
            int read_result = fread(&config, sizeof(struct format_v_2_1_2::st_game_config), 1, fp);
            if (read_result  == -1) {
                printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
                std::fflush(stdout);
                exit(-1);
            }
            fclose(fp);
        }

#endif
        if (config.get_current_platform() != config.platform) {
            config.reset();
        }
#ifndef PC
        config.video_filter = VIDEO_FILTER_NOSCALE;
        std::cout << "IO::load_config - FORCE SET video_filter to " << VIDEO_FILTER_NOSCALE << ", value: " << config.video_filter << std::endl;
#endif
    }

    void file_io::save_config(st_game_config &config) const
    {

#ifdef DREAMCAST
    vmu_pkg_t pkg;
    uint8 *pkg_out;
    file_t f;
    int pkg_size;

    strcpy(pkg.desc_short, "Rockbot");
    strcpy(pkg.desc_long, "Rockbot config file");
    strcpy(pkg.app_id, "RB");
    pkg.icon_cnt = 0;
    pkg.icon_anim_speed = 0;
    pkg.eyecatch_type = VMUPKG_EC_NONE;
    pkg.data_len = sizeof(struct format_v_2_1_2::st_game_config);
    pkg.data = (uint8 *)&config;

    vmu_pkg_build(&pkg, &pkg_out, &pkg_size);
    fs_unlink("/vmu/a1/rbcfg");
    f = fs_open("/vmu/a1/rbcfg", O_WRONLY);

    if(!f) {
        printf("Error writing config file\n");
        return;
    }

    fs_write(f, pkg_out, pkg_size);
    fs_close(f);
#else
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v212.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        fwrite(&config, sizeof(struct format_v_2_1_2::st_game_config), 1, fp);
        fclose(fp);
#endif

    }

    int file_io::read_stage_boss_id(Uint8 stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v212.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }

        // WHY is it missing by 2 bytes?
        int extra_seek = 2 + sizeof(short) + sizeof(char)*CHAR_NAME_SIZE + sizeof(char)*CHAR_FILENAME_SIZE + sizeof(format_v_2_0_3::file_map)*STAGE_MAX_MAPS + sizeof(format_v2_0::file_link)*STAGE_MAX_LINKS;
        fseek(fp, sizeof(format_v_2_1_2::file_stage) * stage_n + extra_seek, SEEK_SET);
        format_v2_0::file_boss boss;
        int read_result = fread(&boss, sizeof(struct format_v2_0::file_boss), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            std::fflush(stdout);
            exit(-1);
        }
        fclose(fp);
        return boss.id_npc;


    }



    void file_io::read_save(format_v_2_1_2::st_save& data_out) const
    {

#ifdef DREAMCAST
    file_t f;
    vmu_pkg_t pkg;
    int filesize;
    uint8 *data;
    printf("Loading save data\n");
    f = fs_open("/vmu/a1/rbsav", O_RDONLY);

    if(f<0) {
        printf("Error reading save data\n");
        return;
    }

    filesize = fs_total(f);
    data=(uint8 *)malloc(filesize);
    fs_read(f, data, filesize);
    vmu_pkg_parse(data, &pkg);
    memcpy(&data_out,pkg.data,sizeof(struct format_v_2_1_2::st_save));
    fs_close(f);

#else
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game212.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save" << std::endl;
            std::fflush(stdout);
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v_2_1_2::st_save), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }

#endif

        if (data_out.items.lifes > 9) {
            data_out.items.lifes = 3;
        }
#ifndef DREAMCAST
        fclose(fp);
#endif
    }

    bool file_io::write_save(format_v_2_1_2::st_save& data_in)
    {

#ifdef DREAMCAST
    vmu_pkg_t pkg;
    uint8 *pkg_out;
    file_t f;
    int pkg_size;

    strcpy(pkg.desc_short, "Rockbot");
    strcpy(pkg.desc_long, "Rockbot save file");
    strcpy(pkg.app_id, "RB");
    pkg.icon_cnt = 0;
    pkg.icon_anim_speed = 0;
    pkg.eyecatch_type = VMUPKG_EC_NONE;
    pkg.data_len = sizeof(struct format_v_2_1_2::st_save);
    pkg.data = (uint8 *)&data_in;

    vmu_pkg_build(&pkg, &pkg_out, &pkg_size);
    fs_unlink("/vmu/a1/rbsav");
    f = fs_open("/vmu/a1/rbsav", O_WRONLY);

    if(!f) {
        printf("Error writing save data\n");
        return false;
    }

    fs_write(f, pkg_out, pkg_size);
    fs_close(f);

    return true;
#else
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game212.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl; std::fflush(stdout);
            return false;
        }
        fwrite(&data_in, sizeof(struct format_v_2_1_2::st_save), 1, fp);
        fclose(fp);
        return true;
    }
#endif
}



namespace format_v_3_0_0 {

    file_io::file_io()
    {

    }


    void file_io::write_game(format_v_3_0_0::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = "";

        /*
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
            //return;
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        */

// -------------------------------------- GAME -------------------------------------- //
        // float version;
        // char name[FS_CHAR_NAME_SIZE];
        // Sint8 semi_charged_projectile_id;
        // Sint8 player_items[FS_PLATER_ITEMS_N];
        // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
        filename = std::string(FILEPATH) + "data/game_properties_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.version), sizeof(float));
        fp.write(reinterpret_cast<char *>(&data_in.name), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.semi_charged_projectile_id), sizeof(Sint8));
        fp.write(reinterpret_cast<char *>(&data_in.player_items), sizeof(Sint8) * FS_PLATER_ITEMS_N);
        fp.write(reinterpret_cast<char *>(&data_in.stage_face_filename), sizeof(char) * MAX_STAGES * FS_FACE_FILENAME_MAX);
        fp.close();

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "data/game_npcs_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        /*
        file_npc_new new_data_npc[FS_GAME_MAX_NPCS];

        for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
            for (int j=0; j<ANIM_TYPE_COUNT; j++) {
                new_data_npc[i].attack_frame_n[j] = data_in.game_npcs[i].attack_frame_n[j];
            }
            sprintf(new_data_npc[i].bg_graphic_filename, "%s", data_in.game_npcs[i].bg_graphic_filename);
            new_data_npc[i].direction = data_in.game_npcs[i].direction;
            new_data_npc[i].facing = data_in.game_npcs[i].facing;
            new_data_npc[i].fly_flag = data_in.game_npcs[i].fly_flag;
            new_data_npc[i].frame_size = data_in.game_npcs[i].frame_size;
            sprintf(new_data_npc[i].graphic_filename, "%s", data_in.game_npcs[i].graphic_filename);
            new_data_npc[i].hp = data_in.game_npcs[i].hp;
            new_data_npc[i].IA_type = data_in.game_npcs[i].IA_type;
            new_data_npc[i].id = data_in.game_npcs[i].id;
            new_data_npc[i].is_boss = data_in.game_npcs[i].is_boss;
            new_data_npc[i].is_ghost = data_in.game_npcs[i].is_ghost;
            new_data_npc[i].is_sub_boss = data_in.game_npcs[i].is_sub_boss;
            sprintf(new_data_npc[i].name, "%s", data_in.game_npcs[i].name);
            for (int j=0; j<FS_NPC_PROJECTILE_N; j++) {
                new_data_npc[i].projectile_id[j] = data_in.game_npcs[i].projectile_id[j];
            }
            new_data_npc[i].shield_type = data_in.game_npcs[i].shield_type;
            new_data_npc[i].speed = data_in.game_npcs[i].speed;
            for (int j=0; j<ANIM_TYPE_COUNT; j++) {
                for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    new_data_npc[i].sprites[j][k] = data_in.game_npcs[i].sprites[j][k];
                }
            }
            new_data_npc[i].sprites_pos_bg = data_in.game_npcs[i].sprites_pos_bg;
            new_data_npc[i].start_point = data_in.game_npcs[i].start_point;
            new_data_npc[i].walk_range = data_in.game_npcs[i].walk_range;
            for (int j=0; j<FS_NPC_WEAKNESSES; j++) {
                new_data_npc[i].weakness[j] = data_in.game_npcs[i].weakness[j];
            }
        }
        fp.write(reinterpret_cast<char *>(&new_data_npc), sizeof(file_npc) * FS_GAME_MAX_NPCS);
        */

        fp.write(reinterpret_cast<char *>(&data_in.game_npcs), sizeof(file_npc) * FS_GAME_MAX_NPCS);
        fp.close();

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "data/game_objects_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.objects), sizeof(file_object) * FS_GAME_MAX_NPCS);
        fp.close();


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "data/game_projectiles_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.projectiles), sizeof(file_projectile) * FS_MAX_PROJECTILES);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "data/game_weapons_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.weapons), sizeof(file_weapon) * FS_MAX_WEAPONS);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "data/game_players_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        /*
        // convert player old to player new
        file_player_new new_data[FS_MAX_PLAYERS];
        for (int i=0; i<FS_MAX_PLAYERS; i++) {
            new_data[i].can_air_dash = data_in.players[i].can_air_dash;
            new_data[i].can_charge_shot = data_in.players[i].can_charge_shot;
            new_data[i].can_double_jump = data_in.players[i].can_double_jump;
            new_data[i].can_shot_diagonal = data_in.players[i].can_shot_diagonal;
            new_data[i].can_slide = data_in.players[i].can_slide;
            for (int j=0; j<FS_COLOR_KEYS_N; j++) {
                new_data[i].color_keys[j] = data_in.players[i].color_keys[j];
            }
            new_data[i].damage_modifier = data_in.players[i].damage_modifier;
            sprintf(new_data[i].face_filename, "%s", data_in.players[i].face_filename);
            new_data[i].full_charged_projectile_id = data_in.players[i].full_charged_projectile_id;
            sprintf(new_data[i].graphic_filename, "%s", data_in.players[i].graphic_filename);
            new_data[i].have_shield = data_in.players[i].have_shield;
            new_data[i].HP = data_in.players[i].HP;
            new_data[i].jump_gravity = data_in.players[i].jump_gravity;
            new_data[i].max_shots = data_in.players[i].max_shots;
            new_data[i].move_speed = data_in.players[i].move_speed;
            sprintf(new_data[i].name, "%s", data_in.players[i].name);
            new_data[i].simultaneous_shots = data_in.players[i].simultaneous_shots;

            //[ANIM_TYPE_COUNT+8][ANIM_FRAMES_COUNT]
            for (int j=0; j<ANIM_TYPE_COUNT; j++) {
                for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    new_data[i].sprites[j][k] = data_in.players[i].sprites[j][k];
                }
            }
            new_data[i].sprite_hit_area = data_in.players[i].sprite_hit_area;
            new_data[i].sprite_size = data_in.players[i].sprite_size;
            for (int j=0; j<MAX_WEAPON_N; j++) {
                new_data[i].weapon_colors[j] = data_in.players[i].weapon_colors[j];
            }
        }

        fp.write(reinterpret_cast<char *>(&new_data), sizeof(file_player_new) * FS_MAX_PLAYERS);
        */

        fp.write(reinterpret_cast<char *>(&data_in.players), sizeof(file_player) * FS_MAX_PLAYERS);
        fp.close();



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "data/game_ai_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.ai_types), sizeof(file_artificial_inteligence) * FS_MAX_AI_TYPES);
        fp.close();


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "data/game_trophies_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.trophies), sizeof(st_file_trophy) * TROPHIES_MAX);
        fp.close();


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "data/game_shop_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_welcome), sizeof(st_shop_dialog));
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_goodbye), sizeof(st_shop_dialog));
        fp.close();


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "data/game_armorPieces_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.armor_pieces), sizeof(st_armor_piece) * FS_PLAYER_ARMOR_PIECES_MAX);
        fp.close();


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "data/game_animTiles_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.anim_tiles), sizeof(st_anim_map_tile) * FS_ANIM_TILES_MAX);
        fp.close();






    }




    void file_io::write_all_stages(format_v_3_0_0::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_3_0_0::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v_3_0_0::file_game& data_out) const {
        FILE *fp;
        std::string filename = "";


        bool USE_NEW_READ = true;
        if (USE_NEW_READ == false) {
            filename = std::string(FILEPATH) + "data/game_v3.dat";
            fp = fopen(filename.c_str(), "rb");
            if (!fp) {
                std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
                fflush(stdout);
                return;
            }
            int read_result = fread(&data_out, sizeof(struct format_v_3_0_0::file_game), 1, fp);
            if (read_result  == -1) {
                std::cout << ">>file_io::read_game - Error reading struct data from game file '" << filename << "'." << std::endl;
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
            return;
        }
// -------------------------------------- GAME -------------------------------------- //
        // float version;
        // char name[FS_CHAR_NAME_SIZE];
        // Sint8 semi_charged_projectile_id;
        // Sint8 player_items[FS_PLATER_ITEMS_N];
        // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
        filename = std::string(FILEPATH) + "data/game_properties_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }


        if (fread(&data_out.version, sizeof(float), 1, fp)  != sizeof(float)) {
            std::cout << ">>file_io::read_game - Error reading struct data [version] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.name, sizeof(char), FS_CHAR_NAME_SIZE, fp)  != (sizeof(char)*FS_CHAR_NAME_SIZE)) {
            std::cout << ">>file_io::read_game - Error reading struct data from [name] game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.semi_charged_projectile_id, sizeof(Sint8), 1, fp)  != sizeof(Sint8)) {
            std::cout << ">>file_io::read_game - Error reading struct data [semi_charged_projectile_id] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.player_items, sizeof(Sint8), FS_PLATER_ITEMS_N, fp) != (sizeof(Sint8)*FS_PLATER_ITEMS_N)) {
            std::cout << ">>file_io::read_game - Error reading struct data [player_items] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        unsigned int var_size = sizeof(char) * MAX_STAGES * FS_FACE_FILENAME_MAX;
        if (fread(&data_out.stage_face_filename, var_size, 1, fp)  != var_size) {
            std::cout << ">>file_io::read_game - Error reading struct data [stage_face_filename] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }


        std::cout << "version: " << data_out.version << ", name: '" << data_out.name << "', semi_charged_projectile_id: " << (int)data_out.semi_charged_projectile_id << std::endl;

        fclose(fp);

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "data/game_npcs_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_npc)*FS_GAME_MAX_NPCS;
        if (fread(&data_out.game_npcs, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[game_npcs] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "data/game_objects_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_object)*FS_GAME_MAX_NPCS;
        if (fread(&data_out.objects, sizeof(file_object), FS_GAME_MAX_NPCS, fp)  != var_size) {
            std::cout << ">>file_io::read_game[objects2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "data/game_projectiles_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_projectile)*FS_MAX_PROJECTILES;
        if (fread(&data_out.projectiles, var_size, 1, fp)  != var_size) {
            std::cout << ">>file_io::read_game[projectiles] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "data/game_weapons_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_weapon)*FS_MAX_WEAPONS;
        if (fread(&data_out.weapons, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[weapons2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- PLAYERS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "data/game_players_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_player)*FS_MAX_PLAYERS;
        if (fread(&data_out.players, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[players2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "data/game_ai_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(file_artificial_inteligence)*FS_MAX_AI_TYPES;
        if (fread(&data_out.ai_types, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[ai_types2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "data/game_trophies_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(st_file_trophy)*TROPHIES_MAX;
        if (fread(&data_out.trophies, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[trophies2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "data/game_shop_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        int read_result1 = fread(&data_out.shop_dialog_welcome, sizeof(st_shop_dialog), 1, fp);
        if (read_result1  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_welcome2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        int read_result2 = fread(&data_out.shop_dialog_goodbye, sizeof(st_shop_dialog), 1, fp);
        if (read_result2  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_goodbye2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "data/game_armorPieces_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(st_armor_piece)*FS_PLAYER_ARMOR_PIECES_MAX;
        if (fread(&data_out.armor_pieces, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[armor_pieces2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "data/game_animTiles_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        var_size = sizeof(st_anim_map_tile)*FS_ANIM_TILES_MAX;
        if (fread(&data_out.anim_tiles, var_size, 1, fp) != var_size) {
            std::cout << ">>file_io::read_game[anim_tiles2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


    }


    void file_io::read_all_stages(format_v_3_0_0::file_stages& stages_data_out)
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v_3_0_0::file_stages));
        fp.close();
    }

    void file_io::read_stage(format_v_3_0_0::file_stage &stages_data_out, short stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            return;
        }
        fseek(fp, sizeof(format_v_3_0_0::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_3_0_0::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }

    bool file_io::file_exists(std::string filename) const
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }

    void file_io::check_conversion() const
    {
        std::string filename_v211 = std::string(FILEPATH) + "data/game_v212.dat";
        std::string filename_v3 = std::string(FILEPATH) + "data/game_v3.dat";


        /// IURI - while we don't finish the new file-format, always convert
        if (file_exists(filename_v3) == false && file_exists(filename_v211) == true) {
            convert convert_obj;
            convert_obj.v212_to_v3();
        }

        // convert save-game to new format
        std::string filename_save_old = std::string(SAVEPATH) + "/game212.sav";

        if (save_exists() == false && file_exists(filename_save_old) == true) {
            convert convert_obj;
            convert_obj.save_v212_to_v3();
        }

        // convert config
        std::string config_v211 = std::string(FILEPATH) + "data/game_v212.dat";
        std::string config_v3 = std::string(FILEPATH) + "data/game_v3.dat";
        if (file_exists(config_v3) == false && file_exists(config_v211) == true) {
            convert convert_obj;
            convert_obj.config_v212_to_v3();
        }
    }

    bool file_io::save_exists() const
    {
        std::string filename = std::string(SAVEPATH) + "/game_v3.sav";
        std::ifstream fp(filename.c_str());
        if (fp.good()) {
            return true;
        }
        return false;
    }

    void file_io::load_config(format_v_3_0_0::st_game_config& config)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v3.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
        } else {
            int read_result = fread(&config, sizeof(struct format_v_3_0_0::st_game_config), 1, fp);
            if (read_result  == -1) {
                printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
        }
        if (config.get_current_platform() != config.platform) {
            config.reset();
        }
#ifndef PC
        config.video_filter = VIDEO_FILTER_NOSCALE;
        std::cout << "IO::load_config - SET video_filter to " << VIDEO_FILTER_NOSCALE << ", value: " << config.video_filter << std::endl;
#endif
    }

    void file_io::save_config(st_game_config &config) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v3.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fwrite(&config, sizeof(struct format_v_3_0_0::st_game_config), 1, fp);
        fclose(fp);
    }

    void file_io::read_save(format_v_3_0_0::st_save& data_out) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game_v3.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save" << std::endl;
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v_3_0_0::st_save), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }

        if (GAME_FLAGS[FLAG_PLAYER_ROCKBOT]) {
            data_out.selected_player = PLAYER_ROCKBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_BETABOT]) {
            data_out.selected_player = PLAYER_BETABOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_CANDYBOT]) {
            data_out.selected_player = PLAYER_CANDYBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_KITTYBOT]) {
            data_out.selected_player = PLAYER_KITTYBOT;
        }

        if (data_out.items.lifes > 9) {
            data_out.items.lifes = 3;
        }

        fclose(fp);
    }

    bool file_io::write_save(format_v_3_0_0::st_save& data_in)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game_v3.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl;
            return false;
        }
        fwrite(&data_in, sizeof(struct format_v_3_0_0::st_save), 1, fp);
        fclose(fp);
        return true;
    }


    int file_io::read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out)
    {
        /// @NOTE: desabilitei essa função pois quando o formato de arquivo muda, causa um crash
        /// Isso provavelmente causará estouro de memória no PSP
        /// Então estou sobrescrevendo o próprio stage_data, passado como referência
        /*
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }

        // WHY is it missing by 2 bytes?
        //int extra_seek = 2 + sizeof(short) + sizeof(char)*CHAR_NAME_SIZE + sizeof(char)*CHAR_FILENAME_SIZE + sizeof(format_v_3_0_0::file_map)*STAGE_MAX_MAPS + sizeof(format_v_3_0_0::file_link)*STAGE_MAX_LINKS;
        int extra_seek = sizeof(short) + sizeof(char)*CHAR_NAME_SIZE + sizeof(char)*CHAR_FILENAME_SIZE + sizeof(format_v_3_0_0::file_map)*STAGE_MAX_MAPS + sizeof(format_v_3_0_0::file_link)*STAGE_MAX_LINKS;
        fseek(fp, sizeof(format_v_3_0_0::file_stage) * stage_n + extra_seek, SEEK_SET);
        format_v_3_0_0::file_boss boss;
        int read_result = fread(&boss, sizeof(struct format_v_3_0_0::file_boss), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            std::fflush(stdout);
            exit(-1);
        }
        std::cout << "FILEIO::READ_STAGE_BOSS_ID::BOSS_ID: " << boss.id_npc << std::endl;
        fclose(fp);
        return boss.id_npc;
        */
        FILE *fp;

        std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v_3_0_0::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_3_0_0::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
        return stages_data_out.boss.id_npc;
    }

    void file_io::read_colormap(SDL_Color (&colormap)[COLOR_COUNT])
    {
        int line_number = 0;
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/rockbot.gpl";

        fp = fopen(filename.c_str(), "rb");

        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }

        char line[128];
        int n = 0;                                      // used to ignore first 4 lines
        while (fgets(line, sizeof line, fp) != NULL) {  // get a line from text
            //std::cout << ">>>>> file_io::read_colormap - line: " << line << std::endl;
            if (n <= 3) {
                n++;
                continue;
            }
            // now split the string by its spaces
            std::vector<std::string> splited_line = split(std::string(line), std::string(" "));
            std::vector<std::string>::iterator it;
            int i = 0;
            for (it=splited_line.begin(); it!=splited_line.end(); it++) {
                //std::cout << "'" << (*it) << "', ";

                int number = 0;
                std::istringstream is((*it));
                is >> number;


                if (i == 0) {
                    colormap[line_number].r = number;
                } else if (i == 1) {
                    colormap[line_number].g = number;
                } else if (i == 2) {
                    colormap[line_number].b = number;
                } else if (i > 2) { // limit to the first 3 elements
                    break;
                }
                //std::cout << "'" << number << "', ";
                i++;
            }
            //std::cout << std::endl;

            line_number++;
            n++;
        }


    }

    void file_io::load_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        scene_sequence.clear();
        format_v_3_0_0::file_scene_sequence temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/sequence.scn";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct format_v_3_0_0::file_scene_sequence));
            scene_sequence.push_back(temp);
        }
        fp.close();
    }

    void file_io::save_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/sequence.scn";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scene_sequence.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scene_sequence.at(i)), sizeof(struct format_v_3_0_0::file_scene_sequence));
        }
        fp.close();
    }

    void file_io::load_scenes(std::vector<file_scene> &scenes)
    {
        scenes.clear();
        format_v_3_0_0::file_scene temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/scenes.scn";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct format_v_3_0_0::file_scene));
            scenes.push_back(temp);
        }
        fp.close();
    }

    void file_io::save_scenes(std::vector<file_scene> &scenes)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/scenes.scn";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scenes.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scenes.at(i)), sizeof(struct format_v_3_0_0::file_scene));
        }
        fp.close();
    }

    std::vector<std::string> file_io::split(std::string str, std::string sep)
    {
        char* cstr = const_cast<char*>(str.c_str());
        char* current;
        std::vector<std::string> arr;
        current = strtok(cstr, sep.c_str());
        while(current != NULL){
            arr.push_back(current);
            current=strtok(NULL, sep.c_str());
        }
        return arr;
    }
}

// ************************************************************************************************************* //

namespace format_v_3_0_1 {

    file_io::file_io()
    {
        sufix = "_v301";
    }



    void file_io::write_game(format_v_3_0_1::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = "";


// -------------------------------------- GAME -------------------------------------- //
        filename = std::string(FILEPATH) + "data/game_properties" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.version), sizeof(float));
        fp.write(reinterpret_cast<char *>(&data_in.name), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.semi_charged_projectile_id), sizeof(Sint8));
        fp.write(reinterpret_cast<char *>(&data_in.player_items), sizeof(Sint8) * FS_PLATER_ITEMS_N);
        fp.write(reinterpret_cast<char *>(&data_in.stage_face_filename), sizeof(char) * MAX_STAGES * FS_FACE_FILENAME_MAX);
        fp.close();

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "data/game_npcs" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        fp.write(reinterpret_cast<char *>(&data_in.game_npcs), sizeof(file_npc) * FS_GAME_MAX_NPCS);
        fp.close();

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "data/game_objects" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.objects), sizeof(file_object) * FS_GAME_MAX_NPCS);
        fp.close();


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "data/game_projectiles" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.projectiles), sizeof(file_projectile) * FS_MAX_PROJECTILES);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "data/game_weapons" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.weapons), sizeof(file_weapon) * FS_MAX_WEAPONS);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "data/game_players" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        fp.write(reinterpret_cast<char *>(&data_in.players), sizeof(file_player) * FS_MAX_PLAYERS);
        fp.close();



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "data/game_ai" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.ai_types), sizeof(file_artificial_inteligence) * FS_MAX_AI_TYPES);
        fp.close();


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "data/game_trophies" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.trophies), sizeof(st_file_trophy) * TROPHIES_MAX);
        fp.close();


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "data/game_shop" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_welcome), sizeof(st_shop_dialog));
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_goodbye), sizeof(st_shop_dialog));
        fp.close();


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "data/game_armorPieces" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.armor_pieces), sizeof(st_armor_piece) * FS_PLAYER_ARMOR_PIECES_MAX);
        fp.close();


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "data/game_animTiles" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.anim_tiles), sizeof(st_anim_map_tile) * FS_ANIM_TILES_MAX);
        fp.close();






    }




    void file_io::write_all_stages(format_v_3_0_1::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_3_0_1::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v_3_0_1::file_game& data_out) const {
        FILE *fp;
        std::string filename = "";


        bool USE_NEW_READ = true;
        if (USE_NEW_READ == false) {
            filename = std::string(FILEPATH) + "data/game" + sufix + ".dat";
            fp = fopen(filename.c_str(), "rb");
            if (!fp) {
                std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
                fflush(stdout);
                return;
            }
            int read_result = fread(&data_out, sizeof(struct format_v_3_0_1::file_game), 1, fp);
            if (read_result  == -1) {
                std::cout << ">>file_io::read_game - Error reading struct data from game file '" << filename << "'." << std::endl;
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
            return;
        }
// -------------------------------------- GAME -------------------------------------- //
        // float version;
        // char name[FS_CHAR_NAME_SIZE];
        // Sint8 semi_charged_projectile_id;
        // Sint8 player_items[FS_PLATER_ITEMS_N];
        // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
        filename = std::string(FILEPATH) + "data/game_properties" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }


        if (unsigned int res = fread(&data_out.version, sizeof(float), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(float): " << sizeof(float) << ", Error reading struct data [version2] from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        if (unsigned int res = fread(&data_out.name, sizeof(char), FS_CHAR_NAME_SIZE, fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(char): " << sizeof(char) << ", Error reading struct data from [name] game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.semi_charged_projectile_id, sizeof(Sint8), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - Error reading struct data [semi_charged_projectile_id] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.player_items, sizeof(Sint8), FS_PLATER_ITEMS_N, fp) != FS_PLATER_ITEMS_N) {
            std::cout << ">>file_io::read_game - Error reading struct data [player_items] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (unsigned int res = fread(&data_out.stage_face_filename, sizeof(char), (MAX_STAGES*FS_FACE_FILENAME_MAX), fp) != MAX_STAGES*FS_FACE_FILENAME_MAX) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stage_face_filename] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }


        std::cout << "version: " << data_out.version << ", name: '" << data_out.name << "', semi_charged_projectile_id: " << (int)data_out.semi_charged_projectile_id << std::endl;

        fclose(fp);

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "data/game_npcs" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.game_npcs, sizeof(file_npc), FS_GAME_MAX_NPCS, fp) != FS_GAME_MAX_NPCS) {
            std::cout << ">>file_io::read_game[game_npcs2] res: - Error reading data from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        fclose(fp);

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "data/game_objects" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.objects, sizeof(file_object), FS_GAME_MAX_NPCS, fp) != FS_GAME_MAX_NPCS) {
            std::cout << ">>file_io::read_game[objects] - Error reading data from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        fclose(fp);


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "data/game_projectiles" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.projectiles, sizeof(file_projectile), FS_MAX_PROJECTILES, fp) != FS_MAX_PROJECTILES) {
            std::cout << ">>file_io::read_game[projectiles2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "data/game_weapons" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.weapons, sizeof(file_weapon), FS_MAX_WEAPONS, fp) != FS_MAX_WEAPONS) {
            std::cout << ">>file_io::read_game[weapons] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- PLAYERS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "data/game_players" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.players, sizeof(file_player), FS_MAX_PLAYERS, fp) != FS_MAX_PLAYERS) {
            std::cout << ">>file_io::read_game[players] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "data/game_ai" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.ai_types, sizeof(file_artificial_inteligence), FS_MAX_AI_TYPES, fp) != FS_MAX_AI_TYPES) {
            std::cout << ">>file_io::read_game[ai_types] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "data/game_trophies" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.trophies, sizeof(st_file_trophy), TROPHIES_MAX, fp) != TROPHIES_MAX) {
            std::cout << ">>file_io::read_game[trophies] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "data/game_shop" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        int read_result1 = fread(&data_out.shop_dialog_welcome, sizeof(st_shop_dialog), 1, fp);
        if (read_result1  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_welcome] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        int read_result2 = fread(&data_out.shop_dialog_goodbye, sizeof(st_shop_dialog), 1, fp);
        if (read_result2  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_goodbye] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "data/game_armorPieces" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.armor_pieces, sizeof(st_armor_piece), FS_PLAYER_ARMOR_PIECES_MAX, fp) != FS_PLAYER_ARMOR_PIECES_MAX) {
            std::cout << ">>file_io::read_game[armor_pieces] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "data/game_animTiles" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.anim_tiles, sizeof(st_anim_map_tile), FS_ANIM_TILES_MAX, fp) != FS_ANIM_TILES_MAX) {
            std::cout << ">>file_io::read_game[anim_tiles] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


    }


    void file_io::read_all_stages(format_v_3_0_1::file_stages& stages_data_out)
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v_3_0_1::file_stages));
        fp.close();
    }

    void file_io::read_stage(format_v_3_0_1::file_stage &stages_data_out, short stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            return;
        }
        fseek(fp, sizeof(format_v_3_0_1::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_3_0_1::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }

    bool file_io::file_exists(std::string filename) const
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }





    void file_io::check_conversion() const
    {
        std::string filename_v3 = std::string(FILEPATH) + "data/game_v3.dat";
        std::string filename_v301 = std::string(FILEPATH) + "data/game" + sufix + ".dat";


        /// IURI - while we don't finish the new file-format, always convert
        if (file_exists(filename_v301) == false && file_exists(filename_v3) == true) {
            std::cout << ">>>>>>>>>>>>> Will convert from 3.0.0 file-format to 3.0.1 <<<<<<<<<<<<<" << std::endl;
            convert convert_obj;
            convert_obj.v300_to_v301();
        }

        // convert save-game to new format
        std::string filename_save_old = std::string(SAVEPATH) + "/game212.sav";
        std::cout << "filename_save_old: '" << filename_save_old << "', new.exists: " << save_exists() << ", old.exists: " << file_exists(filename_save_old) << std::endl;
        if (save_exists() == false && file_exists(filename_save_old) == true) {
            convert convert_obj;
            std::cout << "Convert save 212->301" << std::endl;
            convert_obj.save_v212_to_v301();
        }

        // convert config
        std::string config_v212 = std::string(FILEPATH) + "data/game_v212.dat";
        std::string config_v3 = std::string(FILEPATH) + "data/game_v301.dat";
        if (file_exists(config_v3) == false && file_exists(config_v212) == true) {
            convert convert_obj;
            convert_obj.config_v212_to_v301();
        }

    }

    bool file_io::save_exists() const
    {
        std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
        std::ifstream fp(filename.c_str());
        if (fp.good()) {
            return true;
        }
        return false;
    }

    void file_io::load_config(format_v_3_0_1::st_game_config& config)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v301.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
        } else {
            int read_result = fread(&config, sizeof(struct format_v_3_0_1::st_game_config), 1, fp);
            if (read_result  == -1) {
                printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
        }
        if (config.get_current_platform() != config.platform) {
            config.reset();
        }
#ifndef PC
        config.video_filter = VIDEO_FILTER_NOSCALE;
        std::cout << "IO::load_config - SET video_filter to " << VIDEO_FILTER_NOSCALE << ", value: " << config.video_filter << std::endl;
#endif
        // ### DEBUG ### //
        //config.game_finished = true;
    }

    void file_io::save_config(st_game_config &config) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config_v301.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fwrite(&config, sizeof(struct format_v_3_0_1::st_game_config), 1, fp);
        fclose(fp);
    }

    void file_io::read_save(format_v_3_0_1::st_save& data_out) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save" << std::endl;
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v_3_0_1::st_save), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }


        if (GAME_FLAGS[FLAG_PLAYER_ROCKBOT]) {
            data_out.selected_player = PLAYER_ROCKBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_BETABOT]) {
            data_out.selected_player = PLAYER_BETABOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_CANDYBOT]) {
            data_out.selected_player = PLAYER_CANDYBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_KITTYBOT]) {
            data_out.selected_player = PLAYER_KITTYBOT;
        }




        // ------- DEBUG ------- //
        /*
        data_out.stages[INTRO_STAGE] = 1;
        for (int i=1; i<SKULLCASTLE5; i++) {
            data_out.stages[i] = 1;
        }
        */
        /*
        //data_out.stages[MAGEBOT] = 0;
        //data_out.stages[SEAHORSEBOT] = 1;
        //data_out.stages[DAISIEBOT] = 1;
        //data_out.stages[MUMMYBOT] = 1;
        //data_out.items.lifes = 0;

        data_out.stages[DYNAMITEBOT] = 1;
        data_out.armor_pieces[ARMOR_ARMS] = true;
        data_out.armor_pieces[ARMOR_BODY] = true;
        data_out.armor_pieces[ARMOR_LEGS] = true;
        */

        // ------- DEBUG ------- //


        if (data_out.items.lifes > 9) {
            data_out.items.lifes = 3;
        }

        fclose(fp);
    }

    bool file_io::write_save(format_v_3_0_1::st_save& data_in)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl;
            return false;
        }
        fwrite(&data_in, sizeof(struct format_v_3_0_1::st_save), 1, fp);
        fclose(fp);
        return true;
    }


    int file_io::read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out)
    {
        /// @NOTE: desabilitei essa função pois quando o formato de arquivo muda, causa um crash
        /// Isso provavelmente causará estouro de memória no PSP
        /// Então estou sobrescrevendo o próprio stage_data, passado como referência
        FILE *fp;

        std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v_3_0_1::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v_3_0_1::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
        return stages_data_out.boss.id_npc;
    }

    void file_io::read_colormap(SDL_Color (&colormap)[COLOR_COUNT])
    {
        int line_number = 0;
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "data/rockbot.gpl";

        fp = fopen(filename.c_str(), "rb");

        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            std::fflush(stdout);
            exit(-1);
        }

        char line[128];
        int n = 0;                                      // used to ignore first 4 lines
        while (fgets(line, sizeof line, fp) != NULL) {  // get a line from text
            //std::cout << ">>>>> file_io::read_colormap - line: " << line << std::endl;
            if (n <= 3) {
                n++;
                continue;
            }
            // now split the string by its spaces
            std::vector<std::string> splited_line = split(std::string(line), std::string(" "));
            std::vector<std::string>::iterator it;
            int i = 0;
            for (it=splited_line.begin(); it!=splited_line.end(); it++) {
                //std::cout << "'" << (*it) << "', ";

                int number = 0;
                std::istringstream is((*it));
                is >> number;


                if (i == 0) {
                    colormap[line_number].r = number;
                } else if (i == 1) {
                    colormap[line_number].g = number;
                } else if (i == 2) {
                    colormap[line_number].b = number;
                } else if (i > 2) { // limit to the first 3 elements
                    break;
                }
                //std::cout << "'" << number << "', ";
                i++;
            }
            //std::cout << std::endl;

            line_number++;
            n++;
        }


    }

    void file_io::load_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        scene_sequence.clear();
        format_v_3_0_1::file_scene_sequence temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/sequence.scn";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct format_v_3_0_1::file_scene_sequence));
            scene_sequence.push_back(temp);
        }
        fp.close();
    }

    void file_io::save_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/sequence.scn";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scene_sequence.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scene_sequence.at(i)), sizeof(struct format_v_3_0_1::file_scene_sequence));
        }
        fp.close();
    }

    void file_io::load_scenes(std::vector<file_scene> &scenes)
    {
        scenes.clear();
        format_v_3_0_1::file_scene temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "data/scenes.scn";
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct format_v_3_0_1::file_scene));
            scenes.push_back(temp);
        }
        fp.close();
    }

    void file_io::save_scenes(std::vector<file_scene> &scenes)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "data/scenes.scn";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scenes.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scenes.at(i)), sizeof(struct format_v_3_0_1::file_scene));
        }
        fp.close();
    }

    std::vector<std::string> file_io::split(std::string str, std::string sep)
    {
        char* cstr = const_cast<char*>(str.c_str());
        char* current;
        std::vector<std::string> arr;
        current = strtok(cstr, sep.c_str());
        while(current != NULL){
            arr.push_back(current);
            current=strtok(NULL, sep.c_str());
        }
        return arr;
    }
}



