#include "ps3_platform.h"

#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>

#ifdef PS3
#include <sysutil/sysutil.h>
#include <lv2/sysfs.h>
#include <sys/paths.h>
#include <sys/lock.h>
#endif

namespace ps3_platform {

namespace {

std::string g_app_path;

bool file_exists(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::string dirname_of(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return std::string();
    }
    return path.substr(0, pos + 1);
}

} // namespace

} // namespace ps3_platform

#ifdef PS3
// Rockbot is single-threaded. PSL1GHT's recursive newlib locks fail to
// initialise on the current SDK/RPCS3 combination, which makes __atexit
// registration abort during static construction. The recursive lock is only
// used by atexit; make it a no-op so startup can finish.
extern "C" int __libc_lock_init_recursive(__libc_lock_t *lock)
{
    lock->attribute = 0x10; // mark as initialised
    return 0;
}

extern "C" int __libc_lock_acquire_recursive(__libc_lock_t *)
{
    return 0;
}

extern "C" int __libc_lock_try_acquire_recursive(__libc_lock_t *)
{
    return 0;
}

extern "C" int __libc_lock_release_recursive(__libc_lock_t *)
{
    return 0;
}

extern "C" int __libc_lock_close_recursive(__libc_lock_t *)
{
    return 0;
}
#endif

namespace ps3_platform {

void setup(const char *argv0)
{
    if (argv0 != NULL && argv0[0] == '/') {
        g_app_path = dirname_of(std::string(argv0));
    }
}

void get_filepath(std::string &filepath, int maxpathlen)
{
    (void)maxpathlen;

    // Homebrew launched from the XMB is mounted at /app_home by PSL1GHT.
    // When launched by ps3load/RPCS3 argv[0] points at the EBOOT.BIN instead.
    std::vector<std::string> candidates;
#ifdef PS3
    candidates.push_back(std::string(SYS_APP_HOME) + "/");
    candidates.push_back("/dev_hdd0/game/ROCKBOT01/USRDIR/");
#endif
    if (!g_app_path.empty()) {
        candidates.push_back(g_app_path);
    }

    for (size_t i = 0; i < candidates.size(); i++) {
        if (file_exists(candidates.at(i) + "games")) {
            filepath = candidates.at(i);
            return;
        }
    }

    // Last resort: the directory the EBOOT was loaded from.
    if (!g_app_path.empty()) {
        filepath = g_app_path;
    } else {
        filepath = "/app_home/";
    }
}

void get_savepath(std::string &savepath)
{
    // /dev_hdd0/tmp is writable by homebrew without an installed package.
    savepath = "/dev_hdd0/tmp/rockbot";
#ifdef PS3
    sysFsMkdir(savepath.c_str(), 0777);
#endif
}

void update()
{
#ifdef PS3
    sysUtilCheckCallback();
#endif
}

void get_default_keys(int keys_codes_copy[BTN_COUNT])
{
    // No keyboard on a stock PS3; joystick defaults are used.
    for (int i = 0; i < BTN_COUNT; i++) {
        keys_codes_copy[i] = -1;
    }
}

void get_default_buttons(st_input_button_config button_codes_copy[BTN_COUNT])
{
    // SDL_PSL1GHT joystick button numbers (src/joystick/psl1ght):
    // 0 left, 1 down, 2 right, 3 up, 4 start, 7 select,
    // 8 square, 9 cross, 10 circle, 11 triangle, 12 R1, 13 L1,
    // 14 R2, 15 L2.
    // Directions stay on the left analog stick (axes 0/1), like other ports.
    button_codes_copy[BTN_ATTACK].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_ATTACK].value = 8;   // square
    button_codes_copy[BTN_JUMP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_JUMP].value = 9;     // cross
    button_codes_copy[BTN_DASH].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DASH].value = 10;    // circle
    button_codes_copy[BTN_SHIELD].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_SHIELD].value = 11;  // triangle
    button_codes_copy[BTN_L].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_L].value = 13;       // L1
    button_codes_copy[BTN_R].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_R].value = 12;       // R1
    button_codes_copy[BTN_QUIT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_QUIT].value = 7;     // select
    button_codes_copy[BTN_START].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_START].value = 4;    // start
}

void set_default_input_mode(E_INPUT_TYPES *input_type, E_INPUT_MODES *input_mode)
{
    *input_type = INPUT_TYPE_JOYSTICK;
    *input_mode = INPUT_MODE_DIGITAL;
}

E_PLATFORM get_platform()
{
    return PLATFORM_PS3;
}

} // namespace ps3_platform
