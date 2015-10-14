#ifndef STRINGS_MAP_H
#define STRINGS_MAP_H

#include "defines.h"
#include "file/fio_strings.h"

#include <iostream>

class strings_map
{
public:
    strings_map();
    static strings_map* get_instance();
    std::string get_ingame_string(int n);
    std::string toupper(const std::string & s);

private:
    static strings_map* _instance;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
};

#endif // STRINGS_MAP_H
