#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL/SDL.h>


#include "defines.h"
#include "file/format/st_hitPoints.h"

#include "fio_v1.h"

std::string FILEPATH;
std::string SAVEPATH;


int main(int argc, char *argv[])
{
    std::string EXEC_NAME("conversor");

#ifndef WIN32
    EXEC_NAME = "conversor";
#else
    EXEC_NAME = "conversor.exe";
#endif

    std::string argvString = std::string(argv[0]);
    FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());

}
