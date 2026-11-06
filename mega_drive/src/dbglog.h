#ifndef DBGLOG_H
#define DBGLOG_H

#include <genesis.h>

// Debug log for hardware-only questions (see blastem_debug.md: KDEBUG is not
// capturable here). Lines are written as ASCII into a spare SRAM region and read
// from ~/.local/share/blastem/rom/save.sram after a NORMAL exit of the emulator
// with tools/trans_log.py. Built only with -DMD_TRANS_DEBUG and/or -DMD_LINK_DEBUG.
#if defined(MD_TRANS_DEBUG) || defined(MD_LINK_DEBUG)
void dbg_log(const char *fmt, ...);
#endif

#endif
