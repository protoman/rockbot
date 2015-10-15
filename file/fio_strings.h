#ifndef FIO_STRINGS_H
#define FIO_STRINGS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>

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
    std::vector<std::string> load_game_strings_from_file(std::string filename);
    void save_game_strings(std::vector<std::string> list, std::string filename);

private:

    bool file_exists(std::string filename) const;
    void create_default_ingame_strings();

private:
    std::vector<std::string> string_list;

};
} // namespace

#endif // FIO_STRINGS_H
