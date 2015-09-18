#include "defines.h"
#include "graphicslib.h"
#include "graphic/draw.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
#include "sceneshow.h"

#if defined(LINUX) || defined(OSX)
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/param.h>
    std::string EXEC_NAME("rockbot");
#elif defined(WIN32)
    #include <direct.h>
    #undef main // to build on win32
    std::string EXEC_NAME("rockbot.exe");
#endif


// --- EXTERNAL --- //
graphicsLib graphLib;
timerLib timer;
inputLib input;
soundLib soundManager;
CURRENT_FILE_FORMAT::st_game_config game_config;
CURRENT_FILE_FORMAT::st_save game_save;
CURRENT_FILE_FORMAT::file_io fio;
CURRENT_FILE_FORMAT::file_game game_data;
CURRENT_FILE_FORMAT::file_stage stage_data;
SDL_Event event;
std::string FILEPATH;
std::string SAVEPATH;
bool leave_game = false;
bool GAME_FLAGS[FLAG_COUNT];


void get_filepath()
{
#ifdef WIN32
    char* buffer;
    if( (buffer = _getcwd( NULL, 0 )) != NULL ) {
        FILEPATH = std::string(buffer);
        FILEPATH += "/";
    }
    delete[] buffer;
#else
    char *buffer = new char[MAXPATHLEN];
    char* res = getcwd(buffer, MAXPATHLEN);
    UNUSED(res);
    if(buffer != NULL){
        FILEPATH = std::string(buffer);
    }
    FILEPATH += "/";
    delete[] buffer;
#endif

    std::cout << "get_filepath - FILEPATH:" << FILEPATH << std::endl;
}

int main(int argc, char *argv[])
{
    get_filepath();
    fio.read_game(game_data);
    soundManager.init_audio_system();

    if (graphLib.initGraphics() != true) {
        printf("ERROR intializing graphic\n");
        return -1;
    }

    sceneShow show;
    show.show_image(0);
    show.show_text(0);


    return 1;
}
