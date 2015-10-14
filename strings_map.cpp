#include "strings_map.h"

// Global static pointer used to ensure a single instance of the class.
strings_map* strings_map::_instance = NULL;


strings_map::strings_map()
{
}

strings_map *strings_map::get_instance()
{
    if (!_instance) {
        _instance = new strings_map();
    }
    return _instance;

}

std::string strings_map::get_ingame_string(int n)
{
    return fio_str.get_ingame_string(n);
}

std::string strings_map::toupper(const std::string &s)
{
    std::string ret(s.size(), char());
    for(unsigned int i = 0; i < s.size(); ++i) {
        ret[i] = (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i];
    }
    return ret;
}
