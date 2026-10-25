#ifndef PAL_SAFE_H
#define PAL_SAFE_H

#include <genesis.h>

// CRAM writes that must happen while the display is ON.
//
// PAL0 read back shifted by ten entries right after boot (BUG-044 follow-up:
// 0C00 0E00 x9 then 0000 0222 ... 0AAA, i.e. the grey ramp moved to entries
// 10..15), so a palette transfer landed at the wrong CRAM address. A CPU-method
// CRAM write is a VDP address-port write followed by data-port words: if a DMA
// is still running, or the vblank handler sets the address port in between, the
// data goes to whatever address the VDP holds at that moment.
//
// These wrappers wait the DMA out and keep interrupts off for the whole
// sequence. Use them instead of a bare PAL_setPalette/PAL_setColor with CPU on
// any screen that is already displaying.
static inline void pal_load(u16 pal, const u16 *data)
{
    VDP_waitDMACompletion();
    SYS_disableInts();
    PAL_setPalette(pal, data, CPU);
    SYS_enableInts();
}

static inline void pal_load_n(u16 index, const u16 *data, u16 count)
{
    VDP_waitDMACompletion();
    SYS_disableInts();
    PAL_setColors(index, data, count, CPU);
    SYS_enableInts();
}

static inline void pal_color(u16 index, u16 color)
{
    VDP_waitDMACompletion();
    SYS_disableInts();
    PAL_setColor(index, color);
    SYS_enableInts();
}

#endif
