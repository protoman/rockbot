#ifndef TDUMP_H
#define TDUMP_H

#include <genesis.h>

// Temporary diagnostics for the "white stripes" bug (BUG-044 follow-up).
// Built only with EXTRA_FLAGS=-DMD_TILE_DUMP; prints VDP state through kprintf,
// which BlastEm shows on stdout ("KDEBUG MESSAGE: ..."), so a single log-only
// run answers: is the backdrop/plane/window/CRAM what the code set?
#ifdef MD_TILE_DUMP
void tdump_vdp(const char *tag);
void tdump_cram(const char *tag);
#else
#define tdump_vdp(tag)  do {} while (0)
#define tdump_cram(tag) do {} while (0)

#endif

#endif
