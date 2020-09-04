#include "log.h"

#ifdef ANDROID
#include <android/log.h>
#endif


log* log::_instance = NULL;

log *log::get_instance()
{
    if (!_instance) {
        _instance = new log();
    }
    return _instance;
}

void log::write(std::string log_str)
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", log_str.c_);
#else
    std::cout << log_str << std::endl;
#endif
}

log::log()
{

}
