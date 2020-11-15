#include "defines.h"
#include "graphicslib.h"
#include "graphic/draw.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
#include "sceneshow.h"
#include "file/file_io.h"

#include <sstream>

#if defined(LINUX) || defined(OSX) || defined(RASPBERRY)
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

enum e_EXEC_TYPE {
    e_EXEC_TYPE_NORMAL,
    e_EXEC_TYPE_PARALLAX,
    e_EXEC_TYPE_IMAGE,
    e_EXEC_TYPE_COUNT
};

std::string FILEPATH;
std::string SAVEPATH;
std::string GAMEPATH;
std::string GAMENAME;

bool leave_game = false;
bool GAME_FLAGS[FLAG_COUNT];
int animation_n = 0;
e_EXEC_TYPE exec_type = e_EXEC_TYPE_NORMAL;
int param_number = 0;

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
}



bool check_parameters(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "ERROR: must inform --gamename \"[NAME]\" to run this app." << std::endl;
        return false;
    }


    // check command-line paramethers
    for (int i=1; i<argc; i++) {
        std::string temp_argv(argv[i]);
        if (temp_argv == "--gamename") {
            if (argc <= i+1) {
                std::cout << "ERROR: no [NAME] informed for --gamename flag." << std::endl;
                return false;
            } else {
                GAMENAME = std::string(argv[i+1]);
            }
        } else if (temp_argv == "--scenenumber") {
            if (argc <= i+1) {
                std::cout << "ERROR: no [NUMBER] informed for --scenenumber flag." << std::endl;
                return false;
            } else {
                istringstream ss(argv[i+1]);
                if (!(ss >> animation_n)) {
                    std::cout << "ERROR: Invalid number '" << argv[i+1] << "' for --scenenumber flag." << std::endl;
                    return false;
                }
            }
        } else if (temp_argv == "--parallax") {
            if (argc <= i+1) {
                std::cout << "ERROR: no [NUMBER] informed for --parallax flag." << std::endl;
                return false;
            } else {
                istringstream ss(argv[i+1]);
                exec_type = e_EXEC_TYPE_PARALLAX;
                if (!(ss >> animation_n)) {
                    std::cout << "ERROR: Invalid number '" << argv[i+1] << "' for --scenenumber flag." << std::endl;
                    return false;
                }
            }
        }
    }
    FILEPATH = GAMEPATH + std::string("/games/") + GAMENAME + std::string("/");
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
    if (exec_type == e_EXEC_TYPE_PARALLAX) {
        show.show_parallax(animation_n);
    } else {
        show.show_scene(animation_n);
    }

    int BORDER_SIZE = 4;
    // horizontal lines
    graphLib.draw_rectangle(st_rectangle(0, 0, RES_W, BORDER_SIZE), 0, 200, 0, 180);
    graphLib.draw_rectangle(st_rectangle(0, RES_H-BORDER_SIZE, RES_W, BORDER_SIZE), 0, 200, 0, 180);
    // vertical lines
    graphLib.draw_rectangle(st_rectangle(0, 0, BORDER_SIZE, RES_H), 0, 200, 0, 180);
    graphLib.draw_rectangle(st_rectangle(RES_W-BORDER_SIZE, 0, BORDER_SIZE, RES_H), 0, 200, 0, 180);
    graphLib.updateScreen();


    input.wait_keypress();

    return 1;
}
