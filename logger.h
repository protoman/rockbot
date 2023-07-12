#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class RockbotLogger
{
public:
    static RockbotLogger* get_instance();
    void write(std::string log_str);

private:
    RockbotLogger();

private:
    static RockbotLogger* _instance;
};

#endif // LOGGER_H
