#ifndef FILE_IO_H
#define FILE_IO_H

#include "format.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#ifdef WII
#include <fat.h>
#elif PS2
#include <fileio.h>
#endif

#include "file/fio_common.h"

extern std::string FILEPATH;
extern std::string SAVEPATH;

/**
 * @brief
 *
 */
class file_io
{
public:


};


// ************************************************************************************************************* //

namespace format_v4 {

    class file_io {

    public:
        file_io();
        void read_game(file_game& data_out) const;
        void write_game(file_game& data_in) const;
        // stages
        void write_all_stages(file_stages& stages_data_in) const;
        void read_all_stages(file_stages &stages_data_out);
        void read_stage(file_stage& stages_data_out, unsigned short stage_n);
        // maps
        void read_all_maps(file_map (&data_out)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        //void write_all_maps(file_map (&data_in)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        //void read_stage_maps(int stage_id, file_map (&data_out)[FS_STAGE_MAX_MAPS]);
        // maps v2
        void read_all_maps_v2(file_map_v2 (&data_out)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        void write_all_maps_v2(file_map_v2 (&data_in)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        void read_stage_maps_v2(int stage_id, file_map_v2 (&data_out)[FS_STAGE_MAX_MAPS]);
        std::vector<CURRENT_FILE_FORMAT::file_map_npc_v2> read_map_enemy_list(int stage_id);
        std::vector<CURRENT_FILE_FORMAT::file_map_object_v2> read_map_object_list(int stage_id);


        bool file_exists(std::string filename) const;
        std::vector<std::string> read_game_list();
        std::vector<std::string> read_directory_list(std::string filename, bool dir_only);
        std::vector<std::string> read_file_list(std::string filename);

        std::string get_save_filename(short save_n);
        bool write_save(st_save& data_in, short save_n);
        bool read_save(st_save& data_out, short save_n);
        bool save_exists(short save_n);
        bool have_one_save_file();
        bool check_convert_old_format_save();
        bool can_access_castle(st_save& data_in);

        void load_config(st_game_config &config);
        void save_config(st_game_config &config) const;
        int read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out);

        void generate_files();

        int get_heart_pieces_number(st_save game_save);

        void read_castle_data(file_castle& data_out);
        void write_castle_data(file_castle& data_in);

#ifdef PS2
        //int file_io::listcdvd(const char *path, entries *FileEntry);
        void ps2_listfiles(std::string filepath, std::vector<std::string> &res);
#elif WII
        void wii_convert_game_data(file_game& data_out);
        void wii_convert_map_data(file_map (&data_out)[FS_STAGE_MAX_MAPS]);
#endif
        std::string get_sufix();


    private:
        std::string sufix;
        fio_common fio_cmm;
    };
}



#endif // FILE_IO_H

