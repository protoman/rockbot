#include "mainwindow.h"
#include "../../defines.h"
#include "../../scenes/password_generator.h"

#include <QApplication>

std::string FILEPATH;
std::string SAVEPATH;
bool GAME_FLAGS[FLAG_COUNT];

#define MAXPATHLEN 256

#if defined(LINUX) || defined(OSX)
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/param.h>
#elif defined(WIN32)
    #include <direct.h>
    #undef main // to build on win32
#endif

password_matrix matrix;

void get_filepath()
{
#ifdef WIN32
    char* buffer;
    if( (buffer = _getcwd( NULL, 0 )) != NULL ) {
        FILEPATH = std::string(buffer);
        FILEPATH += "/";
    }
    delete[] buffer;
#elif defined OSX
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        FILEPATH = std::string(path);
        FILEPATH = FILEPATH.substr(0, FILEPATH.length()-7);
    }
#else
    char *buffer = new char[MAXPATHLEN];
    getcwd(buffer, MAXPATHLEN);
    if(buffer != NULL){
        FILEPATH = std::string(buffer);
    }
    FILEPATH += "/";
    delete[] buffer;
#endif

#ifdef LINUX
    SAVEPATH = std::string(getenv("HOME")) + "/.rockbot/";
    mkdir(SAVEPATH.c_str(), 0777);
    //std::cout << "SAVEPATH: " << SAVEPATH << ", mkdir-res: " << res << ", errno: " << errno << std::endl;
#elif WIN32
    SAVEPATH =  std::string(getenv("APPDATA")) + "/rockbot";
    std::cout << "SAVEPATH: " << SAVEPATH << std::endl;
    _mkdir(SAVEPATH.c_str());
#elif DREAMCAST
    SAVEPATH = "/vmu/a1";

#else
    SAVEPATH = FILEPATH;
#endif
}


int main(int argc, char *argv[])
{
    for (int i=0; i<FLAG_COUNT; i++) {
        GAME_FLAGS[i] = false;
    }
    get_filepath();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
