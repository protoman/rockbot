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
    std::vector<st_file_common_string> get_common_strings();
    st_file_common_string get_common_string(int id);
    std::map<int, st_file_common_string> get_common_strings_map(std::vector<int> id_list);



private:
    // === GAME STRINGS === //
    bool file_exists(std::string filename) const;
    void create_default_ingame_strings();

    // === COMMON STRINGS === //
    void create_default_common_strings();

private:
    std::vector<std::string> string_list;

};
} // namespace

#endif // FIO_STRINGS_H