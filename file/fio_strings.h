#ifndef FIO_STRINGS_H
#define FIO_STRINGS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <map>

#include "file/v4/file_strings.h"

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

    // === GAME STRINGS === //
    std::string get_ingame_string(int n);
    std::vector<std::string> load_game_strings();
    std::vector<std::string> load_game_strings_from_file(std::string filename);
    void save_game_strings(std::vector<std::string> list, std::string filename);

    // === COMMON STRINGS === //
    std::string get_common_strings_filename();
    std::string get_game_strings_filename();
    std::vector<std::string> get_common_strings();
    std::string get_common_string(int id);
    void save_common_strings(std::vector<std::string> data);
    void create_files();

private:
    // === GAME STRINGS === //
    bool file_exists(std::string filename) const;
    void create_default_ingame_strings();

    // === COMMON STRINGS === //
    void create_default_common_strings();

private:
    std::vector<std::string> string_list;
    std::vector<std::string> common_strings_list;

};
} // namespace

#endif // FIO_STRINGS_H
