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
        void read_stage(file_stage& stages_data_out, short stage_n);
        // maps
        void read_all_maps(file_map (&data_out)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        void write_all_maps(file_map (&data_in)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS]);
        void read_stage_maps(int stage_id, file_map (&data_out)[FS_STAGE_MAX_MAPS]);

        bool file_exists(std::string filename) const;
        std::vector<std::string> read_game_list() const;
        std::vector<std::string> read_directory_list(std::string filename, bool dir_only) const;
        std::vector<std::string> read_file_list(std::string filename) const;

        bool write_save(st_save& data_in);
        void read_save(st_save& data_out) const;
        bool save_exists() const;

        void load_config(st_game_config &config);
        void save_config(st_game_config &config) const;
        int read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out);

        void generate_files();

        int get_heart_pieces_number(st_save game_save);


    private:
        std::string sufix;
        fio_common fio_cmm;

    };
}



#endif // FILE_IO_H

