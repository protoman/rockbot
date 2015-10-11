#ifndef FIO_STRINGS_H
#define FIO_STRINGS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>

#include "file/v4/file_strings_v4.h"

class fio_strings
{
public:
    fio_strings();
};


// namespace
namespace format_v4 {
class fio_strings
{
public:
    fio_strings();
    std::string get_ingame_string(int n);
    std::vector<std::string> load_game_strings();

private:

    bool file_exists(std::string filename) const;
    void create_default_ingame_strings();

private:
    std::vector<std::string> string_list;

};
} // namespace

#endif // FIO_STRINGS_H
