#ifndef DBGPRINT_H
#define DBGPRINT_H

#include <genesis.h>

// Debug output over KDEBUG. The emulator pops its debug window on the first
// message, so release builds print nothing; build with EXTRA_FLAGS=-DMD_DEBUG
// when a log is wanted.
#if defined(MD_DEBUG) || defined(MD_TILE_DUMP) || defined(MD_WALK_DIAG) || defined(MD_WALK_TIME) || defined(MD_PROFILE) || defined(MD_LINK_TRACE) || defined(MD_HEAP_TRACE) || defined(MD_LADDER_TRACE)
#define dbg_print(...) kprintf(__VA_ARGS__)
#else
#define dbg_print(...) ((void)0)
#endif

#endif
