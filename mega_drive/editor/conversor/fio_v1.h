#ifndef FIO_V1_H
#define FIO_V1_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include "v1.h"

#define COLOR_COUNT 68

class fio_v1
{
public:
    fio_v1();
    void read_game(v1_file_game& data_out) const;
    void write_game(v1_file_game& data_in) const;
    void read_all_stages(v1_file_stages &stages_data_out);
    void read_stage(v1_file_stage& stages_data_out, short stage_n);
    bool file_exists(std::string filename) const;
    void check_conversion() const;

    /// @TODO
    //void read_save(v1_st_save& data_out) const;
    //bool save_exists() const;
    //void load_config(v1_st_game_config &config);

    int read_stage_boss_id(int stage_n, v1_file_stage &stages_data_out);

    void read_colormap(SDL_Color (&colormap)[COLOR_COUNT]);

private:
    std::vector<std::string> split(std::string str,std::string sep);
    std::string sufix;

};

#endif // FIO_V1_H
