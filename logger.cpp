#include "logger.h"

#ifdef ANDROID
#include <android/log.h>
#endif


RockbotLogger* RockbotLogger::_instance = NULL;

RockbotLogger *RockbotLogger::get_instance()
{
    if (!_instance) {
        _instance = new RockbotLogger();
    }
    return _instance;
}

void RockbotLogger::write(std::string log_str)
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "%s", log_str.c_str());
#else
    std::cout << log_str << std::endl;
#endif
}

RockbotLogger::RockbotLogger()
{

}
