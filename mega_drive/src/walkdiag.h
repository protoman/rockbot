#ifndef WALKDIAG_H
#define WALKDIAG_H

#include <genesis.h>

// Diagnostics for BUG-045 (intermittent garbage at the start of the ending's
// walking scene). Built only with EXTRA_FLAGS="-DMD_WALK_DIAG -DMD_TILE_DUMP";
// prints through dbg_print (BlastEm: capture with `script -qf`, blastem_debug.md).
// One run logs every suspect at every step instead of one guess per run.
#ifdef MD_WALK_DIAG
// One compact line of the state that could break VDP work: frame counter,
// interrupt mask, queued DMA, vblank-process flags, heap, Z80/XGM2, VDP layout.
void wd_ck(const char *tag);
// Same plus a probe that goes through the DMA queue (costs one vblank).
void wd_ckq(const char *tag);
#else
#define wd_ck(tag) ((void)0)
#define wd_ckq(tag) ((void)0)
#endif

#endif
