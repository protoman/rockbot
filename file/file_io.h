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

namespace format_v_3_0_1 {

    class file_io {

    public:
        file_io();
        void read_game(format_v_3_0_1::file_game& data_out) const;
        void write_game(format_v_3_0_1::file_game& data_in) const;
        void write_all_stages(format_v_3_0_1::file_stages& stages_data_in) const;
        void read_all_stages(format_v_3_0_1::file_stages &stages_data_out);
        void read_stage(format_v_3_0_1::file_stage& stages_data_out, short stage_n);

        bool file_exists(std::string filename) const;
        bool directory_exists(std::string filename) const;
        std::vector<std::string> read_game_list() const;

        bool write_save(format_v_3_0_1::st_save& data_in);
        void read_save(format_v_3_0_1::st_save& data_out) const;
        bool save_exists() const;

        void load_config(format_v_3_0_1::st_game_config &config);
        void save_config(format_v_3_0_1::st_game_config &config) const;
        int read_stage_boss_id(Uint8 stage_n, format_v_3_0_1::file_stage &stages_data_out);

        void load_scene_sequence(std::vector<format_v_3_0_1::file_scene_sequence>& scene_sequence);
        void save_scene_sequence(std::vector<format_v_3_0_1::file_scene_sequence>& scene_sequence);

        void load_scenes(std::vector<format_v_3_0_1::file_scene>& scenes);
        void save_scenes(std::vector<format_v_3_0_1::file_scene>& scenes);

        void generate_files();


    private:
        std::vector<std::string> split(std::string str,std::string sep);
        std::string sufix;


    };
}



#endif // FILE_IO_H

