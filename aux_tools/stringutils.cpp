#include "stringutils.h"



StringUtils::StringUtils()
{

}

std::vector<std::string> StringUtils::split(std::string str, std::string sep)
{
    char* cstr = const_cast<char*>(str.c_str());
    char* current;
    std::vector<std::string> arr;
    current = strtok(cstr, sep.c_str());
    while(current != NULL){
        arr.push_back(current);
        current=strtok(NULL, sep.c_str());
    }
    return arr;
}

void StringUtils::replace_all(std::string &source, const std::string &from, const std::string &to)
{
    std::string newString;
    newString.reserve( source.length() );  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while( std::string::npos != ( findPos = source.find( from, lastPos )))
    {
        newString.append( source, lastPos, findPos - lastPos );
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr( lastPos );

    source.swap( newString );
}

std::string StringUtils::clean_filename(std::string filename)
{
    replace_all(filename, "//", "/");
    replace_all(filename, "//", "/"); // twice because first time could will have "//" if "///"
    replace_all(filename, "\\", "/");

    return filename;
}

