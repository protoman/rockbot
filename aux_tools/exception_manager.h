#ifndef EXCEPTION_MANAGER_H
#define EXCEPTION_MANAGER_H

#include <string>
#include <SDL/SDL.h>
#include <iostream>
#include <stdexcept>


#ifdef ANDROID
#include <iostream>
#include <iomanip>
#include <unwind.h>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

#endif

struct BacktraceState
{
    void** current;
    void** end;
};

class exception_manager
{
public:
    exception_manager();

    static void throw_param_exception(std::string prefix, std::string param);
    static void throw_file_not_found_exception(std::string prefix, std::string param);
    static void throw_general_exception(std::string prefix, std::string param);
    static std::string get_backtrace();
#ifdef ANDROID
    static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg);
    static size_t captureBacktrace(void** buffer, size_t max);
    static void dumpBacktrace(std::ostream& os, void** buffer, size_t count);
    static void getBacktrace(std::ostringstream& oss, const size_t max);
#endif
};

#endif // EXCEPTION_MANAGER_H
