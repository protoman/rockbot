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
    void save_game_strings(std::vector<std::string> list);

private:

    bool file_exists(std::string filename) const;
    void create_default_ingame_strings();
    void replaceAll( std::string& source, const std::string& from, const std::string& to ) const;

private:
    std::vector<std::string> string_list;

};
} // namespace

#endif // FIO_STRINGS_H
