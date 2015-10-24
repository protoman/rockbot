#include "defines.h"
#include "graphicslib.h"
#include "graphic/draw.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
#include "sceneshow.h"
#include "file/file_io.h"

#include <sstream>

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

#define MAXPATHLEN 256

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
std::string GAMEPATH;
std::string GAMENAME;

bool leave_game = false;
bool GAME_FLAGS[FLAG_COUNT];
int animation_n = 0;


void get_filepath()
{
#ifdef WIN32
    char* buffer;
    if( (buffer = _getcwd( NULL, 0 )) != NULL ) {
        GAMEPATH = std::string(buffer);
        GAMEPATH += "/";
    }
    delete[] buffer;
#else
    char *buffer = new char[MAXPATHLEN];
    char* res = getcwd(buffer, MAXPATHLEN);
    UNUSED(res);
    if(buffer != NULL){
        GAMEPATH = std::string(buffer);
    }
    GAMEPATH += "/";
    delete[] buffer;
#endif

    std::cout << "get_filepath - GAMEPATH:" << GAMEPATH << std::endl;
}

bool check_parameters(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "ERROR: must inform --gamename \"[NAME]\" to run this app." << std::endl;
        return false;
    }


    // check command-line paramethers
    bool ignore_param = false;
    for (int i=1; i<argc; i++) {
        if (ignore_param == true) { // jump a parameter if already was used, no need to re-check
            ignore_param = false;
            continue;
        }
        std::string temp_argv(argv[i]);
        if (temp_argv == "--gamename") {
            if (argc <= i+1) {
                std::cout << "ERROR: no [NAME] informed for --gamename flag." << std::endl;
                return false;
            } else {
                GAMENAME = std::string(argv[i+1]);
                ignore_param = true;
            }
        } else if (temp_argv == "--scenenumber") {
            if (argc <= i+1) {
                std::cout << "ERROR: no [NUMBER] informed for --scenenumber flag." << std::endl;
                return false;
            } else {
                istringstream ss(argv[i+1]);
                int x;
                if (!(ss >> x)) {
                    std::cout << "ERROR: Invalid number '" << argv[i+1] << "' for --scenenumber flag." << std::endl;
                    return false;
                }
                ignore_param = true;
            }
        }
    }
    FILEPATH = GAMEPATH + std::string("/games/") + GAMENAME + std::string("/");
    std::cout << "get_filepath - FILEPATH:" << FILEPATH << std::endl;
    return true;
}

int main(int argc, char *argv[])
{
    get_filepath();

    if (check_parameters(argc, argv) == false) {
        return 0;
    }

    fio.read_game(game_data);
    soundManager.init_audio_system();

    if (graphLib.initGraphics() != true) {
        printf("ERROR intializing graphic\n");
        return -1;
    }

    sceneShow show;
    show.show_scene(0);

    input.wait_keypress();


    return 1;
}
