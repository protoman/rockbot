#ifndef FIO_DIAG_H
#define FIO_DIAG_H

// File operation diagnostics, header only so every platform build picks it up.
//
// Every open/read/write/close in the game goes through these helpers, so a failure is never silent and always says
// WHICH operation, WHICH file and WHY. What the helpers know that a bare "could not open" does not:
//  - errno is read right after the failing call (logging itself uses stdio and would overwrite it),
//  - C++ streams (ifstream/ofstream) do not report errno reliably, so after a failed open the helper tries a plain
//    fopen of its own to get one,
//  - the file is stat()ed (exists? size, mode, owner) and its folder is checked,
//  - on the PS3, PSL1GHT turns any lv2 error code it does not know into EINVAL, so the raw lv2 code is read with a
//    direct sysLv2FsOpen call and logged in hex (e.g. 0x80010006 ENOENT, 0x8001000D EACCES... see sys/lv2errno).
// The same failure of the same file is reported once, so a loop that reads a missing file does not flood the log.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#ifdef _WIN32
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long ms);
#else
#include <unistd.h>
#endif

#ifdef PS3
#include "../ports/ps3/ps3_platform.h"
#include <sys/file.h>
#endif
#ifdef PSP
#include "../ports/psp/psp_platform.h"
#endif
#ifdef VITA
#include "../ports/vita/vita_platform.h"
#endif
#ifdef ANDROID
#include <android/log.h>
#endif

namespace fio_diag {

// one line to stdout and to the platform's own log file
inline void say(const char *fmt, ...)
{
    char buffer[768];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    std::cout << buffer << std::endl;
#ifdef PS3
    ps3_platform::log("%s", buffer);
    ps3_platform::log_flush();
#endif
#ifdef PSP
    psp_platform::log("%s", buffer);
#endif
#ifdef VITA
    vita_platform::log("%s", buffer);
#endif
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "Rockbot", "%s", buffer);
#endif
}

// Waits between two attempts. On the PS3 the system callbacks keep being served meanwhile, so the PS button works.
inline void pause_ms(int ms)
{
#ifdef PS3
    for (int waited = 0; waited < ms; waited += 10) {
        ps3_platform::update();
        usleep(10 * 1000);
    }
#elif defined(_WIN32)
    Sleep((unsigned long)ms);
#else
    usleep((useconds_t)ms * 1000);
#endif
}

// A failed open/read/write is tried again (a console file system can fail once and work on the next call):
// 3 attempts, 60 ms and 150 ms apart. A file that does not exist is not retried, that answer will not change.
const int RETRY_ATTEMPTS = 3;
inline int retry_delay_ms(int attempt) { return attempt == 1 ? 60 : 150; }
inline bool worth_retrying(int err) { return err != ENOENT && err != ENOTDIR; }

// Set by the game at startup. The tools (editor, converters) leave it off, they must survive a missing file.
inline bool &fatal_enabled() { static bool enabled = false; return enabled; }

// true the first time (where, path, what) is seen, and only for the first 300 distinct failures
inline bool first_time(const char *where, const std::string &path, const char *what)
{
    static std::set<std::string> seen;
    std::string key = std::string(where) + "|" + path + "|" + what;
    if (seen.size() >= 300 || seen.count(key) > 0) {
        return false;
    }
    seen.insert(key);
    return true;
}

// folder part of a path ("" when there is none)
inline std::string folder_of(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return "";
    }
    return path.substr(0, pos);
}

// what the file system says about a path, as text
inline std::string describe_path(const std::string &path)
{
    char buffer[200];
    struct stat st;
    errno = 0;
    if (stat(path.c_str(), &st) == 0) {
        snprintf(buffer, sizeof(buffer), "file exists (size=%ld mode=%o uid=%d gid=%d)", (long)st.st_size, (unsigned)st.st_mode, (int)st.st_uid, (int)st.st_gid);
    } else {
        int e = errno;
        snprintf(buffer, sizeof(buffer), "stat failed (errno %d: %s)", e, strerror(e));
    }
    std::string res = buffer;
    std::string folder = folder_of(path);
    if (folder.length() > 0) {
        errno = 0;
        if (stat(folder.c_str(), &st) == 0) {
            snprintf(buffer, sizeof(buffer), "; folder ok (mode=%o)", (unsigned)st.st_mode);
        } else {
            int e = errno;
            snprintf(buffer, sizeof(buffer), "; folder '%s' stat failed (errno %d: %s)", folder.c_str(), e, strerror(e));
        }
        res += buffer;
    }
    return res;
}

#ifdef PS3
// the raw lv2 result of opening the path read-only and (when asked) write-only, without creating or truncating anything
inline std::string raw_lv2_probe(const std::string &path, bool also_write)
{
    char buffer[160];
    s32 fd = -1;
    int r = sysLv2FsOpen(path.c_str(), SYS_O_RDONLY, &fd, 0, NULL, 0);
    if (r == 0) {
        sysLv2FsClose(fd);
    }
    int w = 0;
    if (also_write) {
        fd = -1;
        w = sysLv2FsOpen(path.c_str(), SYS_O_WRONLY, &fd, 0, NULL, 0);
        if (w == 0) {
            sysLv2FsClose(fd);
        }
    }
    if (also_write) {
        snprintf(buffer, sizeof(buffer), "; raw lv2 open: read=0x%08X write=0x%08X (0 = ok)", (unsigned)r, (unsigned)w);
    } else {
        snprintf(buffer, sizeof(buffer), "; raw lv2 open: read=0x%08X (0 = ok)", (unsigned)r);
    }
    return buffer;
}
#endif

inline bool mode_writes(const char *mode)
{
    return (mode != NULL && (strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL || strchr(mode, '+') != NULL));
}

// Reports a failed open. 'err' is the errno read right after the failing call (0 = unknown, the caller used a C++
// stream); 'stream' is true for ifstream/ofstream, where a plain fopen is tried to obtain a real errno.
inline void report_open_failed(const char *where, const std::string &path, const char *mode, int err, bool stream, bool optional)
{
    if (first_time(where, path, (std::string("open ") + (mode != NULL ? mode : "")).c_str()) == false) {
        return;
    }
    int probe_errno = 0;
    if (stream == true) {
        errno = 0;
        FILE *probe = fopen(path.c_str(), mode_writes(mode) ? "r+b" : "rb");
        probe_errno = errno;
        if (probe != NULL) {
            fclose(probe);
            probe_errno = 0;
        }
    }
    std::string state = describe_path(path);
    std::string raw;
#ifdef PS3
    raw = raw_lv2_probe(path, mode_writes(mode));
#endif
    if (stream == true) {
        say("%s: %s: cannot open '%s' (mode %s); stream gives no errno, plain fopen probe: errno %d (%s); %s%s",
            optional ? "NOTE" : "ERROR", where, path.c_str(), mode, probe_errno, probe_errno == 0 ? "opens fine" : strerror(probe_errno),
            state.c_str(), raw.c_str());
    } else {
        say("%s: %s: cannot open '%s' (mode %s): errno %d (%s); %s%s",
            optional ? "NOTE" : "ERROR", where, path.c_str(), mode, err, strerror(err), state.c_str(), raw.c_str());
    }
}

// fopen that retries and explains its failure. optional: a missing file is normal here (it is logged as a note).
inline FILE *fopen_logged(const char *where, const std::string &path, const char *mode, bool optional = false)
{
    int first_err = 0;
    for (int attempt = 1; attempt <= RETRY_ATTEMPTS; attempt++) {
        errno = 0;
        FILE *fp = fopen(path.c_str(), mode);
        int err = errno;
        if (fp != NULL) {
            if (attempt > 1) {
                say("NOTE: %s: '%s' (mode %s) opened on attempt %d, the first failed with errno %d (%s)", where, path.c_str(), mode, attempt, first_err, strerror(first_err));
            }
            return fp;
        }
        if (attempt == 1) {
            first_err = err;
        }
        if (attempt == RETRY_ATTEMPTS || worth_retrying(err) == false) {
            report_open_failed(where, path, mode, err, false, optional);
            errno = err;
            return NULL;
        }
        pause_ms(retry_delay_ms(attempt));
    }
    return NULL;
}

// ifstream open that retries and explains its failure
inline bool open_in(std::ifstream &fp, const char *where, const std::string &path, bool optional = false)
{
    int first_err = 0;
    for (int attempt = 1; attempt <= RETRY_ATTEMPTS; attempt++) {
        errno = 0;
        fp.clear();
        fp.open(path.c_str(), std::ios::in | std::ios::binary);
        int err = errno;
        if (fp.is_open() == true) {
            if (attempt > 1) {
                say("NOTE: %s: '%s' opened on attempt %d", where, path.c_str(), attempt);
            }
            return true;
        }
        if (attempt == 1) {
            first_err = err;
        }
        struct stat st;
        bool missing = (stat(path.c_str(), &st) != 0 && (errno == ENOENT || errno == ENOTDIR)); // a stream gives no errno of its own
        if (attempt == RETRY_ATTEMPTS || missing == true) {
            report_open_failed(where, path, "rb", first_err, true, optional);
            return false;
        }
        pause_ms(retry_delay_ms(attempt));
    }
    return false;
}

// ofstream open that retries and explains its failure
inline bool open_out(std::ofstream &fp, const char *where, const std::string &path)
{
    for (int attempt = 1; attempt <= RETRY_ATTEMPTS; attempt++) {
        errno = 0;
        fp.clear();
        fp.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        int err = errno;
        if (fp.is_open() == true) {
            if (attempt > 1) {
                say("NOTE: %s: '%s' opened for writing on attempt %d", where, path.c_str(), attempt);
            }
            return true;
        }
        if (attempt == RETRY_ATTEMPTS) {
            report_open_failed(where, path, "wb", err, true, false);
            return false;
        }
        pause_ms(retry_delay_ms(attempt));
    }
    return false;
}

// fread result check: got and expected are item counts. A short read says if it was the end of the file or an error.
inline bool read_ok(const char *where, const std::string &path, size_t got, size_t expected, FILE *fp)
{
    if (got == expected) {
        return true;
    }
    int err = errno;
    int is_eof = (fp != NULL && feof(fp) != 0);
    int is_err = (fp != NULL && ferror(fp) != 0);
    if (first_time(where, path, "read")) {
        long size = -1;
        if (fp != NULL) {
            long here = ftell(fp);
            if (fseek(fp, 0, SEEK_END) == 0) {
                size = ftell(fp);
                fseek(fp, here, SEEK_SET);
            }
        }
        say("ERROR: %s: short read of '%s': got %ld of %ld items (%s, errno %d: %s), file size %ld", where, path.c_str(),
            (long)got, (long)expected, is_err ? "read error" : (is_eof ? "end of file" : "unknown"), err, strerror(err), size);
    }
    return false;
}

inline bool write_ok(const char *where, const std::string &path, size_t got, size_t expected);

// fread with the result check built in (returns the item count like fread). A read error (not the end of the file) is
// retried from the same position.
inline size_t fread_logged(const char *where, const std::string &path, void *buffer, size_t size, size_t count, FILE *fp)
{
    long start = ftell(fp);
    size_t got = 0;
    for (int attempt = 1; attempt <= RETRY_ATTEMPTS; attempt++) {
        errno = 0;
        got = fread(buffer, size, count, fp);
        if (got == count || ferror(fp) == 0 || start < 0 || attempt == RETRY_ATTEMPTS) {
            break;
        }
        clearerr(fp);
        if (fseek(fp, start, SEEK_SET) != 0) {
            break;
        }
        pause_ms(retry_delay_ms(attempt));
    }
    read_ok(where, path, got, count, fp);
    return got;
}

// fwrite with the result check built in
inline size_t fwrite_logged(const char *where, const std::string &path, const void *buffer, size_t size, size_t count, FILE *fp)
{
    errno = 0;
    size_t got = fwrite(buffer, size, count, fp);
    write_ok(where, path, got, count);
    return got;
}

// ifstream::read check (bytes)
inline bool stream_read_ok(const char *where, const std::string &path, std::ifstream &fp, size_t expected_bytes)
{
    if (fp.good() && (size_t)fp.gcount() == expected_bytes) {
        return true;
    }
    if (first_time(where, path, "read")) {
        say("ERROR: %s: short read of '%s': got %ld of %ld bytes (%s%s)", where, path.c_str(), (long)fp.gcount(), (long)expected_bytes,
            fp.eof() ? "end of file" : "", fp.bad() ? "stream error" : (fp.fail() && !fp.eof() ? "read failed" : ""));
    }
    return false;
}

// fwrite check: count of items written
inline bool write_ok(const char *where, const std::string &path, size_t got, size_t expected)
{
    if (got == expected) {
        return true;
    }
    int err = errno;
    if (first_time(where, path, "write")) {
        say("ERROR: %s: short write to '%s': %ld of %ld items (errno %d: %s)", where, path.c_str(), (long)got, (long)expected, err, strerror(err));
    }
    return false;
}

// ofstream state after the writes (call before close, and again after close with after_close = true)
inline bool stream_write_ok(const char *where, const std::string &path, std::ofstream &fp)
{
    fp.flush();
    if (fp.good()) {
        return true;
    }
    if (first_time(where, path, "write")) {
        say("ERROR: %s: write to '%s' failed (stream state: %s%s)", where, path.c_str(), fp.bad() ? "bad " : "", fp.fail() ? "fail" : "");
    }
    return false;
}

// fclose check: with PSL1GHT a failed lv2 write only shows up here or in the size on disk
inline bool fclose_logged(const char *where, const std::string &path, FILE *fp)
{
    errno = 0;
    if (fclose(fp) == 0) {
        return true;
    }
    int err = errno;
    if (first_time(where, path, "close")) {
        say("ERROR: %s: fclose of '%s' failed: errno %d (%s)", where, path.c_str(), err, strerror(err));
    }
    return false;
}

// A file the game cannot run without could not be opened even after the retries: say so and close the game
// (only when the game enabled it, see fatal_enabled). Without it the game went on with empty data (a white stage).
inline void fatal(const char *where, const std::string &path)
{
    say("FATAL: %s: the required file '%s' cannot be opened", where, path.c_str());
    if (fatal_enabled() == false) {
        return;
    }
#ifdef PS3
    ps3_platform::fatal_exit("a required game file cannot be opened");
#else
    std::cout.flush();
    exit(1);
#endif
}

inline FILE *fopen_required(const char *where, const std::string &path, const char *mode)
{
    FILE *fp = fopen_logged(where, path, mode);
    if (fp == NULL) {
        fatal(where, path);
    }
    return fp;
}

inline bool open_in_required(std::ifstream &fp, const char *where, const std::string &path)
{
    bool ok = open_in(fp, where, path);
    if (ok == false) {
        fatal(where, path);
    }
    return ok;
}

} // namespace fio_diag

#endif // FIO_DIAG_H
