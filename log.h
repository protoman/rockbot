#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>

class log
{
public:
    static log* get_instance();
    void write(std::string log_str);

private:
    log();

private:
    static log* _instance;
};

#endif // LOG_H
