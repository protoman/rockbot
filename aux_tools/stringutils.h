#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <string.h>

class StringUtils
{
public:
    StringUtils();
    static std::string replace();
    static std::vector<std::string> split(std::string str, std::string sep);
    static void replace_all( std::string& source, const std::string& from, const std::string& to );
    static std::string clean_filename(std::string filename);
};

#endif // STRINGUTILS_H
