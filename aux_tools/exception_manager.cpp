#include "exception_manager.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>


exception_manager::exception_manager()
{

}


void exception_manager::throw_param_exception(std::string prefix, std::string param)
{
    std::string backtrace = get_backtrace();
    char error_msg[512+backtrace.size()];
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "throw_param_exception");
    sprintf(error_msg, "Exception: Invalid parameter [%s] - value[%s]\nBacktrace:\n[%s]", prefix.c_str(), param.c_str(), backtrace.c_str());
    throw std::invalid_argument(error_msg);
}

void exception_manager::throw_file_not_found_exception(std::string prefix, std::string param)
{
    std::string backtrace = get_backtrace();
    char error_msg[512+backtrace.size()];
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "throw_file_not_found_exception");
    sprintf(error_msg, "Exception: file not found[%s] - file[%s]\nBacktrace:\n[%s]", prefix.c_str(), param.c_str(), backtrace.c_str());
    throw std::invalid_argument(error_msg);
}

void exception_manager::throw_general_exception(std::string prefix, std::string param)
{
    std::string backtrace = get_backtrace();
    char error_msg[512+backtrace.size()];
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "throw_general_exception");
    sprintf(error_msg, "Exception: runtime error[%s] - code[%s]\nBacktrace:\n[%s]", prefix.c_str(), param.c_str(), backtrace.c_str());
    throw std::runtime_error(error_msg);
}

#ifdef ANDROID
_Unwind_Reason_Code exception_manager::unwindCallback(struct _Unwind_Context* context, void* arg) {
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

size_t  exception_manager::captureBacktrace(void** buffer, size_t max) {
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::captureBacktrace #1");
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::captureBacktrace #2");
    return state.current - buffer;
}

void exception_manager::dumpBacktrace(std::ostream& os, void** buffer, size_t count) {
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::dumpBacktrace #1");
    for (size_t idx = 0; idx < count; ++idx) {
        const void* addr = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
        }

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
    }
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::dumpBacktrace #2");
}

void exception_manager::getBacktrace(std::ostringstream &oss, const size_t max)
{
    void* buffer[max];
    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
}
#endif

std::string exception_manager::get_backtrace()
{
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace #1");
    std::ostringstream oss;
    getBacktrace(oss, 30);
    //__android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace - res: %s", oss.str().c_str());
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace #2.0");
    char str_msg[50000];
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace #2.1");
    sprintf(str_msg, "%s", oss.str().c_str());

    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace #3");
    std::string str(str_msg);
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "exception_manager::get_backtrace #4 str[%s]", str.c_str());
    return str;
}
