#include "replay.h"

#if defined(PC) && !defined(WIN32) // POSIX only (signals, execinfo): Windows builds get the empty stubs

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <execinfo.h>
#include "../defines.h"

namespace replay {

struct event {
    unsigned long ms;
    int button;      // BTN_xxx, or -1 for END
    int state;       // 1 pressed, 0 released
};

static bool g_checked = false;
static bool g_enabled = false;
static unsigned long g_now = 0;
static std::vector<event> g_events;
static size_t g_next = 0;
static signed char g_held[BTN_COUNT];
static FILE *g_trace = NULL;

static int button_from_name(const std::string &name)
{
    static const struct { const char *name; int button; } table[] = {
        {"UP", BTN_UP}, {"DOWN", BTN_DOWN}, {"LEFT", BTN_LEFT}, {"RIGHT", BTN_RIGHT}, {"JUMP", BTN_JUMP},
        {"ATTACK", BTN_ATTACK}, {"DASH", BTN_DASH}, {"SHIELD", BTN_SHIELD}, {"L", BTN_L}, {"R", BTN_R},
        {"START", BTN_START}
    };
    for (size_t i=0; i<sizeof(table)/sizeof(table[0]); i++) {
        if (name == table[i].name) {
            return table[i].button;
        }
    }
    return -2;
}

static void load_script(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "replay: cannot open script '%s'\n", path);
        exit(2);
    }
    char line[512];
    int line_n = 0;
    while (fgets(line, sizeof(line), fp)) {
        line_n++;
        char *hash = strchr(line, '#');
        if (hash != NULL) {
            *hash = '\0';
        }
        char *save = NULL;
        char *token = strtok_r(line, " \t\r\n", &save);
        if (token == NULL) {
            continue;
        }
        unsigned long ms = strtoul(token, NULL, 10);
        while ((token = strtok_r(NULL, " \t\r\n", &save)) != NULL) {
            event e;
            e.ms = ms;
            if (strcmp(token, "END") == 0) {
                e.button = -1;
                e.state = 0;
            } else if ((token[0] == '+' || token[0] == '-') && button_from_name(token + 1) >= 0) {
                e.button = button_from_name(token + 1);
                e.state = (token[0] == '+') ? 1 : 0;
            } else {
                fprintf(stderr, "replay: %s:%d: unknown token '%s'\n", path, line_n, token);
                exit(2);
            }
            g_events.push_back(e);
        }
    }
    fclose(fp);
}

// Busy waits: a loop that waits for a time to pass without sleeping never ends on a virtual clock. After many clock
// reads with no sleep in between, the clock moves by 10 ms (a count of reads, so it is still deterministic). Normal
// frames end in a sleep (the frame limiter) long before that.
static const unsigned long BUSY_READS = 2000000;
static unsigned long g_busy_reads = 0;
static unsigned long g_busy_steps = 0;

// Every 10 real seconds: if the virtual time did not move, report a hang (backtrace) and end the run so a test can not
// block forever; if it only moved through busy-wait steps, show where the game is waiting (first 3 times).
// Resolve the addresses with: addr2line -f -C -e <binary> <address>
static unsigned long g_watchdog_last_now = (unsigned long)-1;
static unsigned long g_watchdog_last_busy = 0;
static int g_busy_reports = 0;

static void watchdog(int)
{
    bool hang = (g_now == g_watchdog_last_now);
    bool busy = (g_busy_steps != g_watchdog_last_busy);
    g_watchdog_last_now = g_now;
    g_watchdog_last_busy = g_busy_steps;
    if (hang == false && (busy == false || g_busy_reports >= 3)) {
        return;
    }
    static const char hang_msg[] = "replay: HANG - the virtual clock did not move for 10 s. Backtrace:\n";
    static const char busy_msg[] = "replay: busy wait (a loop reads the clock without sleeping). Backtrace:\n";
    if (hang) {
        write(2, hang_msg, sizeof(hang_msg)-1);
    } else {
        write(2, busy_msg, sizeof(busy_msg)-1);
        g_busy_reports++;
    }
    void *frames[40];
    int n = backtrace(frames, 40);
    backtrace_symbols_fd(frames, n, 2);
    if (hang) {
        if (g_trace != NULL) {
            fflush(g_trace);
        }
        _exit(3);
    }
}

bool enabled()
{
    if (g_checked == false) {
        g_checked = true;
        for (int i=0; i<BTN_COUNT; i++) {
            g_held[i] = -1;
        }
        const char *script = getenv("ROCKBOT_REPLAY");
        if (script != NULL && script[0] != '\0') {
            g_enabled = true;
            load_script(script);
            signal(SIGALRM, watchdog);
            struct itimerval timer_value;
            timer_value.it_interval.tv_sec = 10; timer_value.it_interval.tv_usec = 0;
            timer_value.it_value = timer_value.it_interval;
            setitimer(ITIMER_REAL, &timer_value, NULL);
            const char *trace_path = getenv("ROCKBOT_TRACE");
            if (trace_path != NULL && trace_path[0] != '\0') {
                g_trace = fopen(trace_path, "w");
            }
        }
    }
    return g_enabled;
}

unsigned long now()
{
    if (++g_busy_reads >= BUSY_READS) {
        g_busy_reads = 0;
        g_busy_steps++;
        g_now += 10;
    }
    return g_now;
}

void advance(int ms)
{
    if (ms > 0) {
        g_now += (unsigned long)ms;
        g_busy_reads = 0;
    }
}

static void finish()
{
    if (g_trace != NULL) {
        fprintf(g_trace, "END %lu\n", g_now);
        fclose(g_trace);
        g_trace = NULL;
    }
    fflush(stdout);
    _exit(0);
}

void update_buttons()
{
    if (g_enabled == false) {
        return;
    }
    while (g_next < g_events.size() && g_events[g_next].ms <= g_now) {
        const event &e = g_events[g_next++];
        if (e.button < 0) {
            finish();
        } else {
            g_held[e.button] = (signed char)e.state;
        }
    }
}

int held(int button)
{
    if (g_enabled == false || button < 0 || button >= BTN_COUNT) {
        return -1;
    }
    return g_held[button];
}

bool tracing()
{
    return g_trace != NULL;
}

void trace(const char *format, ...)
{
    if (g_trace == NULL) {
        return;
    }
    va_list args;
    va_start(args, format);
    vfprintf(g_trace, format, args);
    va_end(args);
}

} // namespace replay

#endif // PC && !WIN32
