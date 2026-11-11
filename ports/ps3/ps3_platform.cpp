#include "ps3_platform.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>

#ifdef PS3
#include <sysutil/sysutil.h>
#include <lv2/sysfs.h>
#include <sys/paths.h>
#include <sys/lock.h>
#include <sys/thread.h>
#include <sys/tty.h>
#include <lv2/mutex.h>
#include <lv2/process.h>
#include <sys/file.h>
#include <sys/systime.h>
#include <io/pad.h>
#include <rsx/rsx.h>
#include <rsx/commands.h>
#endif

namespace ps3_platform {

namespace {

const unsigned int RB_RSX_MIN_FREE_WORDS = 2048; // 8 KB

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
// Recursive newlib locks.
//
// newlib guards malloc, stdio (every FILE), atexit and the environment with
// recursive locks. PSL1GHT's own implementation allocates them lazily through
// a global "meta lock" that a libc constructor creates; static constructors
// of the game can run before that constructor and then abort() in the first
// atexit registration. An earlier workaround turned all of these locks into
// no-ops. That is wrong: the game is NOT single-threaded (SDL audio, SDL
// timer and SDL_mixer run their own threads), so malloc and stdio were left
// unprotected and the heap/FILE state eventually got corrupted, freezing the
// game after a minute or two.
//
// These replacements are real recursive lwmutexes created lazily on first use
// and guarded by a tiny spinlock, so they do not depend on libc's meta lock
// and are safe during early static construction. The otherwise unused _pad
// field of the lwmutex records the state (0 = not created, 1 = ready,
// 2 = creation failed; in that last case the lock degrades to a no-op instead
// of aborting).
namespace {

enum { RB_LOCK_NEW = 0, RB_LOCK_READY = 1, RB_LOCK_FAILED = 2 };

volatile int g_lock_guard = 0;
volatile int g_lock_failure_reported = 0;

inline sys_lwmutex_t *as_lwmutex(__libc_lock_t *lock)
{
    return reinterpret_cast<sys_lwmutex_t *>(lock);
}

void report_lock_failure(s32 error)
{
    if (__sync_lock_test_and_set(&g_lock_failure_reported, 1) == 0) {
        char msg[64];
        int n = snprintf(msg, sizeof(msg), "rockbot: lwmutex create failed (0x%x), lock disabled\n", (unsigned)error);
        u32 written = 0;
        sysTtyWrite(0 /* stdout */, msg, (u32)n, &written);
    }
}

// Returns the usable lwmutex, or NULL if this lock is disabled.
sys_lwmutex_t *ensure_lock(__libc_lock_t *lock)
{
    sys_lwmutex_t *m = as_lwmutex(lock);
    u32 state = *(volatile u32 *)&m->_pad;
    if (state == RB_LOCK_READY) {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        return m;
    }
    if (state == RB_LOCK_FAILED) {
        return NULL;
    }

    while (__sync_lock_test_and_set(&g_lock_guard, 1) != 0) {
        sysThreadYield();
    }
    state = *(volatile u32 *)&m->_pad;
    if (state == RB_LOCK_NEW) {
        sys_lwmutex_attr_t attr;
        memset(&attr, 0, sizeof(attr));
        attr.attr_protocol = SYS_LWMUTEX_PROTOCOL_PRIO;
        attr.attr_recursive = SYS_LWMUTEX_ATTR_RECURSIVE;
        memcpy(attr.name, "rblock", 6);
        const s32 r = sysLwMutexCreate(m, &attr);
        __sync_synchronize();
        if (r == 0) {
            state = RB_LOCK_READY;
        } else {
            state = RB_LOCK_FAILED;
            report_lock_failure(r);
        }
        *(volatile u32 *)&m->_pad = state;
    }
    __sync_lock_release(&g_lock_guard);
    return state == RB_LOCK_READY ? m : NULL;
}

} // namespace

extern "C" int __libc_lock_init_recursive(__libc_lock_t *lock)
{
    // Lazy: just reset; the lwmutex is created on first acquire.
    memset(lock, 0, sizeof(*lock));
    return 0;
}

extern "C" int __libc_lock_acquire_recursive(__libc_lock_t *lock)
{
    sys_lwmutex_t *m = ensure_lock(lock);
    return m ? sysLwMutexLock(m, 0) : 0;
}

extern "C" int __libc_lock_try_acquire_recursive(__libc_lock_t *lock)
{
    sys_lwmutex_t *m = ensure_lock(lock);
    return m ? sysLwMutexTryLock(m) : 0;
}

extern "C" int __libc_lock_release_recursive(__libc_lock_t *lock)
{
    sys_lwmutex_t *m = as_lwmutex(lock);
    if (*(volatile u32 *)&m->_pad != RB_LOCK_READY) {
        return 0;
    }
    return sysLwMutexUnlock(m);
}

extern "C" int __libc_lock_close_recursive(__libc_lock_t *lock)
{
    sys_lwmutex_t *m = as_lwmutex(lock);
    if (*(volatile u32 *)&m->_pad == RB_LOCK_READY) {
        sysLwMutexDestroy(m);
    }
    memset(lock, 0, sizeof(*lock));
    return 0;
}
#endif

namespace ps3_platform {

#ifdef PS3
// The PS button (and "quit game" in the XMB) arrives as SYSUTIL_EXIT_GAME. SDL turns it into a quit event, but only a
// screen that polls events acts on it: a waiting loop that does not (an error screen waiting for a button, a stuck file
// operation) used to leave the console frozen with the PS button dead. This second callback (slot 1; SDL has its own)
// only records the request; update() quits the process when the game keeps running 3 seconds after it.
static volatile bool g_exit_requested = false;

static void exit_request_callback(u64 status, u64 param, void *usrdata)
{
    (void)param;
    (void)usrdata;
    if (status == SYSUTIL_EXIT_GAME) {
        g_exit_requested = true;
    }
}
#endif

bool exit_requested()
{
#ifdef PS3
    return g_exit_requested;
#else
    return false;
#endif
}

void fatal_exit(const char *reason)
{
#ifdef PS3
    log("FATAL: %s - closing the game", reason != NULL ? reason : "unknown");
    log_flush_now();
    sysProcessExit(1);
#else
    (void)reason;
    exit(1);
#endif
}

#ifdef PS3
static void start_support_threads();
#endif

void setup(const char *argv0)
{
#ifdef PS3
    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT1, exit_request_callback, NULL);
    start_support_threads();
#endif
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

#ifdef PS3
// ---- the log, and two helper threads that keep working when the game thread is stuck -----------------------------------
// A freeze used to leave nothing to look at: the game thread wrote the log itself (open, write, fsync, close), so it could
// be the thing that was blocked, and its buffered lines were lost with it. Now:
//  - log() only copies the line into a ring buffer (never blocks, drops the line when the ring is full or another thread
//    is logging at that very moment);
//  - the "support" thread writes the ring to the log file every 20 ms with raw lv2 calls (no libc, so no libc lock can
//    block it); if the file system blocks, only that thread waits;
//  - it also watches the game thread's heartbeat (update() is called by every pad poll and flip): silent for 3 s it logs
//    "STUCK" with the last log line and the file operation in progress; and when the PS button was pressed while the game
//    thread is silent for 1.5 s it quits the process;
//  - the "watcher" thread serves the system callbacks while the game thread is silent (that is how the PS button press
//    arrives), it may block there without blocking the support thread.
// The helper threads use no libc function (only system calls and the little helpers below).
static const char RB_LOG_PATH[] = "/dev_hdd0/tmp/rockbot/rockbot_ps3.log";
static const u32 RB_RING_SIZE = 32768; // power of 2
static char g_ring[RB_RING_SIZE];
static volatile u32 g_ring_head = 0;      // bytes ever written (producer)
static volatile u32 g_ring_tail = 0;      // bytes ever consumed (support thread)
static volatile int g_producer_busy = 0;
static volatile u32 g_ring_dropped = 0;
static volatile int g_drain_busy = 0;
static volatile u32 g_heartbeat = 0;      // incremented by update()
static volatile u32 g_main_stale_ticks = 0; // 100 ms ticks without a heartbeat, set by the support thread
static char g_last_line[160];
static char g_file_op[220];

static void raw_append(const char *data, u32 len)
{
    s32 fd = -1;
    if (sysLv2FsOpen(RB_LOG_PATH, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_APPEND, &fd, 0666, NULL, 0) != 0) {
        return;
    }
    u64 written = 0;
    sysLv2FsWrite(fd, data, len, &written);
    sysLv2FsFsync(fd);
    sysLv2FsClose(fd);
}

// writes what is in the ring to the file (any thread; one at a time)
static void drain_ring()
{
    if (__sync_lock_test_and_set(&g_drain_busy, 1) != 0) {
        return;
    }
    static char out[RB_RING_SIZE + 64];
    const u32 head = g_ring_head;
    __sync_synchronize();
    u32 tail = g_ring_tail;
    u32 n = 0;
    while (tail != head && n < RB_RING_SIZE) {
        out[n++] = g_ring[tail & (RB_RING_SIZE - 1)];
        tail++;
    }
    const u32 dropped = g_ring_dropped;
    if (dropped != 0) {
        g_ring_dropped = 0;
        const char *msg = "[some log lines were dropped]\n";
        for (u32 i = 0; msg[i] != 0; i++) {
            out[n++] = msg[i];
        }
    }
    if (n > 0) {
        g_ring_tail = tail; // consumed before the write: a blocked write must not make the ring look full forever
        raw_append(out, n);
    }
    __sync_lock_release(&g_drain_busy);
}

static void put_str(char *dst, u32 &n, u32 cap, const char *s)
{
    while (*s != 0 && n + 1 < cap) {
        dst[n++] = *s++;
    }
}

static void put_num(char *dst, u32 &n, u32 cap, u32 v)
{
    char t[12];
    int k = 0;
    if (v == 0) {
        t[k++] = '0';
    }
    while (v > 0 && k < 11) {
        t[k++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (k > 0 && n + 1 < cap) {
        dst[n++] = t[--k];
    }
}

static void raw_log_line(const char *a, u32 number, const char *b, const char *c)
{
    char line[600];
    u32 n = 0;
    put_str(line, n, sizeof(line), a);
    put_num(line, n, sizeof(line), number);
    put_str(line, n, sizeof(line), b);
    put_str(line, n, sizeof(line), c);
    put_str(line, n, sizeof(line), "\n");
    raw_append(line, n);
}

static void support_thread(void *arg)
{
    (void)arg;
    u32 last_heartbeat = g_heartbeat;
    u32 stale = 0;
    u32 tick = 0;
    for (;;) {
        sysUsleep(20000);
        drain_ring();
        tick++;
        if ((tick % 5) != 0) {
            continue;
        }
        const u32 hb = g_heartbeat;
        if (hb != last_heartbeat) {
            last_heartbeat = hb;
            stale = 0;
        } else if (stale < 100000000) {
            stale++;
        }
        g_main_stale_ticks = stale;
        if (stale >= 15 && g_exit_requested == true) {
            raw_log_line("PS button pressed while the game thread was silent for ", stale * 100, " ms: quitting. file operation in progress: ", g_file_op[0] != 0 ? g_file_op : "none");
            raw_log_line("last log line: ", 0, "", g_last_line);
            sysProcessExit(0);
        }
        if (stale == 30 || (stale > 30 && (stale % 100) == 0)) {
            raw_log_line("STUCK: the game thread has been silent for ", stale * 100, " ms. file operation in progress: ", g_file_op[0] != 0 ? g_file_op : "none");
            raw_log_line("STUCK: last log line: ", 0, "", g_last_line);
        }
    }
}

static void watcher_thread(void *arg)
{
    (void)arg;
    for (;;) {
        sysUsleep(100000);
        if (g_main_stale_ticks >= 15) {
            sysUtilCheckCallback(); // delivers the PS button press to exit_request_callback (and to SDL)
        }
    }
}

static void start_support_threads()
{
    static sys_ppu_thread_t support_id, watcher_id;
    sysThreadCreate(&support_id, support_thread, NULL, 800, 0x10000, THREAD_JOINABLE, (char *)"rb_support");
    sysThreadCreate(&watcher_id, watcher_thread, NULL, 850, 0x10000, THREAD_JOINABLE, (char *)"rb_watcher");
}
#endif

void log(const char *fmt, ...)
{
#ifdef PS3
    char msg[192];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(msg, sizeof(msg) - 1, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    if (n > (int)sizeof(msg) - 2) {
        n = (int)sizeof(msg) - 2;
    }
    msg[n++] = '\n';
    u32 written = 0;
    sysTtyWrite(0, msg, (u32)n, &written);
    // for the "STUCK" report: the last line logged, even if it was not written yet
    const int keep = (n - 1 < (int)sizeof(g_last_line) - 1) ? (n - 1) : (int)sizeof(g_last_line) - 1;
    memcpy(g_last_line, msg, (size_t)keep);
    g_last_line[keep] = 0;
    // into the ring; never waits (see above)
    if (__sync_lock_test_and_set(&g_producer_busy, 1) != 0) {
        return;
    }
    const u32 head = g_ring_head;
    if (head - g_ring_tail + (u32)n > RB_RING_SIZE) {
        g_ring_dropped = g_ring_dropped + 1;
    } else {
        for (int i = 0; i < n; i++) {
            g_ring[(head + (u32)i) & (RB_RING_SIZE - 1)] = msg[i];
        }
        __sync_synchronize();
        g_ring_head = head + (u32)n;
    }
    __sync_lock_release(&g_producer_busy);
#else
    (void)fmt;
#endif
}

// The support thread writes the ring every 20 ms; this only makes sure it is written when the caller is about to leave.
void log_flush()
{
#ifdef PS3
    // nothing to wait for: a blocked file system must never block the game thread
#endif
}

void log_flush_now()
{
#ifdef PS3
    drain_ring(); // from the calling thread, for the moments right before the process exits
    sysUsleep(50000);
    drain_ring();
#endif
}

void mark_file(const char *op, const char *path)
{
#ifdef PS3
    u32 n = 0;
    put_str(g_file_op, n, sizeof(g_file_op), op);
    put_str(g_file_op, n, sizeof(g_file_op), " ");
    put_str(g_file_op, n, sizeof(g_file_op), path);
    g_file_op[n] = 0;
#else
    (void)op;
    (void)path;
#endif
}

void mark_file_done()
{
#ifdef PS3
    g_file_op[0] = 0;
#endif
}

// SDL_PSL1GHT initialises RSX with a 64 KB command buffer and every
// SDL_Flip appends ~240 bytes through the system gcm calls (gcmSetFlip,
// gcmSetWaitFlip). Those calls do not hop to the next buffer fragment by
// themselves, so the 256th flip runs off the end and the game stalls forever
// waiting for a flip that never happens. The context's callback (installed by
// the system, PSL1GHT's rsx* helpers call it too) does the hop: it writes a
// jump to the next 32 KB fragment and resets begin/current/end. Call it
// ourselves, between frames, while there is still plenty of room for the jump.
#ifdef PS3
extern "C" s32 rsxContextCallback(gcmContextData *context, u32 count);
#endif

// Logs every 120th flip (and the first ones), so the log file shows whether the frames stop (hang in or before
// the flip) or keep coming (the game runs but ignores the pad).
static int g_debug_variant = 0;
static unsigned int g_variant_changed_flip = 0;
static unsigned int g_flip_counter = 0;

int debug_variant()
{
    return g_debug_variant;
}

void flip_debug_dump(SDL_Surface *screen)
{
#ifdef PS3
    if (screen == NULL) {
        return;
    }
    // 60 frames after a variant change, and every 480 frames: a screenshot of what is presented
    bool after_change = (g_variant_changed_flip != 0 && g_flip_counter == g_variant_changed_flip + 60);
    if (after_change) {
        char path[96];
        if (after_change) {
            snprintf(path, sizeof(path), "/dev_hdd0/tmp/rockbot/dump_v%d.bmp", g_debug_variant);
        } else {
            snprintf(path, sizeof(path), "/dev_hdd0/tmp/rockbot/dump_auto.bmp");
        }
        int r = SDL_SaveBMP(screen, path);
        log("rockbot: screenshot %s (flip #%u, variant %d) result=%d %dx%d bpp=%d", path, g_flip_counter, g_debug_variant, r, screen->w, screen->h,
            screen->format != NULL ? screen->format->BitsPerPixel : -1);
    }
#else
    (void)screen;
#endif
}

unsigned int flip_number()
{
    return g_flip_counter;
}

void flip_heartbeat()
{
    g_flip_counter++;
    if ((g_flip_counter % 60) == 0) {
        log_flush();
    }
#ifdef PS3
    // frames that take much longer than the others (the stutter): when, how long
    static unsigned int last_ms = 0, slow_logged = 0;
    const unsigned int now_ms = SDL_GetTicks();
    if (last_ms != 0 && now_ms - last_ms > 45 && slow_logged < 400) {
        slow_logged++;
        log("slow frame #%u: %u ms", g_flip_counter, now_ms - last_ms);
    }
    last_ms = now_ms;
#endif
    static unsigned int flips = 0;
    flips++;
    if (flips <= 3 || (flips % 120) == 0) {
        log("rockbot: flip #%u", flips);
    }
}

void rsx_command_buffer_guard()
{
#ifdef PS3
    gcmContextData *ctx = gGcmContext;
    if (ctx == NULL) {
        return;
    }
    gcmControlRegister volatile *ctrl = gcmGetControlRegister();
    const u32 remaining = (u32)(ctx->end - ctx->current);
    if (remaining >= RB_RSX_MIN_FREE_WORDS) {
        return;
    }
    static unsigned int wraps = 0;
    wraps++;
    const s32 r = rsxContextCallback(ctx, 4);
    rsxFlushBuffer(ctx);
    if (wraps <= 5 || (wraps % 50) == 0) {
        log("rockbot: RSX wrap #%u (callback=%d): %u words left before; now begin=%08x end=%08x current=%08x put=%x get=%x",
            wraps, (int)r, remaining, (unsigned)(u64)ctx->begin, (unsigned)(u64)ctx->end,
            (unsigned)(u64)ctx->current, (unsigned)ctrl->put, (unsigned)ctrl->get);
    }
#endif
}

// Reads pad 0 straight from libio. SDL_PSL1GHT counts the connected pads only
// once, during SDL_Init, and RPCS3 (and real consoles with a wireless pad) connect
// the pad a little later, so SDL ends up with 0 joysticks and never delivers
// events. Polling libio every frame also handles pads that come and go.
// Bit n of *buttons follows SDL_PSL1GHT's joystick button numbering:
// 0 left, 1 down, 2 right, 3 up, 4 start, 5 R3, 6 L3, 7 select, 8 square,
// 9 cross, 10 circle, 11 triangle, 12 R1, 13 L1, 14 R2, 15 L2.
void pad_read(unsigned int *buttons, int *lx, int *ly)
{
    *buttons = 0;
    *lx = 128;
    *ly = 128;
#ifdef PS3
    static int pad_initialized = 0;
    if (!pad_initialized) {
        ioPadInit(7);   // harmless if SDL already did it
        pad_initialized = 1;
    }
    // ioPadGetData only fills the structure when the input changed since the last
    // call (len == 0 means "no new input"), so remember the last state.
    static unsigned int last_buttons = 0;
    static int last_lx = 128, last_ly = 128;
    padInfo info;
    if (ioPadGetInfo(&info) != 0 || info.status[0] == 0) {
        last_buttons = 0;
        last_lx = last_ly = 128;
        return;
    }
    padData d;
    memset(&d, 0, sizeof(d));
    if (ioPadGetData(0, &d) != 0) {
        return;
    }
    if (d.len == 0) {
        *buttons = last_buttons;
        *lx = last_lx;
        *ly = last_ly;
        return;
    }
    unsigned int b = 0;
    b |= d.BTN_LEFT     ? (1u << 0)  : 0;
    b |= d.BTN_DOWN     ? (1u << 1)  : 0;
    b |= d.BTN_RIGHT    ? (1u << 2)  : 0;
    b |= d.BTN_UP       ? (1u << 3)  : 0;
    b |= d.BTN_START    ? (1u << 4)  : 0;
    b |= d.BTN_R3       ? (1u << 5)  : 0;
    b |= d.BTN_L3       ? (1u << 6)  : 0;
    b |= d.BTN_SELECT   ? (1u << 7)  : 0;
    b |= d.BTN_SQUARE   ? (1u << 8)  : 0;
    b |= d.BTN_CROSS    ? (1u << 9)  : 0;
    b |= d.BTN_CIRCLE   ? (1u << 10) : 0;
    b |= d.BTN_TRIANGLE ? (1u << 11) : 0;
    b |= d.BTN_R1       ? (1u << 12) : 0;
    b |= d.BTN_L1       ? (1u << 13) : 0;
    b |= d.BTN_R2       ? (1u << 14) : 0;
    b |= d.BTN_L2       ? (1u << 15) : 0;
    if (b != last_buttons) {
        log("rockbot: pad buttons %04x", b);
    }
    // (R3 used to cycle the stage-drawing debug variants; removed, the variants stay at 0)
    last_buttons = b;
    last_lx = d.ANA_L_H;
    last_ly = d.ANA_L_V;
    *buttons = b;
    *lx = last_lx;
    *ly = last_ly;
#endif
}

void update()
{
#ifdef PS3
    g_heartbeat++;
    // every pad poll calls this (waiting loops poll about 1000 times a second): at most every 8 ms is plenty
    static u64 last_ms = 0;
    const u64 now_ms = (u64)SDL_GetTicks();
    if (now_ms - last_ms < 8) {
        return;
    }
    last_ms = now_ms;
    sysUtilCheckCallback();
    if (g_exit_requested == true) {
        static u64 first_seen_ms = 0;
        static unsigned int polls_since = 0;
        if (first_seen_ms == 0) {
            first_seen_ms = now_ms == 0 ? 1 : now_ms;
            log("exit requested (PS button): waiting for the game to quit by itself");
            log_flush();
        }
        polls_since++;
        // the game is alive (it keeps polling) but ignores the request: stuck in a loop that does not check for it
        if (now_ms - first_seen_ms >= 3000 && polls_since >= 50) {
            log("exit requested %d ms ago and the game is still running: quitting now", (int)(now_ms - first_seen_ms));
            log_flush_now();
            sysProcessExit(0);
        }
    }
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
    // Directions: D-pad buttons (0 left, 1 down, 2 right, 3 up). RPCS3's keyboard
    // handler maps the arrow keys to the D-pad too. Each command holds a single
    // binding, so the analog stick can be assigned in the in-game controls menu.
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
    button_codes_copy[BTN_LEFT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_LEFT].value = 0;     // d-pad left
    button_codes_copy[BTN_DOWN].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_DOWN].value = 1;     // d-pad down
    button_codes_copy[BTN_RIGHT].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_RIGHT].value = 2;    // d-pad right
    button_codes_copy[BTN_UP].type = JOYSTICK_INPUT_TYPE_BUTTON;
    button_codes_copy[BTN_UP].value = 3;       // d-pad up
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
