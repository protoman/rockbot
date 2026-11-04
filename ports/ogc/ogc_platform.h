#pragma once

// Boot log for the libogc ports (Wii, GameCube): the loaders give no console, so the game appends
// its progress to rockbot_boot.log (next to the game data when known, else the SD card root).

#include <cstdio>
#include <cstdarg>
#include <string>

extern std::string FILEPATH; // main.cpp

namespace ogc_platform {

inline void log(const char *fmt, ...)
{
    std::FILE *f = NULL;
    if (!FILEPATH.empty()) {
        f = std::fopen((FILEPATH + "rockbot_boot.log").c_str(), "a");
    }
    const char *fallbacks[] = { "rockbot_boot.log", "sd:/rockbot_boot.log", "carda:/rockbot_boot.log" };
    for (size_t i = 0; f == NULL && i < sizeof(fallbacks) / sizeof(fallbacks[0]); i++) {
        f = std::fopen(fallbacks[i], "a");
    }
    if (f == NULL) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    std::vfprintf(f, fmt, ap);
    va_end(ap);
    std::fputc('\n', f);
    std::fclose(f);
}

} // namespace ogc_platform
