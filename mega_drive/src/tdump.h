#ifndef TDUMP_H
#define TDUMP_H

#include <genesis.h>

// The transition probes (MD_TRANS_DEBUG) need the VRAM readback.
#if defined(MD_TRANS_DEBUG) && !defined(MD_TILE_DUMP)
#define MD_TILE_DUMP
#endif

// Temporary diagnostics for the "white stripes" bug (BUG-044 follow-up).
// Built only with EXTRA_FLAGS=-DMD_TILE_DUMP; prints VDP state through kprintf,
// which BlastEm shows on stdout ("KDEBUG MESSAGE: ..."), so a single log-only
// run answers: is the backdrop/plane/window/CRAM what the code set?
#ifdef MD_TILE_DUMP
void tdump_vdp(const char *tag);
void tdump_cram(const char *tag);
void tdump_tile(const char *tag, u16 tile);
void tdump_cell(const char *tag, u16 plane, u16 col, u16 row);
void tdump_sprites(const char *tag);
// Read `words` 16-bit words of VRAM at BYTE address `byte_addr` (display state
// untouched; interrupts off and DMA waited out for the duration).
void tdump_read(u16 byte_addr, u16 *dst, u16 words);
#else
#define tdump_vdp(tag)  do {} while (0)
#define tdump_cram(tag) do {} while (0)
#define tdump_tile(tag,tile) do {} while (0)
#define tdump_cell(tag,pl,c,r) do {} while (0)
#define tdump_sprites(tag) do {} while (0)

#endif

#endif
