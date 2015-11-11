#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL/SDL.h>
#include <iostream>

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

    std::cout << "AAA" << std::endl;

    std::string argvString = std::string(argv[0]);
    FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());

    fio_v1 fv1;

    v1_file_game game;
    fv1.read_game(game);

    v1_file_stages stages;
    fv1.read_all_stages(stages);

    std::cout << "AAA" << std::endl;

}
